
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkThread_DEFINED
#define SkThread_DEFINED

#include "SkTypes.h"
#include "SkThread_platform.h"

/****** SkThread_platform needs to define the following...

int32_t sk_atomic_inc(int32_t*);
int32_t sk_atomic_dec(int32_t*);

class SkMutex {
public:
    SkMutex();
    ~SkMutex();

    void    acquire();
    void    release();
};

****************/

class SkAutoMutexAcquire : SkNoncopyable {
public:
    explicit SkAutoMutexAcquire(SkMutex& mutex) : fMutex(&mutex)
    {
        SkASSERT(fMutex != NULL);
        mutex.acquire();
    }
    /** If the mutex has not been release, release it now.
    */
    ~SkAutoMutexAcquire()
    {
        if (fMutex)
            fMutex->release();
    }
    /** If the mutex has not been release, release it now.
    */
    void release()
    {
        if (fMutex)
        {
            fMutex->release();
            fMutex = NULL;
        }
    }
        
private:
    SkMutex* fMutex;
};

#endif
