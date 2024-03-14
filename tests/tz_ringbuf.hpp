#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <strings.h>

namespace tzhu
{

template <typename T, size_t szBuf>
class ringbuf
{
   public:
    ringbuf()
    {
        in_ = 0;
        out_ = 0;
        capacity_ = sizeof(buf) / sizeof(buf[0]);
    }
    bool push(T &v)
    {
        if (true == full()) return false;
        buf[in_] = v;
        if (++in_ >= capacity_) in_ = 0;
        return true;
    }
    bool pop(T &v)
    {
        if (true == empty()) return false;
        v = buf[out_];
        if (++out_ >= capacity_) out_ = 0;
        return true;
    }
    size_t size()
    {
        if (in_ >= out_)
            return in_ - out_;
        else
            return (capacity_ + in_ - out_);
    }
    bool full() { return (size() >= capacity_ - 1) ? true : false; }
    bool empty() { return (size() == 0) ? true : false; }

   protected:
    T buf[szBuf + 1];
    volatile uint32_t in_;
    volatile uint32_t out_;
    uint32_t capacity_;
};
};  // namespace tzhu
