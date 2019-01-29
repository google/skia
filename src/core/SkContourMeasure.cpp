/*
 * Copyright 2019 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPathMeasure.h"
#include "SkPathMeasurePriv.h"
#include "SkGeometry.h"
#include "SkPath.h"
#include "SkTSearch.h"

#define kMaxTValue  0x3FFFFFFF

static void compute_pos_tan(const SkPoint pts[], unsigned segType,
                            SkScalar t, SkPoint* pos, SkVector* tangent) {
    switch (segType) {
        case kLine_SegType:
            if (pos) {
                pos->set(SkScalarInterp(pts[0].fX, pts[1].fX, t),
                         SkScalarInterp(pts[0].fY, pts[1].fY, t));
            }
            if (tangent) {
                tangent->setNormalize(pts[1].fX - pts[0].fX, pts[1].fY - pts[0].fY);
            }
            break;
        case kQuad_SegType:
            SkEvalQuadAt(pts, t, pos, tangent);
            if (tangent) {
                tangent->normalize();
            }
            break;
        case kConic_SegType: {
            SkConic(pts[0], pts[2], pts[3], pts[1].fX).evalAt(t, pos, tangent);
            if (tangent) {
                tangent->normalize();
            }
        } break;
        case kCubic_SegType:
            SkEvalCubicAt(pts, t, pos, tangent, nullptr);
            if (tangent) {
                tangent->normalize();
            }
            break;
        default:
            SkDEBUGFAIL("unknown segType");
    }
}

SkContourMeasure::SkContourMeasure(SkTDArray<Segment>&& segs, SkTDArray<SkPoint>&& pts,
                                   SkScalar length, bool isClosed)
    : fSegments(std::move(segs))
    , fPts(std::move(pts))
    , fLength(length)
    , fIsClosed(isClosed)
{}

SkContourMeasure::~SkContourMeasure() {}

template <typename T, typename K>
int SkTKSearch(const T base[], int count, const K& key) {
    SkASSERT(count >= 0);
    if (count <= 0) {
        return ~0;
    }

    SkASSERT(base != nullptr); // base may be nullptr if count is zero

    unsigned lo = 0;
    unsigned hi = count - 1;

    while (lo < hi) {
        unsigned mid = (hi + lo) >> 1;
        if (base[mid].fDistance < key) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }

    if (base[hi].fDistance < key) {
        hi += 1;
        hi = ~hi;
    } else if (key < base[hi].fDistance) {
        hi = ~hi;
    }
    return hi;
}

const SkContourMeasure::Segment*
SkContourMeasure::distanceToSegment(SkScalar distance, SkScalar* t) const {
    SkDEBUGCODE(SkScalar length = ) this->length();
    SkASSERT(distance >= 0 && distance <= length);

    const Segment*  seg = fSegments.begin();
    int             count = fSegments.count();

    int index = SkTKSearch<Segment, SkScalar>(seg, count, distance);
    // don't care if we hit an exact match or not, so we xor index if it is negative
    index ^= (index >> 31);
    seg = &seg[index];

    // now interpolate t-values with the prev segment (if possible)
    SkScalar    startT = 0, startD = 0;
    // check if the prev segment is legal, and references the same set of points
    if (index > 0) {
        startD = seg[-1].fDistance;
        if (seg[-1].fPtIndex == seg->fPtIndex) {
            SkASSERT(seg[-1].fType == seg->fType);
            startT = seg[-1].getScalarT();
        }
    }

    SkASSERT(seg->getScalarT() > startT);
    SkASSERT(distance >= startD);
    SkASSERT(seg->fDistance > startD);

    *t = startT + (seg->getScalarT() - startT) * (distance - startD) / (seg->fDistance - startD);
    return seg;
}

bool SkContourMeasure::getPosTan(SkScalar distance, SkPoint* pos, SkVector* tangent) const {
    SkScalar    length = this->length();
    int         count = fSegments.count();

    if (count == 0 || length == 0 || SkScalarIsNaN(distance)) {
        return false;
    }

    // pin the distance to a legal range
    if (distance < 0) {
        distance = 0;
    } else if (distance > length) {
        distance = length;
    }

    SkScalar        t;
    const Segment*  seg = this->distanceToSegment(distance, &t);
    if (SkScalarIsNaN(t)) {
        return false;
    }

    compute_pos_tan(&fPts[seg->fPtIndex], seg->fType, t, pos, tangent);
    return true;
}

bool SkContourMeasure::getMatrix(SkScalar distance, SkMatrix* matrix, MatrixFlags flags) const {
    SkPoint     position;
    SkVector    tangent;

    if (this->getPosTan(distance, &position, &tangent)) {
        if (matrix) {
            if (flags & kGetTangent_MatrixFlag) {
                matrix->setSinCos(tangent.fY, tangent.fX, 0, 0);
            } else {
                matrix->reset();
            }
            if (flags & kGetPosition_MatrixFlag) {
                matrix->postTranslate(position.fX, position.fY);
            }
        }
        return true;
    }
    return false;
}

bool SkContourMeasure::getSegment(SkScalar startD, SkScalar stopD, SkPath* dst,
                                  bool startWithMoveTo) const {
    SkASSERT(dst);

    SkScalar length = this->length();

    if (startD < 0) {
        startD = 0;
    }
    if (stopD > length) {
        stopD = length;
    }
    if (!(startD <= stopD)) {   // catch NaN values as well
        return false;
    }
    if (!fSegments.count()) {
        return false;
    }

    SkPoint  p;
    SkScalar startT, stopT;
    const Segment* seg = this->distanceToSegment(startD, &startT);
    if (!SkScalarIsFinite(startT)) {
        return false;
    }
    const Segment* stopSeg = this->distanceToSegment(stopD, &stopT);
    if (!SkScalarIsFinite(stopT)) {
        return false;
    }
    SkASSERT(seg <= stopSeg);
    if (startWithMoveTo) {
        compute_pos_tan(&fPts[seg->fPtIndex], seg->fType, startT, &p, nullptr);
        dst->moveTo(p);
    }

    if (seg->fPtIndex == stopSeg->fPtIndex) {
        SkPathMeasure_segTo(&fPts[seg->fPtIndex], seg->fType, startT, stopT, dst);
    } else {
        do {
            SkPathMeasure_segTo(&fPts[seg->fPtIndex], seg->fType, startT, SK_Scalar1, dst);
            seg = SkPathMeasure::NextSegment(seg);
            startT = 0;
        } while (seg->fPtIndex < stopSeg->fPtIndex);
        SkPathMeasure_segTo(&fPts[seg->fPtIndex], seg->fType, 0, stopT, dst);
    }

    return true;
}
