/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkThreadUtils_PThreadData_DEFINED
#define SkThreadUtils_PThreadData_DEFINED

#include "SkThreadUtils.h"
#include <pthread.h>

class PThreadEvent : SkNoncopyable {
public:
    PThreadEvent();
    ~PThreadEvent();
    void trigger();
    void wait();
    bool isTriggered();

private:
    pthread_cond_t fCondition;
    pthread_mutex_t fConditionMutex;
    bool fConditionFlag;
};

class SkThread_PThreadData : SkNoncopyable {
public:
    SkThread_PThreadData(SkThread::entryPointProc entryPoint, void* data);
    ~SkThread_PThreadData();
    pthread_t fPThread;
    bool fValidPThread;
    PThreadEvent fStarted;
    PThreadEvent fCanceled;

    pthread_attr_t fAttr;

    void* fParam;
    SkThread::entryPointProc fEntryPoint;
};

#endif
