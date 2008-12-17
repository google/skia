#include "SkThread.h"

#include <pthread.h>
#include <errno.h>

SkMutex gAtomicMutex;

int32_t sk_atomic_inc(int32_t* addr)
{
    SkAutoMutexAcquire ac(gAtomicMutex);

    int32_t value = *addr;
    *addr = value + 1;
    return value;
}

int32_t sk_atomic_dec(int32_t* addr)
{
    SkAutoMutexAcquire ac(gAtomicMutex);
    
    int32_t value = *addr;
    *addr = value - 1;
    return value;
}

//////////////////////////////////////////////////////////////////////////////

static void print_pthread_error(int status)
{
    switch (status) {
    case 0: // success
        break;
    case EINVAL:
        printf("pthread error [%d] EINVAL\n", status);
        break;
    case EBUSY:
        printf("pthread error [%d] EBUSY\n", status);
        break;
    default:
        printf("pthread error [%d] unknown\n", status);
        break;
    }
}

SkMutex::SkMutex(bool isGlobal) : fIsGlobal(isGlobal)
{
    if (sizeof(pthread_mutex_t) > sizeof(fStorage))
    {
        SkDEBUGF(("pthread mutex size = %d\n", sizeof(pthread_mutex_t)));
        SkASSERT(!"mutex storage is too small");
    }

    int status;
    pthread_mutexattr_t attr;

    status = pthread_mutexattr_init(&attr);
    print_pthread_error(status);
    SkASSERT(0 == status);
    
    status = pthread_mutex_init((pthread_mutex_t*)fStorage, &attr);
    print_pthread_error(status);
    SkASSERT(0 == status);
}

SkMutex::~SkMutex()
{
    int status = pthread_mutex_destroy((pthread_mutex_t*)fStorage);
    
    // only report errors on non-global mutexes
    if (!fIsGlobal)
    {
        print_pthread_error(status);
        SkASSERT(0 == status);
    }
}

void SkMutex::acquire()
{
    int status = pthread_mutex_lock((pthread_mutex_t*)fStorage);
    print_pthread_error(status);
    SkASSERT(0 == status);
}

void SkMutex::release()
{
    int status = pthread_mutex_unlock((pthread_mutex_t*)fStorage);
    print_pthread_error(status);
    SkASSERT(0 == status);
}

