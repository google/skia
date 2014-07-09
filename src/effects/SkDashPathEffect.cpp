/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDashPathEffect.h"

#include "SkDashPathPriv.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

SkDashPathEffect::SkDashPathEffect(const SkScalar intervals[], int count, SkScalar phase)
        : fPhase(0)
        , fInitialDashLength(0)
        , fInitialDashIndex(0)
        , fIntervalLength(0) {
    SkASSERT(intervals);
    SkASSERT(count > 1 && SkAlign2(count) == count);

    fIntervals = (SkScalar*)sk_malloc_throw(sizeof(SkScalar) * count);
    fCount = count;
    for (int i = 0; i < count; i++) {
        SkASSERT(intervals[i] >= 0);
        fIntervals[i] = intervals[i];
    }

    // set the internal data members
    SkDashPath::CalcDashParameters(phase, fIntervals, fCount,
            &fInitialDashLength, &fInitialDashIndex, &fIntervalLength, &fPhase);
}

SkDashPathEffect::~SkDashPathEffect() {
    sk_free(fIntervals);
}

bool SkDashPathEffect::filterPath(SkPath* dst, const SkPath& src,
                              SkStrokeRec* rec, const SkRect* cullRect) const {
    return SkDashPath::FilterDashPath(dst, src, rec, cullRect, fIntervals, fCount,
                                      fInitialDashLength, fInitialDashIndex, fIntervalLength);
}

// Currently asPoints is more restrictive then it needs to be. In the future
// we need to:
//      allow kRound_Cap capping (could allow rotations in the matrix with this)
//      allow paths to be returned
bool SkDashPathEffect::asPoints(PointData* results,
                                const SkPath& src,
                                const SkStrokeRec& rec,
                                const SkMatrix& matrix,
                                const SkRect* cullRect) const {
    // width < 0 -> fill && width == 0 -> hairline so requiring width > 0 rules both out
    if (fInitialDashLength < 0 || 0 >= rec.getWidth()) {
        return false;
    }

    // TODO: this next test could be eased up. We could allow any number of
    // intervals as long as all the ons match and all the offs match.
    // Additionally, they do not necessarily need to be integers.
    // We cannot allow arbitrary intervals since we want the returned points
    // to be uniformly sized.
    if (fCount != 2 ||
        !SkScalarNearlyEqual(fIntervals[0], fIntervals[1]) ||
        !SkScalarIsInt(fIntervals[0]) ||
        !SkScalarIsInt(fIntervals[1])) {
        return false;
    }

    SkPoint pts[2];

    if (!src.isLine(pts)) {
        return false;
    }

    // TODO: this test could be eased up to allow circles
    if (SkPaint::kButt_Cap != rec.getCap()) {
        return false;
    }

    // TODO: this test could be eased up for circles. Rotations could be allowed.
    if (!matrix.rectStaysRect()) {
        return false;
    }

    SkScalar        length = SkPoint::Distance(pts[1], pts[0]);

    SkVector tangent = pts[1] - pts[0];
    if (tangent.isZero()) {
        return false;
    }

    tangent.scale(SkScalarInvert(length));

    // TODO: make this test for horizontal & vertical lines more robust
    bool isXAxis = true;
    if (SK_Scalar1 == tangent.fX || -SK_Scalar1 == tangent.fX) {
        results->fSize.set(SkScalarHalf(fIntervals[0]), SkScalarHalf(rec.getWidth()));
    } else if (SK_Scalar1 == tangent.fY || -SK_Scalar1 == tangent.fY) {
        results->fSize.set(SkScalarHalf(rec.getWidth()), SkScalarHalf(fIntervals[0]));
        isXAxis = false;
    } else if (SkPaint::kRound_Cap != rec.getCap()) {
        // Angled lines don't have axis-aligned boxes.
        return false;
    }

    if (NULL != results) {
        results->fFlags = 0;
        SkScalar clampedInitialDashLength = SkMinScalar(length, fInitialDashLength);

        if (SkPaint::kRound_Cap == rec.getCap()) {
            results->fFlags |= PointData::kCircles_PointFlag;
        }

        results->fNumPoints = 0;
        SkScalar len2 = length;
        if (clampedInitialDashLength > 0 || 0 == fInitialDashIndex) {
            SkASSERT(len2 >= clampedInitialDashLength);
            if (0 == fInitialDashIndex) {
                if (clampedInitialDashLength > 0) {
                    if (clampedInitialDashLength >= fIntervals[0]) {
                        ++results->fNumPoints;  // partial first dash
                    }
                    len2 -= clampedInitialDashLength;
                }
                len2 -= fIntervals[1];  // also skip first space
                if (len2 < 0) {
                    len2 = 0;
                }
            } else {
                len2 -= clampedInitialDashLength; // skip initial partial empty
            }
        }
        int numMidPoints = SkScalarFloorToInt(SkScalarDiv(len2, fIntervalLength));
        results->fNumPoints += numMidPoints;
        len2 -= numMidPoints * fIntervalLength;
        bool partialLast = false;
        if (len2 > 0) {
            if (len2 < fIntervals[0]) {
                partialLast = true;
            } else {
                ++numMidPoints;
                ++results->fNumPoints;
            }
        }

        results->fPoints = new SkPoint[results->fNumPoints];

        SkScalar    distance = 0;
        int         curPt = 0;

        if (clampedInitialDashLength > 0 || 0 == fInitialDashIndex) {
            SkASSERT(clampedInitialDashLength <= length);

            if (0 == fInitialDashIndex) {
                if (clampedInitialDashLength > 0) {
                    // partial first block
                    SkASSERT(SkPaint::kRound_Cap != rec.getCap()); // can't handle partial circles
                    SkScalar x = pts[0].fX + SkScalarMul(tangent.fX, SkScalarHalf(clampedInitialDashLength));
                    SkScalar y = pts[0].fY + SkScalarMul(tangent.fY, SkScalarHalf(clampedInitialDashLength));
                    SkScalar halfWidth, halfHeight;
                    if (isXAxis) {
                        halfWidth = SkScalarHalf(clampedInitialDashLength);
                        halfHeight = SkScalarHalf(rec.getWidth());
                    } else {
                        halfWidth = SkScalarHalf(rec.getWidth());
                        halfHeight = SkScalarHalf(clampedInitialDashLength);
                    }
                    if (clampedInitialDashLength < fIntervals[0]) {
                        // This one will not be like the others
                        results->fFirst.addRect(x - halfWidth, y - halfHeight,
                                                x + halfWidth, y + halfHeight);
                    } else {
                        SkASSERT(curPt < results->fNumPoints);
                        results->fPoints[curPt].set(x, y);
                        ++curPt;
                    }

                    distance += clampedInitialDashLength;
                }

                distance += fIntervals[1];  // skip over the next blank block too
            } else {
                distance += clampedInitialDashLength;
            }
        }

        if (0 != numMidPoints) {
            distance += SkScalarHalf(fIntervals[0]);

            for (int i = 0; i < numMidPoints; ++i) {
                SkScalar x = pts[0].fX + SkScalarMul(tangent.fX, distance);
                SkScalar y = pts[0].fY + SkScalarMul(tangent.fY, distance);

                SkASSERT(curPt < results->fNumPoints);
                results->fPoints[curPt].set(x, y);
                ++curPt;

                distance += fIntervalLength;
            }

            distance -= SkScalarHalf(fIntervals[0]);
        }

        if (partialLast) {
            // partial final block
            SkASSERT(SkPaint::kRound_Cap != rec.getCap()); // can't handle partial circles
            SkScalar temp = length - distance;
            SkASSERT(temp < fIntervals[0]);
            SkScalar x = pts[0].fX + SkScalarMul(tangent.fX, distance + SkScalarHalf(temp));
            SkScalar y = pts[0].fY + SkScalarMul(tangent.fY, distance + SkScalarHalf(temp));
            SkScalar halfWidth, halfHeight;
            if (isXAxis) {
                halfWidth = SkScalarHalf(temp);
                halfHeight = SkScalarHalf(rec.getWidth());
            } else {
                halfWidth = SkScalarHalf(rec.getWidth());
                halfHeight = SkScalarHalf(temp);
            }
            results->fLast.addRect(x - halfWidth, y - halfHeight,
                                   x + halfWidth, y + halfHeight);
        }

        SkASSERT(curPt == results->fNumPoints);
    }

    return true;
}

