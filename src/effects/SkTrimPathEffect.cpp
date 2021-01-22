/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathMeasure.h"
#include "include/effects/SkTrimPathEffect.h"
#include "include/private/SkTPin.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/SkTrimPE.h"

namespace {

// Returns the number of contours iterated to satisfy the request.
static size_t add_segments(const SkPath& src, SkScalar start, SkScalar stop, SkPath* dst,
                           bool requires_moveto = true) {
    SkASSERT(start < stop);

    SkPathMeasure measure(src, false);

    SkScalar current_segment_offset = 0;
    size_t            contour_count = 1;

    do {
        const auto next_offset = current_segment_offset + measure.getLength();

        if (start < next_offset) {
            measure.getSegment(start - current_segment_offset,
                               stop  - current_segment_offset,
                               dst, requires_moveto);

            if (stop <= next_offset)
                break;
        }

        contour_count++;
        current_segment_offset = next_offset;
    } while (measure.nextContour());

    return contour_count;
}

} // namespace

SkTrimPE::SkTrimPE(SkScalar startT, SkScalar stopT, SkTrimPathEffect::Mode mode)
    : fStartT(startT), fStopT(stopT), fMode(mode) {}

bool SkTrimPE::onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const {
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
    if (fMode == SkTrimPathEffect::Mode::kNormal) {
        // Normal mode -> one span.
        if (arcStart < arcStop) {
            add_segments(src, arcStart, arcStop, dst);
        }
    } else {
        // Inverted mode -> one logical span which wraps around at the end -> two actual spans.
        // In order to preserve closed path continuity:
        //
        //   1) add the second/tail span first
        //
        //   2) skip the head span move-to for single-closed-contour paths

        bool requires_moveto = true;
        if (arcStop < len) {
            // since we're adding the "tail" first, this is the total number of contours
            const auto contour_count = add_segments(src, arcStop, len, dst);

            // if the path consists of a single closed contour, we don't want to disconnect
            // the two parts with a moveto.
            if (contour_count == 1 && src.isLastContourClosed()) {
                requires_moveto = false;
            }
        }
        if (0 <  arcStart) {
            add_segments(src, 0, arcStart, dst, requires_moveto);
        }
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
