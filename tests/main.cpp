#include "tz_ringbuf.hpp"
#include "pt-os.h"
#include <stdio.h>
#include <string.h>
#include "test.h"

void DumpData(const char *name, tzhu::ringbuf<int, testCount> &kData)
{
    int v;
    printf("%s: %ld\n",name,kData.size());
    for (int j = 0; kData.size() > 1; j++)
    {
        kData.pop(v);
        printf("%d, ", v);
    }
    if( true == kData.pop(v))
        printf("%d\n", v);
}

int Test1p1c();
int Test2p1c();
int Test1p2c();
int TestResume();

int main()
{
    int rc = 0;
    rc += Test1p1c();
    rc += Test1p2c();
    rc += Test2p1c();
    rc += TestResume();
    return rc;
}