#include "Base32.h"
#include "sha4.c"

extern const char xdigits[];

int pubk2ip6(const char *pk, char *ip6/*40*/)
{
	const unsigned char *upk = (unsigned char *)pk;
	const unsigned char *p = upk;
	unsigned char buf[64];
	sha4_context sha4ctx;
	int i, n = -3;

	for (; *p != '.'; ++p);

	if (p[1] != 'k')
		goto out;

	if ((n = Base32_decode(buf, sizeof(buf), upk, p - upk)) < 0)
		goto out;

	sha4_starts(&sha4ctx, 0);
	sha4_update(&sha4ctx, buf, n);
	sha4_finish(&sha4ctx, buf);
	sha4_starts(&sha4ctx, 0);
	sha4_update(&sha4ctx, buf, 64);
	sha4_finish(&sha4ctx, buf);

	for (i = 0, p = buf; i < 16; ++i, ++p) {
		unsigned char x = *p;

		*ip6++ = xdigits[x >> 4];
		*ip6++ = xdigits[x & 15];

		if (i & 1)
			*ip6++ = ':';
	}

	*--ip6 = '\0';
	n = 0;
out:
	return n;
}
