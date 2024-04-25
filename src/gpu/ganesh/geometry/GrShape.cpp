/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/geometry/GrShape.h"

#include "include/core/SkArc.h"
#include "include/core/SkScalar.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkRRectPriv.h"

#include <algorithm>

GrShape& GrShape::operator=(const GrShape& shape) {
    switch (shape.type()) {
        case Type::kEmpty:
            this->reset();
            break;
        case Type::kPoint:
            this->setPoint(shape.fPoint);
            break;
        case Type::kRect:
            this->setRect(shape.fRect);
            break;
        case Type::kRRect:
            this->setRRect(shape.fRRect);
            break;
        case Type::kPath:
            this->setPath(shape.fPath);
            break;
        case Type::kArc:
            this->setArc(shape.fArc);
            break;
        case Type::kLine:
            this->setLine(shape.fLine);
            break;
    }

    fStart = shape.fStart;
    fCW = shape.fCW;
    fInverted = shape.fInverted;

    return *this;
}

uint32_t GrShape::stateKey() const {
    // Use the path's full fill type instead of just whether or not it's inverted.
    uint32_t key = this->isPath() ? static_cast<uint32_t>(fPath.getFillType())
                                  : (fInverted ? 1 : 0);
    key |= ((uint32_t) fType) << 2; // fill type was 2 bits
    key |= fStart             << 5; // type was 3 bits, total 5 bits so far
    key |= (fCW ? 1 : 0)      << 8; // start was 3 bits, total 8 bits so far
    return key;
}

bool GrShape::simplifyPath(unsigned flags) {
    SkASSERT(this->isPath());

    SkRect rect;
    SkRRect rrect;
    SkPoint pts[2];

    SkPathDirection dir;
    unsigned start;

    if (fPath.isEmpty()) {
        this->setType(Type::kEmpty);
        return false;
    } else if (fPath.isLine(pts)) {
        this->simplifyLine(pts[0], pts[1], flags);
        return false;
    } else if (SkPathPriv::IsRRect(fPath, &rrect, &dir, &start)) {
        this->simplifyRRect(rrect, dir, start, flags);
        return true;
    } else if (SkPathPriv::IsOval(fPath, &rect, &dir, &start)) {
        // Convert to rrect indexing since oval is not represented explicitly
        this->simplifyRRect(SkRRect::MakeOval(rect), dir, start * 2, flags);
        return true;
    } else if (SkPathPriv::IsSimpleRect(fPath, (flags & kSimpleFill_Flag), &rect, &dir, &start)) {
        // When there is a path effect we restrict rect detection to the narrower API that
        // gives us the starting position. Otherwise, we will retry with the more aggressive
        // isRect().
        this->simplifyRect(rect, dir, start, flags);
        return true;
    } else if (flags & kIgnoreWinding_Flag) {
        // Attempt isRect() since we don't have to preserve any winding info
        bool closed;
        if (fPath.isRect(&rect, &closed) && (closed || (flags & kSimpleFill_Flag))) {
            this->simplifyRect(rect, kDefaultDir, kDefaultStart, flags);
            return true;
        }
    }
    // No further simplification for a path. For performance reasons, we don't query the path to
    // determine it was closed, as whether or not it was closed when it remains a path type is not
    // important for styling.
    return false;
}

