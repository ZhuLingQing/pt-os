#include "os_test.h"
#include <os-timer-port.h>

constexpr long testTimerGain_ = 1000;
constexpr int kMaxTestNum_ = 10;
volatile int kSignalTriggered_ = 0;
uint64_t kStartUs_;
static long testTimerParam[kMaxTestNum_];
constexpr long testTimerGolden[kMaxTestNum_] = {11, 33, 50, 51, 33, 33, 99, 33, 33, 33};
static uint64_t testUsCounter[kMaxTestNum_];
constexpr uint64_t testUsGolden[kMaxTestNum_] = {100, 300, 500, 500, 600, 900, 1000, 1200, 1500, 1800};

void testTimerCallback_(int id, void *param)
{
    testUsCounter[kSignalTriggered_] = portTimerGetUs();
    testTimerParam[kSignalTriggered_] = (long)param;
    OS_TRACE("Timer%d: %lld happened at %llu\n", id, testTimerParam[kSignalTriggered_], testUsCounter[kSignalTriggered_]/1000);
    kSignalTriggered_++;
}

constexpr long kTimerTaskSeconds_ = 3;
long tCount10t = 0;
long tCount1t = 0;
long tCountIdle = 0;

static void timerDelayCallback_(int id, void *param)
{
    bool *triggerred = (bool *)param;
    // OS_TRACE("[TIM]: to %llu\n", portTimerGetUs());
    *triggerred = true;
}

PT_THREAD(testTimerThread10t_(OsTaskId taskId, void *param))
{
    static bool timerTriggered;
    TASK_BEGIN(taskId);
    while (1)
    {
        OsTimerDelayUs(100 * testTimerGain_);
        TASK_YIELD(taskId);  // force Yield
        tCount10t++;
        // OS_TRACE("10t hit %ld, idleYield %ld\n", tCount10t, tCountIdle);
    }
    TASK_END(taskId);
}

PT_THREAD(testTimerThread1t_(OsTaskId taskId, void *param))
{
    static bool timerTriggered;
    TASK_BEGIN(taskId);
    while (1)
    {
        OsTimerDelayUs(10 * testTimerGain_);
        TASK_YIELD(taskId);  // force Yield
        tCount1t++;
        // OS_TRACE("1t hit %ld, idleYield %ld\n", tCount1t, tCountIdle);
    }
    TASK_END(taskId);
}

PT_THREAD(testIdleThread_(OsTaskId taskId, void *param))
{
    TASK_BEGIN(taskId);
    while (1)
    {
        tCountIdle++;
        TASK_YIELD(taskId);  // force Yield
    }
    TASK_END(taskId);
}

int TestTimer()
{
    int repeat_timer_id;
    OS_TRACE("========  %s  ========\n", __FUNCTION__);
    OsInit();
    OsTimerInit();
    kSignalTriggered_ = 0;
    tCount10t = 0;
    tCount1t = 0;
    tCountIdle = 0;
    kStartUs_ = portTimerGetUs();
    OS_TRACE("Timer Init\n", __FUNCTION__);

    OsTimerRegister(testTimerCallback_, (void *)(11), 100 * testTimerGain_, false, nullptr);
    OsTimerRegister(testTimerCallback_, (void *)(50), 500 * testTimerGain_, false, nullptr);
    OsTimerRegister(testTimerCallback_, (void *)(99), 1000 * testTimerGain_, false, nullptr);
    OsTimerRegister(testTimerCallback_, (void *)(33), 300 * testTimerGain_, true, &repeat_timer_id);
    OsTimerRegister(testTimerCallback_, (void *)(51), 500 * testTimerGain_, false, nullptr);

    OS_TRACE("Timer Yield\n", __FUNCTION__);
    while (kSignalTriggered_ < kMaxTestNum_) TaskYield();
    for (int i = 0; i < kMaxTestNum_; i++)
    {
        if (testTimerParam[i] != testTimerGolden[i] || testUsCounter[i] / 200 != testUsGolden[i] * testTimerGain_ / 200)
        {
            OS_TRACE("Test%d Failed\n", i);
            OS_TRACE("Timer [%lld : %lld]. Tick [%llu : %llu]\n", testTimerParam[i], testTimerGolden[i],
                          testUsCounter[i], testUsGolden[i] * testTimerGain_);
            return 1;
        }
    }
    if (OsTimerCount() != 1)
    {
        OS_TRACE("Timer count error 1 - %d\n", OsTimerCount());
        return 2;
    }
    OsTimerKill(repeat_timer_id);
    if (OsTimerCount() != 0)
    {
        OS_TRACE("Timer count error 0 - %d\n", OsTimerCount());
        return 3;
    }

    OS_TRACE("Test Timer in task\n");
    RegisterTask("thr10t", &testTimerThread10t_, nullptr);
    RegisterTask("thr1t", &testTimerThread1t_, nullptr);
    RegisterTask("thrIdle", &testIdleThread_, nullptr);
    OS_TRACE("Test Timer start\n");
    while (tCount10t < kTimerTaskSeconds_)
    {
        TaskYield();
    }
    if ((tCount10t * 10 - tCount1t) <= 2)
    {
        OS_TRACE("Timer task error %ld - %ld\n", tCount10t, tCount1t);
        return 4;
    }
    OS_TRACE("Idle task yield %ld\n", tCountIdle);

    OS_TRACE("Test Passed\n");
    return 0;
}
