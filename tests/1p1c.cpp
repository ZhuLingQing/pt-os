#include "os_test.h"

static constexpr int kThreadNum = 1;

static tzhu::ringbuf<int, 8> rb_;
static volatile bool testFailed;

static OsTaskId kProdId_[kThreadNum];
static OsTaskId kConsId_[kThreadNum];

static tzhu::ringbuf<int, testCount> kProdData_[kThreadNum];
static tzhu::ringbuf<int, testCount> kConsData_[kThreadNum];

static inline void TestInit()
{
    testFailed = false;
}

static inline int TestCheck()
{
    char name[16];
    for(int i = 0; i < kThreadNum; i++)
    {
        sprintf(name, "RPOD%d", i);
        DumpData(name, kProdData_[i]);
    }
    for(int i = 0; i < kThreadNum; i++)
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
    OS_TRACE("%s Begin\n", TaskName(taskId));
    while (1)
    {
        TASK_WAIT_UNTIL(taskId, !rb_.full());
        rb_.push(i);
        kProdData_[(long)param].push(i);
        i++;
        if (i > testCount) TASK_EXIT(taskId);
    }
    TASK_END(taskId);
}

static TASK_DECLARE(consTask(OsTaskId taskId, void *param))
{
    static int i = 1;
    int v;
    TASK_BEGIN(taskId);
    OS_TRACE("%s Begin\n", TaskName(taskId));
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
        i++;
        if (i > testCount) TASK_EXIT(taskId);
    }
    TASK_END(taskId);
}

int Test1p1c()
{
    OS_TRACE("========  %s  ========\n", __FUNCTION__);
    TestInit();

    OsInit();
    kProdId_[0] = RegisterTask("prodTask", prodTask, (void *)0);
    kConsId_[0] = RegisterTask("consTask", consTask, (void *)0);
    OsStart();
    
    return TestCheck();
}