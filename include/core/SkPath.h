/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPath_DEFINED
#define SkPath_DEFINED

#include "SkMatrix.h"
#include "SkPathRef.h"
#include "SkRefCnt.h"

class SkReader32;
class SkWriter32;
class SkAutoPathBoundsUpdate;
class SkString;
class SkRRect;
class SkWStream;

/** \class SkPath

    The SkPath class encapsulates compound (multiple contour) geometric paths
    consisting of straight line segments, quadratic curves, and cubic curves.

    SkPath is not thread safe unless you've first called SkPath::updateBoundsCache().
*/
class SK_API SkPath {
public:
    enum Direction {
        /** clockwise direction for adding closed contours */
        kCW_Direction,
        /** counter-clockwise direction for adding closed contours */
        kCCW_Direction,
    };

    SkPath();
    SkPath(const SkPath&);
    ~SkPath();

    SkPath& operator=(const SkPath&);
    friend  SK_API bool operator==(const SkPath&, const SkPath&);
    friend bool operator!=(const SkPath& a, const SkPath& b) {
        return !(a == b);
    }

    /** Return true if the paths contain an equal array of verbs and weights. Paths
     *  with equal verb counts can be readily interpolated. If the paths contain one
     *  or more conics, the conics' weights must also match.
     *
     *  @param compare  The path to compare.
     *
     *  @return true if the paths have the same verbs and weights.
     */
    bool isInterpolatable(const SkPath& compare) const;

    /** Interpolate between two paths with same-sized point arrays.
     *  The out path contains the verbs and weights of this path.
     *  The out points are a weighted average of this path and the ending path. 
     *
     *  @param ending  The path to interpolate between.
     *  @param weight  The weight, from 0 to 1. The output points are set to
     *                 (this->points * weight) + ending->points * (1 - weight).
     *  @return true if the paths could be interpolated.
     */
    bool interpolate(const SkPath& ending, SkScalar weight, SkPath* out) const;

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    /** Returns true if the caller is the only owner of the underlying path data */
    bool unique() const { return fPathRef->unique(); }
#endif

    enum FillType {
        /** Specifies that "inside" is computed by a non-zero sum of signed
            edge crossings
        */
        kWinding_FillType,
        /** Specifies that "inside" is computed by an odd number of edge
            crossings
        */
        kEvenOdd_FillType,
        /** Same as Winding, but draws outside of the path, rather than inside
        */
        kInverseWinding_FillType,
        /** Same as EvenOdd, but draws outside of the path, rather than inside
         */
        kInverseEvenOdd_FillType
    };

    /** Return the path's fill type. This is used to define how "inside" is
        computed. The default value is kWinding_FillType.

        @return the path's fill type
    */
    FillType getFillType() const { return (FillType)fFillType; }

    /** Set the path's fill type. This is used to define how "inside" is
        computed. The default value is kWinding_FillType.

        @param ft The new fill type for this path
    */
    void setFillType(FillType ft) {
        fFillType = SkToU8(ft);
    }

    /** Returns true if the filltype is one of the Inverse variants */
    bool isInverseFillType() const { return IsInverseFillType((FillType)fFillType); }

    /**
     *  Toggle between inverse and normal filltypes. This reverse the return
     *  value of isInverseFillType()
     */
    void toggleInverseFillType() {
        fFillType ^= 2;
    }

    enum Convexity {
        kUnknown_Convexity,
        kConvex_Convexity,
        kConcave_Convexity
    };

    /**
     *  Return the path's convexity, as stored in the path. If it is currently unknown,
     *  then this function will attempt to compute the convexity (and cache the result).
     */
    Convexity getConvexity() const {
        if (kUnknown_Convexity != fConvexity) {
            return static_cast<Convexity>(fConvexity);
        } else {
            return this->internalGetConvexity();
        }
    }

    /**
     *  Return the currently cached value for convexity, even if that is set to
     *  kUnknown_Convexity. Note: getConvexity() will automatically call
     *  ComputeConvexity and cache its return value if the current setting is
     *  kUnknown.
     */
    Convexity getConvexityOrUnknown() const { return (Convexity)fConvexity; }

    /**
     *  Store a convexity setting in the path. There is no automatic check to
     *  see if this value actually agrees with the return value that would be
     *  computed by getConvexity().
     *
     *  Note: even if this is set to a "known" value, if the path is later
     *  changed (e.g. lineTo(), addRect(), etc.) then the cached value will be
     *  reset to kUnknown_Convexity.
     */
    void setConvexity(Convexity);

    /**
     *  Returns true if the path is flagged as being convex. This is not a
     *  confirmed by any analysis, it is just the value set earlier.
     */
    bool isConvex() const {
        return kConvex_Convexity == this->getConvexity();
    }

