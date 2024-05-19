#ifndef _OS_CONFIG_H_
#define _OS_CONFIG_H_

// Length of TaskName.
// may undef or set to zero for less ram resource.
#define osMaxNameLen (16)

// Number of maximum threads support
#define osMaxThreads (64)

// Number of timer.
#define osMaxTimers (64)

#include <cassert>
#define OS_ASSERT assert

#define OS_PRINT printf

#endif
