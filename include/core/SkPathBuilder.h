/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathBuilder_DEFINED
#define SkPathBuilder_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/SkPathRef.h"
#include "include/private/base/SkTArray.h"

#include <optional>
#include <tuple>

class SkRRect;

class SK_API SkPathBuilder {
public:
    /** Constructs an empty SkPathBuilder. By default, SkPathBuilder has no verbs, no SkPoint, and
        no weights. FillType is set to kWinding.

        @return  empty SkPathBuilder
    */
    SkPathBuilder();

    /** Constructs an empty SkPathBuilder with the given FillType. By default, SkPathBuilder has no
        verbs, no SkPoint, and no weights.

        @param fillType  SkPathFillType to set on the SkPathBuilder.
        @return          empty SkPathBuilder
    */
    SkPathBuilder(SkPathFillType fillType);

    /** Constructs an SkPathBuilder that is a copy of an existing SkPath.
        Copies the FillType and replays all of the verbs from the SkPath into the SkPathBuilder.

        @param path  SkPath to copy
        @return      SkPathBuilder
    */
    SkPathBuilder(const SkPath& path);

    SkPathBuilder(const SkPathBuilder&) = default;
    ~SkPathBuilder();

    /** Sets an SkPathBuilder to be a copy of an existing SkPath.
        Copies the FillType and replays all of the verbs from the SkPath into the SkPathBuilder.

        @param path  SkPath to copy
        @return      SkPathBuilder
    */
    SkPathBuilder& operator=(const SkPath&);

    SkPathBuilder& operator=(const SkPathBuilder&) = default;

    /** Returns SkPathFillType, the rule used to fill SkPath.

        @return  current SkPathFillType setting
    */
    SkPathFillType fillType() const { return fFillType; }

    /** Returns minimum and maximum axes values of SkPoint array.
        Returns (0, 0, 0, 0) if SkPathBuilder contains no points. Returned bounds width and height
        may be larger or smaller than area affected when SkPath is drawn.

        SkRect returned includes all SkPoint added to SkPathBuilder, including SkPoint associated
        with kMove_Verb that define empty contours.

        @return  bounds of all SkPoint in SkPoint array
    */
    SkRect computeBounds() const;

    /** Returns an SkPath representing the current state of the SkPathBuilder. The builder is
        unchanged after returning the path.

        @return  SkPath representing the current state of the builder.
     */
    SkPath snapshot() const;

    /** Returns an SkPath representing the current state of the SkPathBuilder. The builder is
        reset to empty after returning the path.

        @return  SkPath representing the current state of the builder.
     */
    SkPath detach();

    /** Sets SkPathFillType, the rule used to fill SkPath. While there is no
        check that ft is legal, values outside of SkPathFillType are not supported.

        @param ft  SkPathFillType to be used by SKPaths generated from this builder.
        @return    reference to SkPathBuilder
    */
    SkPathBuilder& setFillType(SkPathFillType ft) { fFillType = ft; return *this; }

    /** Specifies whether SkPath is volatile; whether it will be altered or discarded
        by the caller after it is drawn. SkPath by default have volatile set false, allowing
        Skia to attach a cache of data which speeds repeated drawing.

        Mark temporary paths, discarded or modified after use, as volatile
        to inform Skia that the path need not be cached.

        Mark animating SkPath volatile to improve performance.
        Mark unchanging SkPath non-volatile to improve repeated rendering.

        raster surface SkPath draws are affected by volatile for some shadows.
        GPU surface SkPath draws are affected by volatile for some shadows and concave geometries.

        @param isVolatile  true if caller will alter SkPath after drawing
        @return            reference to SkPathBuilder
    */
    SkPathBuilder& setIsVolatile(bool isVolatile) { fIsVolatile = isVolatile; return *this; }

    /** Sets SkPathBuilder to its initial state.
        Removes verb array, SkPoint array, and weights, and sets FillType to kWinding.
        Internal storage associated with SkPathBuilder is released.

        @return  reference to SkPathBuilder
    */
    SkPathBuilder& reset();

    /** Adds beginning of contour at SkPoint p.

        @param p  contour start
        @return   reference to SkPathBuilder
    */
    SkPathBuilder& moveTo(SkPoint pt);

    /** Adds beginning of contour at SkPoint (x, y).

        @param x  x-axis value of contour start
        @param y  y-axis value of contour start
        @return   reference to SkPathBuilder
    */
    SkPathBuilder& moveTo(SkScalar x, SkScalar y) { return this->moveTo(SkPoint::Make(x, y)); }

