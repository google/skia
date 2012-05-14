/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#include "SkThreadUtils.h"
#include "SkThreadUtils_pthread.h"

#include <pthread.h>
#include <signal.h>

PThreadEvent::PThreadEvent() : fConditionFlag(false) {
    pthread_cond_init(&fCondition, NULL);
    pthread_mutex_init(&fConditionMutex, NULL);
}
PThreadEvent::~PThreadEvent() {
    pthread_mutex_destroy(&fConditionMutex);
    pthread_cond_destroy(&fCondition);
}
void PThreadEvent::trigger() {
    pthread_mutex_lock(&fConditionMutex);
    fConditionFlag = true;
    pthread_cond_signal(&fCondition);
    pthread_mutex_unlock(&fConditionMutex);
}
void PThreadEvent::wait() {
    pthread_mutex_lock(&fConditionMutex);
    while (!fConditionFlag) {
        pthread_cond_wait(&fCondition, &fConditionMutex);
    }
    pthread_mutex_unlock(&fConditionMutex);
}
bool PThreadEvent::isTriggered() {
    bool currentFlag;
    pthread_mutex_lock(&fConditionMutex);
    currentFlag = fConditionFlag;
    pthread_mutex_unlock(&fConditionMutex);
    return currentFlag;
}

SkThread_PThreadData::SkThread_PThreadData(SkThread::entryPointProc entryPoint, void* data)
    : fPThread()
    , fValidPThread(false)
    , fParam(data)
    , fEntryPoint(entryPoint)
{
    pthread_attr_init(&fAttr);
    pthread_attr_setdetachstate(&fAttr, PTHREAD_CREATE_JOINABLE);
}

SkThread_PThreadData::~SkThread_PThreadData() {
    pthread_attr_destroy(&fAttr);
}

static void* thread_start(void* arg) {
    SkThread_PThreadData* pthreadData = static_cast<SkThread_PThreadData*>(arg);
    // Wait for start signal
    pthreadData->fStarted.wait();

    // Call entry point only if thread was not canceled before starting.
    if (!pthreadData->fCanceled.isTriggered()) {
        pthreadData->fEntryPoint(pthreadData->fParam);
    }
    return NULL;
}

SkThread::SkThread(entryPointProc entryPoint, void* data) {
    SkThread_PThreadData* pthreadData = new SkThread_PThreadData(entryPoint, data);
    fData = pthreadData;

    int ret = pthread_create(&(pthreadData->fPThread),
                             &(pthreadData->fAttr),
                             thread_start,
                             pthreadData);

    pthreadData->fValidPThread = (0 == ret);
}

SkThread::~SkThread() {
    if (fData != NULL) {
        SkThread_PThreadData* pthreadData = static_cast<SkThread_PThreadData*>(fData);
        // If created thread but start was never called, kill the thread.
        if (pthreadData->fValidPThread && !pthreadData->fStarted.isTriggered()) {
            pthreadData->fCanceled.trigger();
            if (this->start()) {
                this->join();
            }
        }
        delete pthreadData;
    }
}

bool SkThread::start() {
    SkThread_PThreadData* pthreadData = static_cast<SkThread_PThreadData*>(fData);
    if (!pthreadData->fValidPThread) {
        return false;
    }

    if (pthreadData->fStarted.isTriggered()) {
        return false;
    }
    pthreadData->fStarted.trigger();
    return true;
}

void SkThread::join() {
    SkThread_PThreadData* pthreadData = static_cast<SkThread_PThreadData*>(fData);
    if (!pthreadData->fValidPThread || !pthreadData->fStarted.isTriggered()) {
        return;
    }

    pthread_join(pthreadData->fPThread, NULL);
}
