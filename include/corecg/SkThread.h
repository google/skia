/* include/corecg/SkThread.h
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
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

class SkAutoMutexAcquire {
public:
    explicit SkAutoMutexAcquire(SkMutex& mutex) : fMutex(mutex)
    {
        mutex.acquire();
    }
    ~SkAutoMutexAcquire()
    {
        fMutex.release();
    }
private:
    SkMutex&    fMutex;

    // illegal
    SkAutoMutexAcquire& operator=(SkAutoMutexAcquire&);
};

#endif
