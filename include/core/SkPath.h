/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPath_DEFINED
#define SkPath_DEFINED

#include "SkMatrix.h"
#include "../private/SkPathRef.h"

class SkAutoPathBoundsUpdate;
class SkData;
class SkRRect;
class SkWStream;

/** \class SkPath
*/
class SK_API SkPath {
public:

/** \enum SkPath::Direction
    Direction describes whether contour is clockwise or counterclockwise.
    When SkPath contains multiple overlapping contours, Direction together with
    FillType determines whether overlaps are filled or form holes.

    Direction also determines how contour is measured. For instance, dashing
    measures along SkPath to determine where to start and stop stroke; Direction
    will change dashed results as it steps clockwise or counterclockwise.

    Closed contours like SkRect, SkRRect, circles, and ovals added with
    kCW_Direction travel clockwise; the same added with kCCW_Direction
    travel counterclockwise.
*/
enum Direction {
    kCW_Direction,  //!< Contour travels in a clockwise direction.
    kCCW_Direction, //!< Contour travels in a counterclockwise direction.
};

    /** By default, SkPath has no SkPath::Verb, no SkPoint, and no weights.
        SkPath::FillType is set to kWinding_FillType.

        @return  empty SkPath
    */
    SkPath();

    /** Copy constructor makes two paths identical by value. Internally, path and
        the returned result share pointer values. The underlying verb array, SkPoint arrays
        and weights are copied when modified.

        Creating a SkPath copy is very efficient and never allocates memory.
        SkPath are always copied by value from the interface; the underlying shared
        pointers are not exposed.

        @param path  SkPath to copy by value
        @return      Copy of SkPath
    */
    SkPath(const SkPath& path);

    /** Releases ownership of any shared data and deletes data if SkPath is sole owner.
    */
    ~SkPath();

    /** SkPath assignment makes two paths identical by value. Internally, assignment
        shares pointer values. The underlying verb array, SkPoint arrays and weights
        are copied when modified.

        Copying SkPath by assignment is very efficient and never allocates memory.
        SkPath are always copied by value from the interface; the underlying shared
        pointers are not exposed.

        @param path  verb array, SkPoint arrays, weights, amd SkPath::FillType to copy
        @return      SkPath copied by value
    */
    SkPath& operator=(const SkPath& path);

    /** Compares a and b; returns true if SkPath::FillType, verb array, SkPoint arrays, and weights
        are equivalent.

        @param a  SkPath to compare
        @param b  SkPath to compare
        @return   true if SkPath pair are equivalent
    */
    friend SK_API bool operator==(const SkPath& a, const SkPath& b);

    /** Compares a and b; returns true if SkPath::FillType, verb array, SkPoint arrays, and weights
        are not equivalent.

        @param a  SkPath to compare
        @param b  SkPath to compare
        @return   true if SkPath pair are not equivalent
    */
    friend bool operator!=(const SkPath& a, const SkPath& b) {
        return !(a == b);
    }

    /** Return true if SkPath contain equal SkPath::Verb and equal weights.
        If SkPath contain one or more conics, the weights must match.

        conicTo() may add different SkPath::Verb depending on weights, so it is not
        trival to interpolate a pair of SkPath containing conics with different
        weights values.

        @param compare  SkPath to compare
        @return         true if SkPath verb array and weights are equivalent
    */
    bool isInterpolatable(const SkPath& compare) const;

    /** Interpolate between SkPath with equal sized point arrays.
        Copy verb array and weights to out,
        and set out SkPoint arrays to a weighted average of this SkPoint arrays and ending
        SkPoint arrays, using the formula:

        @param ending  SkPoint arrays averaged with this SkPoint arrays
        @param weight  contribution of ending SkPoint arrays, and
                       one minus contribution of this SkPoint arrays
        @param out     SkPath replaced by interpolated averages
        @return        true if SkPath contain same number of SkPoint
    */
    bool interpolate(const SkPath& ending, SkScalar weight, SkPath* out) const;

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    /** To be deprecated; only valid for Android framework.

        @return  true if SkPath has one owner
    */
    bool unique() const { return fPathRef->unique(); }
#endif

    /** \enum SkPath::FillType
        FillType selects the rule used to fill SkPath. SkPath set to kWinding_FillType
        fills if the sum of contour edges is not zero, where clockwise edges add one, and
        counterclockwise edges subtract one. SkPath set to kEvenOdd_FillType fills if the
        number of contour edges is odd. Each FillType has an inverse variant that
        reverses the rule:
        kInverseWinding_FillType fills where the sum of contour edges is zero;
        kInverseEvenOdd_FillType fills where the number of contour edges is even.
    */
    enum FillType {
        /** Specifies fill as area is enclosed by a non-zero sum of contour Direction. */
        kWinding_FillType,

        /** Specifies fill as area enclosed by an odd number of contours. */
        kEvenOdd_FillType,

        /** Specifies fill as area is enclosed by a zero sum of contour Direction. */
        kInverseWinding_FillType,

        /** Specifies fill as area enclosed by an even number of contours. */
        kInverseEvenOdd_FillType,
    };

    /** Returns FillType, the rule used to fill SkPath. FillType of a new SkPath is
        kWinding_FillType.

        @return  one of: kWinding_FillType, kEvenOdd_FillType,  kInverseWinding_FillType,
                 kInverseEvenOdd_FillType
    */
    FillType getFillType() const { return (FillType)fFillType; }

    /** Sets FillType, the rule used to fill SkPath. While there is no check
        that ft is legal, values outside of FillType are not supported.

        @param ft  one of: kWinding_FillType, kEvenOdd_FillType,  kInverseWinding_FillType,
                   kInverseEvenOdd_FillType
    */
    void setFillType(FillType ft) {
        fFillType = SkToU8(ft);
    }

    /** Returns if FillType describes area outside SkPath geometry. The inverse fill area
        extends indefinitely.

        @return  true if FillType is kInverseWinding_FillType or kInverseEvenOdd_FillType
    */
    bool isInverseFillType() const { return IsInverseFillType((FillType)fFillType); }

    /** Replace FillType with its inverse. The inverse of FillType describes the area
        unmodified by the original FillType.
    */
    void toggleInverseFillType() {
        fFillType ^= 2;
    }

    /** \enum SkPath::Convexity
        SkPath is convex if it contains one contour and contour loops no more than
        360 degrees, and contour angles all have same Direction. Convex SkPath
        may have better performance and require fewer resources on GPU surface.

        SkPath is concave when either at least one Direction change is clockwise and
        another is counterclockwise, or the sum of the changes in Direction is not 360
        degrees.

        Initially SkPath Convexity is kUnknown_Convexity. SkPath Convexity is computed
        if needed by destination SkSurface.
    */
    enum Convexity {
        kUnknown_Convexity, //!< Indicates Convexity has not been determined.

        /** SkPath has one contour made of a simple geometry without indentations. */
        kConvex_Convexity,
        kConcave_Convexity, //!< SkPath has more than one contour, or a geometry with indentations.
    };

    /** Computes SkPath::Convexity if required, and returns stored value.
        SkPath::Convexity is computed if stored value is kUnknown_Convexity,
        or if SkPath has been altered since SkPath::Convexity was computed or set.

        @return  Computed or stored SkPath::Convexity
    */
    Convexity getConvexity() const {
        if (kUnknown_Convexity != fConvexity) {
            return static_cast<Convexity>(fConvexity);
        } else {
            return this->internalGetConvexity();
        }
    }

    /** Returns last computed SkPath::Convexity, or kUnknown_Convexity if
        SkPath has been altered since SkPath::Convexity was computed or set.

        @return  Stored SkPath::Convexity
    */
    Convexity getConvexityOrUnknown() const { return (Convexity)fConvexity; }

    /** Stores convexity so that it is later returned by getConvexity() or getConvexityOrUnknown().
        convexity may differ from getConvexity(), although setting an incorrect value may
        cause incorrect or inefficient drawing.

        If convexity is kUnknown_Convexity: getConvexity() will
        compute SkPath::Convexity, and getConvexityOrUnknown() will return kUnknown_Convexity.

        If convexity is kConvex_Convexity or kConcave_Convexity, getConvexity()
        and getConvexityOrUnknown() will return convexity until the path is
        altered.

        @param convexity  one of: kUnknown_Convexity, kConvex_Convexity, or kConcave_Convexity
    */
    void setConvexity(Convexity convexity);