bool GrShape::simplifyArc(unsigned flags) {
    SkASSERT(this->isArc());

    // Arcs can simplify to rrects, lines, points, or empty; regardless of what it simplifies to
    // it was closed if went through the center point.
    bool wasClosed = fArc.isWedge();
    if (fArc.fOval.isEmpty() || !fArc.fSweepAngle) {
        if (flags & kSimpleFill_Flag) {
            // Go straight to empty, since the other degenerate shapes all have 0 area anyway.
            this->setType(Type::kEmpty);
        } else if (!fArc.fSweepAngle) {
            SkPoint center = {fArc.fOval.centerX(), fArc.fOval.centerY()};
            SkScalar startRad = SkDegreesToRadians(fArc.fStartAngle);
            SkPoint start = {center.fX + 0.5f * fArc.fOval.width() * SkScalarCos(startRad),
                                center.fY + 0.5f * fArc.fOval.height() * SkScalarSin(startRad)};
            // Either just the starting point, or a line from the center to the start
            if (fArc.isWedge()) {
                this->simplifyLine(center, start, flags);
             } else {
                this->simplifyPoint(start, flags);
             }
        } else {
            // TODO: Theoretically, we could analyze the arc projected into the empty bounds to
            // determine a line, but that is somewhat complex for little value (since the arc
            // can backtrack on itself if the sweep angle is large enough).
            this->setType(Type::kEmpty);
        }
    } else {
        if ((flags & kSimpleFill_Flag) ||
            ((flags & kIgnoreWinding_Flag) && !fArc.isWedge())) {
            // Eligible to turn into an oval if it sweeps a full circle
            if (fArc.fSweepAngle <= -360.f || fArc.fSweepAngle >= 360.f) {
                this->simplifyRRect(SkRRect::MakeOval(fArc.fOval),
                                    kDefaultDir, kDefaultStart, flags);
                return true;
            }
        }

        if (flags & kMakeCanonical_Flag) {
            // Map start to 0 to 360, sweep is always positive
            if (fArc.fSweepAngle < 0) {
                fArc.fStartAngle = fArc.fStartAngle + fArc.fSweepAngle;
                fArc.fSweepAngle = -fArc.fSweepAngle;
            }

            if (fArc.fStartAngle < 0 || fArc.fStartAngle >= 360.f) {
                fArc.fStartAngle = SkScalarMod(fArc.fStartAngle, 360.f);
            }
        }
    }

    return wasClosed;
}

void GrShape::simplifyRRect(const SkRRect& rrect, SkPathDirection dir, unsigned start,
                            unsigned flags) {
    if (rrect.isEmpty() || rrect.isRect()) {
        // Change index from rrect to rect
        start = ((start + 1) / 2) % 4;
        this->simplifyRect(rrect.rect(), dir, start, flags);
    } else if (!this->isRRect()) {
        this->setType(Type::kRRect);
        fRRect = rrect;
        this->setPathWindingParams(dir, start);
        // A round rect is already canonical, so there's nothing more to do
    } else {
        // If starting as a round rect, the provided rrect/winding params should be already set
        SkASSERT(fRRect == rrect && this->dir() == dir && this->startIndex() == start);
    }
}

void GrShape::simplifyRect(const SkRect& rect, SkPathDirection dir, unsigned start,
                           unsigned flags) {
    if (!rect.width() || !rect.height()) {
        if (flags & kSimpleFill_Flag) {
            // A zero area, filled shape so go straight to empty
            this->setType(Type::kEmpty);
        } else if (!rect.width() ^ !rect.height()) {
            // A line, choose the first point that best matches the starting index
            SkPoint p1 = {rect.fLeft, rect.fTop};
            SkPoint p2 = {rect.fRight, rect.fBottom};
            if (start >= 2 && !(flags & kIgnoreWinding_Flag)) {
                using std::swap;
                swap(p1, p2);
            }
            this->simplifyLine(p1, p2, flags);
        } else {
            // A point (all edges are equal, so start+dir doesn't affect choice)
            this->simplifyPoint({rect.fLeft, rect.fTop}, flags);
        }
    } else {
        if (!this->isRect()) {
            this->setType(Type::kRect);
            fRect = rect;
            this->setPathWindingParams(dir, start);
        } else {
            // If starting as a rect, the provided rect/winding params should already be set
            SkASSERT(fRect == rect && this->dir() == dir && this->startIndex() == start);
        }
        if (flags & kMakeCanonical_Flag) {
            fRect.sort();
        }
    }
}

void GrShape::simplifyLine(const SkPoint& p1, const SkPoint& p2, unsigned flags) {
    if (flags & kSimpleFill_Flag) {
        this->setType(Type::kEmpty);
    } else if (p1 == p2) {
        this->simplifyPoint(p1, false);
    } else {
        if (!this->isLine()) {
            this->setType(Type::kLine);
            fLine.fP1 = p1;
            fLine.fP2 = p2;
        } else {
            // If starting as a line, the provided points should already be set
            SkASSERT(fLine.fP1 == p1 && fLine.fP2 == p2);
        }
        if (flags & kMakeCanonical_Flag) {
             // Sort the end points
             if (fLine.fP2.fY < fLine.fP1.fY ||
                 (fLine.fP2.fY == fLine.fP1.fY && fLine.fP2.fX < fLine.fP1.fX)) {
                using std::swap;
                swap(fLine.fP1, fLine.fP2);
            }
        }
    }
}

