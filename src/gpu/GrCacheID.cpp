/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTypes.h"
#include "SkThread.h"       // for sk_atomic_inc

// Well, the dummy_ "fix" caused a warning on windows, so hiding all of it
// until we can find a universal fix.
#if 0
// This used to be a global scope, but we got a warning about unused variable
// so we moved it into here. We just want it to compile, so we can test the
// static asserts.
static inline void dummy_function_to_avoid_unused_var_warning() {
    GrCacheID::Key kAssertKey;
    GR_STATIC_ASSERT(sizeof(kAssertKey.fData8) == sizeof(kAssertKey.fData32));
    GR_STATIC_ASSERT(sizeof(kAssertKey.fData8) == sizeof(kAssertKey.fData64));
    GR_STATIC_ASSERT(sizeof(kAssertKey.fData8) == sizeof(kAssertKey));
}
#endif

GrCacheID::Domain GrCacheID::GenerateDomain() {
    static int32_t gNextDomain = kInvalid_Domain + 1;

    int32_t domain = sk_atomic_inc(&gNextDomain);
    if (domain >= 1 << (8 * sizeof(Domain))) {
        GrCrash("Too many Cache Domains");
    }

    return static_cast<Domain>(domain);
}
