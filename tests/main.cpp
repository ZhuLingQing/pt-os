#include "os_test.h"

void DumpData(const char *name, tzhu::ringbuf<int, testCount> &kData)
{
    int v;
    OS_TRACE("%s: %ld\n",name,kData.size());
    for (int j = 0; kData.size() > 1; j++)
    {
        kData.pop(v);
        OS_TRACE("%d, ", v);
    }
    if( true == kData.pop(v))
        OS_TRACE("%d\n", v);
}

int Test1p1c();
int Test2p1c();
int Test1p2c();
int TestResume();
int TestSubfunction();
int TestTimer();

int main()
{
    int rc = 0;
    rc += Test1p1c();
    rc += Test1p2c();
    rc += Test2p1c();
    rc += TestResume();
    rc += TestSubfunction();
    rc += TestTimer();
    
    return rc;
}