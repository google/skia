/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/aligned_dashing/SkAlignedDashPathEffect.h"

#include "include/core/SkPathMeasure.h"
#include "src/utils/SkDashPathPriv.h"

#include <numeric>
#include <vector>

namespace {

//////////////////////////////////////////////////////////////////////////////

/** Helper class that measures per-verb path lengths. */
class PathVerbMeasure {
public:
    PathVerbMeasure(const SkPath& path) : fIter(path, false) { nextVerb(); }

    SkScalar currentVerbLength() { return fMeas.getLength(); }

    void nextVerb();

private:
    SkPoint fFirstPointInContour;
    SkPoint fPreviousPoint;
    SkPath fCurrVerb;
    SkPath::Iter fIter;
    SkPathMeasure fMeas;
};

void PathVerbMeasure::nextVerb() {
    SkPoint pts[4];
    SkPath::Verb verb = fIter.next(pts);

    while (verb == SkPath::kMove_Verb || verb == SkPath::kClose_Verb) {
        if (verb == SkPath::kMove_Verb) {
            fFirstPointInContour = pts[0];
            fPreviousPoint = fFirstPointInContour;
        }
        verb = fIter.next(pts);
    }

    fCurrVerb.rewind();
    fCurrVerb.moveTo(fPreviousPoint);
    switch (verb) {
        case SkPath::kLine_Verb:
            fCurrVerb.lineTo(pts[1]);
            break;
        case SkPath::kQuad_Verb:
            fCurrVerb.quadTo(pts[1], pts[2]);
            break;
        case SkPath::kCubic_Verb:
            fCurrVerb.cubicTo(pts[1], pts[2], pts[3]);
            break;
        case SkPath::kConic_Verb:
            fCurrVerb.conicTo(pts[1], pts[2], fIter.conicWeight());
            break;
        case SkPath::kDone_Verb:
            break;
        case SkPath::kClose_Verb:
        case SkPath::kMove_Verb:
            SkASSERT(false);
            break;
    }

    fCurrVerb.getLastPt(&fPreviousPoint);
    fMeas.setPath(&fCurrVerb, false);
}

//////////////////////////////////////////////////////////////////////////////

/** Path effect that performs per-verb dash alignment. */
class SkAlignedDashImpl : public SkPathEffect {
public:
    SkAlignedDashImpl(const SkScalar intervals[], int count);

protected:
    void flatten(SkWriteBuffer&) const override;

    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const override;

private:
    typedef SkPathEffect INHERITED;

    SK_FLATTENABLE_HOOKS(SkAlignedDashImpl)

    std::vector<SkScalar> fIntervals;
    SkScalar fIntervalLength;
    SkScalar fInitialDashLength;
    int fInitialDashIndex;

    /**
     * Computes an "error" metric for aligned dashes. The error metric is the amount of
     * distance left over after tiling the dash intervals an even number of times.
     */
    static double computeAlignedDashError(double length, double intervalLengthSum);
};

SkAlignedDashImpl::SkAlignedDashImpl(const SkScalar intervals[], int count)
        : fIntervals(intervals, intervals + count) {
    SkDashPath::CalcDashParameters(0, intervals, count, &fInitialDashLength, &fInitialDashIndex,
                                   &fIntervalLength, nullptr);
}

void SkAlignedDashImpl::flatten(SkWriteBuffer&) const {}

sk_sp<SkFlattenable> SkAlignedDashImpl::CreateProc(SkReadBuffer& buffer) { return nullptr; }

double SkAlignedDashImpl::computeAlignedDashError(double length, double intervalLengthSum) {
    // Round up to nearest even number of tilings.
    const long n = 2 * std::ceil((length / intervalLengthSum) / 2);
    return (length - n * intervalLengthSum) / n;
}

bool SkAlignedDashImpl::onFilterPath(SkPath* dst,
                                     const SkPath& src,
                                     SkStrokeRec*,
                                     const SkRect*) const {
    const int count = fIntervals.size();
    SkASSERT(count % 2 == 0);

    SkPathMeasure meas(src, false);
    PathVerbMeasure verbMeas(src);
    int segCount = 0;

    do {
        SkScalar currentVerbLength = verbMeas.currentVerbLength();
        SkScalar totalVerbLength = currentVerbLength;
        SkScalar currentVerbAlignErr = computeAlignedDashError(currentVerbLength, fIntervalLength);
        const SkScalar initialDashLength = fIntervals[fInitialDashIndex];
        const SkScalar firstVerbAlignErr = currentVerbAlignErr;
        const SkScalar length = meas.getLength();
        bool skipFirstSegment = meas.isClosed();
        int index = fInitialDashIndex;
        SkScalar dlen = fIntervals[index];
        SkScalar distance = 0;
        bool firstInVerb = true;
        bool firstInContour = true;

        while (!SkScalarNearlyZero(length - distance)) {
            // Modify dlen by fraction of error 'absorbed' by this interval.
            dlen += currentVerbAlignErr * (dlen / fIntervalLength);
            SkASSERT(dlen >= 0);

            const bool lastInVerb = (distance + dlen) >= totalVerbLength;
            if (index % 2 == 0) {
                if (firstInVerb || lastInVerb) {
                    dlen *= 0.5f;
                }

                if (!skipFirstSegment) {
                    ++segCount;
                    const bool moveTo = firstInContour || !firstInVerb;
                    meas.getSegment(distance, distance + dlen, dst, moveTo);

                    // Add a final half-length dash if the contour is closed.
                    const bool lastInContour = SkScalarNearlyZero(length - (distance + dlen));
                    if (meas.isClosed() && lastInContour) {
                        const float f = initialDashLength / fIntervalLength;
                        const float firstDLen = 0.5f * (initialDashLength + firstVerbAlignErr * f);
                        meas.getSegment(0, firstDLen, dst, false);
                    }
                }
            }

            distance += dlen;
            firstInVerb = false;
            firstInContour = false;
            if (SkScalarNearlyZero(totalVerbLength - distance)) {
                verbMeas.nextVerb();
                distance = totalVerbLength;
                currentVerbLength = verbMeas.currentVerbLength();
                totalVerbLength += currentVerbLength;
                currentVerbAlignErr = computeAlignedDashError(currentVerbLength, fIntervalLength);
                firstInVerb = true;
            }

            if (lastInVerb) {
                // With aligned dashing, we always begin and end on the first dash.
                SkASSERT(index == 0);
            } else {
                index = (index + 1) % count;
            }

            skipFirstSegment = false;
            dlen = fIntervals[index];
        }
    } while (meas.nextContour());

    // TODO(tdenniston): Carried over from existing dash path effect.
    //   Is this correctness or performance?
    // if (segCount > 1) {
    //     dst->setConvexityType(SkPathConvexityType::kConcave);
    // }

    return true;
}

}  // namespace

//////////////////////////////////////////////////////////////////////////////

sk_sp<SkPathEffect> SkAlignedDashPathEffect::Make(const SkScalar intervals[], int count) {
    if (!SkDashPath::ValidDashPath(0, intervals, count)) {
        return nullptr;
    }
    return sk_sp<SkPathEffect>(new SkAlignedDashImpl(intervals, count));
}