    /** Adds line from last point to SkPoint p. If SkPathBuilder is empty, or last SkPath::Verb is
        kClose_Verb, last point is set to (0, 0) before adding line.

        lineTo() first appends kMove_Verb to verb array and (0, 0) to SkPoint array, if needed.
        lineTo() then appends kLine_Verb to verb array and SkPoint p to SkPoint array.

        @param p  end SkPoint of added line
        @return   reference to SkPathBuilder
    */
    SkPathBuilder& lineTo(SkPoint pt);

    /** Adds line from last point to (x, y). If SkPathBuilder is empty, or last SkPath::Verb is
        kClose_Verb, last point is set to (0, 0) before adding line.

        lineTo() appends kMove_Verb to verb array and (0, 0) to SkPoint array, if needed.
        lineTo() then appends kLine_Verb to verb array and (x, y) to SkPoint array.

        @param x  end of added line on x-axis
        @param y  end of added line on y-axis
        @return   reference to SkPathBuilder
    */
    SkPathBuilder& lineTo(SkScalar x, SkScalar y) { return this->lineTo(SkPoint::Make(x, y)); }

    /** Adds quad from last point towards SkPoint p1, to SkPoint p2.
        If SkPathBuilder is empty, or last SkPath::Verb is kClose_Verb, last point is set to (0, 0)
        before adding quad.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array, if needed;
        then appends kQuad_Verb to verb array; and SkPoint p1, p2
        to SkPoint array.

        @param p1  control SkPoint of added quad
        @param p2  end SkPoint of added quad
        @return    reference to SkPathBuilder
    */
    SkPathBuilder& quadTo(SkPoint pt1, SkPoint pt2);

