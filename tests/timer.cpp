#include <os.h>
#include <os-timer-port.h>

constexpr int kMaxTestNum_ = 10;
volatile int testCount = 0;
uint64_t tickStart;
static long testTimerParam[kMaxTestNum_];
constexpr long testTimerGolden[kMaxTestNum_] = {11, 33, 50, 51, 33, 99, 33, 33, 33, 33};
static uint64_t testTickCounter[kMaxTestNum_];
constexpr uint64_t testTickGolden[kMaxTestNum_] = {100, 300, 500, 500, 600, 900, 900, 1200, 1500, 1800};

void testTimerCallback_(int id, void *param)
{
    testTickCounter[testCount] = portTimerGetTick() / 1000 - tickStart;
    testTimerParam[testCount] = (long)param;
    OS_TRACE("Timer%d: %lld happened at %llu\n", id, testTimerParam[testCount], testTickCounter[testCount]);
    testCount++;
}

constexpr long kTimerTaskSeconds_ = 3;
long tCount1Ms = 0;
long tCount100Us = 0;
long tCountIdle = 0;

static void timerDelayCallback_(int id, void *param)
{
    bool *triggerred = (bool *)param;
    // OS_TRACE("[TIM]: to %llu\n", portTimerGetTick());
    *triggerred = true;
}

PT_THREAD(testTimerThread1Ms_(OsTaskId taskId, void *param))
{
    static bool timerTriggered;
    TASK_BEGIN(taskId);
    while (1)
    {
        OsTimerDelayUs(1000);
        TASK_YIELD(taskId);  // force Yield
        tCount1Ms++;
        // OS_TRACE("1Ms hit %ld, idleYield %ld\n", tCount1Ms, tCountIdle);
    }
    TASK_END(taskId);
}

PT_THREAD(testTimerThread100Us_(OsTaskId taskId, void *param))
{
    static bool timerTriggered;
    TASK_BEGIN(taskId);
    while (1)
    {
        OsTimerDelayUs(100);
        TASK_YIELD(taskId);  // force Yield
        tCount100Us++;
        // OS_TRACE("100Us hit %ld, idleYield %ld\n", tCount100Us, tCountIdle);
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
    OsInit();
    OsTimerInit();

    tickStart = portTimerGetTick() / 1000;
    LT_ASSERT(kLtSc == OsTimerRegister(testTimerCallback_, (void *)(11), 100, false, nullptr));
    LT_ASSERT(kLtSc == OsTimerRegister(testTimerCallback_, (void *)(50), 500, false, nullptr));
    LT_ASSERT(kLtSc == OsTimerRegister(testTimerCallback_, (void *)(99), 900, false, nullptr));
    LT_ASSERT(kLtSc == OsTimerRegister(testTimerCallback_, (void *)(33), 300, true, &repeat_timer_id));
    LT_ASSERT(kLtSc == OsTimerRegister(testTimerCallback_, (void *)(51), 500, false, nullptr));

    while (testCount < kMaxTestNum_) TaskYield();
    for (int i = 0; i < kMaxTestNum_; i++)
    {
        if (testTimerParam[i] != testTimerGolden[i] || testTickCounter[i] / 10 != testTickGolden[i] / 10)
        {
            OS_TRACE("Test%d Failed\n", i);
            OS_TRACE("Timer [%lld : %lld]. Tick [%llu : %llu]\n", testTimerParam[i], testTimerGolden[i],
                          testTickCounter[i], testTickGolden[i]);
            return 1;
        }
    }
    if (OsTimerCount() != 1)
    {
        OS_TRACE("Timer count error 1 - %d\n", OsTimerCount());
        return 2;
    }
    LT_ASSERT(OsTimerKill(repeat_timer_id) == kLtSc);
    if (OsTimerCount() != 0)
    {
        OS_TRACE("Timer count error 0 - %d\n", OsTimerCount());
        return 3;
    }

    OS_TRACE("Test Timer in task\n");
    RegisterTask("thr1Ms", &testTimerThread1Ms_, nullptr);
    RegisterTask("thr100Us", &testTimerThread100Us_, nullptr);
    RegisterTask("thrIdle", &testIdleThread_, nullptr);
    OS_TRACE("Test Timer start\n");
    while (tCount1Ms < kTimerTaskSeconds_)
    {
        TaskYield();
    }
    if ((tCount1Ms * 10 - tCount100Us) <= 2)
    {
        OS_TRACE("Timer task error %ld - %ld\n", tCount1Ms, tCount100Us);
        return 4;
    }
    OS_TRACE("Idle task yield %ld\n", tCountIdle);

    OS_TRACE("Test Passed\n");
    return 0;
}
