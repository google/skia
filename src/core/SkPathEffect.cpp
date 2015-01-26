
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

SkPathEffect::DashType SkPathEffect::asADash(DashInfo* info) const {
    return kNone_DashType;
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
    buffer.writeFlattenable(fPE0);
    buffer.writeFlattenable(fPE1);
}

#ifndef SK_IGNORE_TO_STRING
void SkPairPathEffect::toString(SkString* str) const {
    str->appendf("first: ");
    if (fPE0) {
        fPE0->toString(str);
    }
    str->appendf(" second: ");
    if (fPE1) {
        fPE1->toString(str);
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////

SkFlattenable* SkComposePathEffect::CreateProc(SkReadBuffer& buffer) {
    SkAutoTUnref<SkPathEffect> pe0(buffer.readPathEffect());
    SkAutoTUnref<SkPathEffect> pe1(buffer.readPathEffect());
    return SkComposePathEffect::Create(pe0, pe1);
}

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


#ifndef SK_IGNORE_TO_STRING
void SkComposePathEffect::toString(SkString* str) const {
    str->appendf("SkComposePathEffect: (");
    this->INHERITED::toString(str);
    str->appendf(")");
}
#endif

///////////////////////////////////////////////////////////////////////////////

SkFlattenable* SkSumPathEffect::CreateProc(SkReadBuffer& buffer) {
    SkAutoTUnref<SkPathEffect> pe0(buffer.readPathEffect());
    SkAutoTUnref<SkPathEffect> pe1(buffer.readPathEffect());
    return SkSumPathEffect::Create(pe0, pe1);
}

bool SkSumPathEffect::filterPath(SkPath* dst, const SkPath& src,
                             SkStrokeRec* rec, const SkRect* cullRect) const {
    // use bit-or so that we always call both, even if the first one succeeds
    return fPE0->filterPath(dst, src, rec, cullRect) |
           fPE1->filterPath(dst, src, rec, cullRect);
}


#ifndef SK_IGNORE_TO_STRING
void SkSumPathEffect::toString(SkString* str) const {
    str->appendf("SkSumPathEffect: (");
    this->INHERITED::toString(str);
    str->appendf(")");
}
#endif
