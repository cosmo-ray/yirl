#include <yirl/basic.h>

static void SIG_IGN(int i) {(void)i;}

typedef void (*sighandler_t)(int);
static inline sighandler_t signal(int signum, sighandler_t handler)
{
	printf("catch signal: %d\n", signum);
	return handler;
}

#define SIGINT 12
#define SIGKILL 15
