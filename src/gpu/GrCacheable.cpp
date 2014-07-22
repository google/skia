/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrCacheable.h"

uint32_t GrCacheable::CreateUniqueID() {
    static int32_t gCacheableID = SK_InvalidUniqueID;
    uint32_t id;
    do {
        id = static_cast<uint32_t>(sk_atomic_inc(&gCacheableID) + 1);
    } while (id == SK_InvalidUniqueID);
    return id;
}
