#include <unistd.h>

#include "cjdc.h"

static char *bskip(char *p)
{
	unsigned int u;
	char c = *p;

	switch (c) {
	case 'd': /* skip dict */
	case 'l': /* skip list */
		++p;

		do {
			if ((p = bskip(p)) == NULL)
				goto err;
		} while (*p != 'e');

		++p;
		break;
	case 'i': /* skip integer */
		while (*p++ != 'e'); 
		break;
	default:
		if (c >= '0' && c <= '9') { /* skip string */
			p = a2u(p, &u);
			++p;
			p += u;
		} else {
err:			write(1, "error\n", 6);//TODO drop msg
			p = NULL;
		}
	}

	return p;
}

static char *bvali(unsigned char i, char **p)
{
	char *q = *p, *s = NULL;

	if (*++q != ':')
		goto err;

	++q;
	q += i;

	if (*q != 'i') {
err:		q = NULL;
		goto out;
	}

	for (s = ++q; *q != 'e'; ++q);
	++q;
out:
	*p = q;
	return s;
}

static char *bvals(unsigned char i, char **p)
{
	char *q = *p, *s = NULL;
	unsigned int u;

	if (*++q != ':')
		goto err;

	++q;
	q += i;
	s = q;

	if (*q >= '0' || *q <= '9') {
		q = a2u(q, &u);
		++q;
		q += u;
	} else
err:		q = NULL;

	*p = q;
	return s;
}

static inline void bputi_(const char *p)
{
	const char *s = p;

	for (; *p != 'e'; ++p);
	write(1, s, p - s);
}

void bputi(const char *p)
{
	unsigned long long m, x, y, z;
	char buf[5] = "  ", *s = buf;
	char unit[] = " KMGTPEZ";
	const char *q = p;
	int i, n = 0, v;

	if (p == NULL) {
		s += 2;
		*s++ = '-';
		*s++ = '0';
		goto out;
	}

	for (x = 0; *p != 'e'; ++p) {
		x *= 10;
		x += *p - '0';
	}

	v = p - q;

	if (v <= 3) {
		s += 3 - v;
		s = u2a(s, x);
		goto out;
	}

	y = x;

	do {
		v = y;
		y >>= 10;

		if (y > 0)
			++n;
	} while (y);

	if (v < 10) {
		for (i = 0, m = 1; i < n; ++i)
			m <<= 10;

		z = x - v * m;
		*s++ = '0' + v;
		*s++ = '.';

		do {
			v = z;
			z >>= 10;
		} while (z);

		if (v >= 1000)
			v = 0;

		*s++ = '0' + (v / 100);
	} else {
		if (v >= 100) {
			*s++ = '0' + (v / 100);
			v %= 100;
		} else
			*s++ = ' ';

		*s++ = '0' + (v / 10);
		*s++ = '0' + (v % 10);
	}

out:
	*s++ = unit[n];
	*s = ' ';
	write(1, buf, 5);
}

static void bputs(const char *s)
{
	unsigned int u;

	if (s) {
		s = a2u((char *)s, &u);
		++s;
		write(1, s, u);
	} else
		write(1, "?", 1);
}



int peers(char *page, unsigned int size)
{
	char *pk, *in, *out, *dup, *los, *oor, *sta, *swi, *usr;
	char ip6[40], *p = page, *end = page + size;
	int more = 0, state = 0;

	in = out = dup = los = oor = NULL;
	pk = sta = swi = usr = NULL;

	if (*p++ != 'd')
		goto err;

	while (p < end) {
		char c =*p;

		if (state & indict) {
			switch (c) {
			case 'e': /* end of dict */
				state &= ~indict;
				++p;

				/* print peer */
				if (pk == NULL)
					goto err; //TODO print error

				while (*pk++ != ':');

				if (pubk2ip6(pk, ip6) < 0)
					goto err;

				ip6[39] = ' ';
				write(1, ip6, 40);
				bputs(swi);
				write(1, " in ", 4);
				bputi(in);
				write(1, "out ", 4);
				bputi(out);

				if (sta) {
					sta += 3;
					sta[4] = ' ';
				} else
					sta = "???? ";

				write(1, sta, 5);
				bputs(sta);
				bputi(dup);
				bputi(los);
				bputi(oor);

				if (usr)
					bputs(usr);

				write(1, "\n", 1);
				in = out = dup = los = oor = NULL;
				pk = sta = swi = usr = NULL;
				break;
			case '1':
				switch (c = *++p) {
				case '0':
					if (p[2] == 'd') /* duplicate */
						dup = bvali(10, &p);
					else
						goto skip2;
					break;
				case '1':
					switch (p[2]) {
					case 'l': /* lostPackets */
						los = bvali(11, &p);
						break;
					case 's': /* switchLabel */
						swi = bvals(11, &p);
						break;
					default:
						goto skip2;
					}
					break;
				case '8': /* 18:receivedOutOfRangei */
					oor = bvali(18, &p);
					break;
				default:
skip2:					--p;
					goto skip;
				}

				break;
			case '4':
				if (p[2] == 'u') /* user */
					usr = bvals(4, &p);
				else
					goto skip;
				break;
			case '5':
				if (p[2] == 's') /* state */
					sta = bvals(5, &p);
				else
					goto skip;
				break;
			case '7': /* 7:bytesIni...e */
				in = bvali(7, &p);
				break;
			case '8': /* 8:bytesOuti...e */
				out = bvali(8, &p);
				break;
			case '9': /* 9:publicKey */
				pk = bvals(9, &p);
				break;
			default:
				goto skip;
			}
		} else if (state & inlist) {
			switch (c) {
			case 'd':
				state |= indict;
				++p;
				break;
			case 'e':
				state &= ~inlist;
				++p;
				break;
			default:
				goto err;
			}
		} else if (state & inpeers) {
			switch (c) {
			case 'l': /* start of list */
				state |= inlist;
				++p;
				break;
			case 'e': /* end of top dict */
				state &= ~inpeers;
				++p;
				goto out;
			default:
				goto skip;
			}
		} else  {
			switch (c) {
			case '4':
				if (p[2] == 'm' && p[7] == '1')
					more = 1;

				goto skip;
			case '5':
				if (p[2] == 'p' && p[6] == 's')
					state |= inpeers;

				goto skip;
			default:
skip:				p = bskip(p);
			}
		}

		if (p == NULL) {
err:			more = -1;
			goto out;
		}
	}
out:
	return more;
}
