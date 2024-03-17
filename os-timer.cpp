#include "os-timer.h"
#include "pt-os.h"
#include "os-timer-port.h"
#include <etl/multiset.h>

static void OsTimerIsrHandler_();

class PtTimer
{
   public:
    typedef struct
    {
        OsTimerCallback_t callback;
        void *param;
        uint64_t period_us;
        uint64_t end_us;
        int id;
        bool repeatable;
    } PtTimer_t;
    struct tick_cmp
    {
        bool operator()(const PtTimer_t &a, const PtTimer_t &b) const { return a.end_us < b.end_us; }
    };
    PtTimer()
    {
        timerTrigged_ = 0;
        timerHandled_ = 0;
        timerIdSeed_ = 0;
    }
    bool NewTimerEventHappened()
    {
        if (timerTrigged_ <= timerHandled_) return false;
        timerHandled_++;
        return true;
    }

    void Init()
    {
        timerObj_.clear();
    }

    void Handle()
    {
        // find the first timer. Which happened most recently.
        auto tim = timerObj_.begin();
        OS_ASSERT(tim != timerObj_.end());  // Shouldn't be empty.
        if (tim->callback) tim->callback(tim->id, tim->param);
        // update the repeatable timer.
        if (tim->repeatable)
        {
            timerObj_.insert(
                {tim->callback, tim->param, tim->period_us, tim->end_us + tim->period_us, tim->id, true});
        }
        // remove the current timer.
        timerObj_.erase(tim);
        // if has one or more timer enable it.
        tim = timerObj_.begin();
        if (tim != timerObj_.end())
            portTimerStart(tim->end_us);
    }
    
    int Register(OsTimerCallback_t callback, void *param, uint64_t period_us, bool repeatable, int *id)
    {
        if (timerObj_.full()) return NO_RESOURCE;
        portTimerStop();
        timerObj_.insert({callback, param, period_us,
                            portTimerGetUs() + period_us, ++timerIdSeed_, repeatable});
        portTimerStart(timerObj_.begin()->end_us);
        if (id) *id = timerIdSeed_;
        return TASK_OP_SUCCESS;
    }

    int Count() { return timerObj_.size(); }

    int Kill(int id)
    {
        for (auto it = timerObj_.begin(); it != timerObj_.end(); it++)
        {
            if (it->id == id)
            {
                timerObj_.erase(it);
                return TASK_OP_SUCCESS;
            }
        }
        return INVALID_TASK_ID;
    }

    int DelayUs(uint64_t delay_us)
    {
        bool timerTriggered = false;
        // HAL_UartPrint("[TIM]: from %llu - %lluUs\n", portTimerGetUs(), delay_us);
        int rc = Register(timerDelayCallback_, &timerTriggered, delay_us, false, 0);
        if (rc != TASK_OP_SUCCESS) return rc;
        while (timerTriggered == false) TaskYield();
        return TASK_OP_SUCCESS;
    }
    
    inline void try_insert(OsTimerCallback_t callback, void *param, uint64_t period_us, uint64_t end_us, bool repeatable)
    {
        
    }
    void EventCallback()
    {
        timerTrigged_ += 1;
        portTimerStop();
    }

   protected:
    static void timerDelayCallback_(int id, void *param)
    {
        bool *triggerred = (bool *)param;
        // HAL_UartPrint("[TIM]: to %llu\n", portTimerGetUs());
        *triggerred = true;
    }
    volatile int timerTrigged_;
    int timerHandled_;
    int timerIdSeed_;
    etl::multiset<PtTimer_t, osMaxTimers, tick_cmp> timerObj_;
};

static PtTimer kTimer_;

static void OsTimerIsrHandler_()
{
    kTimer_.EventCallback();
}

static TASK_DECLARE(OsTimerTask_(OsTaskId taskId, void *))
{
    TASK_BEGIN(taskId);
    while (1)
    {
        TASK_WAIT_UNTIL(taskId, kTimer_.NewTimerEventHappened());

        kTimer_.Handle();
    }
    TASK_END(taskId);
}

void OsTimerInit()
{
    kTimer_.Init();
    portTimerInit(OsTimerIsrHandler_);
    portTimerStop();
    RegisterTask("thrTmr", OsTimerTask_, nullptr);
}

int OsTimerRegister(OsTimerCallback_t callback, void *param, uint64_t period_us, bool repeatable, int *id)
{
    return kTimer_.Register(callback, param, period_us, repeatable, id);
}

int OsTimerCount() { return kTimer_.Count(); }

int OsTimerKill(int id)
{
    return kTimer_.Kill(id);
}

int OsTimerDelayUs(uint64_t delay_us)
{
    return kTimer_.DelayUs(delay_us);
}
