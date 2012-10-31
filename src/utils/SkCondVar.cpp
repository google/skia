/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCondVar.h"

SkCondVar::SkCondVar() {
#ifdef SK_USE_POSIX_THREADS
    pthread_mutex_init(&fMutex, NULL /* default mutex attr */);
    pthread_cond_init(&fCond, NULL /* default cond attr */);
#elif defined(SK_BUILD_FOR_WIN32)
    InitializeCriticalSection(&fCriticalSection);
    InitializeConditionVariable(&fCondition);
#endif
}

SkCondVar::~SkCondVar() {
#ifdef SK_USE_POSIX_THREADS
    pthread_mutex_destroy(&fMutex);
    pthread_cond_destroy(&fCond);
#elif defined(SK_BUILD_FOR_WIN32)
    DeleteCriticalSection(&fCriticalSection);
    // No need to clean up fCondition.
#endif
}

void SkCondVar::lock() {
#ifdef SK_USE_POSIX_THREADS
    pthread_mutex_lock(&fMutex);
#elif defined(SK_BUILD_FOR_WIN32)
    EnterCriticalSection(&fCriticalSection);
#endif
}

void SkCondVar::unlock() {
#ifdef SK_USE_POSIX_THREADS
    pthread_mutex_unlock(&fMutex);
#elif defined(SK_BUILD_FOR_WIN32)
    LeaveCriticalSection(&fCriticalSection);
#endif
}

void SkCondVar::wait() {
#ifdef SK_USE_POSIX_THREADS
    pthread_cond_wait(&fCond, &fMutex);
#elif defined(SK_BUILD_FOR_WIN32)
    SleepConditionVariableCS(&fCondition, &fCriticalSection, INFINITE);
#endif
}

void SkCondVar::signal() {
#ifdef SK_USE_POSIX_THREADS
    pthread_cond_signal(&fCond);
#elif defined(SK_BUILD_FOR_WIN32)
    WakeConditionVariable(&fCondition);
#endif
}

void SkCondVar::broadcast() {
#ifdef SK_USE_POSIX_THREADS
    pthread_cond_broadcast(&fCond);
#elif defined(SK_BUILD_FOR_WIN32)
    WakeAllConditionVariable(&fCondition);
#endif
}
