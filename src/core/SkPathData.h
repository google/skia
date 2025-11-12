/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathData_DEFINED
#define SkPathData_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"

#include "include/private/SkIDChangeListener.h"
#include "include/private/SkPathRef.h"
#include "src/core/SkPathEnums.h"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <optional>

struct SkPathRaw;

/*
 *  Immutable container for path geometry data: points, verbs, bounds, segment masks.
 *
 *  All of the public factories check for valid input
 *  - valid verb sequence
 *  - corresponding # points
 *  - finite point and conic values
 *
 *  If any of these checks fail, null is returned.
 *
 *  A valid sequence of verbs (and corresponding points/conics) is:
 *
 *  Any number of contours (0 or more)
 *  ... Each contour must begin with a single Move verb
 *      ... followed by any number of segments: lines, quads, conics, cubics (0 or more)
 *      ... followed by 0 or 1 Close verb
 *
 *  A minor exception to these rules, is that the last contour may end with a single
 *  Move verb -- this will be ignored in the resulting PathData.
 *
 *  Given a valid verb sequence, there must be the corresponding number of points and
 *  conics to match. If there are more or fewer, null is returned.
 *
 *  Verb    points  conic_weights
 *  -----------------------------
 *  Move    1       0
 *  Line    1       0
 *  Quad    2       0
 *  Conic   2       1
 *  Cubic   3       0
 *  Close   0       0
 */
class SkPathData : public SkNVRefCnt<SkPathData> {
public:
    ~SkPathData();

    /*
     *  Returns an empty pathdata.
     *
     *  Since this is immutable, it may return the same object each time it is called.
     */
    static sk_sp<SkPathData> Empty();

    /*
     *  Return SkPathData with a copy of these buffers, or nullptr if they are illegal.
     *  Illegal = non-finite, or non-sensical verb sequences
     */
    static sk_sp<SkPathData> Make(SkSpan<const SkPoint> pts,
                                  SkSpan<const SkPathVerb> verbs,
                                  SkSpan<const float> conics = {});

    /*
     *  Attempt to transform src by the matrix. On success, return a new SkPathData
     *  with the result, else return {}.
     */
    static sk_sp<SkPathData> MakeTransform(const SkPathRaw& src, const SkMatrix&);

    /*
     *  When a factory takes a startIndex, this refers to the position of the first point
     *  when constructing one of our simple shapes: rect, oval, rrect.
     *  The index is the same as that passed to the equivalent factories in SkPath
     *  and in the associated addRect/Oval/RRect methods on SkPathBuilder.
     */
    static sk_sp<SkPathData> Rect(const SkRect&,
                                  SkPathDirection = SkPathDirection::kDefault,
                                  unsigned startIndex = 0);
    static sk_sp<SkPathData> Oval(const SkRect&,
                                  SkPathDirection = SkPathDirection::kDefault,
                                  unsigned startIndex = 1);

    static sk_sp<SkPathData> RRect(const SkRRect&, SkPathDirection, unsigned startIndex);
    static sk_sp<SkPathData> RRect(const SkRRect& rrect,
                                   SkPathDirection dir = SkPathDirection::kDefault) {
        return RRect(rrect, dir, dir == SkPathDirection::kCW ? 6 : 7);
    }
    static sk_sp<SkPathData> Polygon(SkSpan<const SkPoint> pts, bool isClosed);
    static sk_sp<SkPathData> Line(SkPoint a, SkPoint b) {
        return Polygon({{a, b}}, false);
    }

    friend bool operator==(const SkPathData& a, const SkPathData& b);
    friend bool operator!=(const SkPathData& a, const SkPathData& b) {
        return !(a == b);
    }

    SkSpan<const SkPoint> points() const { return fPoints; }
    SkSpan<const SkPathVerb> verbs() const { return fVerbs; }
    SkSpan<const float> conics() const { return fConics; }
    const SkRect& bounds() const { return fBounds; }
    uint8_t segmentMask() const { return fSegmentMask; }

    // Will never be zero, has the low-2 bits always zero (to store filltype)
    uint32_t uniqueID() const { return fUniqueID; }

    SkPathRaw raw(SkPathFillType, SkResolveConvexity) const;

