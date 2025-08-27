/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathTypes.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkPathRaw.h"

const uint8_t gVerbToSegmentMask[] = {
    0,  // move
    kLine_SkPathSegmentMask,
    kQuad_SkPathSegmentMask,
    kConic_SkPathSegmentMask,
    kCubic_SkPathSegmentMask,
    0,  // close
};

uint8_t SkPathPriv::ComputeSegmentMask(SkSpan<const SkPathVerb> verbs) {
    unsigned mask = 0;
    for (auto v : verbs) {
        unsigned i = static_cast<unsigned>(v);
        SkASSERT(i < std::size(gVerbToSegmentMask));
        mask |= gVerbToSegmentMask[i];
    }
    return SkTo<uint8_t>(mask);
}

std::optional<SkRect> SkPathRaw::isRect() const {
    if (auto rc = SkPathPriv::IsRectContour(fPoints, fVerbs, false)) {
        return rc->fRect;
    }
    return {};
}
