/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkROPath_DEFINED
#define SkROPath_DEFINED

#include "SkMatrix.h"
#include "SkTDArray.h"
#include "SkPathTypes.h"
#include "SkRefCnt.h"

class SkReader32;
class SkWriter32;
class SkString;
class SkRRect;
class SkWStream;

class SkROPath : public SkRefCnt {
public:
    SkROPath();
    ~SkROPath();

    SkPathFillType fillType() const { return SkPathFillType(fFillType); }
    SkPathConvexityType convexityType() const { return SkPathConvexityType(fConvexityType); }
    SkPathSegmentMask segmentMask() const { return SkPathSegmentMask(fSegmentMask); }

    bool isEmpty() const { return 0 == this->segmentMask(); }
    SkRect bounds() const { return fBounds; }
    uint32_t uniqueID() const { return fUniqueID; }

    /**
     *  Returns true if the path specifies a single line (i.e. it contains just
     *  a moveTo and a lineTo). If so, and line[] is not null, it sets the 2
     *  points in line[] to the end-points of the line. If the path is not a
     *  line, returns false and ignores line[].
     */
    bool isLine(SkPoint line[2]) const;

    /**
     * Does a conservative test to see whether a rectangle is inside a path. Currently it only
     * will ever return true for single convex contour paths. The empty-status of the rect is not
     * considered (e.g. a rect that is a point can be inside a path). Points or line segments where
     * the rect edge touches the path border are not considered containment violations.
     */
    bool conservativelyContainsRect(const SkRect& rect) const;

    sk_sp<SkROPath> transform(const SkMatrix&) const;

    /** Iterate through all of the segments (lines, quadratics, cubics) of
     each contours in a path.

     The iterator cleans up the segments along the way, removing degenerate
     segments and adding close verbs where necessary. When the forceClose
     argument is provided, each contour (as defined by a new starting
     move command) will be completed with a close verb regardless of the
     contour's contents.
     */
    class SK_API Iter {
    public:
        Iter(const SkROPath*, bool forceClose);

        /** Return the next verb in this iteration of the path. When all
         segments have been visited, return kDone_Verb.

         @param  pts The points representing the current verb and/or segment
         @param doConsumeDegerates If true, first scan for segments that are
         deemed degenerate (too short) and skip those.
         @return The verb for the current segment
         */
        SkPathVerb next(SkPoint pts[4], bool doConsumeDegerates = true) {
            if (doConsumeDegerates) {
                this->consumeDegenerateSegments();
            }
            return this->doNext(pts);
        }

        /**
         *  Return the weight for the current conic. Only valid if the current
         *  segment return by next() was a conic.
         */
        SkScalar conicWeight() const { return *fConicWeights; }

        /** If next() returns kLine_Verb, then this query returns true if the
         line was the result of a close() command (i.e. the end point is the
         initial moveto for this contour). If next() returned a different
         verb, this returns an undefined value.

         @return If the last call to next() returned kLine_Verb, return true
         if it was the result of an explicit close command.
         */
        bool isCloseLine() const { return SkToBool(fCloseLine); }

        /** Returns true if the current contour is closed (has a kClose_Verb)
         @return true if the current contour is closed (has a kClose_Verb)
         */
        bool isClosedContour() const;

    private:
        const SkPoint*  fPts;
        const uint8_t*  fVerbs;
        const uint8_t*  fVerbStop;
        const SkScalar* fConicWeights;
        SkPoint         fMoveTo;
        SkPoint         fLastPt;
        bool            fForceClose;
        bool            fNeedClose;
        bool            fCloseLine;
        bool            fSegmentState;

        inline const SkPoint& cons_moveTo();
        SkPathVerb autoClose(SkPoint pts[2]);
        void consumeDegenerateSegments();
        SkPathVerb doNext(SkPoint pts[4]);
    };

    /** Iterate through the verbs in the path, providing the associated points.
     */
    class SK_API RawIter {
    public:
        RawIter(const SkROPath*);

        /** Return the next verb in this iteration of the path. When all
         segments have been visited, return kDone_Verb.

         @param  pts The points representing the current verb and/or segment
         This must not be NULL.
         @return The verb for the current segment
         */
        SkPathVerb next(SkPoint pts[4]);

        SkScalar conicWeight() const { return *fConicWeights; }

    private:
        const SkPoint*  fPts;
        const uint8_t*  fVerbs;
        const uint8_t*  fVerbStop;
        const SkScalar* fConicWeights;
        SkPoint         fMoveTo;
        SkPoint         fLastPt;
    };

    /**
     *  Returns true if the point { x, y } is contained by the path, taking into
     *  account the FillType.
     */
    bool contains(SkScalar x, SkScalar y) const;

    void dump(SkWStream* , bool forceClose, bool dumpAsHex) const;
    void dump() const;
    void dumpHex() const;

    /**
     *  Write the path to the buffer, and return the number of bytes written.
     *  If buffer is NULL, it still returns the number of bytes.
     */
    size_t writeToMemory(void* buffer) const;
    /**
     * Initializes the path from the buffer
     *
     * @param buffer Memory to read from
     * @param length Amount of memory available in the buffer
     * @return number of bytes read (must be a multiple of 4) or
     *         0 if there was not enough memory available
     */
    size_t readFromMemory(const void* buffer, size_t length);

private:
    SkTDArray<SkPoint>  fPts;
    SkTDArray<char>     fVerbs;
    SkTDArray<SkScalar> fConicWeights;

    SkRect      fBounds;
    uint32_t    fUniqueID;
    uint8_t     fFillType;
    uint8_t     fConvexityType;
    uint8_t     fSegmentMask;

    SkROPath(SkTDArray<SkPoint>&& pts, SkTDArray<char>&& verbs, SkTDArray<SkScalar>&& weights);

    friend class SkPathBuilder;
};

#endif
