#include <unistd.h>

#include "butils.h"
#include "cjdc.h"

int dump(char *page, unsigned int size)
{
	char *p = page, *end = page + size, *q;
	char *cnt, *ip, *path, *link, *ver;
	int more = 0, state = 0;
	unsigned int u;

	if (*p++ != 'd')
		goto err;

	cnt = NULL;
	goto init;

	while (p < end) {
		char c = *p;

		if (state & indict) {
			switch (c) {
			case 'e': /* end of dict */
				state &= ~indict;
				++p;

				if (!ip || *ip++ != '3' || *ip++ != '9' || *ip++ != ':')
					goto init;
				
				if (!path || *path++ != '1' || *path++ != '9' || *path++ != ':')
					goto init;

				ip[39] = ' ';
				write(1, ip, 40);
				path[19] = ' ';
				write(1, path, 20);

				if (link) {
					a2u(link, &u);
					u /= 5366870;
					*u2a(link = ip, u) = 'e';
				}

				bputi(link);
				bputi(ver);
				write(1, "\n", 1);
init:				ip = path = link = ver = NULL;
				break;
			case '1':
				if (p[1] == '9')
					path = bvals(19, &p);
				else
					goto skip;
				break;
			case '2':
				ip = bvals(2, &p);
				break;
			case '4':
				switch (p[2]) {
				case 'l':
					link = bvali(4, &p);
					break;
				case 'p':
					path = bvals(4, &p);
					break;
				default:
					goto skip;
				}
				break;
			case '7':
				ver = bvali(7, &p);
				break;
			default:
				goto skip;
			}
		} else if (state & inlist) {
			switch (c) {
			case 'd': /* start of dict */
				state |= indict;
				++p;
				break;
			case 'e': /* end of list */
				state &= ~inlist;
				++p;
				break;
			default:
				goto err;
			}
		} else if (state & intable) {
			switch (c) {
			case 'l': /* start of list */
				state |= inlist;
				++p;
				break;
			case 'e': /* end of top dict */
				state &= ~intable;
				++p;
				goto out_;
			default:
				goto skip;
			}
		} else {
			switch (c) {
			case '1': /* routingTable */
				if (p[1] == '2' && p[3] == 'r')
					state |= intable;

				goto skip;
			case '4': /* more */
				if (p[2] == 'm' && p[7] == '1')
					more = 1;

				goto skip;
			case '5':
				if (p[2] == 'c' && p[6] == 't')
					cnt = bvali(5, &p);
				else
					goto skip;
				break;
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
	if (cnt) {
		write(1, "count ", 6);
		for (q = cnt; *q != 'e'; ++q);
		*q++ = '\n';
		write(1, cnt, q - cnt);
	}

	return more;
}
