#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>

#include "cjdc.h"
#include "utils.h"

static char passwd[256]; /* and space for cookie */
static int passwdlen;

#define USAGE \
"usage: cjdc [options] [command]\n"	\
"\n"				\
"commands:\n"			\
"dump\t\tdumps the routing table\n"	\
"peers\t\tlists connected nodes\n"	\
"<pub.k>\t\tpublic key to ipv6\n"	\
"\n"				\
"options:\n"			\
"-v\t\tverbose output\n"

static inline void usage(void)
{
	write(1, USAGE, sizeof(USAGE) - 1);
}

void puterr(const char *msg, int _errno_)
{
	const char *p = msg;
	char buf[1024], *q;

	for (q = buf; *p; *q++ = *p++);
	*q++ = ':';
	*q++ = ' ';
	write(2, buf, q - buf);
	p = strerror(_errno_);
	for (q = buf; *p; *q++ = *p++);
	*q++ = '\n';
	write(2, buf, q - buf);
}

static inline int getAdminFile(void)
{
	static const char cjdnsadmin[] = "/.cjdnsadmin";

	char buf[1024];
	char *p = getenv("HOME");
	char *q = buf;
	int fd;

	while (*p)
		*q++ = *p++;

	memcpy(q, cjdnsadmin, sizeof(cjdnsadmin));

	if ((fd = open(buf, O_RDONLY)) < 0)
		puterr(buf, errno);

	return fd;
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
		puterr("read config", errno);
		fd = -1;
		goto out;
	}

	close(fd);

	char *p = buf, *q = buf;
	int i, j = 0, state = 0;

	passwd[0] = '\0';

	for (i = 0; i < n; ++i, ++p) {
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
	struct addrinfo *res = NULL, *tres;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		puterr("socket", errno);
		goto out;
	}

	if (getaddrinfo(node, port, &hints, &res) == 0) {
		for (tres = res; tres; tres = tres->ai_next)
			if (connect(fd, tres->ai_addr, tres->ai_addrlen) == 0)
				break;
	} else
		puterr("getaddrinfo", errno);

	if (res == NULL) {
		close(fd);
		fd = -1;
	} else
		freeaddrinfo(res);
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
	static char txid[10] = "@BCDEABCDE";
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

static int checkTimeout(int fd, int seconds)
{
	/* timeout to avoid blocking read */
	struct timeval timeout;
	int n, ret = -1;
	fd_set rfds;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	timeout.tv_sec = seconds;
	timeout.tv_usec = 0;

	n = select(fd + 1, &rfds, NULL, NULL, &timeout);

	if (n == 0) {
		write(2, "socket timeout\n", 15);
		goto out;
	} else if (n < 0) {
		puterr("socket select", errno);
		goto out;
	}

	ret = 0;
out:
	return ret;
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

	if ((n = write(fd, buf, p - buf)) < 0) {
		puterr("socket write", errno);
		goto out;
	}

	if (checkTimeout(fd, 2) < 0)
		goto out;

	if ((n = read(fd, buf, sizeof(buf))) < 0) {
		puterr("socket read", errno);
		goto out;
	}

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

static char *geterror(char *p, int n)
{
	char *end = p + n - 7; /* searching for "5:error" */

	for (; p < end; ++p) {
		if (memcmp(p, "5:error", 7) == 0) {
			p += 7;

			if (memcmp(p, "4:none", 6) != 0)
				return p;
		}
	}

	return NULL;
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

		if (checkTimeout(fd, 1) < 0)
			goto out;

		if ((n = read(fd, page, sizeof(page))) < 0)
			goto out;

		if ((p = geterror(page, n))) {
			/* print error */
			unsigned int u;

			p = a2u(p, &u);
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

const char dump_header[] =
"ip6\t\t\t\t\tpath                link  ver\n";
const char peers_header[] =
"ip6\t\t\t\t\tswitch               in   out state dup  los  oor user\n";

int main(int ac, char *av[])
{
	char buf[1024], *cmd = "", *p;
	int fd = -1, ret = 1, _v = 0;

	/* parse options */
	if (ac > 1) {
		p = av[1];

		if (*p++ == '-') {
			--ac;
			++av;

			for (; *p != '\0'; ++p) {
				switch (*p) {
				case 'v':
					++_v;
				default:
					break;
				}
			}
		}
	}

	if (ac > 1)
		cmd = av[1];

	/* try to convert cjdroute publicKey to IPv6 */
	for (p = cmd; *p; ++p);

	if (p[-1] == 'k' && p[-2] == '.') {
		if ((ret = pubk2ip6(cmd, buf)) == 0) {
			buf[39] = '\n';
			write(1, buf, 40);
		} else
			puterr(cmd, EINVAL);

		goto out;
	}

	switch (cmd[0]) {
	case 'd':
	case 'p':
		fd = connectWithAdminInfo();

		if (fd < 0)
			goto out;

	}

	switch (cmd[0]) {
	case 'd': /* dump */
		if (_v)
			write(1, dump_header, sizeof(dump_header) - 1);

		ret = reqPages(fd, "NodeStore_dumpTable", dump);
		break;
	case 'p': /* peers */
		if (_v)
			write(1, peers_header, sizeof(peers_header) - 1);

		ret = reqPages(fd, "InterfaceController_peerStats", peers);
		break;
	default:  /* help */
		usage();
	}
out:
	if (ret < 0)
		ret = -ret;

	if (fd >= 0)
		close(fd);

	return ret;
}
