/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkOnce.h"
#include "src/core/SkTLS.h"

#include <pthread.h>

static pthread_key_t gSkTLSKey;

void* SkTLS::PlatformGetSpecific(bool forceCreateTheSlot) {
    // should we use forceCreateTheSlot to potentially just return nullptr if
    // we've never been called with forceCreateTheSlot==true ?
    static SkOnce once;
    once(pthread_key_create, &gSkTLSKey, SkTLS::Destructor);
    return pthread_getspecific(gSkTLSKey);
}

void SkTLS::PlatformSetSpecific(void* ptr) {
    (void)pthread_setspecific(gSkTLSKey, ptr);
}