    /** Computes SkPath::Convexity if required, and returns true if value is kConvex_Convexity.
        If setConvexity() was called with kConvex_Convexity or kConcave_Convexity, and
        the path has not been altered, SkPath::Convexity is not recomputed.

        @return  true if SkPath::Convexity stored or computed is kConvex_Convexity
    */
    bool isConvex() const {
        return kConvex_Convexity == this->getConvexity();
    }

    /** Deprecated. Use setConvexity().
    */
    SK_ATTR_DEPRECATED("use setConvexity")
    void setIsConvex(bool isConvex) {
        this->setConvexity(isConvex ? kConvex_Convexity : kConcave_Convexity);
    }

    /** Returns true if constructed by addCircle(), addOval(); and in some cases,
        addRoundRect(), addRRect(). SkPath constructed with conicTo() or rConicTo() will not
        return true though SkPath draws ovals.

        rect receives bounds of ovals.
        dir receives SkPath::Direction of ovals: kCW_Direction if clockwise, kCCW_Direction if
        counterclockwise.
        start receives start of ovals: 0 for top, 1 for right, 2 for bottom, 3 for left.

        rect, dir, and start are unmodified if ovals is not found.

        Triggers performance optimizations on some GPU surface implementations.

        @param rect   storage for bounding SkRect of ovals; may be nullptr
        @param dir    storage for SkPath::Direction; may be nullptr
        @param start  storage for start of ovals; may be nullptr
        @return       true if SkPath was constructed by method that reduces to ovals
    */
    bool isOval(SkRect* rect, Direction* dir = nullptr,
                unsigned* start = nullptr) const {
        bool isCCW = false;
        bool result = fPathRef->isOval(rect, &isCCW, start);
        if (dir && result) {
            *dir = isCCW ? kCCW_Direction : kCW_Direction;
        }
        return result;
    }

    /** Returns true if constructed by addRoundRect(), addRRect(); and if construction
        is not empty, not SkRect, and not ovals. SkPath constructed with other other calls
        will not return true though SkPath draws SkRRect.

        rrect receives bounds of SkRRect.
        dir receives SkPath::Direction of ovals: kCW_Direction if clockwise, kCCW_Direction if
        counterclockwise.
        start receives start of SkRRect: 0 for top, 1 for right, 2 for bottom, 3 for left.

        rrect, dir, and start are unmodified if SkRRect is not found.

        Triggers performance optimizations on some GPU surface implementations.

        @param rrect  storage for bounding SkRect of SkRRect; may be nullptr
        @param dir    storage for SkPath::Direction; may be nullptr
        @param start  storage for start of SkRRect; may be nullptr
        @return       true for SkRRect SkPath constructed by addRoundRect() or addRRect()
    */
    bool isRRect(SkRRect* rrect, Direction* dir = nullptr,
                 unsigned* start = nullptr) const {
        bool isCCW = false;
        bool result = fPathRef->isRRect(rrect, &isCCW, start);
        if (dir && result) {
            *dir = isCCW ? kCCW_Direction : kCW_Direction;
        }
        return result;
    }

    /** Sets SkPath to its intial state.
        Removes verb array, SkPoint arrays, and weights, and sets FillType to kWinding_FillType.
        Internal storage associated with SkPath is released.
    */
    void reset();

    /** Sets SkPath to its intial state, preserving internal storage.
        Removes verb array, SkPoint arrays, and weights, and sets FillType to kWinding_FillType.
        Internal storage associated with SkPath is retained.

        Use rewind() instead of reset() if SkPath storage will be reused and performance
        is critical.
    */
    void rewind();

    /** Empty SkPath may have FillType but has no SkPoint, SkPath::Verb, or weights.
        SkPath() constructs empty SkPath; reset() and (rewind) make SkPath empty.

        @return  true if the path contains no SkPath::Verb array
    */
    bool isEmpty() const {
        SkDEBUGCODE(this->validate();)
        return 0 == fPathRef->countVerbs();
    }

    /** Contour is closed if SkPath SkPath::Verb array was last modified by close(). When stroked,
        closed contour draws SkPaint::Join instead of SkPaint::Cap at first and last SkPoint.

        @return  true if the last contour ends with a kClose_Verb
    */
    bool isLastContourClosed() const;

    /** Returns true for finite SkPoint array values between negative SK_ScalarMax and
        positive SK_ScalarMax. Returns false for any SkPoint array value of
        SK_ScalarInfinity, SK_ScalarNegativeInfinity, or SK_ScalarNaN.

        @return  true if all SkPoint values are finite
    */
    bool isFinite() const {
        SkDEBUGCODE(this->validate();)
        return fPathRef->isFinite();
    }

    /** Returns true if the path is volatile; it will not be altered or discarded
        by the caller after it is drawn. SkPath by default have volatile set false, allowing
        SkSurface to attach a cache of data which speeds repeated drawing. If true, SkSurface
        may not speed repeated drawing.

        @return  true if caller will alter SkPath after drawing
    */
    bool isVolatile() const {
        return SkToBool(fIsVolatile);
    }

    /** Specify whether SkPath is volatile; whether it will be altered or discarded
        by the caller after it is drawn. SkPath by default have volatile set false, allowing
        SkBaseDevice to attach a cache of data which speeds repeated drawing.

        Mark temporary paths, discarded or modified after use, as volatile
        to inform SkBaseDevice that the path need not be cached.

        Mark animating SkPath volatile to improve performance.
        Mark unchanging SkPath non-volative to improve repeated rendering.

        raster surface SkPath draws are affected by volatile for some shadows.
        GPU surface SkPath draws are affected by volatile for some shadows and concave geometries.

        @param isVolatile  true if caller will alter SkPath after drawing
    */
    void setIsVolatile(bool isVolatile) {
        fIsVolatile = isVolatile;
    }

    /** Test if lines between SkPoint pair is degenerate.
        Lines with no length or that moves a very short distance is degenerate; it is
        treated as a point.

        @param p1     Lines start point
        @param p2     Lines end point
        @param exact  If true, returns true only if p1 equals p2. If false, returns true
                      if p1 equals or nearly equals p2
        @return       true if lines is degenerate; its length is effectively zero
    */
    static bool IsLineDegenerate(const SkPoint& p1, const SkPoint& p2, bool exact) {
        return exact ? p1 == p2 : p1.equalsWithinTolerance(p2);
    }

    /** Test if quads is degenerate.
        Quads with no length or that moves a very short distance is degenerate; it is
        treated as a point.

        @param p1     Quads start point
        @param p2     Quads control point
        @param p3     Quads end point
        @param exact  If true, returns true only if p1, p2, and p3 are equal.
                      If false, returns true if p1, p2, and p3 are equal or nearly equal
        @return       true if quads is degenerate; its length is effectively zero
    */
    static bool IsQuadDegenerate(const SkPoint& p1, const SkPoint& p2,
                                 const SkPoint& p3, bool exact) {
        return exact ? p1 == p2 && p2 == p3 : p1.equalsWithinTolerance(p2) &&
               p2.equalsWithinTolerance(p3);
    }

    /** Test if cubics is degenerate.
        Cubics with no length or that moves a very short distance is degenerate; it is
        treated as a point.

        @param p1     Cubics start point
        @param p2     Cubics control point 1
        @param p3     Cubics control point 2
        @param p4     Cubics end point
        @param exact  If true, returns true only if p1, p2, p3, and p4 are equal.
                      If false, returns true if p1, p2, p3, and p4 are equal or nearly equal
        @return       true if cubics is degenerate; its length is effectively zero
    */
    static bool IsCubicDegenerate(const SkPoint& p1, const SkPoint& p2,
                                  const SkPoint& p3, const SkPoint& p4, bool exact) {
        return exact ? p1 == p2 && p2 == p3 && p3 == p4 : p1.equalsWithinTolerance(p2) &&
               p2.equalsWithinTolerance(p3) &&
               p3.equalsWithinTolerance(p4);
    }

    /** Returns true if SkPath contains only one lines;
        SkPath::Verb array has two entries: kMove_Verb, kLine_Verb.
        If SkPath contains one lines and line is not nullptr, line is set to
        lines start point and lines end point.
        Returns false if SkPath is not one lines; line is unaltered.

        @param line  storage for lines. May be nullptr
        @return      true if SkPath contains exactly one lines
    */
    bool isLine(SkPoint line[2]) const;

    /** Returns the number of points in SkPath.
        SkPoint count is initially zero.

        @return  SkPath SkPoint array length
    */
    int countPoints() const;