    /**
     *  Set the isConvex flag to true or false. Convex paths may draw faster if
     *  this flag is set, though setting this to true on a path that is in fact
     *  not convex can give undefined results when drawn. Paths default to
     *  isConvex == false
     */
    SK_ATTR_DEPRECATED("use setConvexity")
    void setIsConvex(bool isConvex) {
        this->setConvexity(isConvex ? kConvex_Convexity : kConcave_Convexity);
    }

    /** Returns true if the path is an oval.
     *
     * @param rect      returns the bounding rect of this oval. It's a circle
     *                  if the height and width are the same.
     * @param dir       is the oval CCW (or CW if false).
     * @param start     indicates where the contour starts on the oval (see
     *                  SkPath::addOval for intepretation of the index).
     * @return true if this path is an oval.
     *              Tracking whether a path is an oval is considered an
     *              optimization for performance and so some paths that are in
     *              fact ovals can report false.
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

    /** Returns true if the path is a round rect.
     *
     * @param rrect  Returns the bounding rect and radii of this round rect.
     * @param dir    is the rrect CCW (or CW if false).
     * @param start  indicates where the contour starts on the rrect (see
     *               SkPath::addRRect for intepretation of the index).
     *
     * @return true if this path is a round rect.
     *              Tracking whether a path is a round rect is considered an
     *              optimization for performance and so some paths that are in
     *              fact round rects can report false.
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

    /** Clear any lines and curves from the path, making it empty. This frees up
        internal storage associated with those segments.
        On Android, does not change fSourcePath.
    */
    void reset();

    /** Similar to reset(), in that all lines and curves are removed from the
        path. However, any internal storage for those lines/curves is retained,
        making reuse of the path potentially faster.
        On Android, does not change fSourcePath.
    */
    void rewind();

    /** Returns true if the path is empty (contains no lines or curves)

        @return true if the path is empty (contains no lines or curves)
    */
    bool isEmpty() const {
        SkDEBUGCODE(this->validate();)
        return 0 == fPathRef->countVerbs();
    }

    /** Return true if the last contour of this path ends with a close verb.
     */
    bool isLastContourClosed() const;

    /**
     *  Returns true if all of the points in this path are finite, meaning there
     *  are no infinities and no NaNs.
     */
    bool isFinite() const {
        SkDEBUGCODE(this->validate();)
        return fPathRef->isFinite();
    }

    /** Returns true if the path is volatile (i.e. should not be cached by devices.)
     */
    bool isVolatile() const {
        return SkToBool(fIsVolatile);
    }

    /** Specify whether this path is volatile. Paths are not volatile by
     default. Temporary paths that are discarded or modified after use should be
     marked as volatile. This provides a hint to the device that the path
     should not be cached. Providing this hint when appropriate can
     improve performance by avoiding unnecessary overhead and resource
     consumption on the device.
     */
    void setIsVolatile(bool isVolatile) {
        fIsVolatile = isVolatile;
    }

    /** Test a line for zero length

        @return true if the line is of zero length; otherwise false.
    */
    static bool IsLineDegenerate(const SkPoint& p1, const SkPoint& p2, bool exact) {
        return exact ? p1 == p2 : p1.equalsWithinTolerance(p2);
    }

    /** Test a quad for zero length

        @return true if the quad is of zero length; otherwise false.
    */
    static bool IsQuadDegenerate(const SkPoint& p1, const SkPoint& p2,
                                 const SkPoint& p3, bool exact) {
        return exact ? p1 == p2 && p2 == p3 : p1.equalsWithinTolerance(p2) &&
               p2.equalsWithinTolerance(p3);
    }

    /** Test a cubic curve for zero length

        @return true if the cubic is of zero length; otherwise false.
    */
    static bool IsCubicDegenerate(const SkPoint& p1, const SkPoint& p2,
                                  const SkPoint& p3, const SkPoint& p4, bool exact) {
        return exact ? p1 == p2 && p2 == p3 && p3 == p4 : p1.equalsWithinTolerance(p2) &&
               p2.equalsWithinTolerance(p3) &&
               p3.equalsWithinTolerance(p4);
    }

    /**
     *  Returns true if the path specifies a single line (i.e. it contains just
     *  a moveTo and a lineTo). If so, and line[] is not null, it sets the 2
     *  points in line[] to the end-points of the line. If the path is not a
     *  line, returns false and ignores line[].
     */
    bool isLine(SkPoint line[2]) const;

    /** Return the number of points in the path
     */
    int countPoints() const;

