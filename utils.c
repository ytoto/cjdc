#include <stdint.h>

#include "sha2.c"
#include "utils.h"

const char xdigits[16] = "0123456789abcdef";

char *bgeti(unsigned char i, char **p)
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

char *bgets(unsigned char i, char **p)
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

char *bputi(char *dst, char *p)
{
	char unit[] = " KMGTPEZ";/*TODO static ? */
	uint64_t m, x, y, z;
	const char *q = p;
	int i, n = 0, v;

	if (p == NULL) {
		*dst++ = ' ';
		*dst++ = ' ';
		*dst++ = '-';
		*dst++ = '0';
		goto out;
	}

	for (x = 0; *p != 'e'; ++p) {
		x *= 10;
		x += *p - '0';
	}

	v = p - q;

	if (v <= 3) {
		for (i = 3 - v; i--; *dst++ = ' ');
		dst = u2a(dst, x);
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
		*dst++ = '0' + v;
		*dst++ = '.';

		do {
			v = z;
			z >>= 10;
		} while (z);

		if (v > 999) /* 1000..1023 */
			v = 0;

		*dst++ = '0' + (v / 100);
	} else {
		if (v > 999) /* 1000..1023 */
			v = 999;
			
		if (v >= 100) {
			*dst++ = '0' + (v / 100);
			v %= 100;
		} else
			*dst++ = ' ';

		*dst++ = '0' + (v / 10);
		*dst++ = '0' + (v % 10);
	}

out:
	*dst++ = unit[n];
	*dst++ = ' ';
	return dst;
}

char *bputs(char *dst, char *src, unsigned int padding)
{
	unsigned int i, u;

	if (src && *src >= '0' && *src <= '9') {
		src = a2u(src, &u);
		src++;
	} else {
		src = "?";
		u = 1;
	}

	if (u > padding)
		u = padding;

	for (i = 0; i < u; ++i, *dst++ = *src++);
	for (; i < padding; ++i, *dst++ = ' ');
	return dst;
}

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
	default: /* skip string */
		if (c >= '0' && c <= '9') {
			p = a2u(p, &u);
			++p;
			p += u;
		} else
err:			p = NULL;
	}

	return p;
}

char *u2a(char *p, unsigned int x)
{
	char *q = p, *s;

	do {
		*q++ = xdigits[x % 10];
		x /= 10;
	} while (x);

	/* reverse string */
	for (s = q--; q > p; ++p, --q) {
		char c = *p;

		*p = *q;
		*q = c;
	}

	return s;
}

char *a2u(char *p, unsigned int *x)
{
	unsigned int u = 0;
	char c;

	for (c = *p; c >= '0' && c <= '9'; c = *++p) {
		u *= 10;
		u += c - '0';
	}

	*x = u;
	return p;
}

void sha256hex(char *out, const char *buf, int size)
{
	unsigned char bin[32], *p = bin;
	sha2_context ctx;
	char *q = out;
	int i;

	sha2_starts(&ctx, 0);
	sha2_update(&ctx, (unsigned char *)buf, size);
	sha2_finish(&ctx, bin);

	for (i = 0; i < 32; ++i, ++p) {
		unsigned char x = *p;

		*q++ = xdigits[x >> 4];
		*q++ = xdigits[x & 15];
	}
}
