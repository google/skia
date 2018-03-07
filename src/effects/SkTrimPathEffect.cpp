/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPathMeasure.h"
#include "SkTrimPathEffect.h"
#include "SkTrimPE.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

SkTrimPE::SkTrimPE(SkScalar startT, SkScalar stopT) : fStartT(startT), fStopT(stopT) {
    SkASSERT(startT >= 0 && startT <= 1);
    SkASSERT(stopT >= 0 && stopT <= 1);
    SkASSERT(startT != stopT);
}

bool SkTrimPE::filterPath(SkPath* dst, const SkPath& src, SkStrokeRec* rec,
                            const SkRect* cullRect) const {
    SkPathMeasure meas(src, false);
    SkScalar length = meas.getLength();

    if (fStartT < fStopT) {
        meas.getSegment(fStartT * length, fStopT * length, dst, true);
    } else {
        meas.getSegment(0, fStopT * length, dst, true);
        meas.getSegment(fStartT * length, length, dst, true);
    }
    return true;
}

void SkTrimPE::flatten(SkWriteBuffer& buffer) const {
    buffer.writeScalar(fStartT);
    buffer.writeScalar(fStopT);
}

sk_sp<SkFlattenable> SkTrimPE::CreateProc(SkReadBuffer& buffer) {
    SkScalar start = buffer.readScalar();
    SkScalar stop = buffer.readScalar();
    return SkTrimPathEffect::Make(start, stop);
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
