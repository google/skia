/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathTypes.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkPathMakers.h"
#include "src/core/SkPathRawShapes.h"

const SkPathFillType kDefFillType = SkPathFillType::kWinding;

const SkPathVerb gRectVerbs[] = {
    SkPathVerb::kMove,
    SkPathVerb::kLine,
    SkPathVerb::kLine,
    SkPathVerb::kLine,
    SkPathVerb::kLine,
    SkPathVerb::kClose
};

const uint8_t gRectSegMask = kLine_SkPathSegmentMask;

SkPathRawShapes::Rect::Rect(const SkRect& r, SkPathDirection dir, unsigned index)
    : SkPathRaw{fStorage, gRectVerbs, {}, r, kDefFillType, true, gRectSegMask}
{
    SkPath_RectPointIterator iter(r, dir, index);

    fStorage[0] = iter.current();
    fStorage[1] = iter.next();
    fStorage[2] = iter.next();
    fStorage[3] = iter.next();
    fStorage[4] = fStorage[0];  // add explict 4th line, in case we're stroked
}

//////////////////

const SkPathVerb gOvalVerbs[] = {
    SkPathVerb::kMove,
    SkPathVerb::kConic,
    SkPathVerb::kConic,
    SkPathVerb::kConic,
    SkPathVerb::kConic,
    SkPathVerb::kClose
};

const uint8_t gOvalSegMask = kConic_SkPathSegmentMask;

const float gOvalConics[] = {
    SK_ScalarRoot2Over2,
    SK_ScalarRoot2Over2,
    SK_ScalarRoot2Over2,
    SK_ScalarRoot2Over2,
};

SkPathRawShapes::Oval::Oval(const SkRect& r, SkPathDirection dir, unsigned index)
    : SkPathRaw{fStorage, gOvalVerbs, gOvalConics, r, kDefFillType, true, gOvalSegMask}
{
    SkPath_OvalPointIterator ovalIter(r, dir, index);
    SkPath_RectPointIterator rectIter(r, dir, index + (dir == SkPathDirection::kCW ? 0 : 1));

    fStorage[0] = ovalIter.current();
    for (unsigned i = 0; i < 4; ++i) {
        fStorage[i*2 + 1] = rectIter.next();
        fStorage[i*2 + 2] = ovalIter.next();
    }
}
