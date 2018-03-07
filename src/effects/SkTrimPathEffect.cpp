/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTrimPathEffect.h"
#include "SkTrimPE.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkStrokeRec.h"

SkTrimPE::SkTrimPE(SkScalar startT, SkScalar stopT) : fStartT(startT), fStopT(stopT) {
    SkASSERT(startT >= 0 && startT <= 1);
    SkASSERT(stopT >= 0 && stopT <= 1);
    SkASSERT(startT != stopT);
}

bool SkTrimPE::filterPath(SkPath* dst, const SkPath& src, SkStrokeRec* rec,
                            const SkRect* cullRect) const {
    return SkDashPath::InternalFilter(dst, src, rec, cullRect, fIntervals, fCount,
                                      fInitialDashLength, fInitialDashIndex, fIntervalLength);
}

void SkTrimPE::flatten(SkWriteBuffer& buffer) const {
    buffer.writeScalar(fPhase);
    buffer.writeScalarArray(fIntervals, fCount);
}

sk_sp<SkFlattenable> SkTrimPE::CreateProc(SkReadBuffer& buffer) {
    const SkScalar phase = buffer.readScalar();
    uint32_t count = buffer.getArrayCount();
    SkAutoSTArray<32, SkScalar> intervals(count);
    if (buffer.readScalarArray(intervals.get(), count)) {
        return SkDashPathEffect::Make(intervals.get(), SkToInt(count), phase);
    }
    return nullptr;
}

#ifndef SK_IGNORE_TO_STRING
void SkTrimPE::toString(SkString* str) const {
    str->appendf("SkTrimPathEffect: (%g %g)", fStartT, fStopT);
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkPathEffect> SkTrimPathEffect::Make(SkScalar startT, SkScalar stopT) {
    if (!SkScalarsAreFinite(startT, stopT)) {
        return nullptr;
    }
    startT = SkTPin(startT, 0.f, 1.f);
    stopT  = SkTPin(stopT,  0.f, 1.f);
    if (startT == stopT) {
        return nullptr;
    }
    return sk_sp<SkPathEffect>(new SkTrimPE(startT, stopT));
}