void GrShape::simplifyPoint(const SkPoint& point, unsigned flags) {
    if (flags & kSimpleFill_Flag) {
        this->setType(Type::kEmpty);
    } else if (!this->isPoint()) {
        this->setType(Type::kPoint);
        fPoint = point;
    } else {
        // If starting as a point, the provided position should already be set
        SkASSERT(point == fPoint);
    }
}

bool GrShape::simplify(unsigned flags) {
    // Verify that winding parameters are valid for the current type.
    SkASSERT((fType == Type::kRect || fType == Type::kRRect) ||
             (this->dir() == kDefaultDir && this->startIndex() == kDefaultStart));

    // The type specific functions automatically fall through to the simpler shapes, so
    // we only need to start in the right place.
    bool wasClosed = false;
    switch (fType) {
        case Type::kEmpty:
            // do nothing
            break;
        case Type::kPoint:
            this->simplifyPoint(fPoint, flags);
            break;
        case Type::kLine:
            this->simplifyLine(fLine.fP1, fLine.fP2, flags);
            break;
        case Type::kRect:
            this->simplifyRect(fRect, this->dir(), this->startIndex(), flags);
            wasClosed = true;
            break;
        case Type::kRRect:
            this->simplifyRRect(fRRect, this->dir(), this->startIndex(), flags);
            wasClosed = true;
            break;
        case Type::kPath:
            wasClosed = this->simplifyPath(flags);
            break;
        case Type::kArc:
            wasClosed = this->simplifyArc(flags);
            break;

        default:
            SkUNREACHABLE;
    }

    if (((flags & kIgnoreWinding_Flag) || (fType != Type::kRect && fType != Type::kRRect))) {
        // Reset winding parameters if we don't need them anymore
        this->setPathWindingParams(kDefaultDir, kDefaultStart);
    }

    return wasClosed;
}

bool GrShape::conservativeContains(const SkRect& rect) const {
    switch (this->type()) {
        case Type::kEmpty:
        case Type::kPoint: // fall through since a point has 0 area
        case Type::kLine:  // fall through, "" (currently choosing not to test if 'rect' == line)
            return false;
        case Type::kRect:
            return fRect.contains(rect);
        case Type::kRRect:
            return fRRect.contains(rect);
        case Type::kPath:
            return fPath.conservativelyContainsRect(rect);
        case Type::kArc:
            if (fArc.fType == SkArc::Type::kWedge) {
                SkPath arc;
                this->asPath(&arc);
                return arc.conservativelyContainsRect(rect);
            } else {
                return false;
            }
    }
    SkUNREACHABLE;
}

bool GrShape::conservativeContains(const SkPoint& point) const {
    switch (this->type()) {
        case Type::kEmpty:
        case Type::kPoint: // fall through, currently choosing not to test if shape == point
        case Type::kLine:  // fall through, ""
        case Type::kArc:
            return false;
        case Type::kRect:
            return fRect.contains(point.fX, point.fY);
        case Type::kRRect:
            return SkRRectPriv::ContainsPoint(fRRect, point);
        case Type::kPath:
            return fPath.contains(point.fX, point.fY);
    }
    SkUNREACHABLE;
}

bool GrShape::closed() const {
    switch (this->type()) {
        case Type::kEmpty: // fall through
        case Type::kRect:  // fall through
        case Type::kRRect:
            return true;
        case Type::kPath:
            // SkPath doesn't keep track of the closed status of each contour.
            return SkPathPriv::IsClosedSingleContour(fPath);
        case Type::kArc:
            return fArc.fType == SkArc::Type::kWedge;
        case Type::kPoint: // fall through
        case Type::kLine:
            return false;
    }
    SkUNREACHABLE;
}

