#include <unistd.h>

#include "butils.h"
#include "cjdc.h"

int peers(char *page, unsigned int size)
{
	char *pk, *in, *out, *dup, *los, *oor, *sta, *swi, *usr;
	char ip6[40], *p = page, *end = page + size;
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

				/* print peer */
				if (pk == NULL) {
					pk = ":";
					goto err;
				}

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
					while (*sta++ != ':');
					sta[4] = ' ';
				} else
					sta = "???? ";

				write(1, sta, 5);
				bputi(dup);
				bputi(los);
				bputi(oor);

				if (usr)
					bputs(usr);

				write(1, "\n", 1);
init:				in = out = dup = los = oor = NULL;
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
	return more;
}
