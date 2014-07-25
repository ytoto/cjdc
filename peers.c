#include <unistd.h>

#include "cjdc.h"
#include "utils.h"

int peers(char *page, unsigned int size)
{
	char *pk, *in, *out, *dup, *los, *oor, *sta, *swi, *usr;
	char ip6[43], *p = page, *end = page + size;
	char str[1024], *q = str;
	int more = 0, state = 0;

	if (*p++ != 'd')
		goto err;

	goto init;

	while (p < end) {
		char c =*p;

		if (state & indict) {
			switch (c) {
			case 'e': /* end of dict */
				state &= ~indict;
				++p;

				if (pk == NULL)
					goto pk_invalid;

				while (*pk++ != ':');

				if (pubk2ip6(pk, ip6 + 3) < 0) {
pk_invalid:				ip6[0] = '1';
					ip6[1] = ':';
					ip6[2] = '?';
				} else {
					ip6[0] = '3';
					ip6[1] = '9';
					ip6[2] = ':';
				}

				q = bputs(q, ip6, 40);
				q = bputs(q, swi, 20);
				q = bputi(q, in);
				q = bputi(q, out);
				q = bputs(q, sta, 4);
				*q++ = ' ';
				q = bputi(q, dup);
				q = bputi(q, los);
				q = bputi(q, oor);

				if (usr)
					q = bputs(q, usr, 16);

				*q++ = '\n';
init:				in = out = dup = los = oor = NULL;
				pk = sta = swi = usr = NULL;
				break;
			case '1':
				switch (c = *++p) {
				case '0':
					if (p[2] == 'd') /* duplicate */
						dup = bgeti(10, &p);
					else
						goto skip2;
					break;
				case '1':
					switch (p[2]) {
					case 'l': /* lostPackets */
						los = bgeti(11, &p);
						break;
					case 's': /* switchLabel */
						swi = bgets(11, &p);
						break;
					default:
						goto skip2;
					}
					break;
				case '8': /* 18:receivedOutOfRange */
					oor = bgeti(18, &p);
					break;
				default:
skip2:					--p;
					goto skip;
				}

				break;
			case '4':
				if (p[2] == 'u') /* user */
					usr = bgets(4, &p);
				else
					goto skip;
				break;
			case '5':
				if (p[2] == 's') /* state */
					sta = bgets(5, &p);
				else
					goto skip;
				break;
			case '7': /* 7:bytesIn */
				in = bgeti(7, &p);
				break;
			case '8': /* 8:bytesOut */
				out = bgeti(8, &p);
				break;
			case '9': /* 9:publicKey */
				pk = bgets(9, &p);
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
				goto out_;
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
			break;
		}
	}
out_:
	write(1, str, q - str);
	return more;
}
