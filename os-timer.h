#ifndef _OS_TIMER_H_
#define _OS_TIMER_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif
    typedef void (*OsTimerCallback_t)(int id, void *param);
    /*****************************************************************************\
     * @description : Initialize the OsTimer module.
     * @return       {*}
    \*****************************************************************************/
    void OsTimerInit();

    /*****************************************************************************\
     * @description : Register a timer
     * ------------
     * @callback    [I]: timer triggered callback function.
     * @param       [I]: input parameter for callback function.
     * @period_us   [I]: micro second delay to trigger the timer.
     * @repeatable  [I]: true=auto reload period_us.
     * @id          [O]: output the timer id for Kill. (Will be auto killed after
     *                   triggered, if not repeatable timer)
     * @return      error or success
    \*****************************************************************************/
    int OsTimerRegister(OsTimerCallback_t callback, void *param, uint64_t period_us, bool repeatable, int *id);

    /*****************************************************************************\
     * @description : return number of active timer
     * @return       number of active timer
    \*****************************************************************************/
    int OsTimerCount();

    /*****************************************************************************\
     * @description : Kill a timer
     * @id           [I]: timer id, which given by OsTimerRegister(...,&id)
     * @return       error or success
    \*****************************************************************************/
    int OsTimerKill(int id);

    /*****************************************************************************\
     * @description : delay micro seconds
     * @param        [I]: delay_us
     * @return       error or success
    \*****************************************************************************/
    int OsTimerDelayUs(uint64_t delay_us);

#ifdef __cplusplus
}
#endif

#endif
