#ifndef _PT_OS_H_
#define _PT_OS_H_

#include <pt.h>
#include "pt-osConfig.h"
#include "os-timer.h"

#define TASK_BEGIN(id) PT_BEGIN(id)
#define TASK_YIELD(id) PT_YIELD(id)
#define TASK_WAIT_UNTIL(id, cond) PT_WAIT_UNTIL(id, cond)
#define TASK_EXIT(id) return PT_ENDED
#define TASK_END(id) PT_END(id)
#define TASK_DECLARE(thread_declare) PT_THREAD(thread_declare)

#define TaskWaitUntil(cond)          \
    do                               \
    {                                \
        while (!(cond)) TaskYield(); \
    } while (0)

#if __cplusplus
extern "C"
{
#endif
    typedef struct pt *OsTaskId;
#define OsInvlidTaskId ((OsTaskId)INVALID_TASK_ID)
#define OsSelfId ((OsTaskId)0)

// return enumerations
#define TASK_OP_SUCCESS (0)
#define INVALID_TASK_ID (-1)
#define INVALID_TASK_STATUS (-2)

    typedef enum
    {
        OsTaskWaiting = PT_WAITING,
        OsTaskYield = PT_YIELDED,
        OsTaskSuspend = PT_EXITED,
        OsTaskExit = PT_ENDED,
        OsTaskNotExist = -1,
    } OsTaskStatus;

    typedef char (*TaskFunction)(OsTaskId, void *);

    OsTaskId RegisterTask(const char *name, TaskFunction task, void *param);

    // Cause Registered Service get scheduled
    void TaskYield(void);

    const char *TaskName(OsTaskId taskId);

    int TaskDelete(OsTaskId taskId);

    int TaskSuspend(OsTaskId taskId);

    // Resume the task from OsTaskSuspend.
    int TaskResume(OsTaskId taskId);

    // Get the OsTaskStatus.
    OsTaskStatus TaskStatus(OsTaskId taskId);

    int OsInit(void);

    // Start scheduling all the tasks, only return while all tasks are OsTaskExit or OsTaskNotExist.
    void OsStart(void);

#if __cplusplus
}
#endif

#endif
