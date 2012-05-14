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

class SkThread_PThreadData {
public:
    SkThread_PThreadData(SkThread::entryPointProc entryPoint, void* data);
    ~SkThread_PThreadData();
    pthread_t fPThread;
    bool fValidPThread;
    pthread_mutex_t fStartMutex;
    pthread_cond_t fStartCondition;
    pthread_attr_t fAttr;

    void* fParam;
    SkThread::entryPointProc fEntryPoint;
    bool fStarted;
};

#endif