    /** Return the point at the specified index. If the index is out of range
         (i.e. is not 0 <= index < countPoints()) then the returned coordinates
         will be (0,0)
     */
    SkPoint getPoint(int index) const;

    /** Returns the number of points in the path. Up to max points are copied.

        @param points If not null, receives up to max points
        @param max The maximum number of points to copy into points
        @return the actual number of points in the path
    */
    int getPoints(SkPoint points[], int max) const;

    /** Return the number of verbs in the path
     */
    int countVerbs() const;

    /** Returns the number of verbs in the path. Up to max verbs are copied. The
        verbs are copied as one byte per verb.

        @param verbs If not null, receives up to max verbs
        @param max The maximum number of verbs to copy into verbs
        @return the actual number of verbs in the path
    */
    int getVerbs(uint8_t verbs[], int max) const;

    //! Swap contents of this and other. Guaranteed not to throw
    void swap(SkPath& other);

    /**
     *  Returns the bounds of the path's points. If the path contains zero points/verbs, this
     *  will return the "empty" rect [0, 0, 0, 0].
     *  Note: this bounds may be larger than the actual shape, since curves
     *  do not extend as far as their control points. Additionally this bound encompases all points,
     *  even isolated moveTos either preceeding or following the last non-degenerate contour.
    */
    const SkRect& getBounds() const {
        return fPathRef->getBounds();
    }

    /** Calling this will, if the internal cache of the bounds is out of date,
        update it so that subsequent calls to getBounds will be instantaneous.
        This also means that any copies or simple transformations of the path
        will inherit the cached bounds.
     */
    void updateBoundsCache() const {
        // for now, just calling getBounds() is sufficient
        this->getBounds();
    }

    /**
     *  Computes a bounds that is conservatively "snug" around the path. This assumes that the
     *  path will be filled. It does not attempt to collapse away contours that are logically
     *  empty (e.g. moveTo(x, y) + lineTo(x, y)) but will include them in the calculation.
     *
     *  It differs from getBounds() in that it will look at the snug bounds of curves, whereas
     *  getBounds() just returns the bounds of the control-points. Thus computing this may be
     *  slower than just calling getBounds().
     *
     *  If the path is empty (i.e. no points or verbs), it will return SkRect::MakeEmpty().
     */
    SkRect computeTightBounds() const;

    /**
     * Does a conservative test to see whether a rectangle is inside a path. Currently it only
     * will ever return true for single convex contour paths. The empty-status of the rect is not
     * considered (e.g. a rect that is a point can be inside a path). Points or line segments where
     * the rect edge touches the path border are not considered containment violations.
     */
    bool conservativelyContainsRect(const SkRect& rect) const;

    //  Construction methods

    /** Hint to the path to prepare for adding more points. This can allow the
        path to more efficiently grow its storage.

        @param extraPtCount The number of extra points the path should
                            preallocate for.
    */
    void incReserve(unsigned extraPtCount);

    /** Set the beginning of the next contour to the point (x,y).

        @param x    The x-coordinate of the start of a new contour
        @param y    The y-coordinate of the start of a new contour
    */
    void moveTo(SkScalar x, SkScalar y);

    /** Set the beginning of the next contour to the point

        @param p    The start of a new contour
    */
    void moveTo(const SkPoint& p) {
        this->moveTo(p.fX, p.fY);
    }

    /** Set the beginning of the next contour relative to the last point on the
        previous contour. If there is no previous contour, this is treated the
        same as moveTo().

        @param dx   The amount to add to the x-coordinate of the end of the
                    previous contour, to specify the start of a new contour
        @param dy   The amount to add to the y-coordinate of the end of the
                    previous contour, to specify the start of a new contour
    */
    void rMoveTo(SkScalar dx, SkScalar dy);

    /** Add a line from the last point to the specified point (x,y). If no
        moveTo() call has been made for this contour, the first point is
        automatically set to (0,0).

        @param x    The x-coordinate of the end of a line
        @param y    The y-coordinate of the end of a line
    */
    void lineTo(SkScalar x, SkScalar y);

    /** Add a line from the last point to the specified point. If no moveTo()
        call has been made for this contour, the first point is automatically
        set to (0,0).

        @param p    The end of a line
    */
    void lineTo(const SkPoint& p) {
        this->lineTo(p.fX, p.fY);
    }

    /** Same as lineTo, but the coordinates are considered relative to the last
        point on this contour. If there is no previous point, then a moveTo(0,0)
        is inserted automatically.

        @param dx   The amount to add to the x-coordinate of the previous point
                    on this contour, to specify a line
        @param dy   The amount to add to the y-coordinate of the previous point
                    on this contour, to specify a line
    */
    void rLineTo(SkScalar dx, SkScalar dy);

