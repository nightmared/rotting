//#include <linux/time.h>
#include <time.h>
#include <stdio.h>
struct timeval {
    int         tv_sec;         /* seconds */
    int    tv_usec;        /* microseconds */
};


int main() {
    while (1) {
        struct timeval tv;
        gettimeofday(&tv, 0);
        printf("%i\n", tv.tv_sec);
        sleep(1);
    }
}
