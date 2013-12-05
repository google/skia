/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPath.h"

GrResourceKey GrPath::ComputeKey(const SkPath& path, const SkStrokeRec& stroke) {
    static const GrResourceKey::ResourceType gPathResourceType = GrResourceKey::GenerateResourceType();
    static const GrCacheID::Domain gPathDomain = GrCacheID::GenerateDomain();

    GrCacheID::Key key;
    uint32_t* keyData = key.fData32;
    keyData[0] = path.getGenerationID();

    SK_COMPILE_ASSERT(SkPaint::kJoinCount <= 3, cap_shift_will_be_wrong);
    keyData[1] = stroke.needToApply();
    if (0 != keyData[1]) {
        keyData[1] |= stroke.getJoin() << 1;
        keyData[1] |= stroke.getCap() << 3;
        keyData[2] = static_cast<uint32_t>(stroke.getMiter());
        keyData[3] = static_cast<uint32_t>(stroke.getWidth());
    } else {
        keyData[2] = 0;
        keyData[3] = 0;
    }

    return GrResourceKey(GrCacheID(gPathDomain, key), gPathResourceType, 0);
}