    /** Add a quadratic bezier from the last point, approaching control point
        (x1,y1), and ending at (x2,y2). If no moveTo() call has been made for
        this contour, the first point is automatically set to (0,0).

        @param x1   The x-coordinate of the control point on a quadratic curve
        @param y1   The y-coordinate of the control point on a quadratic curve
        @param x2   The x-coordinate of the end point on a quadratic curve
        @param y2   The y-coordinate of the end point on a quadratic curve
    */
    void quadTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2);

    /** Add a quadratic bezier from the last point, approaching control point
        p1, and ending at p2. If no moveTo() call has been made for this
        contour, the first point is automatically set to (0,0).

        @param p1   The control point on a quadratic curve
        @param p2   The end point on a quadratic curve
    */
    void quadTo(const SkPoint& p1, const SkPoint& p2) {
        this->quadTo(p1.fX, p1.fY, p2.fX, p2.fY);
    }

    /** Same as quadTo, but the coordinates are considered relative to the last
        point on this contour. If there is no previous point, then a moveTo(0,0)
        is inserted automatically.

        @param dx1   The amount to add to the x-coordinate of the last point on
                this contour, to specify the control point of a quadratic curve
        @param dy1   The amount to add to the y-coordinate of the last point on
                this contour, to specify the control point of a quadratic curve
        @param dx2   The amount to add to the x-coordinate of the last point on
                     this contour, to specify the end point of a quadratic curve
        @param dy2   The amount to add to the y-coordinate of the last point on
                     this contour, to specify the end point of a quadratic curve
    */
    void rQuadTo(SkScalar dx1, SkScalar dy1, SkScalar dx2, SkScalar dy2);

    void conicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                 SkScalar w);
    void conicTo(const SkPoint& p1, const SkPoint& p2, SkScalar w) {
        this->conicTo(p1.fX, p1.fY, p2.fX, p2.fY, w);
    }
    void rConicTo(SkScalar dx1, SkScalar dy1, SkScalar dx2, SkScalar dy2,
                  SkScalar w);

    /** Add a cubic bezier from the last point, approaching control points
        (x1,y1) and (x2,y2), and ending at (x3,y3). If no moveTo() call has been
        made for this contour, the first point is automatically set to (0,0).

        @param x1   The x-coordinate of the 1st control point on a cubic curve
        @param y1   The y-coordinate of the 1st control point on a cubic curve
        @param x2   The x-coordinate of the 2nd control point on a cubic curve
        @param y2   The y-coordinate of the 2nd control point on a cubic curve
        @param x3   The x-coordinate of the end point on a cubic curve
        @param y3   The y-coordinate of the end point on a cubic curve
    */
    void cubicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                 SkScalar x3, SkScalar y3);

    /** Add a cubic bezier from the last point, approaching control points p1
        and p2, and ending at p3. If no moveTo() call has been made for this
        contour, the first point is automatically set to (0,0).

        @param p1   The 1st control point on a cubic curve
        @param p2   The 2nd control point on a cubic curve
        @param p3   The end point on a cubic curve
    */
    void cubicTo(const SkPoint& p1, const SkPoint& p2, const SkPoint& p3) {
        this->cubicTo(p1.fX, p1.fY, p2.fX, p2.fY, p3.fX, p3.fY);
    }

    /** Same as cubicTo, but the coordinates are considered relative to the
        current point on this contour. If there is no previous point, then a
        moveTo(0,0) is inserted automatically.

        @param dx1   The amount to add to the x-coordinate of the last point on
                this contour, to specify the 1st control point of a cubic curve
        @param dy1   The amount to add to the y-coordinate of the last point on
                this contour, to specify the 1st control point of a cubic curve
        @param dx2   The amount to add to the x-coordinate of the last point on
                this contour, to specify the 2nd control point of a cubic curve
        @param dy2   The amount to add to the y-coordinate of the last point on
                this contour, to specify the 2nd control point of a cubic curve
        @param dx3   The amount to add to the x-coordinate of the last point on
                     this contour, to specify the end point of a cubic curve
        @param dy3   The amount to add to the y-coordinate of the last point on
                     this contour, to specify the end point of a cubic curve
    */
    void rCubicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                  SkScalar x3, SkScalar y3);

    /**
     *  Append the specified arc to the path. If the start of the arc is different from the path's
     *  current last point, then an automatic lineTo() is added to connect the current contour
     *  to the start of the arc. However, if the path is empty, then we call moveTo() with
     *  the first point of the arc. The sweep angle is treated mod 360.
     *
     *  @param oval The bounding oval defining the shape and size of the arc
     *  @param startAngle Starting angle (in degrees) where the arc begins
     *  @param sweepAngle Sweep angle (in degrees) measured clockwise. This is treated mod 360.
     *  @param forceMoveTo If true, always begin a new contour with the arc
     */
    void arcTo(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle, bool forceMoveTo);

    /**
     *  Append a line and arc to the current path. This is the same as the PostScript call "arct".
     */
    void arcTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar radius);

    /** Append a line and arc to the current path. This is the same as the
        PostScript call "arct".
    */
    void arcTo(const SkPoint p1, const SkPoint p2, SkScalar radius) {
        this->arcTo(p1.fX, p1.fY, p2.fX, p2.fY, radius);
    }

    enum ArcSize {
        /** the smaller of the two possible SVG arcs. */
        kSmall_ArcSize,
        /** the larger of the two possible SVG arcs. */
        kLarge_ArcSize,
    };

    /**
     *  Append an elliptical arc from the current point in the format used by SVG.
     *  The center of the ellipse is computed to satisfy the constraints below.
     *
     *  @param rx,ry The radii in the x and y directions respectively.
     *  @param xAxisRotate The angle in degrees relative to the x-axis.
     *  @param largeArc Determines whether the smallest or largest arc possible
     *         is drawn.
     *  @param sweep Determines if the arc should be swept in an anti-clockwise or
     *         clockwise direction. Note that this enum value is opposite the SVG
     *         arc sweep value.
     *  @param x,y The destination coordinates.
     */
    void arcTo(SkScalar rx, SkScalar ry, SkScalar xAxisRotate, ArcSize largeArc,
               Direction sweep, SkScalar x, SkScalar y);

    void arcTo(const SkPoint r, SkScalar xAxisRotate, ArcSize largeArc, Direction sweep,
               const SkPoint xy) {
        this->arcTo(r.fX, r.fY, xAxisRotate, largeArc, sweep, xy.fX, xy.fY);
    }

    /** Same as arcTo format used by SVG, but the destination coordinate is relative to the
     *  last point on this contour. If there is no previous point, then a
     *  moveTo(0,0) is inserted automatically.
     *
     *  @param rx,ry The radii in the x and y directions respectively.
     *  @param xAxisRotate The angle in degrees relative to the x-axis.
     *  @param largeArc Determines whether the smallest or largest arc possible
     *         is drawn.
     *  @param sweep Determines if the arc should be swept in an anti-clockwise or
     *         clockwise direction. Note that this enum value is opposite the SVG
     *         arc sweep value.
     *  @param dx,dy The destination coordinates relative to the last point.
     */
    void rArcTo(SkScalar rx, SkScalar ry, SkScalar xAxisRotate, ArcSize largeArc,
                Direction sweep, SkScalar dx, SkScalar dy);

    /** Close the current contour. If the current point is not equal to the
        first point of the contour, a line segment is automatically added.
    */
    void close();

    /**
     *  Returns whether or not a fill type is inverted
     *
     *  kWinding_FillType        -> false
     *  kEvenOdd_FillType        -> false
     *  kInverseWinding_FillType -> true
     *  kInverseEvenOdd_FillType -> true
     */
    static bool IsInverseFillType(FillType fill) {
        static_assert(0 == kWinding_FillType, "fill_type_mismatch");
        static_assert(1 == kEvenOdd_FillType, "fill_type_mismatch");
        static_assert(2 == kInverseWinding_FillType, "fill_type_mismatch");
        static_assert(3 == kInverseEvenOdd_FillType, "fill_type_mismatch");
        return (fill & 2) != 0;
    }

    /**
     *  Returns the equivalent non-inverted fill type to the given fill type
     *
     *  kWinding_FillType        -> kWinding_FillType
     *  kEvenOdd_FillType        -> kEvenOdd_FillType
     *  kInverseWinding_FillType -> kWinding_FillType
     *  kInverseEvenOdd_FillType -> kEvenOdd_FillType
     */
    static FillType ConvertToNonInverseFillType(FillType fill) {
        static_assert(0 == kWinding_FillType, "fill_type_mismatch");
        static_assert(1 == kEvenOdd_FillType, "fill_type_mismatch");
        static_assert(2 == kInverseWinding_FillType, "fill_type_mismatch");
        static_assert(3 == kInverseEvenOdd_FillType, "fill_type_mismatch");
        return (FillType)(fill & 1);
    }

    /**
     *  Chop a conic into N quads, stored continguously in pts[], where
     *  N = 1 << pow2. The amount of storage needed is (1 + 2 * N)
     */
    static int ConvertConicToQuads(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2,
                                   SkScalar w, SkPoint pts[], int pow2);

    /**
     *  Returns true if the path specifies a rectangle.
     *
     *  If this returns false, then all output parameters are ignored, and left
     *  unchanged. If this returns true, then each of the output parameters
     *  are checked for NULL. If they are not, they return their value.
     *
     *  @param rect If not null, set to the bounds of the rectangle.
     *              Note : this bounds may be smaller than the path's bounds, since it is just
     *              the bounds of the "drawable" parts of the path. e.g. a trailing MoveTo would
     *              be ignored in this rect, but not by the path's bounds
     *  @param isClosed If not null, set to true if the path is closed
     *  @param direction If not null, set to the rectangle's direction
     *  @return true if the path specifies a rectangle
     */
    bool isRect(SkRect* rect, bool* isClosed = NULL, Direction* direction = NULL) const;

    /** Returns true if the path specifies a pair of nested rectangles, or would draw a
        pair of nested rectangles when filled. If so, and if
        rect is not null, set rect[0] to the outer rectangle and rect[1] to the inner
        rectangle. If so, and dirs is not null, set dirs[0] to the direction of
        the outer rectangle and dirs[1] to the direction of the inner rectangle. If
        the path does not specify a pair of nested rectangles, return
        false and ignore rect and dirs.

        @param rect If not null, returns the path as a pair of nested rectangles
        @param dirs If not null, returns the direction of the rects
        @return true if the path describes a pair of nested rectangles
    */
    bool isNestedFillRects(SkRect rect[2], Direction dirs[2] = NULL) const;

    /**
     *  Add a closed rectangle contour to the path
     *  @param rect The rectangle to add as a closed contour to the path
     *  @param dir  The direction to wind the rectangle's contour.
     *
     *  Note: the contour initial point index is 0 (as defined below).
     */
    void addRect(const SkRect& rect, Direction dir = kCW_Direction);

    /**
     *  Add a closed rectangle contour to the path
     *  @param rect  The rectangle to add as a closed contour to the path
     *  @param dir   The direction to wind the rectangle's contour.
     *  @param start Initial point of the contour (initial moveTo), expressed as
     *               a corner index, starting in the upper-left position, clock-wise:
     *
     *  0         1
     *   *-------*
     *   |       |
     *   *-------*
     *  3         2
     */
    void addRect(const SkRect& rect, Direction dir, unsigned start);

    /**
     *  Add a closed rectangle contour to the path
     *
     *  @param left     The left side of a rectangle to add as a closed contour
     *                  to the path
     *  @param top      The top of a rectangle to add as a closed contour to the
     *                  path
     *  @param right    The right side of a rectangle to add as a closed contour
     *                  to the path
     *  @param bottom   The bottom of a rectangle to add as a closed contour to
     *                  the path
     *  @param dir  The direction to wind the rectangle's contour.
     *
     *  Note: the contour initial point index is 0 (as defined above).
     */
    void addRect(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom,
                 Direction dir = kCW_Direction);

    /**
     *  Add a closed oval contour to the path
     *
     *  @param oval The bounding oval to add as a closed contour to the path
     *  @param dir  The direction to wind the oval's contour.
     *
     *  Note: the contour initial point index is 1 (as defined below).
     */
    void addOval(const SkRect& oval, Direction dir = kCW_Direction);

    /**
     *  Add a closed oval contour to the path
     *
     *  @param oval  The bounding oval to add as a closed contour to the path
     *  @param dir   The direction to wind the oval's contour.
     *  @param start Initial point of the contour (initial moveTo), expressed
     *               as an ellipse vertex index, starting at the top, clock-wise
     *               (90/0/270/180deg order):
     *
     *        0
     *       -*-
     *     |     |
     *   3 *     * 1
     *     |     |
     *       -*-
     *        2
     */
    void addOval(const SkRect& oval, Direction dir, unsigned start);

    /**
     *  Add a closed circle contour to the path. The circle contour begins at
     *  the right-most point (as though 1 were passed to addOval's 'start' param).
     *
     *  @param x        The x-coordinate of the center of a circle to add as a
     *                  closed contour to the path
     *  @param y        The y-coordinate of the center of a circle to add as a
     *                  closed contour to the path
     *  @param radius   The radius of a circle to add as a closed contour to the
     *                  path
     *  @param dir  The direction to wind the circle's contour.
     */
    void addCircle(SkScalar x, SkScalar y, SkScalar radius,
                   Direction dir = kCW_Direction);

    /** Add the specified arc to the path as a new contour.

        @param oval The bounds of oval used to define the size of the arc
        @param startAngle Starting angle (in degrees) where the arc begins
        @param sweepAngle Sweep angle (in degrees) measured clockwise
    */
    void addArc(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle);

    /**
     *  Add a closed round-rectangle contour to the path
     *  @param rect The bounds of a round-rectangle to add as a closed contour
     *  @param rx   The x-radius of the rounded corners on the round-rectangle
     *  @param ry   The y-radius of the rounded corners on the round-rectangle
     *  @param dir  The direction to wind the rectangle's contour.
     */
    void addRoundRect(const SkRect& rect, SkScalar rx, SkScalar ry,
                      Direction dir = kCW_Direction);

    /**
     *  Add a closed round-rectangle contour to the path. Each corner receives
     *  two radius values [X, Y]. The corners are ordered top-left, top-right,
     *  bottom-right, bottom-left.
     *  @param rect The bounds of a round-rectangle to add as a closed contour
     *  @param radii Array of 8 scalars, 4 [X,Y] pairs for each corner
     *  @param dir  The direction to wind the rectangle's contour.
     * Note: The radii here now go through the same constraint handling as the
     *       SkRRect radii (i.e., either radii at a corner being 0 implies a
     *       sqaure corner and oversized radii are proportionally scaled down).
     */
    void addRoundRect(const SkRect& rect, const SkScalar radii[],
                      Direction dir = kCW_Direction);

    /**
     *  Add an SkRRect contour to the path
     *  @param rrect The rounded rect to add as a closed contour
     *  @param dir   The winding direction for the new contour.
     *
     *  Note: the contour initial point index is either 6 (for dir == kCW_Direction)
     *        or 7 (for dir == kCCW_Direction), as defined below.
     *
     */
    void addRRect(const SkRRect& rrect, Direction dir = kCW_Direction);

    /**
     *  Add an SkRRect contour to the path
     *  @param rrect The rounded rect to add as a closed contour
     *  @param dir   The winding direction for the new contour.
     *  @param start Initial point of the contour (initial moveTo), expressed as
     *               an index of the radii minor/major points, ordered clock-wise:
     *
     *      0    1
     *      *----*
     *   7 *      * 2
     *     |      |
     *   6 *      * 3
     *      *----*
     *      5    4
     */
    void addRRect(const SkRRect& rrect, Direction dir, unsigned start);

    /**
     *  Add a new contour made of just lines. This is just a fast version of
     *  the following:
     *      this->moveTo(pts[0]);
     *      for (int i = 1; i < count; ++i) {
     *          this->lineTo(pts[i]);
     *      }
     *      if (close) {
     *          this->close();
     *      }
     */
    void addPoly(const SkPoint pts[], int count, bool close);

    enum AddPathMode {
        /** Source path contours are added as new contours.
        */
        kAppend_AddPathMode,
        /** Path is added by extending the last contour of the destination path
            with the first contour of the source path. If the last contour of
            the destination path is closed, then it will not be extended.
            Instead, the start of source path will be extended by a straight
            line to the end point of the destination path.
        */
        kExtend_AddPathMode
    };

    /** Add a copy of src to the path, offset by (dx,dy)
        @param src  The path to add as a new contour
        @param dx   The amount to translate the path in X as it is added
        @param dx   The amount to translate the path in Y as it is added
    */
    void addPath(const SkPath& src, SkScalar dx, SkScalar dy,
                 AddPathMode mode = kAppend_AddPathMode);

    /** Add a copy of src to the path
    */
    void addPath(const SkPath& src, AddPathMode mode = kAppend_AddPathMode) {
        SkMatrix m;
        m.reset();
        this->addPath(src, m, mode);
    }

    /** Add a copy of src to the path, transformed by matrix
        @param src  The path to add as a new contour
        @param matrix  Transform applied to src
        @param mode  Determines how path is added
    */
    void addPath(const SkPath& src, const SkMatrix& matrix, AddPathMode mode = kAppend_AddPathMode);

    /**
     *  Same as addPath(), but reverses the src input
     */
    void reverseAddPath(const SkPath& src);

    /** Offset the path by (dx,dy), returning true on success

        @param dx   The amount in the X direction to offset the entire path
        @param dy   The amount in the Y direction to offset the entire path
        @param dst  The translated path is written here
    */
    void offset(SkScalar dx, SkScalar dy, SkPath* dst) const;

    /** Offset the path by (dx,dy), returning true on success

        @param dx   The amount in the X direction to offset the entire path
        @param dy   The amount in the Y direction to offset the entire path
    */
    void offset(SkScalar dx, SkScalar dy) {
        this->offset(dx, dy, this);
    }

    /** Transform the points in this path by matrix, and write the answer into
        dst.

        @param matrix   The matrix to apply to the path
        @param dst      The transformed path is written here
    */
    void transform(const SkMatrix& matrix, SkPath* dst) const;

    /** Transform the points in this path by matrix

        @param matrix The matrix to apply to the path
    */
    void transform(const SkMatrix& matrix) {
        this->transform(matrix, this);
    }

    /** Return the last point on the path. If no points have been added, (0,0)
        is returned. If there are no points, this returns false, otherwise it
        returns true.

        @param lastPt   The last point on the path is returned here
    */
    bool getLastPt(SkPoint* lastPt) const;

    /** Set the last point on the path. If no points have been added,
        moveTo(x,y) is automatically called.

        @param x    The new x-coordinate for the last point
        @param y    The new y-coordinate for the last point
    */
    void setLastPt(SkScalar x, SkScalar y);

    /** Set the last point on the path. If no points have been added, moveTo(p)
        is automatically called.

        @param p    The new location for the last point
    */
    void setLastPt(const SkPoint& p) {
        this->setLastPt(p.fX, p.fY);
    }

    enum SegmentMask {
        kLine_SegmentMask   = 1 << 0,
        kQuad_SegmentMask   = 1 << 1,
        kConic_SegmentMask  = 1 << 2,
        kCubic_SegmentMask  = 1 << 3,
    };

    /**
     *  Returns a mask, where each bit corresponding to a SegmentMask is
     *  set if the path contains 1 or more segments of that type.
     *  Returns 0 for an empty path (no segments).
     */
    uint32_t getSegmentMasks() const { return fPathRef->getSegmentMasks(); }

    enum Verb {
        kMove_Verb,     //!< iter.next returns 1 point
        kLine_Verb,     //!< iter.next returns 2 points
        kQuad_Verb,     //!< iter.next returns 3 points
        kConic_Verb,    //!< iter.next returns 3 points + iter.conicWeight()
        kCubic_Verb,    //!< iter.next returns 4 points
        kClose_Verb,    //!< iter.next returns 0 points
        kDone_Verb,     //!< iter.next returns 0 points
    };

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
        Iter();
        Iter(const SkPath&, bool forceClose);

        void setPath(const SkPath&, bool forceClose);

        /** Return the next verb in this iteration of the path. When all
            segments have been visited, return kDone_Verb.

            @param  pts The points representing the current verb and/or segment
            @param doConsumeDegerates If true, first scan for segments that are
                   deemed degenerate (too short) and skip those.
            @param exact if doConsumeDegenerates is true and exact is true, skip only
                   degenerate elements with lengths exactly equal to zero. If exact
                   is false, skip degenerate elements with lengths close to zero. If
                   doConsumeDegenerates is false, exact has no effect.
            @return The verb for the current segment
        */
        Verb next(SkPoint pts[4], bool doConsumeDegerates = true, bool exact = false) {
            if (doConsumeDegerates) {
                this->consumeDegenerateSegments(exact);
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
        SkBool8         fForceClose;
        SkBool8         fNeedClose;
        SkBool8         fCloseLine;
        SkBool8         fSegmentState;

        inline const SkPoint& cons_moveTo();
        Verb autoClose(SkPoint pts[2]);
        void consumeDegenerateSegments(bool exact);
        Verb doNext(SkPoint pts[4]);
    };

    /** Iterate through the verbs in the path, providing the associated points.
    */
    class SK_API RawIter {
    public:
        RawIter() {}
        RawIter(const SkPath& path) {
            setPath(path);
        }

        void setPath(const SkPath& path) {
            fRawIter.setPathRef(*path.fPathRef.get());
        }

        /** Return the next verb in this iteration of the path. When all
            segments have been visited, return kDone_Verb.

            @param  pts The points representing the current verb and/or segment
                        This must not be NULL.
            @return The verb for the current segment
        */
        Verb next(SkPoint pts[4]) {
            return (Verb) fRawIter.next(pts);
        }

        /** Return what the next verb will be, but do not visit the next segment.

            @return The verb for the next segment
        */
        Verb peek() const {
            return (Verb) fRawIter.peek();
        }

        SkScalar conicWeight() const {
            return fRawIter.conicWeight();
        }

    private:
        SkPathRef::Iter fRawIter;
        friend class SkPath;
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

    /** Returns a non-zero, globally unique value corresponding to the set of verbs
        and points in the path (but not the fill type [except on Android skbug.com/1762]).
        Each time the path is modified, a different generation ID will be returned.
    */
    uint32_t getGenerationID() const;

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    static const int kPathRefGenIDBitCnt = 30; // leave room for the fill type (skbug.com/1762)
#else
    static const int kPathRefGenIDBitCnt = 32;
#endif

    SkDEBUGCODE(void validate() const;)
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

    // called by stroker to see if all points are equal and worthy of a cap
    // equivalent to a short-circuit version of getBounds().isEmpty() 
    bool isZeroLength() const;

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
