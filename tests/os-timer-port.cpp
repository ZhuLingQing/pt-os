#include "os-timer-port.h"
#include "os-timer.h"
#include "pt-os.h"
#include <stdio.h>

#if __linux__

#include <signal.h>
#include <time.h>
#include <sys/time.h>

static void (*TimerCallback_)();
static long TimerBaseUs_ = 0;

static void TimerRoutine(int signo)
{
    struct itimerval value, ovalue;
    switch (signo){
    case SIGALRM:
        portTimerStop();
        TimerCallback_();
        break;
    }
}

void portTimerInit( void (*callback)())
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    TimerBaseUs_ = tv.tv_sec*1000000 + tv.tv_usec;
    TimerCallback_ = callback;
    signal(SIGALRM, TimerRoutine);
}

void portTimerStop()
{
    struct itimerval value, ovalue;
    value.it_value.tv_sec = 0;
    value.it_value.tv_usec = 0;
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &value, &ovalue);
}

long portTimerGetUs()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (tv.tv_sec*1000000 + tv.tv_usec - TimerBaseUs_);
}

void portTimerStart(long end_us)
{
    struct itimerval value, ovalue;
    long offset = end_us - portTimerGetUs();
    if (offset < 10) offset = 100;
    value.it_value.tv_sec = offset/1000000;
    value.it_value.tv_usec = offset%1000000;
    value.it_interval.tv_sec = value.it_value.tv_sec;
    value.it_interval.tv_usec = value.it_value.tv_usec;
    setitimer(ITIMER_REAL, &value, &ovalue);
}

#endif