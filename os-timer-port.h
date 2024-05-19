#ifndef _OS_TIMER_PORT_H_
#define _OS_TIMER_PORT_H_

#if __linux__

void portTimerInit( void (*callback)());

void portTimerStop();

#define portTimerTicksPerUs() 1

long portTimerGetUs();

void portTimerStart(long end_us);

#elif __riscv__

#include "hal/timer/mtime.h"
#include "hal/timer/riscv_mtime_csr.h"

#define portTimerInit(cb)  \
    do                     \
    {                      \
        MtimeSetPeriod(0); \
        MtimeRegister(cb); \
    } while (0)

#define portTimerStop() __mtime_set_compare(~0ULL)

#define portTimerTicksPerUs() ((MtimeGetPeriod() + 1) * 1000)

#define portTimerGetUs() __mtime_get_count()

#define portTimerStart(end) __mtime_set_compare(end)

#endif

#endif
