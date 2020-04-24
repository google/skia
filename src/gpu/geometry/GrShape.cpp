/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/geometry/GrShape.h"

#include "src/core/SkPathPriv.h"

GrShape& GrShape::operator=(const GrShape& shape) {
    switch(static_cast<Type>(shape.fType)) {
        case Type::kEmpty:
            this->reset();
            break;
        case Type::kPoint:
            this->setPoint(shape.point());
            break;
        case Type::kRect:
            this->setRect(shape.rect());
            break;
        case Type::kRRect:
            this->setRRect(shape.rrect());
            break;
        case Type::kPath:
            this->setPath(shape.path());
            break;
        case Type::kArc:
            this->setArc(shape.arc());
            break;
        case Type::kLine:
            this->setLine(shape.line());
            break;
        default:
            SkUNREACHABLE;
    }

    fStart = shape.fStart;
    fDir = shape.fDir;
    fInverted = shape.fInverted;

    return *this;
}

void GrShape::simplify(unsigned flags) {
    using std::swap;

    bool willBeFilled = SkToBool(flags & kWillBeFilled_Flag);
    bool ignoreWinding = SkToBool(flags & kIgnoreWinding_Flag);
    bool makeCanonical = SkToBool(flags & kMakeCanonical_Flag);

    // Remember old winding parameters/invertedness before any simplification
    // since we may restore it after a call to setX().
    SkPathDirection dir = this->dir();
    unsigned start = this->startIndex();
    bool inverted = this->inverted();

    // Paths can simplify to rrect, rect, line, or empty
    if (this->isPath()) {
        SkRect rect;
        SkRRect rrect;
        SkPoint pts[2];

        if (fPath.isEmpty()) {
            this->reset();
        } else if (fPath.isLine(pts)) {
            this->setLine({pts[0], pts[1]});
        } else if (SkPathPriv::IsRRect(fPath, &rrect, &dir, &start)) {
            this->setRRect(rrect);
        } else if (SkPathPriv::IsOval(fPath, &rect, &dir, &start)) {
            this->setRRect(SkRRect::MakeOval(rect));
            // Convert to rrect indexing since oval is not represented explicitly
            start *= 2;
        } else if (SkPathPriv::IsSimpleClosedRect(fPath, &rect, &dir, &start)) {
            // When there is a path effect we restrict rect detection to the narrower API that
            // gives us the starting position. Otherwise, we will retry with the more aggressive
            // isRect().
            this->setRect(rect);
        } else if (ignoreWinding) {
            // Attempt isRect() since we don't have to preserve any winding info
            bool closed;
            if (fPath.isRect(&rect, &closed) && (closed || willBeFilled)) {
                this->setRect(rect);
            }
        }
    }

    // Arcs can simplify to rrects, lines, points, or empty
    if (this->isArc()) {
        if (fArc.fOval.isEmpty() || !fArc.fSweepAngle) {
            if (willBeFilled) {
                // Go straight to empty, since the other degenerate shapes all have 0 area anyway.
                this->reset();
            } else if (!fArc.fSweepAngle) {
                SkPoint center = {fArc.fOval.centerX(), fArc.fOval.centerY()};
                SkScalar startRad = SkDegreesToRadians(fArc.fStartAngle);
                SkPoint start = {center.fX + 0.5f * fArc.fOval.width() * SkScalarCos(startRad),
                                 center.fY + 0.5f * fArc.fOval.height() * SkScalarSin(startRad)};
                // Either just the starting point, or a line from the center to the start
                if (fArc.fUseCenter) {
                    this->setLine({center, start});
                } else {
                    this->setPoint(start);
                }
            } else {
                // TODO: Theoretically, we could analyze the arc projected into the empty bounds to
                // determine a line, but that is somewhat complex for little value (since the arc
                // can backtrack on itself if the sweep angle is large enough).
                this->reset();
            }
        } else if (willBeFilled || (ignoreWinding && !fArc.fUseCenter)) {
            // Eligible to turned into an oval if it sweeps a full circle
            if (fArc.fSweepAngle <= -360.f || fArc.fSweepAngle >= 360.f) {
                this->setRRect(SkRRect::MakeOval(fArc.fOval));
            }
        }
    }

    // Round rects can simplify to rect, line, point, or empty
    if (this->isRRect()) {
        if (fRRect.isEmpty() || fRRect.isRect()) {
            // Change index from rrect to rect
            start = ((start + 1) / 2) % 4;
            // Will fall through to rect simplification for converting to a line or point
            this->setRect(fRRect.rect());
        }
    }

    // Rects can simplify to line, point, or empty
    if (this->isRect()) {
        // Can't use SkRect::isEmpty since it treats unsorted rects as empty
        if (!fRect.width() || !fRect.height()) {
            if (willBeFilled) {
                // A zero area shape so go straight to empty
                this->reset();
            } else if (!fRect.width() ^ !fRect.height()) {
                // A line, choose the first point that best matches the starting index
                SkPoint p1 = {fRect.fLeft, fRect.fTop};
                SkPoint p2 = {fRect.fRight, fRect.fBottom};
                if (start >= 2 && !ignoreWinding) {
                    swap(p1, p2);
                }
                this->setLine({p1, p2});
            } else {
                // A point (all edges are equal, so start+dir doesn't affect choice)
                this->setPoint({fRect.fLeft, fRect.fTop});
            }
        }
    }

    // Lines can simplify to a point or empty
    if (this->isLine()) {
        if (fLine.fP1 == fLine.fP2) {
            if (willBeFilled) {
                this->reset();
            } else {
                this->setPoint(fLine.fP1);
            }
        }
    }

    // Apply canonicalization logic after the shapes have been fully reduced.
    // Canonicalization may change the geometry, but it will not change the shape type.
    if (makeCanonical) {
        if (this->isArc()) {
            // Map start to 0 to 360, sweep is always positive
            if (fArc.fSweepAngle < 0) {
                fArc.fStartAngle = fArc.fStartAngle + fArc.fSweepAngle;
                fArc.fSweepAngle = -fArc.fSweepAngle;
            }

            if (fArc.fStartAngle < 0 || fArc.fStartAngle >= 360.f) {
                fArc.fStartAngle = SkScalarMod(fArc.fStartAngle, 360.f);
            }
        } else if (this->isRect()) {
            // Sort the edges
            fRect.sort();
        } else if (this->isLine()) {
            // Sort the end points
            if (fLine.fP2.fY < fLine.fP1.fY ||
                (fLine.fP2.fY == fLine.fP1.fY && fLine.fP2.fX < fLine.fP1.fX)) {
                swap(fLine.fP1, fLine.fP2);
            }
        }
    }

    // Preserve the start and winding if necessary, otherwise store the default
    if (!ignoreWinding && (this->isRRect() || this->isRect())) {
        this->setPathWindingParams(dir, start);
    } else {
        this->setPathWindingParams(kDefaultDir, kDefaultStart);
    }
    this->setInverted(inverted);
}

