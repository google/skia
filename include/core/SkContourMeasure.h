/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkContourMeasure_DEFINED
#define SkContourMeasure_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/private/base/SkAPI.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTDArray.h"

#include <cstddef>
#include <memory>

class SkMatrix;
class SkPath;
enum class SkPathVerb;

class SK_API SkContourMeasure : public SkRefCnt {
public:
    /** Return the length of the contour.
     */
    SkScalar length() const { return fLength; }

    /** Pins distance to 0 <= distance <= length(), and then computes the corresponding
     *  position and tangent.
     */
    [[nodiscard]] bool getPosTan(SkScalar distance, SkPoint* position, SkVector* tangent) const;

    enum MatrixFlags {
        kGetPosition_MatrixFlag     = 0x01,
        kGetTangent_MatrixFlag      = 0x02,
        kGetPosAndTan_MatrixFlag    = kGetPosition_MatrixFlag | kGetTangent_MatrixFlag
    };

    /** Pins distance to 0 <= distance <= getLength(), and then computes
     the corresponding matrix (by calling getPosTan).
     Returns false if there is no path, or a zero-length path was specified, in which case
     matrix is unchanged.
     */
    [[nodiscard]] bool getMatrix(SkScalar distance, SkMatrix* matrix,
                                 MatrixFlags flags = kGetPosAndTan_MatrixFlag) const;

    /** Given a start and stop distance, return in dst the intervening segment(s).
     If the segment is zero-length, return false, else return true.
     startD and stopD are pinned to legal values (0..getLength()). If startD > stopD
     then return false (and leave dst untouched).
     Begin the segment with a moveTo if startWithMoveTo is true
     */
    [[nodiscard]] bool getSegment(SkScalar startD, SkScalar stopD, SkPath* dst,
                                  bool startWithMoveTo) const;

    /** Return true if the contour is closed()
     */
    bool isClosed() const { return fIsClosed; }

    /** Measurement data for individual verbs.
     */
    struct VerbMeasure {
        SkScalar              fDistance; // Cumulative distance along the current contour.
        SkPathVerb            fVerb;     // Verb type.
        SkSpan<const SkPoint> fPts;      // Verb points.
    };

private:
    struct Segment;

public:
    /** Utility for iterating over the current contour verbs:
     *
     *   for (const auto verb_measure : contour_measure) {
     *     ...
     *   }
     */
    class ForwardVerbIterator final {
    public:
        VerbMeasure operator*() const;

        ForwardVerbIterator& operator++() {
            SkASSERT(!fSegments.empty());

            fSegments = LastSegForCurrentVerb(fSegments.subspan(1));

            return *this;
        }

        bool operator==(const ForwardVerbIterator& other) {
            SkASSERT(fSegments.data() != other.fSegments.data() ||
                     fSegments.size() == other.fSegments.size());
            return fSegments.data() == other.fSegments.data();
        }

        bool operator!=(const ForwardVerbIterator& other) {
            return !((*this) == other);
        }

    private:
        friend class SkContourMeasure;

        ForwardVerbIterator(SkSpan<const Segment> segs, SkSpan<const SkPoint> pts)
            : fSegments(LastSegForCurrentVerb(segs))
            , fPts(pts) {}

        static SkSpan<const Segment> LastSegForCurrentVerb(const SkSpan<const Segment>& segs) {
            size_t i = 1;
            while (i < segs.size() && segs[0].fPtIndex == segs[i].fPtIndex) {
                ++i;
            }

            return segs.subspan(i - 1);
        }

        // Remaining segments for forward iteration. The first segment in the span is
        // adjusted to always point to the last segment of the current verb, such that its distance
        // corresponds to the verb distance.
        SkSpan<const Segment> fSegments;

        // All path points (indexed in segments).
        SkSpan<const SkPoint> fPts;
    };

    ForwardVerbIterator begin() const {
        return ForwardVerbIterator(fSegments, fPts);
    }
    ForwardVerbIterator end() const {
        return ForwardVerbIterator(SkSpan(fSegments.end(), 0), fPts);
    }

private:
    struct Segment {
        SkScalar    fDistance;  // total distance up to this point
        unsigned    fPtIndex; // index into the fPts array
        unsigned    fTValue : 30;
        unsigned    fType : 2;  // actually the enum SkSegType
        // See SkPathMeasurePriv.h

        SkScalar getScalarT() const;

        static const Segment* Next(const Segment* seg) {
            unsigned ptIndex = seg->fPtIndex;
            do {
                ++seg;
            } while (seg->fPtIndex == ptIndex);
            return seg;
        }

    };

    const SkTDArray<Segment>  fSegments;
    const SkTDArray<SkPoint>  fPts; // Points used to define the segments

    const SkScalar fLength;
    const bool fIsClosed;

    SkContourMeasure(SkTDArray<Segment>&& segs, SkTDArray<SkPoint>&& pts,
                     SkScalar length, bool isClosed);
    ~SkContourMeasure() override {}

    const Segment* distanceToSegment(SkScalar distance, SkScalar* t) const;

    friend class SkContourMeasureIter;
    friend class SkPathMeasurePriv;
};

class SK_API SkContourMeasureIter {
public:
    SkContourMeasureIter();
    /**
     *  Initialize the Iter with a path.
     *  The parts of the path that are needed are copied, so the client is free to modify/delete
     *  the path after this call.
     *
     *  resScale controls the precision of the measure. values > 1 increase the
     *  precision (and possibly slow down the computation).
     */
    SkContourMeasureIter(const SkPath& path, bool forceClosed, SkScalar resScale = 1);
    ~SkContourMeasureIter();

    SkContourMeasureIter(SkContourMeasureIter&&);
    SkContourMeasureIter& operator=(SkContourMeasureIter&&);

    /**
     *  Reset the Iter with a path.
     *  The parts of the path that are needed are copied, so the client is free to modify/delete
     *  the path after this call.
     */
    void reset(const SkPath& path, bool forceClosed, SkScalar resScale = 1);

    /**
     *  Iterates through contours in path, returning a contour-measure object for each contour
     *  in the path. Returns null when it is done.
     *
     *  This only returns non-zero length contours, where a contour is the segments between
     *  a kMove_Verb and either ...
     *      - the next kMove_Verb
     *      - kClose_Verb (1 or more)
     *      - kDone_Verb
     *  If it encounters a zero-length contour, it is skipped.
     */
    sk_sp<SkContourMeasure> next();

private:
    class Impl;

    std::unique_ptr<Impl> fImpl;
};

#endif
