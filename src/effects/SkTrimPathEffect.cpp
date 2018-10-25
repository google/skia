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

namespace {

class Segmentator : public SkNoncopyable {
public:
    Segmentator(const SkPath& src, SkPath* dst)
        : fMeasure(src, false)
        , fDst(dst) {}

    void add(SkScalar start, SkScalar stop) {
        SkASSERT(start < stop);

        // TODO: we appear to skip zero-length contours.
        do {
            const auto nextOffset = fCurrentSegmentOffset + fMeasure.getLength();

            if (start < nextOffset) {
                fMeasure.getSegment(start - fCurrentSegmentOffset,
                                    stop  - fCurrentSegmentOffset,
                                    fDst, true);

                if (stop < nextOffset)
                    break;
            }

            fCurrentSegmentOffset = nextOffset;
        } while (fMeasure.nextContour());
    }

private:
    SkPathMeasure fMeasure;
    SkPath*       fDst;

    SkScalar fCurrentSegmentOffset = 0;

    using INHERITED = SkNoncopyable;
};

} // namespace

SkTrimPE::SkTrimPE(SkScalar startT, SkScalar stopT, SkTrimPathEffect::Mode mode)
    : fStartT(startT), fStopT(stopT), fMode(mode) {}

bool SkTrimPE::onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec* rec,
                            const SkRect* cullRect) const {
    if (fStartT >= fStopT) {
        SkASSERT(fMode == SkTrimPathEffect::Mode::kNormal);
        return true;
    }

    // First pass: compute the total len.
    SkScalar len = 0;
    SkPathMeasure meas(src, false);
    do {
        len += meas.getLength();
    } while (meas.nextContour());

    const auto arcStart = len * fStartT,
               arcStop  = len * fStopT;

    // Second pass: actually add segments.
    Segmentator segmentator(src, dst);
    if (fMode == SkTrimPathEffect::Mode::kNormal) {
        if (arcStart < arcStop) segmentator.add(arcStart, arcStop);
    } else {
        if (0 <  arcStart) segmentator.add(0,  arcStart);
        if (arcStop < len) segmentator.add(arcStop, len);
    }

    return true;
}

void SkTrimPE::flatten(SkWriteBuffer& buffer) const {
    buffer.writeScalar(fStartT);
    buffer.writeScalar(fStopT);
    buffer.writeUInt(static_cast<uint32_t>(fMode));
}

sk_sp<SkFlattenable> SkTrimPE::CreateProc(SkReadBuffer& buffer) {
    const auto start = buffer.readScalar(),
               stop  = buffer.readScalar();
    const auto mode  = buffer.readUInt();

    return SkTrimPathEffect::Make(start, stop,
        (mode & 1) ? SkTrimPathEffect::Mode::kInverted : SkTrimPathEffect::Mode::kNormal);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkPathEffect> SkTrimPathEffect::Make(SkScalar startT, SkScalar stopT, Mode mode) {
    if (!SkScalarsAreFinite(startT, stopT)) {
        return nullptr;
    }

    if (startT <= 0 && stopT >= 1 && mode == Mode::kNormal) {
        return nullptr;
    }

    startT = SkTPin(startT, 0.f, 1.f);
    stopT  = SkTPin(stopT,  0.f, 1.f);

    if (startT >= stopT && mode == Mode::kInverted) {
        return nullptr;
    }

    return sk_sp<SkPathEffect>(new SkTrimPE(startT, stopT, mode));
}
