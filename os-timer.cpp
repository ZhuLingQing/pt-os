#include "os_timer.h"
#include "pt-os.h"
#include "os_timer_port.h"
#include <etl/set.h>

constexpr int kMaxTimerItem = 16;

typedef struct
{
    OsTimerCallback_t callback;
    void *param;
    uint64_t period_tick;
    uint64_t end_tick;
    int id;
    bool repeatable;
} OsTimer_t;

volatile int kOsTimerTrigged_ = 0;
int kOsTimerHandled_ = 0;
int kOsTimerIdSeed_ = 0;
uint64_t kOsTimerTicksPerUs_ = 0;

struct tick_cmp
{
    bool operator()(const OsTimer_t &a, const OsTimer_t &b) const { return a.end_tick < b.end_tick; }
};
etl::set<OsTimer_t, kMaxTimerItem, tick_cmp> gOsTimerObj_;

static void OsTimerIsrHandler_()
{
    kOsTimerTrigged_ += 1;
    portTimerStop();
}

TASK_DECLARE(OsTimerTask_(OsTaskId taskId, void *))
{
    TASK_BEGIN(taskId);
    while (1)
    {
        TASK_WAIT_UNTIL(taskId, kOsTimerTrigged_ > kOsTimerHandled_);
        kOsTimerHandled_++;
        // find the first timer. Which happened most recently.
        auto tim = gOsTimerObj_.begin();
        OS_ASSERT(tim != gOsTimerObj_.end());  // Shouldn't be empty.
        if (tim->callback) tim->callback(tim->id, tim->param);
        // update the repeatable timer.
        if (tim->repeatable)
        {
            gOsTimerObj_.insert(
                {tim->callback, tim->param, tim->period_tick, tim->end_tick + tim->period_tick, tim->id, true});
        }
        // remove the current timer.
        gOsTimerObj_.erase(tim);
        // if has one or more timer enable it.
        if (gOsTimerObj_.size())
        {
            tim = gOsTimerObj_.begin();
            portTimerStart(tim->end_tick);
        }
    }
    TASK_END(taskId);
}

void OsTimerInit()
{
    portTimerInit(OsTimerIsrHandler_);
    portTimerStop();
    kOsTimerTicksPerUs_ = portTimerTicksPerUs();
    RegisterTask("thrTmr", OsTimerTask_, nullptr);
}

int OsTimerRegister(OsTimerCallback_t callback, void *param, uint64_t period_us, bool repeatable, int *id)
{
    if (gOsTimerObj_.full()) return kLtMallocFail;
    portTimerStop();
    gOsTimerObj_.insert({callback, param, period_us * kOsTimerTicksPerUs_,
                         portTimerComputeEndTick(period_us * kOsTimerTicksPerUs_), ++kOsTimerIdSeed_, repeatable});
    portTimerStart(gOsTimerObj_.begin()->end_tick);
    if (id) *id = kOsTimerIdSeed_;
    return TASK_OP_SUCCESS;
}

int OsTimerCount() { return gOsTimerObj_.size(); }

int OsTimerKill(int id)
{
    for (auto it = gOsTimerObj_.begin(); it != gOsTimerObj_.end(); it++)
    {
        if (it->id == id)
        {
            gOsTimerObj_.erase(it);
            return TASK_OP_SUCCESS;
        }
    }
}

static void timerDelayCallback_(int id, void *param)
{
    bool *triggerred = (bool *)param;
    // HAL_UartPrint("[TIM]: to %llu\n", portTimerGetTick());
    *triggerred = true;
}

int OsTimerDelayUs(uint64_t delay_us)
{
    bool timerTriggered = false;
    // HAL_UartPrint("[TIM]: from %llu - %lluUs\n", portTimerGetTick(), delay_us);
    int rc = OsTimerRegister(timerDelayCallback_, &timerTriggered, delay_us, false, 0);
    if (rc != TASK_OP_SUCCESS) return rc;
    while (timerTriggered == false) TaskYield();
    return TASK_OP_SUCCESS;
}
