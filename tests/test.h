#ifndef _TEST_H_
#define _TEST_H_

#define testCount 100

void DumpData(const char *name, tzhu::ringbuf<int, testCount> &kData);

#endif  // _TEST_H_