
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkThread.h"
#include "SkTLS.h"

int32_t sk_atomic_inc(int32_t* addr) {
    int32_t value = *addr;
    *addr = value + 1;
    return value;
}

int32_t sk_atomic_dec(int32_t* addr) {
    int32_t value = *addr;
    *addr = value - 1;
    return value;
}
void sk_membar_aquire__after_atomic_dec() { }

int32_t sk_atomic_conditional_inc(int32_t* addr) {
    int32_t value = *addr;
    if (value != 0) ++*addr;
    return value;
}
void sk_membar_aquire__after_atomic_conditional_inc() { }

SkMutex::SkMutex() {}

SkMutex::~SkMutex() {}

#ifndef SK_USE_POSIX_THREADS
void SkMutex::acquire() {}
void SkMutex::release() {}
#endif

//////////////////////////////////////////////////////////////////////////

static void* gSpecific;

void* SkTLS::PlatformGetSpecific(bool) {
    return gSpecific;
}

void SkTLS::PlatformSetSpecific(void* ptr) {
    gSpecific = ptr;
}
