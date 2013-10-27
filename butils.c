#include <stdint.h>
#include <unistd.h>

#include "cjdc.h"

char *bskip(char *p)
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

char *bvali(unsigned char i, char **p)
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

char *bvals(unsigned char i, char **p)
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
	uint64_t m, x, y, z;
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

		if (v > 999) /* 1000..1023 */
			v = 0;

		*s++ = '0' + (v / 100);
	} else {
		if (v > 999) /* 1000..1023 */
			v = 999;
			
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

void bputs(const char *s)
{
	unsigned int u;

	if (s) {
		s = a2u((char *)s, &u);
		++s;
		write(1, s, u);
	} else
		write(1, "?", 1);
}

