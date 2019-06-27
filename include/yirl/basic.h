#define NULL	0
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

typedef int_ptr_t size_t;
typedef int_ptr_t intptr_t;
typedef void FILE;

void	fflushout(void);
int open(const char *path, int oflag, ...);
size_t read(int fd, void *buf, size_t count);
int printf(const char *format, ...);
int sprintf(char *str, const char *format, ...);
int snprintf(char *str, size_t size, const char *format, ...);
char *strncpy(char *dest, const char *src, size_t n);
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
