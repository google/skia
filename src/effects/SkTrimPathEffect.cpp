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

SkTrimPE::SkTrimPE(SkScalar startT, SkScalar stopT, SkTrimPathEffect::Mode mode)
    : fStartT(startT), fStopT(stopT), fMode(mode) {}

static SkScalar add_segments(SkPath* dst, SkPathMeasure* meas,
                             SkScalar arcStart, SkScalar arcStop, SkScalar offset) {
    SkASSERT(arcStart < arcStop);

    auto nextOffset = offset;
    do {
        const auto offset = nextOffset;
        nextOffset += meas->getLength();

        if (arcStart >= nextOffset) continue;

        meas->getSegment(arcStart - offset, arcStop - offset, dst, true);

        if (arcStop <= nextOffset) break;

    } while (meas->nextContour());

    return offset;
}

bool SkTrimPE::filterPath(SkPath* dst, const SkPath& src, SkStrokeRec* rec,
                            const SkRect* cullRect) const {
    // First pass: compute the total len.
    SkScalar len = 0;
    SkPathMeasure meas(src, false);
    do {
        len += meas.getLength();
    } while (meas.nextContour());

    const auto arcStart = SkTPin(len * fStartT, 0.0f, len),
               arcStop  = SkTPin(len * fStopT , 0.0f, len);

    printf("** start: %f, stop: %f\n", arcStart, arcStop);
    if (arcStart >= arcStop) {
        if (fMode == SkTrimPathEffect::Mode::kInverted) {
            *dst = src;
        }
        return true;
    }

    // Second pass: actually add segments.
    meas.setPath(&src, false);

    add_segments(dst, &meas, arcStart, arcStop, 0);
//    if (fStartT < fStopT) {
//        meas.getSegment(fStartT * len, fStopT * len, dst, true);
//    } else {
//        meas.getSegment(0, fStopT * len, dst, true);
//        meas.getSegment(fStartT * len, len, dst, true);
//    }
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

sk_sp<SkPathEffect> SkTrimPathEffect::Make(SkScalar startT, SkScalar stopT, Mode mode) {
    if (!SkScalarsAreFinite(startT, stopT)) {
        return nullptr;
    }

    if (startT >= stopT && mode == Mode::kInverted) {
        return nullptr;
    }

    return sk_sp<SkPathEffect>(new SkTrimPE(startT, stopT, mode));
}
