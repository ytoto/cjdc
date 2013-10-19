#include "sha2.c"

const char xdigits[16] = "0123456789abcdef";

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
