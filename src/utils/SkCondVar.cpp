/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCondVar.h"

#if defined(SK_BUILD_FOR_WIN32)
    static void (WINAPI *initialize_condition_variable)(PCONDITION_VARIABLE);
    static BOOL (WINAPI *sleep_condition_variable)(PCONDITION_VARIABLE, PCRITICAL_SECTION, DWORD);
    static void (WINAPI *wake_condition_variable)(PCONDITION_VARIABLE);
    static void (WINAPI *wake_all_condition_variable)(PCONDITION_VARIABLE);

    template <typename T>
    static void set_fn_ptr(T* ptr, FARPROC fn) { *ptr = reinterpret_cast<T>(fn); }
#endif

bool SkCondVar::Supported() {
#ifdef SK_USE_POSIX_THREADS
    return true;
#elif defined(SK_BUILD_FOR_WIN32)
    // If we're >= Vista we'll find these functions.  Otherwise (XP) SkCondVar is not supported.
    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    set_fn_ptr(&initialize_condition_variable,
               GetProcAddress(kernel32, "InitializeConditionVariable"));
    set_fn_ptr(&sleep_condition_variable,
               GetProcAddress(kernel32, "SleepConditionVariableCS"));
    set_fn_ptr(&wake_condition_variable,
               GetProcAddress(kernel32, "WakeConditionVariable"));
    set_fn_ptr(&wake_all_condition_variable,
               GetProcAddress(kernel32, "WakeAllConditionVariable"));
    return initialize_condition_variable
        && sleep_condition_variable
        && wake_condition_variable
        && wake_all_condition_variable;
#else
    return false;
#endif
}

SkCondVar::SkCondVar() {
#ifdef SK_USE_POSIX_THREADS
    pthread_mutex_init(&fMutex, NULL /* default mutex attr */);
    pthread_cond_init(&fCond, NULL /* default cond attr */);
#elif defined(SK_BUILD_FOR_WIN32)
    InitializeCriticalSection(&fCriticalSection);
    SkASSERT(initialize_condition_variable);
    initialize_condition_variable(&fCondition);
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
    SkASSERT(sleep_condition_variable);
    sleep_condition_variable(&fCondition, &fCriticalSection, INFINITE);
#endif
}

void SkCondVar::signal() {
#ifdef SK_USE_POSIX_THREADS
    pthread_cond_signal(&fCond);
#elif defined(SK_BUILD_FOR_WIN32)
    SkASSERT(wake_condition_variable);
    wake_condition_variable(&fCondition);
#endif
}

void SkCondVar::broadcast() {
#ifdef SK_USE_POSIX_THREADS
    pthread_cond_broadcast(&fCond);
#elif defined(SK_BUILD_FOR_WIN32)
    SkASSERT(wake_all_condition_variable);
    wake_all_condition_variable(&fCondition);
#endif
}
