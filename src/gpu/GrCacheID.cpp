/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTypes.h"
#include "SkThread.h"       // for sk_atomic_inc

static GrCacheID::Key kAssertKey;
GR_STATIC_ASSERT(sizeof(kAssertKey.fData8)  == sizeof(kAssertKey.fData32));
GR_STATIC_ASSERT(sizeof(kAssertKey.fData8) == sizeof(kAssertKey.fData64));
GR_STATIC_ASSERT(sizeof(kAssertKey.fData8) == sizeof(kAssertKey));

GrCacheID::Domain GrCacheID::GenerateDomain() {
    static int32_t gNextDomain = kInvalid_Domain + 1;

    int32_t domain = sk_atomic_inc(&gNextDomain);
    if (domain >= 1 << (8 * sizeof(Domain))) {
        GrCrash("Too many Cache Domains");
    }

    return static_cast<Domain>(domain);
}
