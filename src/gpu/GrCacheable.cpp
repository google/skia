/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrCacheable.h"

uint32_t GrCacheable::getGenerationID() const {
    static int32_t gPathRefGenerationID;
    while (!fGenID) {
        fGenID = static_cast<uint32_t>(sk_atomic_inc(&gPathRefGenerationID) + 1);
    }
    return fGenID;
}