bool GrShape::contains(const SkRect& rect) const {
    switch(static_cast<Type>(fType)) {
        case Type::kEmpty:
        case Type::kPoint: // fall through since a point has 0 area
        case Type::kLine:  // fall through, "" (currently choosing not to test if 'rect' == line)
            return false;
        case Type::kRect:
            return this->rect().contains(rect);
        case Type::kRRect:
            return this->rrect().contains(rect);
        case Type::kPath:
            return this->path().conservativelyContainsRect(rect);
        case Type::kArc:
            if (this->arc().fUseCenter) {
                SkPath arc;
                this->asPath(&arc);
                return arc.conservativelyContainsRect(rect);
            } else {
                return false;
            }
        default:
            SkUNREACHABLE;
    }
}

bool GrShape::closed() const {
    switch(static_cast<Type>(fType)) {
        case Type::kEmpty: // fall through
        case Type::kRect:  // fall through
        case Type::kRRect:
            return true;
        case Type::kPath:
            // SkPath doesn't keep track of the closed status of each contour.
            return SkPathPriv::IsClosedSingleContour(this->path());
        case Type::kArc:
            return this->arc().fUseCenter;
        case Type::kPoint: // fall through
        case Type::kLine:
            return false;
        default:
            SkUNREACHABLE;
    }
}

