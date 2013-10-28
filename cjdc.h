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

extern char *a2u(char *p, unsigned int *x);
extern char *u2a(char *p, unsigned int x);

extern void puterr(const char *msg, int _errno_);

extern int pubk2ip6(const char *pk, char *out/*40*/);
extern void sha256hex(char *out/*64*/, const char *input, int size);

/* commands */
extern int dump(char *page, unsigned int size);
extern int peers(char *page, unsigned int size);