    /** Returns SkPoint at index in SkPoint arrays. Valid range for index is
        0 to countPoints() - 1.
        Returns (0, 0) if index is out of range.

        @param index  SkPoint arrays element selector
        @return       SkPoint arrays value or (0, 0)
    */
    SkPoint getPoint(int index) const;

    /** Returns number of points in SkPath. Up to max points are copied.
        points may be nullptr; then, max must be zero.
        If max is greater than number of points, excess points storage is unaltered.

        @param points  storage for SkPath SkPoint array. May be nullptr
        @param max     Number of points alloted in points storage; must be greater than or equal to zero
        @return        SkPath SkPoint array length
    */
    int getPoints(SkPoint points[], int max) const;

    /** Returns the number of SkPath::Verb: kMove_Verb, kLine_Verb, kQuad_Verb, kConic_Verb,
        kCubic_Verb, and kClose_Verb; added to SkPath.

        @return  Length of verb array
    */
    int countVerbs() const;

    /** Returns the number of verbs in the path. Up to max verbs are copied. The
        verbs are copied as one byte per verb.

        @param verbs  If not null, receives up to max verbs
        @param max    The maximum number of verbs to copy into verbs
        @return       the actual number of verbs in the path
    */
    int getVerbs(uint8_t verbs[], int max) const;

    /** Exchanges the verb array, SkPoint arrays, weights, and SkPath::FillType with other.
        Cached state is also exchanged. swap() internally exchanges pointers, so
        it is lightweight and does not allocate memory.

        swap() usage has largely been replaced by operator=(const SkPath& path).
        SkPath do not copy their content on assignment util they are written to,
        making assignment as efficient as swap().

        @param other  SkPath exchanged by value
    */
    void swap(SkPath& other);

    /** Returns minimum and maximum x and y values of SkPoint arrays.
        Returns (0, 0, 0, 0) if SkPath contains no points. Returned bounds width and height may
        be larger or smaller than area affected when SkPath is drawn.

        SkRect returned includes all SkPoint added to SkPath, including SkPoint associated with
        kMove_Verb that define empty contours.

        @return  bounds of all SkPoint in SkPoint arrays
    */
    const SkRect& getBounds() const {
        return fPathRef->getBounds();
    }

    /** Update internal bounds so that subsequent calls to getBounds() are instantaneous.
        Unaltered copies of SkPath may also access cached bounds through getBounds().

        For now, identical to calling getBounds() and ignoring the returned value.

        Call to prepare SkPath subsequently drawn from multiple threads,
        to avoid a race condition where each draw separately computes the bounds.
    */
    void updateBoundsCache() const {
        // for now, just calling getBounds() is sufficient
        this->getBounds();
    }

    /** Returns minimum and maximum x and y values of the lines and curves in SkPath.
        Returns (0, 0, 0, 0) if SkPath contains no points.
        Returned bounds width and height may be larger or smaller than area affected
        when SkPath is drawn.

        Includes SkPoint associated with kMove_Verb that define empty
        contours.

        Behaves identically to getBounds() when SkPath contains
        only lines. If SkPath contains curves, computed bounds includes
        the maximum extent of the quads, conics, or cubics; is slower than getBounds();
        and unlike getBounds(), does not cache the result.

        @return  tight bounds of curves in SkPath
    */
    SkRect computeTightBounds() const;

    /** Returns true if rect is contained by SkPath.
        May return false when rect is contained by SkPath.

        For now, only returns true if SkPath has one contour and is convex.
        rect may share points and edges with SkPath and be contained.
        Returns true if rect is empty, that is, it has zero width or height; and
        the SkPoint or lines described by rect is contained by SkPath.

        @param rect  SkRect, lines, or SkPoint checked for containment
        @return      true if rect is contained
    */
    bool conservativelyContainsRect(const SkRect& rect) const;

    /** grows SkPath verb array and SkPoint arrays to contain extraPtCount additional SkPoint.
        May improve performance and use less memory by
        reducing the number and size of allocations when creating SkPath.

        @param extraPtCount  number of additional SkPoint to preallocate
    */
    void incReserve(unsigned extraPtCount);

    /** Adds beginning of contour at SkPoint (x, y).

        @param x  x-coordinate of contour start
        @param y  y-coordinate of contour start
    */
    void moveTo(SkScalar x, SkScalar y);

    /** Adds beginning of contour at SkPoint p.

        @param p  Contour start
    */
    void moveTo(const SkPoint& p) {
        this->moveTo(p.fX, p.fY);
    }

    /** Adds beginning of contour relative to last point.
        If SkPath is empty, starts contour at (dx, dy).
        Otherwise, start contour at last point offset by (dx, dy).
        Function name stands for relative move to.

        @param dx  offset from last point x to contour start x
        @param dy  offset from last point y to contour start y
    */
    void rMoveTo(SkScalar dx, SkScalar dy);

    /** Adds lines from last point to (x, y). If SkPath is empty, or last SkPath::Verb is
        kClose_Verb, last point is set to (0, 0) before adding lines.

        lineTo() appends kMove_Verb to verb array and (0, 0) to SkPoint arrays, if needed.
        lineTo() then appends kLine_Verb to verb array and (x, y) to SkPoint arrays.

        @param x  end of added lines in x
        @param y  end of added lines in y
    */
    void lineTo(SkScalar x, SkScalar y);

    /** Adds lines from last point to SkPoint p. If SkPath is empty, or last SkPath::Verb is
        kClose_Verb, last point is set to (0, 0) before adding lines.

        lineTo() first appends kMove_Verb to verb array and (0, 0) to SkPoint arrays, if needed.
        lineTo() then appends kLine_Verb to verb array and SkPoint p to SkPoint arrays.

        @param p  end SkPoint of added lines
    */
    void lineTo(const SkPoint& p) {
        this->lineTo(p.fX, p.fY);
    }

    /** Adds lines from last point to SkVector (dx, dy). If SkPath is empty, or last SkPath::Verb is
        kClose_Verb, last point is set to (0, 0) before adding lines.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint arrays, if needed;
        then appends kLine_Verb to verb array and lines end to SkPoint arrays.
        Lines end is last point plus SkVector (dx, dy).
        Function name stands for relative line to.

        @param dx  offset from last point x to lines end x
        @param dy  offset from last point y to lines end y
    */
    void rLineTo(SkScalar dx, SkScalar dy);

