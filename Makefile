CC	= gcc
CFLAGS	= -g -Wall
LDFLAGS	=

OBJS	= main.o butils.o utils.o pubk2ip6.o dump.o peers.o
SRCS	= main.c butils.c utils.c pubk2ip6.c dump.c peers.c

TESTS	=\
	t_peers		\

TSRCS	=\
	t_peers.c	\

all: cjdc

cjdc: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

clean:
	rm -f cjdc $(OBJS) $(TESTS) t_*.o

depend:
	mkdep -MM $(SRCS) $(TSRCS)

tests: $(TESTS)

t_peers: t_peers.o peers.o pubk2ip6.o butils.o utils.o
	$(CC) $^$> $(LDFLAGS) -o $@

.PHONY: all clean depend tests