    /** Adds quad from last point towards (x1, y1), to (x2, y2).
        If SkPath is empty, or last SkPath::Verb is kClose_Verb, last point is set to (0, 0)
        before adding quad.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array, if needed;
        then appends kQuad_Verb to verb array; and (x1, y1), (x2, y2)
        to SkPoint array.

        @param x1  control SkPoint of quad on x-axis
        @param y1  control SkPoint of quad on y-axis
        @param x2  end SkPoint of quad on x-axis
        @param y2  end SkPoint of quad on y-axis
        @return    reference to SkPath

        example: https://fiddle.skia.org/c/@Path_quadTo
    */
    SkPathBuilder& quadTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2) {
        return this->quadTo(SkPoint::Make(x1, y1), SkPoint::Make(x2, y2));
    }

    /** Adds quad from last point towards the first SkPoint in pts, to the second.
        If SkPathBuilder is empty, or last SkPath::Verb is kClose_Verb, last point is set to (0, 0)
        before adding quad.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array, if needed;
        then appends kQuad_Verb to verb array; and the SkPoints to SkPoint array.

        @param pts  control point and endpoint of added quad.
        @return     reference to SkPathBuilder
    */
    SkPathBuilder& quadTo(const SkPoint pts[2]) { return this->quadTo(pts[0], pts[1]); }

    /** Adds conic from last point towards pt1, to pt2, weighted by w.
        If SkPathBuilder is empty, or last SkPath::Verb is kClose_Verb, last point is set to (0, 0)
        before adding conic.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array, if needed.

        If w is finite and not one, appends kConic_Verb to verb array;
        and pt1, pt2 to SkPoint array; and w to conic weights.

        If w is one, appends kQuad_Verb to verb array, and
        pt1, pt2 to SkPoint array.

        If w is not finite, appends kLine_Verb twice to verb array, and
        pt1, pt2 to SkPoint array.

        @param pt1  control SkPoint of conic
        @param pt2  end SkPoint of conic
        @param w   weight of added conic
        @return    reference to SkPathBuilder
    */
    SkPathBuilder& conicTo(SkPoint pt1, SkPoint pt2, SkScalar w);

    /** Adds conic from last point towards (x1, y1), to (x2, y2), weighted by w.
        If SkPathBuilder is empty, or last SkPath::Verb is kClose_Verb, last point is set to (0, 0)
        before adding conic.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array, if needed.

        If w is finite and not one, appends kConic_Verb to verb array;
        and (x1, y1), (x2, y2) to SkPoint array; and w to conic weights.

        If w is one, appends kQuad_Verb to verb array, and
        (x1, y1), (x2, y2) to SkPoint array.

        If w is not finite, appends kLine_Verb twice to verb array, and
        (x1, y1), (x2, y2) to SkPoint array.

        @param x1  control SkPoint of conic on x-axis
        @param y1  control SkPoint of conic on y-axis
        @param x2  end SkPoint of conic on x-axis
        @param y2  end SkPoint of conic on y-axis
        @param w   weight of added conic
        @return    reference to SkPathBuilder
    */
    SkPathBuilder& conicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar w) {
        return this->conicTo(SkPoint::Make(x1, y1), SkPoint::Make(x2, y2), w);
    }

    /** Adds conic from last point towards SkPoint p1, to SkPoint p2, weighted by w.
        If SkPathBuilder is empty, or last SkPath::Verb is kClose_Verb, last point is set to (0, 0)
        before adding conic.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array, if needed.

        If w is finite and not one, appends kConic_Verb to verb array;
        and SkPoint p1, p2 to SkPoint array; and w to conic weights.

        If w is one, appends kQuad_Verb to verb array, and SkPoint p1, p2
        to SkPoint array.

        If w is not finite, appends kLine_Verb twice to verb array, and
        SkPoint p1, p2 to SkPoint array.

        @param p1  control SkPoint of added conic
        @param p2  end SkPoint of added conic
        @param w   weight of added conic
        @return    reference to SkPathBuilder
    */
    SkPathBuilder& conicTo(const SkPoint pts[2], SkScalar w) {
        return this->conicTo(pts[0], pts[1], w);
    }

    /** Adds cubic from last point towards SkPoint p1, then towards SkPoint p2, ending at
        SkPoint p3. If SkPathBuilder is empty, or last SkPath::Verb is kClose_Verb, last point is
        set to (0, 0) before adding cubic.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array, if needed;
        then appends kCubic_Verb to verb array; and SkPoint p1, p2, p3
        to SkPoint array.

        @param p1  first control SkPoint of cubic
        @param p2  second control SkPoint of cubic
        @param p3  end SkPoint of cubic
        @return    reference to SkPathBuilder
    */
    SkPathBuilder& cubicTo(SkPoint pt1, SkPoint pt2, SkPoint pt3);

    /** Adds cubic from last point towards (x1, y1), then towards (x2, y2), ending at
        (x3, y3). If SkPathBuilder is empty, or last SkPath::Verb is kClose_Verb, last point is set
        to (0, 0) before adding cubic.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array, if needed;
        then appends kCubic_Verb to verb array; and (x1, y1), (x2, y2), (x3, y3)
        to SkPoint array.

        @param x1  first control SkPoint of cubic on x-axis
        @param y1  first control SkPoint of cubic on y-axis
        @param x2  second control SkPoint of cubic on x-axis
        @param y2  second control SkPoint of cubic on y-axis
        @param x3  end SkPoint of cubic on x-axis
        @param y3  end SkPoint of cubic on y-axis
        @return    reference to SkPathBuilder
    */
    SkPathBuilder& cubicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar x3, SkScalar y3) {
        return this->cubicTo(SkPoint::Make(x1, y1), SkPoint::Make(x2, y2), SkPoint::Make(x3, y3));
    }

    /** Adds cubic from last point towards the first SkPoint, then towards the second, ending at
        the third. If SkPathBuilder is empty, or last SkPath::Verb is kClose_Verb, last point is
        set to (0, 0) before adding cubic.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array, if needed;
        then appends kCubic_Verb to verb array; and SkPoint p1, p2, p3
        to SkPoint array.

        @param pts  first and second control SkPoints of cubic, and end SkPoint.
        @return     reference to SkPathBuilder
    */
    SkPathBuilder& cubicTo(const SkPoint pts[3]) {
        return this->cubicTo(pts[0], pts[1], pts[2]);
    }

    /** Appends kClose_Verb to SkPathBuilder. A closed contour connects the first and last SkPoint
        with line, forming a continuous loop. Open and closed contour draw the same
        with SkPaint::kFill_Style. With SkPaint::kStroke_Style, open contour draws
        SkPaint::Cap at contour start and end; closed contour draws
        SkPaint::Join at contour start and end.

        close() has no effect if SkPathBuilder is empty or last SkPath SkPath::Verb is kClose_Verb.

        @return  reference to SkPathBuilder
    */
    SkPathBuilder& close();

    /** Append a series of lineTo(...)

        @param pts    span of SkPoint
        @return reference to SkPathBuilder.
    */
    SkPathBuilder& polylineTo(SkSpan<const SkPoint> pts);

    // Relative versions of segments, relative to the previous position.

    /** Adds line from last point to vector given by pt. If SkPathBuilder is empty, or last
        SkPath::Verb is kClose_Verb, last point is set to (0, 0) before adding line.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array, if needed;
        then appends kLine_Verb to verb array and line end to SkPoint array.
        Line end is last point plus vector given by pt.
        Function name stands for "relative line to".

        @param pt  vector offset from last point to line end
        @return    reference to SkPathBuilder
    */
    SkPathBuilder& rLineTo(SkPoint pt);

    /** Adds line from last point to vector (dx, dy). If SkPathBuilder is empty, or last
        SkPath::Verb is kClose_Verb, last point is set to (0, 0) before adding line.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array, if needed;
        then appends kLine_Verb to verb array and line end to SkPoint array.
        Line end is last point plus vector (dx, dy).
        Function name stands for "relative line to".

        @param dx  offset from last point to line end on x-axis
        @param dy  offset from last point to line end on y-axis
        @return    reference to SkPathBuilder
    */
    SkPathBuilder& rLineTo(SkScalar x, SkScalar y) { return this->rLineTo({x, y}); }

    /** Adds quad from last point towards vector pt1, to vector pt2.
        If SkPathBuilder is empty, or last SkPath::Verb
        is kClose_Verb, last point is set to (0, 0) before adding quad.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array,
        if needed; then appends kQuad_Verb to verb array; and appends quad
        control and quad end to SkPoint array.
        Quad control is last point plus vector pt1.
        Quad end is last point plus vector pt2.
        Function name stands for "relative quad to".

        @param pt1  offset vector from last point to quad control
        @param pt2  offset vector from last point to quad end
        @return     reference to SkPathBuilder
    */
    SkPathBuilder& rQuadTo(SkPoint pt1, SkPoint pt2);

    /** Adds quad from last point towards vector (dx1, dy1), to vector (dx2, dy2).
        If SkPathBuilder is empty, or last SkPath::Verb
        is kClose_Verb, last point is set to (0, 0) before adding quad.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array,
        if needed; then appends kQuad_Verb to verb array; and appends quad
        control and quad end to SkPoint array.
        Quad control is last point plus vector (dx1, dy1).
        Quad end is last point plus vector (dx2, dy2).
        Function name stands for "relative quad to".

        @param dx1  offset from last point to quad control on x-axis
        @param dy1  offset from last point to quad control on y-axis
        @param dx2  offset from last point to quad end on x-axis
        @param dy2  offset from last point to quad end on y-axis
        @return     reference to SkPathBuilder
    */
    SkPathBuilder& rQuadTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2) {
        return this->rQuadTo({x1, y1}, {x2, y2});
    }

    /** Adds conic from last point towards vector p1, to vector p2,
        weighted by w. If SkPathBuilder is empty, or last SkPath::Verb
        is kClose_Verb, last point is set to (0, 0) before adding conic.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array,
        if needed.

        If w is finite and not one, next appends kConic_Verb to verb array,
        and w is recorded as conic weight; otherwise, if w is one, appends
        kQuad_Verb to verb array; or if w is not finite, appends kLine_Verb
        twice to verb array.

        In all cases appends SkPoint control and end to SkPoint array.
        control is last point plus vector p1.
        end is last point plus vector p2.

        Function name stands for "relative conic to".

        @param p1  offset vector from last point to conic control
        @param p2  offset vector from last point to conic end
        @param w   weight of added conic
        @return    reference to SkPathBuilder
    */
    SkPathBuilder& rConicTo(SkPoint p1, SkPoint p2, SkScalar w);

    /** Adds conic from last point towards vector (dx1, dy1), to vector (dx2, dy2),
        weighted by w. If SkPathBuilder is empty, or last SkPath::Verb
        is kClose_Verb, last point is set to (0, 0) before adding conic.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array,
        if needed.

        If w is finite and not one, next appends kConic_Verb to verb array,
        and w is recorded as conic weight; otherwise, if w is one, appends
        kQuad_Verb to verb array; or if w is not finite, appends kLine_Verb
        twice to verb array.

        In all cases appends SkPoint control and end to SkPoint array.
        control is last point plus vector (dx1, dy1).
        end is last point plus vector (dx2, dy2).

        Function name stands for "relative conic to".

        @param dx1  offset from last point to conic control on x-axis
        @param dy1  offset from last point to conic control on y-axis
        @param dx2  offset from last point to conic end on x-axis
        @param dy2  offset from last point to conic end on y-axis
        @param w    weight of added conic
        @return     reference to SkPathBuilder
    */
    SkPathBuilder& rConicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar w) {
        return this->rConicTo({x1, y1}, {x2, y2}, w);
    }

    /** Adds cubic from last point towards vector pt1, then towards
        vector pt2, to vector pt3.
        If SkPathBuilder is empty, or last SkPath::Verb
        is kClose_Verb, last point is set to (0, 0) before adding cubic.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array,
        if needed; then appends kCubic_Verb to verb array; and appends cubic
        control and cubic end to SkPoint array.
        Cubic control is last point plus vector (dx1, dy1).
        Cubic end is last point plus vector (dx2, dy2).
        Function name stands for "relative cubic to".

        @param pt1  offset vector from last point to first cubic control
        @param pt2  offset vector from last point to second cubic control
        @param pt3  offset vector from last point to cubic end
        @return    reference to SkPathBuilder
    */
    SkPathBuilder& rCubicTo(SkPoint pt1, SkPoint pt2, SkPoint pt3);

    /** Adds cubic from last point towards vector (dx1, dy1), then towards
        vector (dx2, dy2), to vector (dx3, dy3).
        If SkPathBuilder is empty, or last SkPath::Verb
        is kClose_Verb, last point is set to (0, 0) before adding cubic.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array,
        if needed; then appends kCubic_Verb to verb array; and appends cubic
        control and cubic end to SkPoint array.
        Cubic control is last point plus vector (dx1, dy1).
        Cubic end is last point plus vector (dx2, dy2).
        Function name stands for "relative cubic to".

        @param dx1  offset from last point to first cubic control on x-axis
        @param dy1  offset from last point to first cubic control on y-axis
        @param dx2  offset from last point to second cubic control on x-axis
        @param dy2  offset from last point to second cubic control on y-axis
        @param dx3  offset from last point to cubic end on x-axis
        @param dy3  offset from last point to cubic end on y-axis
        @return    reference to SkPathBuilder
    */
    SkPathBuilder& rCubicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar x3, SkScalar y3) {
        return this->rCubicTo({x1, y1}, {x2, y2}, {x3, y3});
    }

    // Arcs

    /** Appends arc to the builder. Arc added is part of ellipse
        bounded by oval, from startAngle through sweepAngle. Both startAngle and
        sweepAngle are measured in degrees, where zero degrees is aligned with the
        positive x-axis, and positive sweeps extends arc clockwise.

        arcTo() adds line connecting the builder's last point to initial arc point if forceMoveTo
        is false and the builder is not empty. Otherwise, added contour begins with first point
        of arc. Angles greater than -360 and less than 360 are treated modulo 360.

        @param oval          bounds of ellipse containing arc
        @param startAngleDeg starting angle of arc in degrees
        @param sweepAngleDeg sweep, in degrees. Positive is clockwise; treated modulo 360
        @param forceMoveTo   true to start a new contour with arc
        @return              reference to the builder
    */
    SkPathBuilder& arcTo(const SkRect& oval, SkScalar startAngleDeg, SkScalar sweepAngleDeg,
                         bool forceMoveTo);

    /** Appends arc to SkPath, after appending line if needed. Arc is implemented by conic
        weighted to describe part of circle. Arc is contained by tangent from
        last SkPath point to p1, and tangent from p1 to p2. Arc
        is part of circle sized to radius, positioned so it touches both tangent lines.

        If last SkPath SkPoint does not start arc, arcTo() appends connecting line to SkPath.
        The length of vector from p1 to p2 does not affect arc.

        Arc sweep is always less than 180 degrees. If radius is zero, or if
        tangents are nearly parallel, arcTo() appends line from last SkPath SkPoint to p1.

        arcTo() appends at most one line and one conic.
        arcTo() implements the functionality of PostScript arct and HTML Canvas arcTo.

        @param p1      SkPoint common to pair of tangents
        @param p2      end of second tangent
        @param radius  distance from arc to circle center
        @return        reference to SkPath
    */
    SkPathBuilder& arcTo(SkPoint p1, SkPoint p2, SkScalar radius);

    enum ArcSize {
        kSmall_ArcSize, //!< smaller of arc pair
        kLarge_ArcSize, //!< larger of arc pair
    };

    /** Appends arc to SkPath. Arc is implemented by one or more conic weighted to describe
        part of oval with radii (r.fX, r.fY) rotated by xAxisRotate degrees. Arc curves
        from last SkPath SkPoint to (xy.fX, xy.fY), choosing one of four possible routes:
        clockwise or counterclockwise,
        and smaller or larger.

        Arc sweep is always less than 360 degrees. arcTo() appends line to xy if either
        radii are zero, or if last SkPath SkPoint equals (xy.fX, xy.fY). arcTo() scales radii r to
        fit last SkPath SkPoint and xy if both are greater than zero but too small to describe
        an arc.

        arcTo() appends up to four conic curves.
        arcTo() implements the functionality of SVG arc, although SVG sweep-flag value is
        opposite the integer value of sweep; SVG sweep-flag uses 1 for clockwise, while
        kCW_Direction cast to int is zero.

        @param r            radii on axes before x-axis rotation
        @param xAxisRotate  x-axis rotation in degrees; positive values are clockwise
        @param largeArc     chooses smaller or larger arc
        @param sweep        chooses clockwise or counterclockwise arc
        @param xy           end of arc
        @return             reference to SkPath
    */
    SkPathBuilder& arcTo(SkPoint r, SkScalar xAxisRotate, ArcSize largeArc, SkPathDirection sweep,
                         SkPoint xy);

    /** Appends arc to the builder, as the start of new contour. Arc added is part of ellipse
        bounded by oval, from startAngle through sweepAngle. Both startAngle and
        sweepAngle are measured in degrees, where zero degrees is aligned with the
        positive x-axis, and positive sweeps extends arc clockwise.

        If sweepAngle <= -360, or sweepAngle >= 360; and startAngle modulo 90 is nearly
        zero, append oval instead of arc. Otherwise, sweepAngle values are treated
        modulo 360, and arc may or may not draw depending on numeric rounding.

        @param oval          bounds of ellipse containing arc
        @param startAngleDeg starting angle of arc in degrees
        @param sweepAngleDeg sweep, in degrees. Positive is clockwise; treated modulo 360
        @return              reference to this builder
    */
    SkPathBuilder& addArc(const SkRect& oval, SkScalar startAngleDeg, SkScalar sweepAngleDeg);

    /** Adds a new contour to the SkPathBuilder, defined by the rect, and wound in the
        specified direction. The verbs added to the path will be:

        kMove, kLine, kLine, kLine, kClose

        start specifies which corner to begin the contour:
            0: upper-left  corner
            1: upper-right corner
            2: lower-right corner
            3: lower-left  corner

        This start point also acts as the implied beginning of the subsequent,
        contour, if it does not have an explicit moveTo(). e.g.

            path.addRect(...)
            // if we don't say moveTo() here, we will use the rect's start point
            path.lineTo(...)

        @param rect   SkRect to add as a closed contour
        @param dir    SkPath::Direction to orient the new contour
        @param start  initial corner of SkRect to add
        @return       reference to SkPathBuilder
     */
    SkPathBuilder& addRect(const SkRect&, SkPathDirection, unsigned startIndex);

    /** Adds a new contour to the SkPathBuilder, defined by the rect, and wound in the
        specified direction. The verbs added to the path will be:

        kMove, kLine, kLine, kLine, kClose

        The contour starts at the upper-left corner of the rect, which also acts as the implied
        beginning of the subsequent contour, if it does not have an explicit moveTo(). e.g.

            path.addRect(...)
            // if we don't say moveTo() here, we will use the rect's upper-left corner
            path.lineTo(...)

        @param rect   SkRect to add as a closed contour
        @param dir    SkPath::Direction to orient the new contour
        @return       reference to SkPathBuilder
     */
    SkPathBuilder& addRect(const SkRect& rect, SkPathDirection dir = SkPathDirection::kCW) {
        return this->addRect(rect, dir, 0);
    }

    /** Adds oval to SkPathBuilder, appending kMove_Verb, four kConic_Verb, and kClose_Verb.
        Oval is upright ellipse bounded by SkRect oval with radii equal to half oval width
        and half oval height. Oval begins at (oval.fRight, oval.centerY()) and continues
        clockwise if dir is kCW_Direction, counterclockwise if dir is kCCW_Direction.

        @param oval  bounds of ellipse added
        @param dir   SkPath::Direction to wind ellipse
        @return      reference to SkPathBuilder
    */
    SkPathBuilder& addOval(const SkRect&, SkPathDirection, unsigned startIndex);

    /** Appends SkRRect to SkPathBuilder, creating a new closed contour. If dir is kCW_Direction,
        SkRRect winds clockwise. If dir is kCCW_Direction, SkRRect winds counterclockwise.

        After appending, SkPathBuilder may be empty, or may contain: SkRect, oval, or SkRRect.

        @param rrect  SkRRect to add
        @param dir    SkPath::Direction to wind SkRRect
        @param start  index of initial point of SkRRect
        @return       reference to SkPathBuilder
    */
    SkPathBuilder& addRRect(const SkRRect& rrect, SkPathDirection, unsigned start);

    /** Appends SkRRect to SkPathBuilder, creating a new closed contour. If dir is kCW_Direction,
        SkRRect starts at top-left of the lower-left corner and winds clockwise. If dir is
        kCCW_Direction, SkRRect starts at the bottom-left of the upper-left corner and winds
        counterclockwise.

        After appending, SkPathBuilder may be empty, or may contain: SkRect, oval, or SkRRect.

        @param rrect  SkRRect to add
        @param dir    SkPath::Direction to wind SkRRect
        @return       reference to SkPathBuilder
    */
    SkPathBuilder& addRRect(const SkRRect& rrect, SkPathDirection dir = SkPathDirection::kCW) {
        // legacy start indices: 6 (CW) and 7 (CCW)
        return this->addRRect(rrect, dir, dir == SkPathDirection::kCW ? 6 : 7);
    }

    /** Adds oval to SkPathBuilder, appending kMove_Verb, four kConic_Verb, and kClose_Verb.
        Oval is upright ellipse bounded by SkRect oval with radii equal to half oval width
        and half oval height. Oval begins at start and continues
        clockwise if dir is kCW_Direction, counterclockwise if dir is kCCW_Direction.

        @param oval   bounds of ellipse added
        @param dir    SkPath::Direction to wind ellipse
        @return       reference to SkPath

        example: https://fiddle.skia.org/c/@Path_addOval_2
    */
    SkPathBuilder& addOval(const SkRect& oval, SkPathDirection dir = SkPathDirection::kCW) {
        // legacy start index: 1
        return this->addOval(oval, dir, 1);
    }

    /** Adds circle centered at (x, y) of size radius to SkPathBuilder, appending kMove_Verb,
        four kConic_Verb, and kClose_Verb. Circle begins at: (x + radius, y), continuing
        clockwise if dir is kCW_Direction, and counterclockwise if dir is kCCW_Direction.

        Has no effect if radius is zero or negative.

        @param x       center of circle
        @param y       center of circle
        @param radius  distance from center to edge
        @param dir     SkPath::Direction to wind circle
        @return        reference to SkPathBuilder
    */
    SkPathBuilder& addCircle(SkScalar x, SkScalar y, SkScalar radius,
                             SkPathDirection dir = SkPathDirection::kCW);

    /** Adds contour created from line array, adding (pts.size() - 1) line segments.
        Contour added starts at pts[0], then adds a line for every additional SkPoint
        in pts array. If close is true, appends kClose_Verb to SkPath, connecting
        pts[count - 1] and pts[0].

        @param pts    array of line sharing end and start SkPoint
        @param close  true to add line connecting contour end and start
        @return       reference to SkPath
    */
    SkPathBuilder& addPolygon(SkSpan<const SkPoint> pts, bool close);

    /** Appends src to SkPathBuilder, offset by (dx, dy).

        If mode is kAppend_AddPathMode, src verb array, SkPoint array, and conic weights are
        added unaltered. If mode is kExtend_AddPathMode, add line before appending
        verbs, SkPoint, and conic weights.

        @param src   SkPath verbs, SkPoint, and conic weights to add
        @param dx    offset added to src SkPoint array x-axis coordinates
        @param dy    offset added to src SkPoint array y-axis coordinates
        @param mode  kAppend_AddPathMode or kExtend_AddPathMode
        @return      reference to SkPathBuilder
    */
    SkPathBuilder& addPath(const SkPath& src, SkScalar dx, SkScalar dy,
                           SkPath::AddPathMode mode = SkPath::kAppend_AddPathMode);

    /** Appends src to SkPathBuilder.

        If mode is kAppend_AddPathMode, src verb array, SkPoint array, and conic weights are
        added unaltered. If mode is kExtend_AddPathMode, add line before appending
        verbs, SkPoint, and conic weights.

        @param src   SkPath verbs, SkPoint, and conic weights to add
        @param mode  kAppend_AddPathMode or kExtend_AddPathMode
        @return      reference to SkPathBuilder
    */
    SkPathBuilder& addPath(const SkPath& src,
                           SkPath::AddPathMode mode = SkPath::kAppend_AddPathMode) {
        SkMatrix m;
        m.reset();
        return this->addPath(src, m, mode);
    }

    /** Appends src to SkPathBuilder, transformed by matrix. Transformed curves may have different
        verbs, SkPoint, and conic weights.

        If mode is kAppend_AddPathMode, src verb array, SkPoint array, and conic weights are
        added unaltered. If mode is kExtend_AddPathMode, add line before appending
        verbs, SkPoint, and conic weights.

        @param src     SkPath verbs, SkPoint, and conic weights to add
        @param matrix  transform applied to src
        @param mode    kAppend_AddPathMode or kExtend_AddPathMode
        @return        reference to SkPathBuilder
    */
    SkPathBuilder& addPath(const SkPath& src, const SkMatrix& matrix,
                           SkPath::AddPathMode mode = SkPath::AddPathMode::kAppend_AddPathMode);

    // Performance hint, to reserve extra storage for subsequent calls to lineTo, quadTo, etc.

    /** Grows SkPathBuilder verb array and SkPoint array to contain additional space.
        May improve performance and use less memory by
        reducing the number and size of allocations when creating SkPathBuilder.

        @param extraPtCount    number of additional SkPoint to allocate
        @param extraVerbCount  number of additional verbs
    */
    void incReserve(int extraPtCount, int extraVerbCount);

    /** Grows SkPathBuilder verb array and SkPoint array to contain additional space.
        May improve performance and use less memory by
        reducing the number and size of allocations when creating SkPathBuilder.

        @param extraPtCount    number of additional SkPoints and verbs to allocate
    */
    void incReserve(int extraPtCount) {
        this->incReserve(extraPtCount, extraPtCount);
    }

    /** Offsets SkPoint array by (dx, dy).

        @param dx   offset added to SkPoint array x-axis coordinates
        @param dy   offset added to SkPoint array y-axis coordinates
    */
    SkPathBuilder& offset(SkScalar dx, SkScalar dy);

    /** Transforms verb array, SkPoint array, and weight by matrix.
        transform may change verbs and increase their number.

        @param matrix  SkMatrix to apply to SkPath
        @param pc      whether to apply perspective clipping
    */
    SkPathBuilder& transform(const SkMatrix& matrix,
                             SkApplyPerspectiveClip pc = SkApplyPerspectiveClip::kYes);

    /** Replaces SkPathFillType with its inverse. The inverse of SkPathFillType describes the area
        unmodified by the original SkPathFillType.
    */
    SkPathBuilder& toggleInverseFillType() {
        fFillType = (SkPathFillType)((unsigned)fFillType ^ 2);
        return *this;
    }

    /** Returns if SkPath is empty.
        Empty SkPathBuilder may have FillType but has no SkPoint, SkPath::Verb, or conic weight.
        SkPathBuilder() constructs empty SkPathBuilder; reset() and rewind() make SkPath empty.

        @return  true if the path contains no SkPath::Verb array
    */
    bool isEmpty() const { return fVerbs.empty(); }

    /** Returns last point on SkPathBuilder. Returns nullopt if SkPoint array is empty.

        @return  last SkPoint if SkPoint array contains one or more SkPoint, otherwise nullopt

        example: https://fiddle.skia.org/c/@Path_getLastPt
    */
    std::optional<SkPoint> getLastPt() const;

    /** Sets the last point on the path. If SkPoint array is empty, append kMove_Verb to
        verb array and append p to SkPoint array.

        @param x  x-value of last point
        @param y  y-value of last point
    */
    void setLastPt(SkScalar x, SkScalar y);

    /** Returns the number of points in SkPathBuilder.
        SkPoint count is initially zero.

        @return  SkPathBuilder SkPoint array length
    */
    int countPoints() const { return fPts.size(); }

    /** Returns if SkPathFillType describes area outside SkPath geometry. The inverse fill area
        extends indefinitely.

        @return  true if FillType is kInverseWinding or kInverseEvenOdd
    */
    bool isInverseFillType() const { return SkPathFillType_IsInverse(fFillType); }