    /** Adds quads from last point towards (x1, y1), to (x2, y2).
        If SkPath is empty, or last SkPath::Verb is kClose_Verb, last point is set to (0, 0)
        before adding quads.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint arrays, if needed;
        then appends kQuad_Verb to verb array; and (x1, y1), (x2, y2)
        to SkPoint arrays.

        @param x1  control SkPoint of quads in x
        @param y1  control SkPoint of quads in y
        @param x2  end SkPoint of quads in x
        @param y2  end SkPoint of quads in y
    */
    void quadTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2);

    /** Adds quads from last point towards SkPoint p1, to SkPoint p2.
        If SkPath is empty, or last SkPath::Verb is kClose_Verb, last point is set to (0, 0)
        before adding quads.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint arrays, if needed;
        then appends kQuad_Verb to verb array; and SkPoint p1, p2
        to SkPoint arrays.

        @param p1  control SkPoint of added quads
        @param p2  end SkPoint of added quads
    */
    void quadTo(const SkPoint& p1, const SkPoint& p2) {
        this->quadTo(p1.fX, p1.fY, p2.fX, p2.fY);
    }

    /** Adds quads from last point towards SkVector (dx1, dy1), to SkVector (dx2, dy2).
        If SkPath is empty, or last SkPath::Verb
        is kClose_Verb, last point is set to (0, 0) before adding quads.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint arrays,
        if needed; then appends kQuad_Verb to verb array; and appends quads
        control and quads end to SkPoint arrays.
        Quads control is last point plus SkVector (dx1, dy1).
        Quads end is last point plus SkVector (dx2, dy2).
        Function name stands for relative quad to.

        @param dx1  offset from last point x to quads control x
        @param dy1  offset from last point x to quads control y
        @param dx2  offset from last point x to quads end x
        @param dy2  offset from last point x to quads end y
    */
    void rQuadTo(SkScalar dx1, SkScalar dy1, SkScalar dx2, SkScalar dy2);

    /** Adds conics from last point towards (x1, y1), to (x2, y2), weighted by w.
        If SkPath is empty, or last SkPath::Verb is kClose_Verb, last point is set to (0, 0)
        before adding conics.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint arrays, if needed.

        If w is finite and not one, appends kConic_Verb to verb array;
        and (x1, y1), (x2, y2) to SkPoint arrays; and w to weights.

        If w is one, appends kQuad_Verb to verb array, and
        (x1, y1), (x2, y2) to SkPoint arrays.

        If w is not finite, appends kLine_Verb twice to verb array, and
        (x1, y1), (x2, y2) to SkPoint arrays.

        @param x1  control SkPoint of conics in x
        @param y1  control SkPoint of conics in y
        @param x2  end SkPoint of conics in x
        @param y2  end SkPoint of conics in y
        @param w   weight of added conics
    */
    void conicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                 SkScalar w);

    /** Adds conics from last point towards SkPoint p1, to SkPoint p2, weighted by w.
        If SkPath is empty, or last SkPath::Verb is kClose_Verb, last point is set to (0, 0)
        before adding conics.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint arrays, if needed.

        If w is finite and not one, appends kConic_Verb to verb array;
        and SkPoint p1, p2 to SkPoint arrays; and w to weights.

        If w is one, appends kQuad_Verb to verb array, and SkPoint p1, p2
        to SkPoint arrays.

        If w is not finite, appends kLine_Verb twice to verb array, and
        SkPoint p1, p2 to SkPoint arrays.

        @param p1  control SkPoint of added conics
        @param p2  end SkPoint of added conics
        @param w   weight of added conics
    */
    void conicTo(const SkPoint& p1, const SkPoint& p2, SkScalar w) {
        this->conicTo(p1.fX, p1.fY, p2.fX, p2.fY, w);
    }

    /** Adds conics from last point towards SkVector (dx1, dy1), to SkVector (dx2, dy2),
        weighted by w. If SkPath is empty, or last SkPath::Verb
        is kClose_Verb, last point is set to (0, 0) before adding conics.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint arrays,
        if needed.

        If w is finite and not one, next appends kConic_Verb to verb array,
        and w is recorded as weights; otherwise, if w is one, appends
        kQuad_Verb to verb array; or if w is not finite, appends kLine_Verb
        twice to verb array.

        In all cases appends SkPoint control and end to SkPoint arrays.
        control is last point plus SkVector (dx1, dy1).
        end is last point plus SkVector (dx2, dy2).

        Function name stands for relative conic to.

        @param dx1  offset from last point x to conics control x
        @param dy1  offset from last point x to conics control y
        @param dx2  offset from last point x to conics end x
        @param dy2  offset from last point x to conics end y
        @param w    weight of added conics
    */
    void rConicTo(SkScalar dx1, SkScalar dy1, SkScalar dx2, SkScalar dy2,
                  SkScalar w);

    /** Adds cubics from last point towards (x1, y1), then towards (x2, y2), ending at
        (x3, y3). If SkPath is empty, or last SkPath::Verb is kClose_Verb, last point is set to
        (0, 0) before adding cubics.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint arrays, if needed;
        then appends kCubic_Verb to verb array; and (x1, y1), (x2, y2), (x3, y3)
        to SkPoint arrays.

        @param x1  first control SkPoint of cubics in x
        @param y1  first control SkPoint of cubics in y
        @param x2  second control SkPoint of cubics in x
        @param y2  second control SkPoint of cubics in y
        @param x3  end SkPoint of cubics in x
        @param y3  end SkPoint of cubics in y
    */
    void cubicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                 SkScalar x3, SkScalar y3);

    /** Adds cubics from last point towards SkPoint p1, then towards SkPoint p2, ending at
        SkPoint p3. If SkPath is empty, or last SkPath::Verb is kClose_Verb, last point is set to
        (0, 0) before adding cubics.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint arrays, if needed;
        then appends kCubic_Verb to verb array; and SkPoint p1, p2, p3
        to SkPoint arrays.

        @param p1  first control SkPoint of cubics
        @param p2  second control SkPoint of cubics
        @param p3  end SkPoint of cubics
    */
    void cubicTo(const SkPoint& p1, const SkPoint& p2, const SkPoint& p3) {
        this->cubicTo(p1.fX, p1.fY, p2.fX, p2.fY, p3.fX, p3.fY);
    }

    /** Adds cubics from last point towards SkVector (dx1, dy1), then towards
        SkVector (dx2, dy2), to SkVector (dx3, dy3).
        If SkPath is empty, or last SkPath::Verb
        is kClose_Verb, last point is set to (0, 0) before adding cubics.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint arrays,
        if needed; then appends kCubic_Verb to verb array; and appends cubics
        control and cubics end to SkPoint arrays.
        Cubics control is last point plus SkVector (dx1, dy1).
        Cubics end is last point plus SkVector (dx2, dy2).
        Function name stands for relative cubic to.

        @param x1  offset from last point x to first cubics control x
        @param y1  offset from last point x to first cubics control y
        @param x2  offset from last point x to second cubics control x
        @param y2  offset from last point x to second cubics control y
        @param x3  offset from last point x to cubics end x
        @param y3  offset from last point x to cubics end y
    */
    void rCubicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                  SkScalar x3, SkScalar y3);

    /** Append arcs to SkPath. Arcs added is part of ellipse
        bounded by oval, from startAngle through sweepAngle. Both startAngle and
        sweepAngle are measured in degrees, where zero degrees is aligned with the
        positive x-axis, and positive sweeps extends arcs clockwise.

        arcTo() adds lines connecting SkPath last SkPoint to initial arcs SkPoint if forceMoveTo
        is false and SkPath is not empty. Otherwise, added contour begins with first point
        of arcs. Angles greater than -360 and less than 360 are treated modulo 360.

        @param oval         bounds of ellipse containing arcs
        @param startAngle   starting angle of arcs in degrees
        @param sweepAngle   sweep, in degrees. Positive is clockwise; treated modulo 360
        @param forceMoveTo  true to start a new contour with arcs
    */
    void arcTo(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle, bool forceMoveTo);

    /** Append arcs to SkPath, after appending lines if needed. Arcs is implemented by conics
        weighted to describe part of circles. Arcs is contained by tangent from
        last SkPath point (x0, y0) to (x1, y1), and tangent from (x1, y1) to (x2, y2). Arcs
        is part of circles sized to radius, positioned so it touches both tangent lines.

        @param x1      x common to pair of tangents
        @param y1      y common to pair of tangents
        @param x2      x end of second tangent
        @param y2      y end of second tangent
        @param radius  distance from arcs to circles center
    */
    void arcTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar radius);

    /** Append arcs to SkPath, after appending lines if needed. Arcs is implemented by conics
        weighted to describe part of circles. Arcs is contained by tangent from
        last SkPath point to p1, and tangent from p1 to p2. Arcs
        is part of circles sized to radius, positioned so it touches both tangent lines.

        If last SkPath SkPoint does not start arcs, arcTo() appends connecting lines to SkPath.
        The length of SkVector from p1 to p2 does not affect arcs.

        Arcs sweep is always less than 180 degrees. If radius is zero, or if
        tangents are nearly parallel, arcTo() appends lines from last SkPath SkPoint to p1.

        arcTo() appends at most one lines and one conics.
        arcTo() implements the functionality of PostScript_arct and HTML_Canvas_arcTo.

        @param p1      SkPoint common to pair of tangents
        @param p2      end of second tangent
        @param radius  distance from arcs to circles center
    */
    void arcTo(const SkPoint p1, const SkPoint p2, SkScalar radius) {
        this->arcTo(p1.fX, p1.fY, p2.fX, p2.fY, radius);
    }

    /** \enum SkPath::ArcSize
        Four ovals parts with radii (rx, ry) start at last SkPath SkPoint and ends at (x, y).
        ArcSize and Direction select one of the four ovals parts.
    */
    enum ArcSize {
        kSmall_ArcSize, //!< Smaller of arcs pair.
        kLarge_ArcSize, //!< Larger of arcs pair.
    };

    /** Append arcs to SkPath. Arcs is implemented by one or more conics weighted to describe part of ovals
        with radii (rx, ry) rotated by xAxisRotate degrees. Arcs curves from last SkPath SkPoint to (x, y),
        choosing one of four possible routes: clockwise or counterclockwise, and smaller or larger.

        Arcs sweep is always less than 360 degrees. arcTo() appends lines to (x, y) if either radii are zero,
        or if last SkPath SkPoint equals (x, y). arcTo() scales radii (rx, ry) to fit last SkPath SkPoint and
        (x, y) if both are greater than zero but too small.

        arcTo() appends up to four conics curves.
        arcTo() implements the functionatlity of svg arc, although SVG sweep-flag value is
        opposite the integer value of sweep; SVG sweep-flag uses 1 for clockwise, while kCW_Direction
        cast to int is zero.

        @param rx           radius in x before x-axis rotation
        @param ry           radius in y before x-axis rotation
        @param xAxisRotate  x-axis rotation in degrees; positve values are clockwise
        @param largeArc     chooses smaller or larger arcs
        @param sweep        chooses clockwise or counterclockwise arcs
        @param x            end of arcs
        @param y            end of arcs
    */
    void arcTo(SkScalar rx, SkScalar ry, SkScalar xAxisRotate, ArcSize largeArc,
               Direction sweep, SkScalar x, SkScalar y);

    /** Append arcs to SkPath. Arcs is implemented by one or more conics weighted to describe part of ovals
        with radii (r.fX, r.fY) rotated by xAxisRotate degrees. Arcs curves from last SkPath SkPoint to
        (xy.fX, xy.fY), choosing one of four possible routes: clockwise or counterclockwise,
        and smaller or larger.

        Arcs sweep is always less than 360 degrees. arcTo() appends lines to xy if either radii are zero,
        or if last SkPath SkPoint equals (x, y). arcTo() scales radii r to fit last SkPath SkPoint and
        xy if both are greater than zero but too small.

        arcTo() appends up to four conics curves.
        arcTo() implements the functionatlity of svg arc, although SVG sweep-flag value is
        opposite the integer value of sweep; SVG sweep-flag uses 1 for clockwise, while kCW_Direction
        cast to int is zero.

        @param r            radii in x and y before x-axis rotation
        @param xAxisRotate  x-axis rotation in degrees; positve values are clockwise
        @param largeArc     chooses smaller or larger arcs
        @param sweep        chooses clockwise or counterclockwise arcs
        @param xy           end of arcs
    */
    void arcTo(const SkPoint r, SkScalar xAxisRotate, ArcSize largeArc, Direction sweep,
               const SkPoint xy) {
        this->arcTo(r.fX, r.fY, xAxisRotate, largeArc, sweep, xy.fX, xy.fY);
    }

    /** Append arcs to SkPath, relative to last SkPath SkPoint. Arcs is implemented by one or
        more conics, weighted to describe part of ovals with radii (r.fX, r.fY) rotated by
        xAxisRotate degrees. Arcs curves from last SkPath SkPoint (x0, y0) to

        @param rx           radius in x before x-axis rotation
        @param ry           radius in y before x-axis rotation
        @param xAxisRotate  x-axis rotation in degrees; positve values are clockwise
        @param largeArc     chooses smaller or larger arcs
        @param sweep        chooses clockwise or counterclockwise arcs
        @param dx           x offset end of arcs from last SkPath SkPoint
        @param dy           y offset end of arcs from last SkPath SkPoint
    */
    void rArcTo(SkScalar rx, SkScalar ry, SkScalar xAxisRotate, ArcSize largeArc,
                Direction sweep, SkScalar dx, SkScalar dy);

    /** Append kClose_Verb to SkPath. A closed contour connects the first and last SkPoint
        with lines, forming a continous loop. Open and closed contour draw the same
        with SkPaint::kFill_Style. With SkPaint::kStroke_Style, open contour draws
        SkPaint::Cap at contour start and end; closed contour draws
        SkPaint::Join at contour start and end.

        close() has no effect if SkPath is empty or last SkPath SkPath::Verb is kClose_Verb.
    */
    void close();

    /** Returns true if fill is inverted and SkPath with fill represents area outside
        of its geometric bounds.

        @param fill  one of: kWinding_FillType, kEvenOdd_FillType,
                     kInverseWinding_FillType, kInverseEvenOdd_FillType
        @return      true if SkPath fills outside its bounds
    */
    static bool IsInverseFillType(FillType fill) {
        static_assert(0 == kWinding_FillType, "fill_type_mismatch");
        static_assert(1 == kEvenOdd_FillType, "fill_type_mismatch");
        static_assert(2 == kInverseWinding_FillType, "fill_type_mismatch");
        static_assert(3 == kInverseEvenOdd_FillType, "fill_type_mismatch");
        return (fill & 2) != 0;
    }

    /** Returns equivalent SkPath::FillType representing SkPath fill inside its bounds.
        .

        @param fill  one of: kWinding_FillType, kEvenOdd_FillType,
                     kInverseWinding_FillType, kInverseEvenOdd_FillType
        @return      fill, or kWinding_FillType or kEvenOdd_FillType if fill is inverted
    */
    static FillType ConvertToNonInverseFillType(FillType fill) {
        static_assert(0 == kWinding_FillType, "fill_type_mismatch");
        static_assert(1 == kEvenOdd_FillType, "fill_type_mismatch");
        static_assert(2 == kInverseWinding_FillType, "fill_type_mismatch");
        static_assert(3 == kInverseEvenOdd_FillType, "fill_type_mismatch");
        return (FillType)(fill & 1);
    }

    /** Approximates conics with quads array. Conics is constructed from start SkPoint p0,
        control SkPoint p1, end SkPoint p2, and weight w.
        Quads array is stored in pts; this storage is supplied by caller.
        Maximum quads count is 2 to the pow2.
        Every third point in array shares last SkPoint of previous quads and first SkPoint of
        next quads. Maximum pts storage size is given by:

        @param p0    Conics start SkPoint
        @param p1    Conics control SkPoint
        @param p2    Conics end SkPoint
        @param w     Conics weight
        @param pts   storage for quads array
        @param pow2  Quads count, as power of two, normally 0 to 5 (1 to 32 quads curves)
        @return      Number of quads curves written to pts
    */
    static int ConvertConicToQuads(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2,
                                   SkScalar w, SkPoint pts[], int pow2);

    /** Returns true if SkPath is eqivalent to SkRect when filled.
        If false: rect, isClosed, and direction are unchanged.
        If true: rect, isClosed, and direction are written to if not nullptr.

        rect may be smaller than the SkPath bounds. SkPath bounds may include kMove_Verb points
        that do not alter the area drawn by the returned rect.

        @param rect       storage for bounds of SkRect; may be nullptr
        @param isClosed   storage set to true if SkPath is closed; may be nullptr
        @param direction  storage set to SkRect direction; may be nullptr
        @return           true if SkPath contains SkRect
    */
    bool isRect(SkRect* rect, bool* isClosed = NULL, Direction* direction = NULL) const;

    /** Returns true if SkPath is equivalent to nested SkRect pair when filled.
        If false, rect and dirs are unchanged.
        If true, rect and dirs are written to if not nullptr:
        setting rect[0] to outer SkRect, and rect[1] to inner SkRect;
        setting dirs[0] to SkPath::Direction of outer SkRect, and dirs[1] to SkPath::Direction of inner
        SkRect.

        @param rect  storage for SkRect pair; may be nullptr
        @param dirs  storage for SkPath::Direction pair; may be nullptr
        @return      true if SkPath contains nested SkRect pair
    */
    bool isNestedFillRects(SkRect rect[2], Direction dirs[2] = NULL) const;

    /** Add SkRect to SkPath, appending kMove_Verb, three kLine_Verb, and kClose_Verb,
        starting with top-left corner of SkRect; followed by top-right, bottom-right,
        and bottom-left if dir is kCW_Direction; or followed by bottom-left,
        bottom-right, and top-right if dir is kCCW_Direction.

        @param rect  SkRect to add as a closed contour
        @param dir   SkPath::Direction to wind added contour
    */
    void addRect(const SkRect& rect, Direction dir = kCW_Direction);

    /** Add SkRect to SkPath, appending kMove_Verb, three kLine_Verb, and kClose_Verb.
        If dir is kCW_Direction, SkRect corners are added clockwise; if dir is
        kCCW_Direction, SkRect corners are added counterclockwise.
        start determines the first corner added.

        @param rect   SkRect to add as a closed contour
        @param dir    SkPath::Direction to wind added contour
        @param start  Initial corner of SkRect to add
    */
    void addRect(const SkRect& rect, Direction dir, unsigned start);

    /** Add SkRect (left, top, right, bottom) to SkPath,
        appending kMove_Verb, three kLine_Verb, and kClose_Verb,
        starting with top-left corner of SkRect; followed by top-right, bottom-right,
        and bottom-left if dir is kCW_Direction; or followed by bottom-left,
        bottom-right, and top-right if dir is kCCW_Direction.

        @param left    smaller x of SkRect
        @param top     smaller y of SkRect
        @param right   larger x of SkRect
        @param bottom  larger y of SkRect
        @param dir     SkPath::Direction to wind added contour
    */
    void addRect(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom,
                 Direction dir = kCW_Direction);

    /** Add ovals to path, appending kMove_Verb, four kConic_Verb, and kClose_Verb.
        Ovals is upright ellipse bounded by SkRect oval with radii equal to half oval width
        and half oval height. Ovals begins at (oval.fRight, oval.centerY()) and continues
        clockwise if dir is kCW_Direction, counterclockwise if dir is kCCW_Direction.

        This form is identical to addOval(oval, dir, 1).

        @param oval  bounds of ellipse added
        @param dir   SkPath::Direction to wind ellipse
    */
    void addOval(const SkRect& oval, Direction dir = kCW_Direction);

    /** Add ovals to SkPath, appending kMove_Verb, four kConic_Verb, and kClose_Verb.
        Ovals is upright ellipse bounded by SkRect oval with radii equal to half oval width
        and half oval height. Ovals begins at start and continues
        clockwise if dir is kCW_Direction, counterclockwise if dir is kCCW_Direction.

        @param oval   bounds of ellipse added
        @param dir    SkPath::Direction to wind ellipse
        @param start  index of initial point of ellipse
    */
    void addOval(const SkRect& oval, Direction dir, unsigned start);

    /** Add circles centered at (x, y) of size radius to SkPath, appending kMove_Verb,
        four kConic_Verb, and kClose_Verb. Circles begins at

        @param x       center of circles
        @param y       center of circles
        @param radius  distance from center to edge
        @param dir     SkPath::Direction to wind circles
    */
    void addCircle(SkScalar x, SkScalar y, SkScalar radius,
                   Direction dir = kCW_Direction);

    /** Append arcs to SkPath, as the start of new contour. Arcs added is part of ellipse
        bounded by oval, from startAngle through sweepAngle. Both startAngle and
        sweepAngle are measured in degrees, where zero degrees is aligned with the
        positive x-axis, and positive sweeps extends arcs clockwise.

        If sweepAngle <= -360, or sweepAngle >= 360; and startAngle modulo 90 is nearly
        zero, append ovals instead of arcs. Otherwise, sweepAngle values are treated
        modulo 360, and arcs may or may not draw depending on numeric rounding.

        @param oval        bounds of ellipse containing arcs
        @param startAngle  starting angle of arcs in degrees
        @param sweepAngle  sweep, in degrees. Positive is clockwise; treated modulo 360
    */
    void addArc(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle);

    /** Append SkRRect to SkPath, creating a new closed contour. SkRRect has bounds
        equal to rect; each corner is 90 degrees of an ellipse with radii (rx, ry). If
        dir is kCW_Direction, SkRRect starts at top-left of the lower-left corner and
        winds clockwise. If dir is kCCW_Direction, SkRRect starts at the bottom-left
        of the upper-left corner and winds counterclockwise.

        If either rx or ry is too large, rx and ry are scaled uniformly until the
        corners fit. If rx or ry is less than or equal to zero, addRoundRect() appends
        SkRect rect to SkPath.

        After appending, SkPath may be empty, or may contain: SkRect, ovals, or RoundRect.

        @param rect  bounds of SkRRect
        @param rx    x-radius of rounded corners on the SkRRect
        @param ry    y-radius of rounded corners on the SkRRect
        @param dir   SkPath::Direction to wind SkRRect
    */
    void addRoundRect(const SkRect& rect, SkScalar rx, SkScalar ry,
                      Direction dir = kCW_Direction);

    /** Append SkRRect to SkPath, creating a new closed contour. SkRRect has bounds
        equal to rect; each corner is 90 degrees of an ellipse with radii from the
        array.

        @param rect   bounds of SkRRect
        @param radii  array of 8 SkScalar values, a radius pair for each corner
        @param dir    SkPath::Direction to wind SkRRect
    */
    void addRoundRect(const SkRect& rect, const SkScalar radii[],
                      Direction dir = kCW_Direction);

    /** Add rrect to SkPath, creating a new closed contour. If
        dir is kCW_Direction, rrect starts at top-left of the lower-left corner and
        winds clockwise. If dir is kCCW_Direction, rrect starts at the bottom-left
        of the upper-left corner and winds counterclockwise.

        After appending, SkPath may be empty, or may contain: SkRect, ovals, or SkRRect.

        @param rrect  bounds and radii of rounded rectangle
        @param dir    SkPath::Direction to wind SkRRect
    */
    void addRRect(const SkRRect& rrect, Direction dir = kCW_Direction);

    /** Add rrect to SkPath, creating a new closed contour. If dir is kCW_Direction, rrect
        winds clockwise; if dir is kCCW_Direction, rrect winds counterclockwise.
        start determines the first point of rrect to add.

        @param rrect  bounds and radii of rounded rectangle
        @param dir    SkPath::Direction to wind SkRRect
        @param start  Index of initial point of SkRRect
    */
    void addRRect(const SkRRect& rrect, Direction dir, unsigned start);

    /** Add contour created from lines array, adding

        @param pts    Array of lines sharing end and start SkPoint
        @param count  Length of SkPoint array
        @param close  true to add lines connecting contour end and start
    */
    void addPoly(const SkPoint pts[], int count, bool close);

    /** \enum SkPath::AddPathMode
        AddPathMode chooses how addPath() appends. Adding one SkPath to another can extend
        the last contour or start a new contour.
    */
    enum AddPathMode {
        /** Since SkPath verb array begins with kMove_Verb if src is not empty, this
            starts a new contour.
        */
        kAppend_AddPathMode,

        /** is not empty, add lines from last point to added SkPath first SkPoint. Skip added
            SkPath initial kMove_Verb, then append remining Verb, SkPoint, and weights.
        */
        kExtend_AddPathMode,
    };

    /** Append src to SkPath, offset by (dx, dy).

        If mode is kAppend_AddPathMode, src verb array, SkPoint arrays, and weights are
        added unaltered. If mode is kExtend_AddPathMode, add lines before appending
        SkPath::Verb, SkPoint, and weights.

        @param src   SkPath SkPath::Verb, SkPoint, and weights to add
        @param dx    offset added to src SkPoint arrays x coordinates
        @param dy    offset added to src SkPoint arrays y coordinates
        @param mode  kAppend_AddPathMode or kExtend_AddPathMode
    */
    void addPath(const SkPath& src, SkScalar dx, SkScalar dy,
                 AddPathMode mode = kAppend_AddPathMode);

    /** Append src to SkPath.

        If mode is kAppend_AddPathMode, src verb array, SkPoint arrays, and weights are
        added unaltered. If mode is kExtend_AddPathMode, add lines before appending
        SkPath::Verb, SkPoint, and weights.

        @param src   SkPath SkPath::Verb, SkPoint, and weights to add
        @param mode  kAppend_AddPathMode or kExtend_AddPathMode
    */
    void addPath(const SkPath& src, AddPathMode mode = kAppend_AddPathMode) {
        SkMatrix m;
        m.reset();
        this->addPath(src, m, mode);
    }

    /** Append src to SkPath, transformed by matrix. Transformed curves may have different
        SkPath::Verb, SkPoint, and weights.

        If mode is kAppend_AddPathMode, src verb array, SkPoint arrays, and weights are
        added unaltered. If mode is kExtend_AddPathMode, add lines before appending
        SkPath::Verb, SkPoint, and weights.

        @param src     SkPath SkPath::Verb, SkPoint, and weights to add
        @param matrix  Transform applied to src
        @param mode    kAppend_AddPathMode or kExtend_AddPathMode
    */
    void addPath(const SkPath& src, const SkMatrix& matrix, AddPathMode mode = kAppend_AddPathMode);

    /** Append src to SkPath, from back to front.
        Reversed src always appends a new contour to SkPath.

        @param src  SkPath SkPath::Verb, SkPoint, and weights to add
    */
    void reverseAddPath(const SkPath& src);

    /** Offset SkPoint arrays by (dx, dy). Offset SkPath replaces dst.
        If dst is nullptr, SkPath is replaced by offset data.

        @param dx   offset added to SkPoint arrays x coordinates
        @param dy   offset added to SkPoint arrays y coordinates
        @param dst  overwritten, translated copy of SkPath; may be nullptr
    */
    void offset(SkScalar dx, SkScalar dy, SkPath* dst) const;

    /** Offset SkPoint arrays by (dx, dy). Offset SkPath replaces dst.
        If dst is nullptr, SkPath is replaced by offset data.

        @param dx   offset added to SkPoint arrays x coordinates
        @param dy   offset added to SkPoint arrays y coordinates
        @param dst  overwritten, translated copy of SkPath; may be nullptr
    */
    void offset(SkScalar dx, SkScalar dy) {
        this->offset(dx, dy, this);
    }

    /** Transform verb array, SkPoint arrays, and weight by matrix.
        transform may change SkPath::Verb and increase their number.
        Transformed SkPath replaces dst; if dst is nullptr, original data
        is replaced.

        @param matrix  SkMatrix to apply to SkPath
        @param dst     overwritten, transformed copy of SkPath; may be nullptr
    */
    void transform(const SkMatrix& matrix, SkPath* dst) const;

    /** Transform verb array, SkPoint arrays, and weight by matrix.
        transform may change SkPath::Verb and increase their number.
        Transformed SkPath replaces dst; if dst is nullptr, original data
        is replaced.

        @param matrix  SkMatrix to apply to SkPath
        @param dst     overwritten, transformed copy of SkPath; may be nullptr
    */
    void transform(const SkMatrix& matrix) {
        this->transform(matrix, this);
    }

    /** Returns last point on SkPath in lastPt. Returns false if SkPoint arrays is empty,
        storing (0, 0) if lastPt is not nullptr.

        @param lastPt  storage for final SkPoint in SkPoint arrays; may be nullptr
        @return        true if SkPoint arrays contains one or more SkPoint
    */
    bool getLastPt(SkPoint* lastPt) const;

    /** Set last point to (x, y). If SkPoint arrays is empty, append kMove_Verb to
        verb array and (x, y) to SkPoint arrays.

        @param x  set x-coordinate of last point
        @param y  set y-coordinate of last point
    */
    void setLastPt(SkScalar x, SkScalar y);

    /** Set the last point on the path. If no points have been added, moveTo(p)
        is automatically called.

        @param p  set value of last point
    */
    void setLastPt(const SkPoint& p) {
        this->setLastPt(p.fX, p.fY);
    }

    /** \enum SkPath::SegmentMask
        SegmentMask constants correspond to each drawing Verb type in SkPath; for
        instance, if SkPath only contains lines, only the kLine_SegmentMask bit is set.
    */
    enum SegmentMask {
        kLine_SegmentMask  = 1 << 0, //!< Set if verb array contains kLine_Verb.

        /** Set if verb array contains kQuad_Verb. Note that conicTo() may add a quads. */
        kQuad_SegmentMask  = 1 << 1,
        kConic_SegmentMask = 1 << 2, //!< Set if verb array contains kConic_Verb.
        kCubic_SegmentMask = 1 << 3, //!< Set if verb array contains kCubic_Verb.
    };

    /** Returns a mask, where each set bit corresponds to a SegmentMask constant
        if SkPath contains one or more SkPath::Verb of that type.
        Returns zero if SkPath contains no lines, or curves: quads, conics, or cubics.

        getSegmentMasks() returns a cached result; it is very fast.

        @return  SegmentMask bits or zero
    */
    uint32_t getSegmentMasks() const { return fPathRef->getSegmentMasks(); }

    /** \enum SkPath::Verb
        Verb instructs SkPath how to interpret one or more SkPoint and optional weights;
        manage contour, and terminate SkPath.
    */
    enum Verb {
        kMove_Verb,  //!< Starts new contour at next SkPoint.

        /** Adds lines from last point to next SkPoint.
            Lines is a straight segment from SkPoint to SkPoint.
        */
        kLine_Verb,

        /** Adds quads from last point, using control SkPoint, and end SkPoint.
            Quads is a parabolic section within tangents from last point to control SkPoint,
            and control SkPoint to end SkPoint.
        */
        kQuad_Verb,

        /** Adds conics from last point, using control SkPoint, end SkPoint, and weights.
            Conics is a elliptical, parabolic, or hyperbolic section within tangents
            from last point to control SkPoint, and control SkPoint to end SkPoint, constrained
            by weights. weights less than one is elliptical; equal to one is
            parabolic (and identical to Quad); greater than one hyperbolic.
        */
        kConic_Verb,

        /** Adds cubics from last point, using two control SkPoint, and end SkPoint.
            Cubics is a third-order Bezier section within tangents from last point to
            first control SkPoint, and from second control SkPoint to end SkPoint.
        */
        kCubic_Verb,
        kClose_Verb, //!< Closes contour, connecting last point to kMove_Verb SkPoint.
        kDone_Verb,  //!< Terminates SkPath. Not in verb array, but returned by SkPath iterator.
    };

    /** \class SkPath::Iter
    */
    class SK_API Iter {

        public:

        /** Initializes iter with an empty SkPath. next() on iter returns kDone_Verb.
            Call setPath to initialize iter at a later time.

            @return  Iter of empty SkPath
        */
        Iter();

        /** Sets iter to return elements of verb array, SkPoint arrays, and weights in path.
            If forceClose is true, iter will add kLine_Verb and kClose_Verb after each
            open contour. path is not altered.

            @param path        SkPath to iterate
            @param forceClose  true if open contours generate kClose_Verb
            @return            Iter of path
        */
        Iter(const SkPath& path, bool forceClose);

        /** Sets iter to return elements of verb array, SkPoint arrays, and weights in path.
            If forceClose is true, iter will add kLine_Verb and kClose_Verb after each
            open contour. path is not altered.

            @param path        SkPath to iterate
            @param forceClose  true if open contours generate kClose_Verb
        */
        void setPath(const SkPath& path, bool forceClose);

        /** Returns next SkPath::Verb in verb array, and advances iter.
            When verb array is exhausted, returns kDone_Verb.
            Zero to four points are stored in pts, depending on the returned SkPath::Verb.
            If doConsumeDegenerates is true, skip consecutive kMove_Verb entries, returning
            only the last in the series; and skip very small lines, quads, and conics; and
            skip kClose_Verb following kMove_Verb.
            if doConsumeDegenerates is true and exact is true, only skip lines, quads, and
            conics with zero lengths.

            @param pts                   Storage for SkPoint data describing returned SkPath::Verb
            @param doConsumeDegenerates  If true, skip degenerate verbs
            @param exact                 If true, skip zero length curves. Has no effect if doConsumeDegenerates
                                         is false
            @return                      next SkPath::Verb from verb array
        */
        Verb next(SkPoint pts[4], bool doConsumeDegenerates = true, bool exact = false) {
            if (doConsumeDegenerates) {
                this->consumeDegenerateSegments(exact);
            }
            return this->doNext(pts);
        }

        /** Returns weights if next() returned kConic_Verb.

            If next() has not been called, or next() did not return kConic_Verb,
            result is undefined.

            @return  weights for conics points returned by next()
        */
        SkScalar conicWeight() const { return *fConicWeights; }

        /** Returns true if last kLine_Verb returned by next() was generated
            by kClose_Verb. When true, the end point returned by next() is
            also the start point of contour.

            If next() has not been called, or next() did not return kLine_Verb,
            result is undefined.

            @return  true if last kLine_Verb was generated by kClose_Verb
        */
        bool isCloseLine() const { return SkToBool(fCloseLine); }

        /** Returns true if subsequent calls to next() return kClose_Verb before returning
            kMove_Verb. if true, contour iter is processing may end with kClose_Verb, or
            iter may have been initialized with force close set to true.

            @return  true if contour is closed
        */
        bool isClosedContour() const;

    private:
        const SkPoint*  fPts;
        const uint8_t*  fVerbs;
        const uint8_t*  fVerbStop;
        const SkScalar* fConicWeights;
        SkPoint         fMoveTo;
        SkPoint         fLastPt;
        SkBool8         fForceClose;
        SkBool8         fNeedClose;
        SkBool8         fCloseLine;
        SkBool8         fSegmentState;

        inline const SkPoint& cons_moveTo();
        Verb autoClose(SkPoint pts[2]);
        void consumeDegenerateSegments(bool exact);
        Verb doNext(SkPoint pts[4]);

    };

    /** \class SkPath::RawIter
    */
    class SK_API RawIter {

        public:

        /** Initializes RawIter with an empty SkPath. next() on RawIter returns kDone_Verb.
            Call setPath to initialize iter at a later time.

            @return  RawIter of empty SkPath
        */
        RawIter() {}

        /** Sets RawIter to return elements of verb array, SkPoint arrays, and weights in path.

            @param path  SkPath to iterate
            @return      RawIter of path
        */
        RawIter(const SkPath& path) {
            setPath(path);
        }

        /** Sets iter to return elements of verb array, SkPoint arrays, and weights in path.

            @param path  SkPath to iterate
        */
        void setPath(const SkPath& path) {
            fRawIter.setPathRef(*path.fPathRef.get());
        }

        /** Returns next SkPath::Verb in verb array, and advances RawIter.
            When verb array is exhausted, returns kDone_Verb.
            Zero to four points are stored in pts, depending on the returned SkPath::Verb.

            @param pts  Storage for SkPoint data describing returned SkPath::Verb
            @return     next SkPath::Verb from verb array
        */
        Verb next(SkPoint pts[4]) {
            return (Verb) fRawIter.next(pts);
        }

        /** Returns next SkPath::Verb, but does not advance RawIter.

            @return  next SkPath::Verb from verb array
        */
        Verb peek() const {
            return (Verb) fRawIter.peek();
        }

        /** Returns weights if next() returned kConic_Verb.

            If next() has not been called, or next() did not return kConic_Verb,
            result is undefined.

            @return  weights for conics points returned by next()
        */
        SkScalar conicWeight() const {
            return fRawIter.conicWeight();
        }

    private:
        SkPathRef::Iter fRawIter;
        friend class SkPath;

    };

    /** Returns true if the point (x, y) is contained by SkPath, taking into
        account FillType.

        @param x  x-coordinate of containment test
        @param y  y-coordinate of containment test
        @return   true if SkPoint is in SkPath
    */
    bool contains(SkScalar x, SkScalar y) const;

    /** Writes text representation of SkPath to stream. If stream is nullptr, dump() writes to
        stdout. Set forceClose to true to get
        edges used to fill SkPath. Set dumpAsHex true to get exact binary representations
        of floating point numbers used in SkPoint arrays and conic weights.

        @param stream      writable SkFlattenable receiving SkPath text representation; may be nullptr
        @param forceClose  true if missing kClose_Verb is output
        @param dumpAsHex   true if SkScalar values are written as hexidecimal
    */
    void dump(SkWStream* stream, bool forceClose, bool dumpAsHex) const;

    /** Writes text representation of SkPath to stream. If stream is nullptr, dump() writes to
        stdout. Set forceClose to true to get
        edges used to fill SkPath. Set dumpAsHex true to get exact binary representations
        of floating point numbers used in SkPoint arrays and conic weights.

        @param stream      writable SkFlattenable receiving SkPath text representation; may be nullptr
        @param forceClose  true if missing kClose_Verb is output
        @param dumpAsHex   true if SkScalar values are written as hexidecimal
    */
    void dump() const;

    /** Writes text representation of SkPath to stdout. The representation may be
        directly compiled as C++ code. Floating point values are written
        in hexadecimal to preserve their exact bit pattern. The output reconstructs the
        original SkPath.

        Use instead of dump() when submitting
    */
    void dumpHex() const;

    /** Writes SkPath to buffer, returning the number of bytes written.
        Pass nullptr to obtain the storage size.

        Writes SkPath::FillType, verb array, SkPoint arrays, weights, and
        additionally writes computed information like SkPath::Convexity and bounds.

        Use only be used in concert with readFromMemory();
        the format used for SkPath in memory is not guaranteed.

        @param buffer  storage for SkPath; may be nullptr
        @return        size of storage required for SkPath; always a multiple of 4
    */
    size_t writeToMemory(void* buffer) const;

    /** Write SkPath to buffer, returning the buffer written to, wrapped in data.

        serialize() writes SkPath::FillType, verb array, SkPoint arrays, weights, and
        additionally writes computed information like SkPath::Convexity and bounds.

        serialize() should only be used in concert with readFromMemory().
        The format used for SkPath in memory is not guaranteed.

        @return  SkPath data wrapped in data buffer
    */
    sk_sp<SkData> serialize() const;

    /** Initializes SkPath from buffer of size length. Returns zero if the buffer is
        data is inconsistent, or the length is too small.

        Reads SkPath::FillType, verb array, SkPoint arrays, weights, and
        additionally reads computed information like SkPath::Convexity and bounds.

        Used only in concert with writeToMemory();
        the format used for SkPath in memory is not guaranteed.

        @param buffer  storage for SkPath
        @param length  buffer size in bytes; must be multiple of 4
        @return        number of bytes read, or zero on failure
    */
    size_t readFromMemory(const void* buffer, size_t length);

    /** Returns a non-zero, globally unique value. A different value is returned
        if verb array, SkPoint arrays, or weights changes.

        Setting SkPath::FillType does not change generation id.

        Each time the path is modified, a different generation id will be returned.

        @return  non-zero, globally unique value
    */
    uint32_t getGenerationID() const;

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    static const int kPathRefGenIDBitCnt = 30; // leave room for the fill type (skbug.com/1762)
#else
    static const int kPathRefGenIDBitCnt = 32;
