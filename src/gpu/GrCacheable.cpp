/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrCacheable.h"

uint64_t GrCacheable::CreateInstanceID() {
    static int64_t gCacheableID;
    uint64_t id;
    do {
        id = static_cast<uint64_t>(sk_atomic_inc(&gCacheableID) + 1);
    } while (!id);
    return id;
}
