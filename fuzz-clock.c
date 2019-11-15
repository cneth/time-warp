#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/timex.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#define USEC_PER_MSEC 1000L
#define FUZZ_AMOUNT 500L


ssize_t format_timeval(struct timeval *tv, char *buf, size_t sz)
{
    ssize_t written = -1;
    struct tm *gm = (struct tm *)gmtime(&tv->tv_sec);

    if (gm)
    {
        written = (ssize_t)strftime(buf, sz, "%Y-%m-%dT%H:%M:%S", gm);
        if ((written > 0) && ((size_t)written < sz))
        {
            int w = snprintf(buf+written, sz-(size_t)written, ".%06dZ", tv->tv_usec);
            written = (w > 0) ? written + w : -1;
        }
    }
    return written;
}

int main(int argc, char *argv[]) {
    struct timeval tv={0};
    int rc;
    char buf[28];
    long long int error;
    long int fuzz_amount = FUZZ_AMOUNT;


    if ( argc > 2 ) {
       fprintf(stderr, "Usage: %s num - where num is number of ms to add to clock\n", argv[0]);
       return 1;
    } 
    else if (argc == 2) {
       fuzz_amount = atoi(argv[1]);
    }


    rc = gettimeofday(&tv, NULL);
    if (rc == -1) {
        perror("gettimeofday");
        fprintf(stderr, "gettimeofday() returned -1 indicating error. Unable to get clock. See above perror message for details.\n");
        return 1;
    }
    
    if (format_timeval(&tv, buf, sizeof(buf)) > 0) {
        fprintf(stderr, "Current time is: %s\n", buf);
    }

    fprintf(stderr, "Adjusting system time backwards by %dms.\n", fuzz_amount);

    if (tv.tv_usec < (fuzz_amount*USEC_PER_MSEC)) {
       tv.tv_sec -= 1;
       tv.tv_usec += ((1000L * USEC_PER_MSEC) - (fuzz_amount * USEC_PER_MSEC));
    }
    else {
    	tv.tv_usec -= (fuzz_amount * USEC_PER_MSEC);
    }

    rc = settimeofday(&tv, NULL);
    if (rc == -1) {
        perror("settimeofday");
        fprintf(stderr, "settimeofday() returned -1 indicating error. Unable to adjust clock - make sure you are root\n");
        return 1;
    }

    rc = gettimeofday(&tv, NULL);
    if (rc == -1) {
        perror("gettimeofday");
        fprintf(stderr, "gettimeofday() returned -1 indicating error. Unable to get clock  See above perror message for details.\n");
        return 1;
    }

    if (format_timeval(&tv, buf, sizeof(buf)) > 0) {
        fprintf(stderr, "Time is now: %s\n", buf);
    }

    return 0;
}
