#ifndef _PT_OS_H_
#define _PT_OS_H_

#include "pt-osConfig.h"
#include <pt.h>

#define TASK_BEGIN(id) PT_BEGIN(id)
#define TASK_YIELD(id) PT_YIELD(id)
#define TASK_WAIT_UNTIL(id,cond) PT_WAIT_UNTIL(id, cond)
#define TASK_SUSPEND(id) do { if (taskId == id) PT_EXIT(id); else TaskSuspend(id); } while(0)
#define TASK_DELETE(id) do { if (taskId == id) PT_END(id); else TaskDelete(id); } while(0)
#define TASK_EXIT(id) return PT_ENDED
#define TASK_END(id) PT_END(id)
#define TASK_DECLARE(thread_declare) PT_THREAD(thread_declare)

#if __cplusplus
extern "C"
{
#endif
    typedef struct pt * OsTaskId;
    #define OsInvlidTaskId ((OsTaskId)INVALID_TASK_ID)

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
    }OsTaskStatus;

    typedef char (*TaskFunction)(OsTaskId, void *);

    OsTaskId RegisterTask(const char *name, TaskFunction task, void *param);

    // Cause Registered Service get scheduled
    void TaskYield(void);

    const char *TaskName(OsTaskId taskId);

    // Call TASK_DELETE, it will identify this task or not.
    int TaskDelete(OsTaskId taskId);

    // Call TASK_SUSPEND, it will identify this task or not.
    int TaskSuspend(OsTaskId taskId);

    int TaskResume(OsTaskId taskId);

    OsTaskStatus TaskStatus(OsTaskId taskId);

    void OsInit(void);

    void OsStart(void);

#if __cplusplus
}
#endif

#endif
