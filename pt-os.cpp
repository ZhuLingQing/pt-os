#include "pt-os.h"
#include <string.h>

static inline size_t min_(size_t a, size_t b) { return a < b ? a : b; }

#define OsTaskId2Idx(id) (int)(id - kPt_)

class TaskControlBlock
{
   public:
    TaskControlBlock() { Reset(); }

    void Reset()
    {
#if defined(osMaxNameLen) && osMaxNameLen > 0
        memset(kName_, 0, sizeof(kName_));
#endif
        kTask_ = nullptr;
        kPtStatus_ = OsTaskNotExist;
    }

    void Register(const char *name, TaskFunction task, void *param)
    {
#if defined(osMaxNameLen) && osMaxNameLen > 0
        memcpy(kName_, name, min_(sizeof(kName_) - 1, strlen(name)));
#endif
        kTask_ = task;
        kParam_ = param;
        kEntered_ = false;
        kPtStatus_ = OsTaskWaiting;
    }

    bool Schedule(OsTaskId taskId)
    {
        OsTaskStatus newStatus;
        if (kPtStatus_ == OsTaskExit) return false;
        if (!kEntered_ && kPtStatus_ != OsTaskSuspend)
        {
            kEntered_ = true;
            newStatus = (OsTaskStatus)kTask_(taskId, kParam_);
            kEntered_ = false;
            if (kPtStatus_ < OsTaskSuspend)
            {
                kPtStatus_ = newStatus;
                return (kPtStatus_ == OsTaskExit) ? true : false;
            }
            return false;
        }
        return (kPtStatus_ == OsTaskExit) ? true : false;
    }

    OsTaskStatus Status() { return kPtStatus_; }
    int Suspend()
    {
        if (kPtStatus_ == OsTaskNotExist) return INVALID_TASK_ID;
        if (kPtStatus_ == OsTaskExit) return INVALID_TASK_STATUS;
        kPtStatus_ = OsTaskSuspend;
        return TASK_OP_SUCCESS;
    }
    int Resume()
    {
        if (kPtStatus_ == OsTaskNotExist) return INVALID_TASK_ID;
        if (kPtStatus_ == OsTaskExit) return INVALID_TASK_STATUS;
        kPtStatus_ = OsTaskWaiting;
        return TASK_OP_SUCCESS;
    }

    int Delete()
    {
        if (kPtStatus_ == OsTaskNotExist) return INVALID_TASK_ID;
        if (kPtStatus_ == OsTaskExit) return INVALID_TASK_STATUS;
        kPtStatus_ = OsTaskExit;
        return TASK_OP_SUCCESS;
    }

    const char *Name()
    {
#if defined(osMaxNameLen) && osMaxNameLen > 0
        return kName_;
#else
        return "N/A";
#endif
    }

   protected:
    TaskFunction kTask_;
#if defined(osMaxNameLen) && osMaxNameLen > 0
    char kName_[osMaxNameLen];
#endif
    void *kParam_;
    bool kEntered_;
    OsTaskStatus kPtStatus_;
};

class PtOs
{
   public:
    PtOs() { Reset(); }

    OsTaskId RegisterTask(const char *name, TaskFunction task, void *param)
    {
        if (kIdx_ >= osMaxThreads) return OsInvlidTaskId;
        kTaskCb_[kIdx_].Register(name, task, param);
        kIdx_++;
        kNumLiveTasks_++;
        PT_INIT(&kPt_[kIdx_ - 1]);
        return &kPt_[kIdx_ - 1];
    }

    void Schedule()
    {
        for (int i = 0; i < kIdx_; i++)
        {
            kCurrentTaskId_ = kPt_ + i;
            if (true == kTaskCb_[i].Schedule(kPt_ + i)) kNumLiveTasks_--;
        }
    }

    const char *Name(OsTaskId taskId)
    {
        if (taskId == OsSelfId) taskId = kCurrentTaskId_;
        OS_ASSERT((taskId >= &kPt_[0] && taskId < &kPt_[osMaxThreads]));
        return kTaskCb_[OsTaskId2Idx(taskId)].Name();
    }

