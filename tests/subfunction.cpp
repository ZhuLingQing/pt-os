#include "os_test.h"

static constexpr int kThreadNum = 2;

static tzhu::ringbuf<int, 8> rb_;
static volatile bool testFailed;

static OsTaskId kProdId_[kThreadNum];
static OsTaskId kConsId_[kThreadNum];

static tzhu::ringbuf<int, testCount> kProdData_[kThreadNum];
static tzhu::ringbuf<int, testCount> kConsData_[kThreadNum];

static inline void TestInit() { testFailed = false; }
static inline int TestCheck()
{
    char name[16];
    for (int i = 0; i < kThreadNum; i++)
    {
        sprintf(name, "RPOD%d", i);
        DumpData(name, kProdData_[i]);
    }
    for (int i = 0; i < kThreadNum; i++)
    {
        sprintf(name, "CONS%d", i);
        DumpData(name, kConsData_[i]);
    }
    if (testFailed) OS_TRACE("Test failed.\n");
    return testFailed ? 1 : 0;
}

void prodSubFunction(long id)
{
    static int i = 1;
    TaskWaitUntil(!rb_.full());
    rb_.push(i);
    kProdData_[id].push(i);
    OS_TRACE("%s %d\n", TaskName(OsSelfId), i);
    i++;
}

static TASK_DECLARE(prodTask(OsTaskId taskId, void *param))
{
    TASK_BEGIN(taskId);
    OS_TRACE("%s Begin\n", TaskName(OsSelfId));
    while (1)
    {
        prodSubFunction((long)param);
        TASK_YIELD(taskId);
    }
    TASK_END(taskId);
}

int consSubFunction(long id)
{
    int v;
    TaskWaitUntil(!rb_.empty());
    rb_.pop(v);
    kConsData_[id].push(v);
    return v;
}

static TASK_DECLARE(consTask(OsTaskId taskId, void *param))
{
    static int i = 1;
    int v;
    TASK_BEGIN(taskId);
    OS_TRACE("%s Begin\n", TaskName(OsSelfId));
    while (1)
    {
        v = consSubFunction((long)param);
        if (v != i)
        {
            testFailed = true;
            TASK_EXIT(taskId);
        }
        if (i == testCount / 4 * 1)
        {
            OS_TRACE("%s suspend %s @%d\n", TaskName(taskId), TaskName(kProdId_[0]), i);
            TaskSuspend(kProdId_[0]);
        }
        else if (i == testCount / 4 * 2)
        {
            OS_TRACE("%s resume %s @%d\n", TaskName(taskId), TaskName(kProdId_[0]), i);
            TaskResume(kProdId_[0]);
        }
        else if (i == testCount / 4 * 3)
        {
            OS_TRACE("%s suspend %s @%d\n", TaskName(taskId), TaskName(kProdId_[1]), i);
            TaskSuspend(kProdId_[1]);
        }
        i++;
        if (i > testCount)
        {
            OS_TRACE("%s Kill %s, %s\n", TaskName(taskId), TaskName(kProdId_[0]), TaskName(kProdId_[1]));
            TaskDelete(kProdId_[0]);
            TaskDelete(kProdId_[1]);
            TASK_EXIT(taskId);
        };
    }
    TASK_END(taskId);
}

int TestSubfunction()
{
    OS_TRACE("========  %s  ========\n", __FUNCTION__);
    TestInit();

    OsInit();
    kProdId_[0] = RegisterTask("prodTask1", prodTask, (void *)0);
    kProdId_[1] = RegisterTask("prodTask2", prodTask, (void *)1);
    kConsId_[0] = RegisterTask("consTask", consTask, (void *)0);
    OsStart();

    return TestCheck();
}