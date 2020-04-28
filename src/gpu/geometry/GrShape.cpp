/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/geometry/GrShape.h"

#include "src/core/SkPathPriv.h"

GrShape& GrShape::operator=(const GrShape& shape) {
    switch(shape.type()) {
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
        default:
            SkUNREACHABLE;
    }

    fStart = shape.fStart;
    fDir = shape.fDir;
    fInverted = shape.fInverted;

    return *this;
}

uint32_t GrShape::stateKey() const {
    // Use the path's full fill type instead of just whether or not it's inverted.
    uint32_t fillType = this->isPath() ? static_cast<uint32_t>(fPath.getFillType()) : fInverted;
    return (fType    << 0) | // 3 bits
           (fillType << 3) | // 1 bits
           (fStart   << 4) | // 3 bits
           (fDir     << 7); // 1 bit, but nothing else is needed in the key
}

bool GrShape::simplify(unsigned flags) {
    using std::swap;

    bool simpleFill = SkToBool(flags & kSimpleFill_Flag);
    bool ignoreWinding = SkToBool(flags & kIgnoreWinding_Flag);
    bool makeCanonical = SkToBool(flags & kMakeCanonical_Flag);

    Type oldType = this->type();

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
            if (fPath.isRect(&rect, &closed) && (closed || simpleFill)) {
                this->setRect(rect);
            }
        }
    }

    // Arcs can simplify to rrects, lines, points, or empty
    if (this->isArc()) {
        if (fArc.fOval.isEmpty() || !fArc.fSweepAngle) {
            if (simpleFill) {
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
        } else if (simpleFill || (ignoreWinding && !fArc.fUseCenter)) {
            // Eligible to turn into an oval if it sweeps a full circle
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
            if (simpleFill) {
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
        if (simpleFill) {
            this->reset();
        } else if (fLine.fP1 == fLine.fP2) {
            this->setPoint(fLine.fP1);
        }
    }

    // Fill-only points become empty
    if (this->isPoint() && simpleFill) {
        this->reset();
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

    return this->type() != oldType;
}

bool GrShape::contains(const SkRect& rect) const {
    switch(this->type()) {
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
            if (fArc.fUseCenter) {
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
    switch(this->type()) {
        case Type::kEmpty: // fall through
        case Type::kRect:  // fall through
        case Type::kRRect:
            return true;
        case Type::kPath:
            // SkPath doesn't keep track of the closed status of each contour.
            return SkPathPriv::IsClosedSingleContour(fPath);
        case Type::kArc:
            return fArc.fUseCenter;
        case Type::kPoint: // fall through
        case Type::kLine:
            return false;
        default:
            SkUNREACHABLE;
    }
}

bool GrShape::convex(bool simpleFill) const {
    switch(this->type()) {
        case Type::kEmpty: // fall through
        case Type::kRect:  // fall through
        case Type::kRRect:
            return true;
        case Type::kPath:
            // SkPath.isConvex() really means "is this path convex were it to be closed".
            // Convex paths may only have one contour hence isLastContourClosed() is sufficient.
            return (simpleFill || fPath.isLastContourClosed()) && fPath.isConvex();
        case Type::kArc:
            return SkPathPriv::DrawArcIsConvex(fArc.fSweepAngle, fArc.fUseCenter, simpleFill);
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
    switch(this->type()) {
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
        default:
            SkUNREACHABLE;
    }
}

uint32_t GrShape::segmentMask() const {
    // In order to match what a path would report, this has to inspect the shapes slightly
    // to reflect what they might simplify to.
    switch(this->type()) {
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
            if (fArc.fUseCenter) {
                return SkPath::kConic_SegmentMask | SkPath::kLine_SegmentMask;
            } else {
                return SkPath::kConic_SegmentMask;
            }
        case Type::kPoint: // fall through
        case Type::kLine:  // ""
        case Type::kRect:
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

    switch(this->type()) {
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
            *out = this->path();
            return;
        case Type::kArc:
            SkPathPriv::CreateDrawArcPath(out, fArc.fOval, fArc.fStartAngle, fArc.fSweepAngle,
                                          fArc.fUseCenter, simpleFill);
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
        default:
            SkUNREACHABLE;
    }
}
