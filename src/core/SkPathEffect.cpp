
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPathEffect.h"
#include "SkPath.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

///////////////////////////////////////////////////////////////////////////////

void SkPathEffect::computeFastBounds(SkRect* dst, const SkRect& src) const {
    *dst = src;
}

bool SkPathEffect::asPoints(PointData* results, const SkPath& src,
                    const SkStrokeRec&, const SkMatrix&, const SkRect*) const {
    return false;
}

///////////////////////////////////////////////////////////////////////////////

SkPairPathEffect::SkPairPathEffect(SkPathEffect* pe0, SkPathEffect* pe1)
        : fPE0(pe0), fPE1(pe1) {
    SkASSERT(pe0);
    SkASSERT(pe1);
    fPE0->ref();
    fPE1->ref();
}

SkPairPathEffect::~SkPairPathEffect() {
    SkSafeUnref(fPE0);
    SkSafeUnref(fPE1);
}

/*
    Format: [oe0-factory][pe1-factory][pe0-size][pe0-data][pe1-data]
*/
void SkPairPathEffect::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeFlattenable(fPE0);
    buffer.writeFlattenable(fPE1);
}

SkPairPathEffect::SkPairPathEffect(SkReadBuffer& buffer) {
    fPE0 = buffer.readPathEffect();
    fPE1 = buffer.readPathEffect();
    // either of these may fail, so we have to check for nulls later on
}

///////////////////////////////////////////////////////////////////////////////

bool SkComposePathEffect::filterPath(SkPath* dst, const SkPath& src,
                             SkStrokeRec* rec, const SkRect* cullRect) const {
    // we may have failed to unflatten these, so we have to check
    if (!fPE0 || !fPE1) {
        return false;
    }

    SkPath          tmp;
    const SkPath*   ptr = &src;

    if (fPE1->filterPath(&tmp, src, rec, cullRect)) {
        ptr = &tmp;
    }
    return fPE0->filterPath(dst, *ptr, rec, cullRect);
}

///////////////////////////////////////////////////////////////////////////////

bool SkSumPathEffect::filterPath(SkPath* dst, const SkPath& src,
                             SkStrokeRec* rec, const SkRect* cullRect) const {
    // use bit-or so that we always call both, even if the first one succeeds
    return fPE0->filterPath(dst, src, rec, cullRect) |
           fPE1->filterPath(dst, src, rec, cullRect);
}