#ifdef SK_SUPPORT_UNSPANNED_APIS
    SkPathBuilder& addPolygon(const SkPoint pts[], int count, bool close) {
        return this->addPolygon({pts, count}, close);
    }
    SkPathBuilder& polylineTo(const SkPoint pts[], int count) {
        return this->polylineTo({pts, count});
    }
#endif

private:
    SkPathRef::PointsArray fPts;
    SkPathRef::VerbsArray fVerbs;
    SkPathRef::ConicWeightsArray fConicWeights;

    SkPathFillType      fFillType;
    bool                fIsVolatile;

    unsigned    fSegmentMask;
    SkPoint     fLastMovePoint;
    int         fLastMoveIndex; // only needed until SkPath is immutable
    bool        fNeedsMoveVerb;

    enum IsA {
        kIsA_JustMoves,     // we only have 0 or more moves
        kIsA_MoreThanMoves, // we have verbs other than just move
        kIsA_Oval,          // we are 0 or more moves followed by an oval
        kIsA_RRect,         // we are 0 or more moves followed by a rrect
    };
    IsA fIsA      = kIsA_JustMoves;
    int fIsAStart = -1;     // tracks direction iff fIsA is not unknown
    bool fIsACCW  = false;  // tracks direction iff fIsA is not unknown

    // called right before we add a (non-move) verb
    void ensureMove() {
        fIsA = kIsA_MoreThanMoves;
        if (fNeedsMoveVerb) {
            this->moveTo(fLastMovePoint);
        }
    }

    SkPath make(sk_sp<SkPathRef>) const;

    bool isZeroLengthSincePoint(int startPtIndex) const;

    SkPathBuilder& privateReverseAddPath(const SkPath&);
    SkPathBuilder& privateReversePathTo(const SkPath&);

    std::tuple<SkPoint*, SkScalar*> growForVerbsInPath(const SkPathRef& path);

    friend class SkPathPriv;
};

#endif