    /**
     * Return true if the path contains no points or verbs
     */
    bool empty() const { return fVerbs.empty(); }

    SkRect computeTightBounds() const;

    /**
     * Returns true if the pathdata is convex.
     * Note: if necessary, it will first compute the convexity (and cache it).
     */
    bool isConvex() const;

    /**
     * Returns two points if SkPath contains only one line. If not, return {}.
     */
    std::optional<std::array<SkPoint, 2>> asLine() const;

    /**
     * If the pathdata is recognized as a rect, return it and its direction and open/closed.
     * If not, return {}
     */
    std::optional<SkPathRectInfo> asRect() const;

    /**
     * If the path is recognized as an oval, return its bounds. If not, return {}.
     */
    std::optional<SkPathOvalInfo> asOval() const;

    /**
     * If the path is recognized as a round-rect, return it. If not, return {}.
     */
    std::optional<SkPathRRectInfo> asRRect() const;

    /**
     *  Attempt to transform the pathdata by the matrix. If this succeeds, return a new
     *  pathdata object: note, this may have different verbs / number-of-points, if the
     *  matrix contained perspective.
     *
     *  If the matrix has no effect on the coordinates (e.g. it is identity), then this may
     *  return a ref to the same pathdata object.
     *
     *  If the result of applying the matrix creates any non-finite coordinates, this returns
     *  nullptr.
     */
    sk_sp<SkPathData> makeTransform(const SkMatrix&) const;
    sk_sp<SkPathData> makeOffset(SkVector) const;

    bool contains(SkPoint, SkPathFillType) const;

    void addGenIDChangeListener(sk_sp<SkIDChangeListener>) const;
    int genIDChangeListenerCount() const { return fGenIDChangeListeners.count(); }

private:
    friend class SkNVRefCnt<SkPathData>;
    friend class SkPathPriv;
    friend class SkPath;
    friend class SkPathBuilder;

    // notify these in our destructor
    mutable SkIDChangeListener::List fGenIDChangeListeners;

    SkSpan<SkPoint>    fPoints;
    SkSpan<float>      fConics;
    SkSpan<SkPathVerb> fVerbs;
    SkRect             fBounds;

    uint32_t           fUniqueID;   // never 0

    /*
     *  Convexity can be slow to compute, and (in theory) it can't always survive a matrix
     *  transform (due to numeric instability). Therefore we will lazily compute it as
     *  requested. Since we are technically always immutable, we have to store this field
     *  in an atomic.
     */
    mutable std::atomic<uint8_t> fConvexity;    // SkPathConvexity
    uint8_t                      fSegmentMask;  // SkPathSegmentMask
    SkPathIsAType                fType;
    SkPathIsAData                fIsA {};

    //
    // Memory layout after this (assuming we're not empty)
    //
    //  [point data]
    //  [conic data]
    //  [verb  data]
    //

    SkPathData(size_t npts, size_t nvbs, size_t ncns);

    // Ensure the unsized delete is called (since we're manually allocating the storage)
    void operator delete(void* p);

    // internal finisher when building a PathData.
    // If the optional value is not present, it will be computed (else checked in debug mode).
    //
    // In particular, if bounds is not provided, it will be computed, and if it proves
    // to be non-finite, false will be returned.
    bool finishInit(std::optional<SkRect> bounds, std::optional<uint8_t> segmentMask);

    // If we know we're a special shape, call this after the normal initialization
    void setupIsA(SkPathIsAType, SkPathDirection dir, unsigned startIndex);

    SkPathConvexity getConvexityOrUnknown() const;          // may return kUnknown
    SkPathConvexity getResolvedConvexity() const;           // never returns kUnknown
    void setConvexity(SkPathConvexity) const;               // const -- but convexity is mutable

    static SkPathData* PeekEmptySingleton();

    static sk_sp<SkPathData> Alloc(size_t npts, size_t nvbs, size_t ncns);

    static sk_sp<SkPathData> MakeNoCheck(SkSpan<const SkPoint> pts,
                                         SkSpan<const SkPathVerb> verbs,
                                         SkSpan<const float> conics,
                                         std::optional<SkRect> bounds,
                                         std::optional<unsigned> segmentMask);
    static sk_sp<SkPathData> MakeNoCheck(const SkPathRaw&);
};

#endif