bool GrShape::convex(bool simpleFill) const {
    switch (this->type()) {
        case Type::kEmpty: // fall through
        case Type::kRect:  // fall through
        case Type::kRRect:
            return true;
        case Type::kPath:
            // SkPath.isConvex() really means "is this path convex were it to be closed".
            // Convex paths may only have one contour hence isLastContourClosed() is sufficient.
            return (simpleFill || fPath.isLastContourClosed()) && fPath.isConvex();
        case Type::kArc:
            return SkPathPriv::DrawArcIsConvex(fArc.fSweepAngle, fArc.fType, simpleFill);
        case Type::kPoint: // fall through
        case Type::kLine:
            return false;
    }
    SkUNREACHABLE;
}

SkRect GrShape::bounds() const {
    // Bounds where left == bottom or top == right can indicate a line or point shape. We return
    // inverted bounds for a truly empty shape.
    static constexpr SkRect kInverted = SkRect::MakeLTRB(1, 1, -1, -1);
    switch (this->type()) {
        case Type::kEmpty:
            return kInverted;
        case Type::kPoint:
            return {fPoint.fX, fPoint.fY, fPoint.fX, fPoint.fY};
        case Type::kRect:
            return fRect.makeSorted();
        case Type::kRRect:
            return fRRect.getBounds();
        case Type::kPath:
            return fPath.getBounds();
        case Type::kArc:
            return fArc.fOval;
        case Type::kLine: {
            SkRect b = SkRect::MakeLTRB(fLine.fP1.fX, fLine.fP1.fY,
                                        fLine.fP2.fX, fLine.fP2.fY);
            b.sort();
            return b; }
    }
    SkUNREACHABLE;
}

uint32_t GrShape::segmentMask() const {
    // In order to match what a path would report, this has to inspect the shapes slightly
    // to reflect what they might simplify to.
    switch (this->type()) {
        case Type::kEmpty:
            return 0;
        case Type::kRRect:
            if (fRRect.isEmpty() || fRRect.isRect()) {
                return SkPath::kLine_SegmentMask;
            } else if (fRRect.isOval()) {
                return SkPath::kConic_SegmentMask;
            } else {
                return SkPath::kConic_SegmentMask | SkPath::kLine_SegmentMask;
            }
        case Type::kPath:
            return fPath.getSegmentMasks();
        case Type::kArc:
            if (fArc.fType == SkArc::Type::kWedge) {
                return SkPath::kConic_SegmentMask | SkPath::kLine_SegmentMask;
            } else {
                return SkPath::kConic_SegmentMask;
            }
        case Type::kPoint: // fall through
        case Type::kLine:  // ""
        case Type::kRect:
            return SkPath::kLine_SegmentMask;
    }
    SkUNREACHABLE;
}

void GrShape::asPath(SkPath* out, bool simpleFill) const {
    if (!this->isPath() && !this->isArc()) {
        // When not a path, we need to set fill type on the path to match invertedness.
        // All the non-path geometries produce equivalent shapes with either even-odd or winding
        // so we can use the default fill type.
        out->reset();
        out->setFillType(kDefaultFillType);
        if (fInverted) {
            out->toggleInverseFillType();
        }
    } // Else when we're already a path, that will assign the fill type directly to 'out'.

    switch (this->type()) {
        case Type::kEmpty:
            return;
        case Type::kPoint:
            // A plain moveTo() or moveTo+close() does not match the expected path for a
            // point that is being dashed (see SkDashPath's handling of zero-length segments).
            out->moveTo(fPoint);
            out->lineTo(fPoint);
            return;
        case Type::kRect:
            out->addRect(fRect, this->dir(), this->startIndex());
            return;
        case Type::kRRect:
            out->addRRect(fRRect, this->dir(), this->startIndex());
            return;
        case Type::kPath:
            *out = fPath;
            return;
        case Type::kArc:
            SkPathPriv::CreateDrawArcPath(out, fArc, simpleFill);
            // CreateDrawArcPath resets the output path and configures its fill type, so we just
            // have to ensure invertedness is correct.
            if (fInverted) {
                out->toggleInverseFillType();
            }
            return;
        case Type::kLine:
            out->moveTo(fLine.fP1);
            out->lineTo(fLine.fP2);
            return;
    }
    SkUNREACHABLE;
}
