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

SkPairPathEffect::SkPairPathEffect(sk_sp<SkPathEffect> pe0, sk_sp<SkPathEffect> pe1)
    : fPE0(std::move(pe0)), fPE1(std::move(pe1))
{
    SkASSERT(fPE0.get());
    SkASSERT(fPE1.get());
}

/*
    Format: [oe0-factory][pe1-factory][pe0-size][pe0-data][pe1-data]
*/
void SkPairPathEffect::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fPE0.get());
    buffer.writeFlattenable(fPE1.get());
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

sk_sp<SkFlattenable> SkComposePathEffect::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkPathEffect> pe0(buffer.readPathEffect());
    sk_sp<SkPathEffect> pe1(buffer.readPathEffect());
    return SkComposePathEffect::Make(std::move(pe0), std::move(pe1));
}

bool SkComposePathEffect::filterPath(SkPath* dst, const SkPath& src,
                             SkStrokeRec* rec, const SkRect* cullRect) const {
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

sk_sp<SkFlattenable> SkSumPathEffect::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkPathEffect> pe0(buffer.readPathEffect());
    sk_sp<SkPathEffect> pe1(buffer.readPathEffect());
    return SkSumPathEffect::Make(pe0, pe1);
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
