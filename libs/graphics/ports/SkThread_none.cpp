#include "SkThread.h"

int32_t sk_atomic_inc(int32_t* addr)
{
    int32_t value = *addr;
    *addr = value + 1;
    return value;
}

int32_t sk_atomic_dec(int32_t* addr)
{
    int32_t value = *addr;
    *addr = value - 1;
    return value;
}

SkMutex::SkMutex()
{
}

SkMutex::~SkMutex()
{
}

void SkMutex::acquire()
{
}

void SkMutex::release()
{
}