SkPathEffect::DashType SkDashPathEffect::asADash(DashInfo* info) const {
    if (info) {
        if (info->fCount >= fCount && NULL != info->fIntervals) {
            memcpy(info->fIntervals, fIntervals, fCount * sizeof(SkScalar));
        }
        info->fCount = fCount;
        info->fPhase = fPhase;
    }
    return kDash_DashType;
}

SkFlattenable::Factory SkDashPathEffect::getFactory() const {
    return CreateProc;
}

void SkDashPathEffect::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fPhase);
    buffer.writeScalarArray(fIntervals, fCount);
}

SkFlattenable* SkDashPathEffect::CreateProc(SkReadBuffer& buffer) {
    return SkNEW_ARGS(SkDashPathEffect, (buffer));
}

SkDashPathEffect::SkDashPathEffect(SkReadBuffer& buffer)
        : INHERITED(buffer)
        , fPhase(0)
        , fInitialDashLength(0)
        , fInitialDashIndex(0)
        , fIntervalLength(0) {
    bool useOldPic = buffer.isVersionLT(SkReadBuffer::kDashWritesPhaseIntervals_Version);
    if (useOldPic) {
        fInitialDashIndex = buffer.readInt();
        fInitialDashLength = buffer.readScalar();
        fIntervalLength = buffer.readScalar();
        buffer.readBool(); // Dummy for old ScalarToFit field
    } else {
        fPhase = buffer.readScalar();
    }

    fCount = buffer.getArrayCount();
    size_t allocSize = sizeof(SkScalar) * fCount;
    if (buffer.validateAvailable(allocSize)) {
        fIntervals = (SkScalar*)sk_malloc_throw(allocSize);
        buffer.readScalarArray(fIntervals, fCount);
    } else {
        fIntervals = NULL;
    }

    if (useOldPic) {
        fPhase = 0;
        if (fInitialDashLength != -1) { // Signal for bad dash interval
            for (int i = 0; i < fInitialDashIndex; ++i) {
                fPhase += fIntervals[i];
            }
            fPhase += fIntervals[fInitialDashIndex] - fInitialDashLength;
        }
    } else {
        // set the internal data members, fPhase should have been between 0 and intervalLength
        // when written to buffer so no need to adjust it
        SkDashPath::CalcDashParameters(fPhase, fIntervals, fCount,
                &fInitialDashLength, &fInitialDashIndex, &fIntervalLength);
    }
}