    int Delete(OsTaskId taskId)
    {
        if (taskId == OsSelfId) taskId = kCurrentTaskId_;
        OS_ASSERT((taskId >= &kPt_[0] && taskId < &kPt_[osMaxThreads]));
        auto rc = kTaskCb_[OsTaskId2Idx(taskId)].Delete();
        if (rc == TASK_OP_SUCCESS) kNumLiveTasks_--;
        return rc;
    }

    int Suspend(OsTaskId taskId)
    {
        if (taskId == OsSelfId) taskId = kCurrentTaskId_;
        OS_ASSERT((taskId >= &kPt_[0] && taskId < &kPt_[osMaxThreads]));
        return kTaskCb_[OsTaskId2Idx(taskId)].Suspend();
    }

    int Resume(OsTaskId taskId)
    {
        if (taskId == OsSelfId) taskId = kCurrentTaskId_;
        OS_ASSERT((taskId >= &kPt_[0] && taskId < &kPt_[osMaxThreads]));
        return kTaskCb_[OsTaskId2Idx(taskId)].Resume();
    }

    OsTaskStatus Status(OsTaskId taskId)
    {
        if (taskId == OsSelfId) taskId = kCurrentTaskId_;
        OS_ASSERT((taskId >= &kPt_[0] && taskId < &kPt_[osMaxThreads]));
        return kTaskCb_[OsTaskId2Idx(taskId)].Status();
    }

    int NumOfLivingTasks() { return kNumLiveTasks_; }

    void Reset()
    {
        for (int i = 0; i < osMaxThreads; i++) kTaskCb_[i].Reset();
        kIdx_ = 0;
        kNumLiveTasks_ = 0;
    }

    OsTaskId CurrentTaskId() { return kCurrentTaskId_; }

   protected:
    TaskControlBlock kTaskCb_[osMaxThreads];
    struct pt kPt_[osMaxThreads];
    int kIdx_;
    int kNumLiveTasks_;
    OsTaskId kCurrentTaskId_;
};

static PtOs sOsControlBlock_;

OsTaskId RegisterTask(const char *name, TaskFunction task, void *param)
{
    return sOsControlBlock_.RegisterTask(name, task, param);
}

void TaskYield(void) { sOsControlBlock_.Schedule(); }

const char *TaskName(OsTaskId taskId) { return sOsControlBlock_.Name(taskId); }

int TaskDelete(OsTaskId taskId)
{
    if (sOsControlBlock_.CurrentTaskId() == taskId) return INVALID_TASK_ID;
    return sOsControlBlock_.Delete(taskId);
}

int TaskSuspend(OsTaskId taskId)
{
    int rc = sOsControlBlock_.Suspend(taskId);
    if (rc != TASK_OP_SUCCESS) return rc;
    if (sOsControlBlock_.CurrentTaskId() == taskId)
    {
        while (TaskStatus(taskId) == OsTaskSuspend) sOsControlBlock_.Schedule();
    }
    return TASK_OP_SUCCESS;
}

int TaskResume(OsTaskId taskId) { return sOsControlBlock_.Resume(taskId); }

OsTaskStatus TaskStatus(OsTaskId taskId) { return sOsControlBlock_.Status(taskId); }

int OsInit(void)
{
    // make sure all tasks are released
    OS_ASSERT(0 == sOsControlBlock_.NumOfLivingTasks());
    sOsControlBlock_.Reset();
    return TASK_OP_SUCCESS;
}

static TaskFunction idleTask = nullptr;

void OsStart(void)
{
    if (idleTask) RegisterTask("Idle", idleTask, nullptr);
    while (sOsControlBlock_.NumOfLivingTasks() > 0) sOsControlBlock_.Schedule();
}

void OsAddIdleTask(TaskFunction task) { idleTask = task; }