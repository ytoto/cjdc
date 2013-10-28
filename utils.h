/* ascii <-> unsigned int */
extern char *a2u(char *p, unsigned int *x);
extern char *u2a(char *p, unsigned int x);

/* benc utils */
extern char *bskip(char *p);
extern char *bgeti(unsigned char i, char **p);
extern char *bgets(unsigned char i, char **p);
extern char *bputi(char *dst, char *src);
extern char *bputs(char *dst, char *src, unsigned int n);