#endif

    /** Returns if SkPath data is consistent. Corrupt SkPath data is detected if
        internal values are out of range or internal storage does not match
        array dimensions.

        @return  true if SkPath data is consistent
    */
    bool isValid() const;

    /** Returns if SkPath data is consistent.

        @return  true if SkPath data is consistent
    */
    bool pathRefIsValid() const { return fPathRef->isValid(); }
    SkDEBUGCODE(void validate() const { SkASSERT(this->isValid()); } )
    SkDEBUGCODE(void experimentalValidateRef() const { fPathRef->validate(); } )

private:
    enum SerializationOffsets {
        // 1 free bit at 29
        kUnused1_SerializationShift = 28,    // 1 free bit
        kDirection_SerializationShift = 26, // requires 2 bits
        kIsVolatile_SerializationShift = 25, // requires 1 bit
        // 1 free bit at 24
        kConvexity_SerializationShift = 16, // requires 8 bits
        kFillType_SerializationShift = 8,   // requires 8 bits
        // low-8-bits are version
    };

    enum SerializationVersions {
        kPathPrivFirstDirection_Version = 1,
        kPathPrivLastMoveToIndex_Version = 2,
        kCurrent_Version = 2
    };

    sk_sp<SkPathRef>                                   fPathRef;
    int                                                fLastMoveToIndex;
    uint8_t                                            fFillType;
    mutable uint8_t                                    fConvexity;
    mutable SkAtomic<uint8_t, sk_memory_order_relaxed> fFirstDirection;// SkPathPriv::FirstDirection
    SkBool8                                            fIsVolatile;

    /** Resets all fields other than fPathRef to their initial 'empty' values.
     *  Assumes the caller has already emptied fPathRef.
     *  On Android increments fGenerationID without reseting it.
     */
    void resetFields();

    /** Sets all fields other than fPathRef to the values in 'that'.
     *  Assumes the caller has already set fPathRef.
     *  Doesn't change fGenerationID or fSourcePath on Android.
     */
    void copyFields(const SkPath& that);

    friend class Iter;
    friend class SkPathPriv;
    friend class SkPathStroker;

    /*  Append, in reverse order, the first contour of path, ignoring path's
        last point. If no moveTo() call has been made for this contour, the
        first point is automatically set to (0,0).
    */
    void reversePathTo(const SkPath&);

    // called before we add points for lineTo, quadTo, cubicTo, checking to see
    // if we need to inject a leading moveTo first
    //
    //  SkPath path; path.lineTo(...);   <--- need a leading moveTo(0, 0)
    // SkPath path; ... path.close(); path.lineTo(...) <-- need a moveTo(previous moveTo)
    //
    inline void injectMoveToIfNeeded();

    inline bool hasOnlyMoveTos() const;

    Convexity internalGetConvexity() const;

    bool isRectContour(bool allowPartial, int* currVerb, const SkPoint** pts,
                       bool* isClosed, Direction* direction) const;

    // called by stroker to see if all points (in the last contour) are equal and worthy of a cap
    bool isZeroLengthSincePoint(int startPtIndex) const;

    /** Returns if the path can return a bound at no cost (true) or will have to
        perform some computation (false).
     */
    bool hasComputedBounds() const {
        SkDEBUGCODE(this->validate();)
        return fPathRef->hasComputedBounds();
    }


    // 'rect' needs to be sorted
    void setBounds(const SkRect& rect) {
        SkPathRef::Editor ed(&fPathRef);

        ed.setBounds(rect);
    }

    void setPt(int index, SkScalar x, SkScalar y);

    friend class SkAutoPathBoundsUpdate;
    friend class SkAutoDisableOvalCheck;
    friend class SkAutoDisableDirectionCheck;
    friend class SkPathWriter;
    friend class SkOpBuilder;
    friend class SkBench_AddPathTest; // perf test reversePathTo
    friend class PathTest_Private; // unit test reversePathTo
    friend class ForceIsRRect_Private; // unit test isRRect
};

#endif
