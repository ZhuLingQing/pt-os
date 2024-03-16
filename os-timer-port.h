#ifndef _OS_TIMER_PORT_H_
#define _OS_TIMER_PORT_H_

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

#define portTimerGetTick() __mtime_get_count()

#define portTimerComputeEndTick(tick) (portTimerGetTick() + (tick))

#define portTimerStart(end) __mtime_set_compare(end)

#endif
