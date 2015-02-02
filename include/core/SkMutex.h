#ifndef SkMutex_DEFINED
#define SkMutex_DEFINED

// This file is not part of the public Skia API.
#include "SkTypes.h"

#if defined(SK_BUILD_FOR_WIN)
    #include "../ports/SkMutex_win.h"
#else
    #include "../ports/SkMutex_pthread.h"
#endif

class SkAutoMutexAcquire : SkNoncopyable {
public:
    explicit SkAutoMutexAcquire(SkBaseMutex& mutex) : fMutex(&mutex) {
        SkASSERT(fMutex != NULL);
        mutex.acquire();
    }

    explicit SkAutoMutexAcquire(SkBaseMutex* mutex) : fMutex(mutex) {
        if (mutex) {
            mutex->acquire();
        }
    }

    /** If the mutex has not been released, release it now. */
    ~SkAutoMutexAcquire() {
        if (fMutex) {
            fMutex->release();
        }
    }

    /** If the mutex has not been released, release it now. */
    void release() {
        if (fMutex) {
            fMutex->release();
            fMutex = NULL;
        }
    }

    /** Assert that we're holding the mutex. */
    void assertHeld() {
        SkASSERT(fMutex);
        fMutex->assertHeld();
    }

private:
    SkBaseMutex* fMutex;
};
#define SkAutoMutexAcquire(...) SK_REQUIRE_LOCAL_VAR(SkAutoMutexAcquire)


#endif//SkMutex_DEFINED
