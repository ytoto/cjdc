CC	= clang
CFLAGS	= -g -Wall
LDFLAGS	=

OBJS	= main.o utils.o pubk2ip6.o dump.o peers.o
SRCS	= main.c utils.c pubk2ip6.c dump.c peers.c

TESTS	=\
	t_peers		\
	t_peers2	\

TSRCS	=\
	t_peers.c	\
	t_peers2.c	\

all: cjdc

cjdc: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

clean:
	rm -f cjdc $(OBJS) $(TESTS) t_*.o

depend:
	mkdep -MM $(SRCS) $(TSRCS)

loc:
	@cat $(SRCS) | wc -l

tests: $(TESTS)

t_peers: t_peers.o peers.o pubk2ip6.o utils.o
	$(CC) $^$> $(LDFLAGS) -o $@

t_peers2: t_peers2.o peers.o pubk2ip6.o utils.o
	$(CC) $^$> $(LDFLAGS) -o $@

.PHONY: all clean depend loc tests
