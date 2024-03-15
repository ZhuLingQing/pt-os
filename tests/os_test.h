#ifndef _OS_TEST_H_
#define _OS_TEST_H_

#include <stdio.h>
#include <string.h>
#include <tz_ringbuf.hpp>
#include "pt-os.h"

#define testCount 100

#define OS_TRACE(...) printf(__VA_ARGS__)

void DumpData(const char *name, tzhu::ringbuf<int, testCount> &kData);

#endif  // _OS_TEST_H_