bool GrShape::convex(bool simpleFill) const {
    switch(static_cast<Type>(fType)) {
        case Type::kEmpty: // fall through
        case Type::kRect:  // fall through
        case Type::kRRect:
            return true;
        case Type::kPath:
            // SkPath.isConvex() really means "is this path convex were it to be closed".
            // Convex paths may only have one contour hence isLastContourClosed() is sufficient.
            return (simpleFill || this->path().isLastContourClosed()) && this->path().isConvex();
        case Type::kArc:
            return SkPathPriv::DrawArcIsConvex(this->arc().fSweepAngle, this->arc().fUseCenter,
                                               simpleFill);
        case Type::kPoint: // fall through
        case Type::kLine:
            return false;
        default:
            SkUNREACHABLE;
    }
}

SkRect GrShape::bounds() const {
    // Bounds where left == bottom or top == right can indicate a line or point shape. We return
    // inverted bounds for a truly empty shape.
    static constexpr SkRect kInverted = SkRect::MakeLTRB(1, 1, -1, -1);
    switch(static_cast<Type>(fType)) {
        case Type::kEmpty:
            return kInverted;
        case Type::kPoint:
            return {fPoint.fX, fPoint.fY, fPoint.fX, fPoint.fY};
        case Type::kRect:
            return this->rect().makeSorted();
        case Type::kRRect:
            return this->rrect().getBounds();
        case Type::kPath:
            return this->path().getBounds();
        case Type::kArc:
            return this->arc().fOval;
        case Type::kLine: {
            SkRect b = SkRect::MakeLTRB(this->line().fP1.fX, this->line().fP1.fY,
                                        this->line().fP2.fX, this->line().fP2.fY);
            b.sort();
            return b; }
        default:
            SkUNREACHABLE;
    }
}

uint32_t GrShape::segmentMask() const {
    // In order to match what a path would report, this has to inspect the shapes slightly
    // to reflect what they might simplify to.
    switch(static_cast<Type>(fType)) {
        case Type::kEmpty: // fall through
        case Type::kPoint:
            return 0;
        case Type::kRect:
            return SkPath::kLine_SegmentMask;
        case Type::kRRect:
            if (this->rrect().isEmpty() || this->rrect().isRect()) {
                return SkPath::kLine_SegmentMask;
            } else if (this->rrect().isOval()) {
                return SkPath::kConic_SegmentMask;
            } else {
                return SkPath::kConic_SegmentMask | SkPath::kLine_SegmentMask;
            }
        case Type::kPath:
            return this->path().getSegmentMasks();
        case Type::kArc:
            if (this->arc().fUseCenter) {
                return SkPath::kConic_SegmentMask | SkPath::kLine_SegmentMask;
            } else {
                return SkPath::kConic_SegmentMask;
            }
        case Type::kLine:
            return SkPath::kLine_SegmentMask;
        default:
            SkUNREACHABLE;
    }
}

void GrShape::asPath(SkPath* out, bool simpleFill) const {
    out->reset();
    if (!this->isPath() && !this->isArc()) {
        // When not a path, we need to set fill type on the path to match invertedness.
        // All the non-path geometries produce equivalent shapes with either even-odd or winding
        // so we can use the default fill type.
        out->setFillType(kDefaultFillType);
        if (fInverted) {
            out->toggleInverseFillType();
        }
    } // Else when we're already a path, that will assign the fill type directly to 'out'.

    switch(static_cast<Type>(fType)) {
        case Type::kEmpty:
            return;
        case Type::kPoint:
            out->moveTo(this->point());
            return;
        case Type::kRect:
            out->addRect(this->rect(), this->dir(), this->startIndex());
            return;
        case Type::kRRect:
            out->addRRect(this->rrect(), this->dir(), this->startIndex());
            return;
        case Type::kPath:
            *out = this->path();
            return;
        case Type::kArc:
            SkPathPriv::CreateDrawArcPath(out, this->arc().fOval, this->arc().fStartAngle,
                                          this->arc().fSweepAngle, this->arc().fUseCenter,
                                          simpleFill);
            // CreateDrawArcPath resets the output path and configures its fill type, so we just
            // have to ensure invertedness is correct.
            if (fInverted) {
                out->toggleInverseFillType();
            }
            return;
        case Type::kLine:
            out->moveTo(this->line().fP1);
            out->lineTo(this->line().fP2);
            return;
        default:
            SkUNREACHABLE;
    }
}
