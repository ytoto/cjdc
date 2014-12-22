enum {
	incolon		= (1 << 0),
	indict		= (1 << 1),
	inlist		= (1 << 2),
	inpassk		= (1 << 3),
	inpassv		= (1 << 4),
	inpeers		= (1 << 5),
	inquote		= (1 << 6),
	intable		= (1 << 7),
};

extern int pubk2ip6(const char *pk, char *out/*40*/);
extern void sha256hex(char *out/*64*/, const char *input, int size);

/* commands */
typedef struct {
	int	vinfo_len;
	char	vinfo[128];
} cjdc_ctx;

extern int dump(char *page, unsigned int size, cjdc_ctx *);
extern int peers(char *page, unsigned int size, cjdc_ctx *);
