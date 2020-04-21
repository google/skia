/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/geometry/GrShape.h"

#include "src/core/SkPathPriv.h"

GrShape& GrShape::operator=(const GrShape& shape) {
    switch(shape.fType) {
        case Type::kEmpty:
            this->reset();
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
    return *this;
}

bool GrShape::simplify(bool simpleFill, SkPathDirection* dir, unsigned* start) {
    // Proceed through the types from most complex to least; as the shape is simplified, the next
    // tier of reductions will be automatically considered.
    bool forPathEffect = SkToBool(dir);
    SkASSERT(!forPathEffect || start);
    SkASSERT(!forPathEffect || !simpleFill);

    Type oldType = fType;

    // Paths can simplify to rrect, rect, line, or empty
    if (this->isPath()) {
        SkRect rect;
        SkRRect rrect;
        SkPoint pts[2];

        // For use when these parameters do not need to be exported
        SkPathDirection unusedDir;
        unsigned unusedStart;
        if (!dir) {
            dir = &unusedDir;
            start = &unusedStart;
        }

        if (fPath.isEmpty()) {
            this->reset();
        } else if (fPath.isLine(pts)) {
            this->setLine({pts[0], pts[1]});
        } else if (SkPathPriv::IsRRect(fPath, &rrect, dir, start)) {
            this->setRRect(rrect);
        } else if (SkPathPriv::IsOval(fPath, &rect, dir, start)) {
            this->setRRect(SkRRect::MakeOval(rect));
            // Convert to rrect indexing
            *start *= 2;
        } else if (SkPathPriv::IsSimpleClosedRect(fPath, &rect, dir, start)) {
            // When there is a path effect we restrict rect detection to the narrower API that
            // gives us the starting position. Otherwise, we will retry with the more aggressive
            // isRect().
            this->setRect(rect);
            // Convert to rrect index
            *start *= 2;
        } else if (!forPathEffect) {
            // Attempt isRect() since we don't have to export dir/start
            bool closed;
            if (fPath.isRect(&rect, &closed) && (closed || simpleFill)) {
                this->setRect(rect);
            }
        }
    }

    // Arcs can simplify to rrects or empty
    if (this->isArc()) {
        if (fArc.fOval.isEmpty() || !fArc.fSweepAngle) {
            this->reset();
        } else if (simpleFill || (!forPathEffect && !fArc.fUseCenter)) {
            // Eligible to turned into an oval if it sweeps a full circle
            if (fArc.fSweepAngle <= -360.f || fArc.fSweepAngle >= 360.f) {
                this->setRRect(SkRRect::MakeOval(fArc.fOval));
            }
        }
    }

    // Round rects can simplify to rect or empty
    if (this->isRRect()) {
        if (simpleFill && fRRect.isEmpty()) {
            // For historical reasons, we preserve the geometry type when stroking might be involved
            // with empty round rects/rects/lines
            this->reset();
        } else if (fRRect.isRect()) {
            this->setRect(fRRect.rect());
        }
    }

    // Rect and line can simplify to empty
    if (this->isRect()) {
        if (simpleFill && fRect.isEmpty()) {
            this->reset();
        }
    }
    if (this->isLine()) {
        if (simpleFill && fLine.fP1 == fLine.fP2) {
            this->reset();
        }
    }

    return fType != oldType;
}

bool GrShape::contains(const SkRect& rect) const {
    switch(fType) {
        case Type::kEmpty:
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
        case Type::kLine:
            return false;
        default:
            SkUNREACHABLE;
    }
}

bool GrShape::closed() const {
    switch(fType) {
        case Type::kEmpty: // fall through
        case Type::kRect:  // fall through
        case Type::kRRect:
            return true;
        case Type::kPath:
            // SkPath doesn't keep track of the closed status of each contour.
            return SkPathPriv::IsClosedSingleContour(this->path());
        case Type::kArc:
            return this->arc().fUseCenter;
        case Type::kLine:
            return false;
        default:
            SkUNREACHABLE;
    }
}

bool GrShape::convex(bool simpleFill) const {
    switch(fType) {
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
    switch(fType) {
        case Type::kEmpty:
            return kInverted;
        case Type::kRect:
            return this->rect();
        case Type::kRRect:
            return this->rrect().getBounds();
        case Type::kPath:
            return this->path().getBounds();
        case Type::kArc:
            return this->arc().fOval;
        case Type::kLine:
            SkRect b = SkRect::MakeLTRB(this->line().fP1.fX, this->line().fP1.fY,
                                        this->line().fP2.fX, this->line().fP2.fY);
            b.sort();
            return b;
    }
}

uint32_t GrShape::segmentMask() const {
    // In order to match what a path would report, this has to inspect the shapes slightly
    // to reflect what they might simplify to.
    switch(fType) {
        case Type::kEmpty:
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

void GrShape::asPath(SkPath* out) const {
    out->reset();
    switch(fType) {
        case Type::kEmpty:
            return;
        case Type::kRect:
            out->addRect(this->rect());
            return;
        case Type::kRRect:
            out->addRRect(this->rrect());
            return;
        case Type::kPath:
            *out = this->path();
            return;
        case Type::kArc:
            // GrShape does not consider styling at all, but passing 'false' for isFillNoPathEffect
            // ensures the conversion is not lossy. Any arc that could be an oval will likely be
            // simplified anyways, and simplify accepts parameters to ensure conversion is not lossy
            SkPathPriv::CreateDrawArcPath(out, this->arc().fOval, this->arc().fStartAngle,
                                          this->arc().fSweepAngle, this->arc().fUseCenter, false);
            return;
        case Type::kLine:
            out->moveTo(this->line().fP1);
            out->lineTo(this->line().fP2);
            return;
        default:
            SkUNREACHABLE;
    }
}
