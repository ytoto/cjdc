#include "Base32.h"
#include "sha4.c"

extern const char xdigits[];

#define Key_parse_TOO_SHORT	-1
#define Key_parse_MALFORMED	-2
#define Key_parse_DECODE_FAILED	-3
#define Key_parse_INVALID	-4

int pubk2ip6(const char *pk, char *ip6/*40*/)
{
	const unsigned char *upk = (unsigned char *)pk;
	const unsigned char *p = upk;

	unsigned char buf[64];
	sha4_context sha4ctx;
	int i, ret;

	if (pk == NULL) {
		ret = Key_parse_INVALID;
		goto out;
	}

	while (*p++ != '.');

	if (*p-- != 'k') {
		ret = Key_parse_MALFORMED;
		goto out;
	}

	if (p - upk < 52) {
		ret = Key_parse_TOO_SHORT;
		goto out;
	}

	if (Base32_decode(buf, sizeof(buf), upk, p - upk) != 32) {
		ret = Key_parse_DECODE_FAILED;
		goto out;
	}

	sha4_starts(&sha4ctx, 0);
	sha4_update(&sha4ctx, buf, 32);
	sha4_finish(&sha4ctx, buf);
	sha4_starts(&sha4ctx, 0);
	sha4_update(&sha4ctx, buf, 64);
	sha4_finish(&sha4ctx, buf);

	if (buf[0] != 0xfc) {
		ret = Key_parse_INVALID;
		goto out;
	}

	/* convert to ascii */
	for (i = 0, p = buf; i < 16; ++i, ++p) {
		unsigned char x = *p;

		*ip6++ = xdigits[x >> 4];
		*ip6++ = xdigits[x & 15];

		if (i & 1)
			*ip6++ = ':';
	}

	*--ip6 = '\0';
	ret = 0;
out:
	return ret;
}
