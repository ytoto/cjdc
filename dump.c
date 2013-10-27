#include <unistd.h>

#include "cjdc.h"

int dump(char *page, unsigned int size)
{
	int more = 0;
	write(1, "dump\n", 5);
	return more;
}
