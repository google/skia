/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCacheID.h"
#include "SkThread.h"       // for sk_atomic_inc

uint8_t GrCacheID::GetNextDomain() {
    // 0 reserved for kUnrestricted_ResourceDomain
    static int32_t gNextDomain = 1;

    int32_t domain = sk_atomic_inc(&gNextDomain);
    if (domain >= 256) {
        GrCrash("Too many Cache Domains");
    }

    return (uint8_t) domain;
}

uint8_t GrCacheID::GetNextResourceType() {
    // 0 reserved for kInvalid_ResourceType
    static int32_t gNextResourceType = 1;

    int32_t type = sk_atomic_inc(&gNextResourceType);
    if (type >= 256) {
        GrCrash("Too many Cache Resource Types");
    }

    return (uint8_t) type;
}

void GrCacheID::toRaw(uint32_t v[4]) {
    GrAssert(4*sizeof(uint32_t) == sizeof(GrCacheID));

    v[0] = (uint32_t) (fPublicID & 0xffffffffUL);
    v[1] = (uint32_t) ((fPublicID >> 32) & 0xffffffffUL);
    v[2] = fResourceSpecific32;
    v[3] = fDomain << 24 |
           fResourceType << 16 |
           fResourceSpecific16;
}
