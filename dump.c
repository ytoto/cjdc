#include <unistd.h>

#include "cjdc.h"
#include "utils.h"

int dump(char *page, unsigned int size, cjdc_ctx *ctx)
{
	char *p = page, *end = page + size;
	char *cnt, *ip, *path, *link, *ver;
	char str[1024], *q = str;
	int more = 0, state = 0;
	unsigned int u;

	cnt = NULL;

	if (*p++ != 'd')
		goto err;

	goto init;

	while (p < end) {
		char c = *p;

		if (state & indict) {
			switch (c) {
			case 'e': /* end of dict */
				state &= ~indict;
				++p;
				q = bputs(q, ip, 40);
				q = bputs(q, path, 20);

				if (link) {
					a2u(link, &u);
					u /= 5366870;
					*u2a(link = ip, u) = 'e';
				}

				q = bputi(q, link);
				q = bputi(q, ver);
				*q++ = '\n';
init:				ip = path = link = ver = NULL;
				break;
                        case '0': /* skip leading zeros */
skipzeros:			while (*++p == '0');
				break;
			case '2': /* ip */
				ip = bgets(2, &p);
				break;
			case '4':
				switch (p[2]) {
				case 'l': /* link */
					link = bgeti(4, &p);
					break;
				case 'p': /* path */
					path = bgets(4, &p);
					break;
				default:
					goto skip;
				}
				break;
			case '7': /* version */
				ver = bgeti(7, &p);
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
			case '0':
				goto skipzeros;
			case '1': /* routingTable */
				if (p[1] == '2' && p[3] == 'r')
					state |= intable;

				goto skip;
			case '4': /* more */
				if (p[2] == 'm' && p[7] == '1')
					more = 1;

				goto skip;
			case '5':
				if (p[2] == 'c' && p[6] == 't') {
					cnt = &p[2];
					bgeti(5, &p);
				} else
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
		/* copy 'count xxx\n' to ctx->vinfo */
		cnt[5] = ' ';
		p = ctx->vinfo;
		ctx->vinfo_len = 1;
		for (; *cnt != 'e'; *p++ = *cnt++, ++ctx->vinfo_len);
		*p++ = '\n';
	}

	write(1, str, q - str);
	return more;
}
