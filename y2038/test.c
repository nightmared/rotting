/*#include <linux/time.h>
#include <bits/types.h>
typedef __clockid_t clockid_t;


extern int clock_gettime (clockid_t __clock_id, struct timespec *__tp);
*/
#include <linux/time.h>
#include <dlfcn.h>
extern char *getenv (const char *__name) __THROW __nonnull ((1)) __wur;
extern int atoi (const char *__nptr);


	static int base = 0;
int gettimeofday(struct timeval *tv, void *tzp) {
	void *libc_ptr = dlopen("/usr/lib/libc.so.6", RTLD_LAZY);
	int (*gtod)(struct timeval *restrict tp, void *restrict tzp) = dlsym(libc_ptr, "gettimeofday");
	gtod(tv, 0);
	char *secs = getenv("SECONDS");
	if (base == 0)
		base = tv->tv_sec;

	int diff = tv->tv_sec - base;
	tv->tv_sec = (1<<31) - (secs ? atoi(secs) : 5) + diff;
	dlclose(libc_ptr);
	return 0;
}
