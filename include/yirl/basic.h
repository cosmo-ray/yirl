#ifndef BASIC_H_
#define BASIC_H_

#include <stddef.h>

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef OK
#define OK 0
#endif

#ifndef ERR
#define ERR -1
#endif

#define INT_MIN       (-INT_MAX - 1)
#define INT_MAX       2147483647

#define M_E            2.7182818284590452354   /* e */
#define M_LOG2E        1.4426950408889634074   /* log_2 e */
#define M_LOG10E       0.43429448190325182765  /* log_10 e */
#define M_LN2          0.69314718055994530942  /* log_e 2 */
#define M_LN10         2.30258509299404568402  /* log_e 10 */
#define M_PI           3.14159265358979323846  /* pi */
#define M_PI_2         1.57079632679489661923  /* pi/2 */
#define M_PI_4         0.78539816339744830962  /* pi/4 */
#define M_1_PI         0.31830988618379067154  /* 1/pi */
#define M_2_PI         0.63661977236758134308  /* 2/pi */
#define M_2_SQRTPI     1.12837916709551257390  /* 2/sqrt(pi) */
#define M_SQRT2        1.41421356237309504880  /* sqrt(2) */
#define M_SQRT1_2      0.70710678118654752440  /* 1/sqrt(2) */

#define O_ACCMODE          0003
#define O_RDONLY             00
#define O_WRONLY             01
#define O_RDWR               02
#ifndef O_CREAT
# define O_CREAT           0100 /* Not fcntl.  */
#endif
#ifndef O_EXCL
# define O_EXCL            0200 /* Not fcntl.  */
#endif
#ifndef O_NOCTTY
# define O_NOCTTY          0400 /* Not fcntl.  */
#endif
#ifndef O_TRUNC
# define O_TRUNC          01000 /* Not fcntl.  */
#endif
#ifndef O_APPEND
# define O_APPEND         02000
#endif
#ifndef O_NONBLOCK
# define O_NONBLOCK       04000
#endif
#ifndef O_NDELAY
# define O_NDELAY       O_NONBLOCK
#endif
#ifndef O_SYNC
# define O_SYNC        04010000
#endif
#define O_FSYNC         O_SYNC
#ifndef O_ASYNC
# define O_ASYNC         020000
#endif

typedef struct __FILE FILE;

#define EOF (-1)
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

typedef unsigned int time_t;

void	fflushout(void);
int open(const char *path, int oflag, ...);
int close(int fildes);

FILE *fopen(const char *pathname, const char *mode);
int fclose(FILE *stream);
int fscanf(FILE *stream, const char *format, ...);

time_t time(time_t *tloc);

ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);

int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int sprintf(char *str, const char *format, ...);
int snprintf(char *str, size_t size, const char *format, ...);
char *strncpy(char *dest, const char *src, size_t n);
char *strcpy(char *dest, const char *src);
void free(void *);
void *malloc(size_t);
void *realloc(void *, size_t);
void *calloc(size_t, size_t);
size_t strlen(const char *s);
char *strdup(const char *s);
char *strndup(const char *s, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);

int fflush(FILE *stream);

void *memset(void *s, int c, size_t n);
int atoi(const char *str);
long strtol(const char *restrict str, char **restrict endptr, int base);

double cos(double);
double sin(double);

int abs(int);
double sqrt(double x);
double floor(double);

/* seems from greping in /usr/include that it should be u32 */
typedef unsigned int useconds_t;

int usleep(useconds_t);

void abort(void);
int isdigit(int);

/* Because I have no fucking idea of to define
 * a non function symbole on windows
 * I can't define stderr */
#ifdef _WIN32
#define fprintf(a, b, args...)			\
  printf(b, args);
#endif


#ifdef NDEBUG
#define assert(expr)
#else
#define assert(expr)					\
  if (!expr) {						\
    fprintf(stderr, "assertion fail at %s:%d\n",	\
	    __LINE__, __FILE__);			\
    abort();						\
  };
#endif

int rand(void);
void srand(unsigned int seed);

#endif
