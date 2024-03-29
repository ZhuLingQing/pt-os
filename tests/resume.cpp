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

static TASK_DECLARE(prodTask(OsTaskId taskId, void *param))
{
    static int i = 1;
    TASK_BEGIN(taskId);
    OS_TRACE("%s Begin\n", TaskName(OsSelfId));
    while (1)
    {
        TASK_WAIT_UNTIL(taskId, !rb_.full());
        rb_.push(i);
        kProdData_[(long)param].push(i);
        i++;
    }
    TASK_END(taskId);
}

static TASK_DECLARE(consTask(OsTaskId taskId, void *param))
{
    static int i = 1;
    int v;
    TASK_BEGIN(taskId);
    OS_TRACE("%s Begin\n", TaskName(OsSelfId));
    while (1)
    {
        TASK_WAIT_UNTIL(taskId, !rb_.empty());
        rb_.pop(v);
        kConsData_[(long)param].push(v);
        if (v != i)
        {
            testFailed = true;
            TASK_EXIT(taskId);
        }
        if (i == testCount / 4 * 1)
        {
            OS_TRACE("%s suspend %s\n", TaskName(taskId), TaskName(kProdId_[0]));
            TaskSuspend(kProdId_[0]);
        }
        else if (i == testCount / 4 * 2)
        {
            OS_TRACE("%s resume %s\n", TaskName(taskId), TaskName(kProdId_[0]));
            TaskResume(kProdId_[0]);
        }
        else if (i == testCount / 4 * 3)
        {
            OS_TRACE("%s suspend %s\n", TaskName(taskId), TaskName(kProdId_[1]));
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

int TestResume()
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