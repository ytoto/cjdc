#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "cjdc.h"

static char passwd[256]; /* and space for cookie */
static int passwdlen;

#define USAGE \
"usage: cjdc [command]\n"	\
"\n"				\
"commands:\n"			\
"dump\t\tdumps the routing table\n"	\
"peers\t\tlists connected nodes\n"	\

static inline void usage(void)
{
	write(1, USAGE, sizeof(USAGE) - 1);
}

static inline int getAdminFile(void)
{
	static const char cjdnsadmin[] = "/.cjdnsadmin";

	char buf[1024];
	char *p = getenv("HOME");
	char *q = buf;

	while (*p)
		*q++ = *p++;

	memcpy(q, cjdnsadmin, sizeof(cjdnsadmin));
	return open(buf, O_RDONLY);
}

static inline int connectWithAdminInfo(void)
{
	static const char node[] = "127.0.0.1"; /* TODO parse */
	static const char port[] = "11234";

	char buf[1024];
	int fd, n;

	if ((fd = getAdminFile()) < 0)
		goto out;

	if ((n = read(fd, buf, sizeof(buf))) < 0) {
		fd = -1;
		goto out;
	}

	close(fd);

	char *p = buf, *q = buf;
	int j = 0, state = 0;

	passwd[0] = '\0';

	for (int i = 0; i < n; ++i, ++p) {
		if (*p == '"')  {
			if (state & inpassv) {
				break;
			} else if (state & inquote) {
				*p = '\0';

				if (memcmp(q, "password", 8) == 0)
					state |= inpassk;

				state &= ~inquote;
			} else if (state & inpassk) {
				state |= inpassv;
			} else {
				q = ++p;
				state |= inquote;
			}
		} else if (state & inpassv)
			passwd[j++] = *p;
	}

	passwdlen = j;

	struct addrinfo hints = { .ai_socktype = SOCK_DGRAM };
	struct addrinfo *res = NULL;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		goto out;

	if (getaddrinfo(node, port, &hints, &res) == 0)
		for (; res; res = res->ai_next)
			if (connect(fd, res->ai_addr, res->ai_addrlen) == 0)
				break;

	if (res == NULL) {
		close(fd);
		fd = -1;
	}
out:
	return fd;
}

static char *baddu(char *p, unsigned int x)
{
	*p++ = 'i';
	p = u2a(p, x);
	*p++ = 'e';
	return p;
}

static char *badds(char *p, const char *s)
{
	unsigned int n = 0;
	const char *q = s;

	for (; *q; ++n, ++q);
	p = u2a(p, n);
	*p++ = ':';
	memcpy(p, s, n);
	return p + n;
}

static char *btxid(char *p)
{
	static char txid[10] = "ABCDEABCDE";
	char *q = txid;

	p = badds(p, "txid");
	for (; *q >= 'Z'; ++q);
	++(*q);
	*p++ = '1';
	*p++ = '0';
	*p++ = ':';
	memcpy(p, txid, 10);
	return p + 10;
}

static int reqCookie(int fd, char *out)
{
	char buf[1024], *p;
	int len = -1, i, n;

	p = buf;
	*p++ = 'd';
	*p++ = '1';
	*p++ = ':';
	*p++ = 'q';
	p = badds(p, "cookie");
	p = btxid(p);
	*p++ = 'e';

	if ((n = write(fd, buf, p - buf)) < 0)
		goto out;

	if ((n = read(fd, buf, sizeof(buf))) < 0)
		goto out;

	for (i = 0, p = buf; i < n; ++i, ++p) {
		if (*p == ':') {
			if (memcmp(++p, "cookie10", 8) == 0) {
				len = 10;
				memcpy(out, p + 9, len);
				break;
			}
		}
	}
out:
	return len;
}

typedef int parsePage(char *page, unsigned int size);

static int reqPages(int fd, const char *func, parsePage *pfunc)
{
	char *h, *p, *q = passwd + passwdlen, *s, req[1024], page[4096];
	int i = 0, n;

	p = req;
	*p++ = 'd';
	p = btxid(p);
	p = badds(p, "q");
	p = badds(p, "auth");
	p = badds(p, "aq");
	p = badds(p, func);
	p = badds(p, "args");
	*p++ = 'd';
	s = p;

	do {
		if ((n = reqCookie(fd, q)) < 0)
			goto out;

		p = badds(s, "page");
		p = baddu(p, i++);
		*p++ = 'e'; /* end args dict */
		p = badds(p, "cookie");
		p = badds(p, q);
		p = badds(p, "hash");
		*p++ = '6';
		*p++ = '4';
		*p++ = ':';
		h = p;
		p += 64;
		*p++ = 'e'; // end req dict
		n = p - req;
		sha256hex(h, passwd, passwdlen + 10);
		sha256hex(h, req, n);

		if ((n = write(fd, req, n)) < 0)
			goto out;

		if ((n = read(fd, page, sizeof(page))) < 0)
			goto out;

		if (memcmp(page, "d5:error", 8) == 0) {
			/* print error */
			unsigned int u;

			p = a2u(page + 8, &u);
			++p;
			p[u++] = '\n';
			write(1, p, u);
			n = -1;
			goto out;
		}

		n = pfunc(page, n);
	} while (n > 0);
out:
	return n;
}

int main(int ac, char *av[])
{
	int fd = connectWithAdminInfo();
	char buf[1024], *cmd = "", *p;
	int ret = 1;

	if (fd < 0)
		goto out;

	if (ac > 1)
		cmd = av[1];

	switch (cmd[0]) {
	case 'd':
		ret = reqPages(fd, "NodeStore_dumpTable", dump);
		break;
	case 'p':
		ret = reqPages(fd, "InterfaceController_peerStats", peers);
		break;
	default:
		/* try to convert cjdroute publicKey */
		for (p = cmd; *p; ++p);

		if (p[-1] == 'k' && p[-2] == '.') {
			if ((ret = pubk2ip6(cmd, buf)) >= 0) {
				buf[39] = '\n';
				write(1, buf, 40);
			}
		} else
			usage();
	}

	if (ret < 0)
		ret = -ret;

	close(fd);
out:
	return ret;
}