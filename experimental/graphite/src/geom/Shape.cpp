/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/geom/Shape.h"

#include "src/core/SkPathPriv.h"
#include "src/core/SkRRectPriv.h"
#include "src/utils/SkPolyUtils.h"

namespace skgpu {

Shape& Shape::operator=(const Shape& shape) {
    switch (shape.type()) {
        case Type::kEmpty: this->reset();                         break;
        case Type::kLine:  this->setLine(shape.p0(), shape.p1()); break;
        case Type::kRect:  this->setRect(shape.rect());           break;
        case Type::kRRect: this->setRRect(shape.rrect());         break;
        case Type::kPath:  this->setPath(shape.path());           break;
    }

    fInverted = shape.fInverted;
    return *this;
}

bool Shape::conservativeContains(const Rect& rect) const {
    switch (fType) {
        case Type::kEmpty: return false;
        case Type::kLine:  return false;
        case Type::kRect:  return fRect.contains(rect);
        case Type::kRRect: return fRRect.contains(rect.asSkRect());
        case Type::kPath:  return fPath.conservativelyContainsRect(rect.asSkRect());
    }
    SkUNREACHABLE;
}

bool Shape::conservativeContains(float2 point) const {
    switch (fType) {
        case Type::kEmpty: return false;
        case Type::kLine:  return false;
        case Type::kRect:  return fRect.contains(Rect::Point(point));
        case Type::kRRect: return SkRRectPriv::ContainsPoint(fRRect, {point.x(), point.y()});
        case Type::kPath:  return fPath.contains(point.x(), point.y());
    }
    SkUNREACHABLE;
}

bool Shape::closed() const {
    switch (fType) {
        case Type::kEmpty: return true;
        case Type::kLine:  return false;
        case Type::kRect:  return true;
        case Type::kRRect: return true;
        case Type::kPath:  return SkPathPriv::IsClosedSingleContour(fPath);
    }
    SkUNREACHABLE;
}

bool Shape::convex(bool simpleFill) const {
    if (this->isPath()) {
        // SkPath.isConvex() really means "is this path convex were it to be closed".
        return (simpleFill || fPath.isLastContourClosed()) && fPath.isConvex();
    } else {
        // Every other shape type is convex by construction.
        return true;
    }
}

Rect Shape::bounds() const {
    switch (fType) {
        case Type::kEmpty: return Rect(0, 0, 0, 0);
        case Type::kLine:  return fRect.makeSorted(); // sorting corners computes bbox of segment
        case Type::kRect:  return fRect; // assuming it's sorted
        case Type::kRRect: return fRRect.getBounds();
        case Type::kPath:  return fPath.getBounds();
    }
    SkUNREACHABLE;
}

SkPath Shape::asPath() const {
    if (fType == Type::kPath) {
        return fPath;
    }

    SkPathBuilder builder(this->fillType());
    switch (fType) {
        case Type::kEmpty: /* do nothing */                            break;
        case Type::kLine:  builder.moveTo(fRect.left(), fRect.top())
                                  .lineTo(fRect.right(), fRect.bot()); break;
        case Type::kRect:  builder.addRect(fRect.asSkRect());          break;
        case Type::kRRect: builder.addRRect(fRRect);                   break;
        case Type::kPath:  SkUNREACHABLE;
    }
    return builder.detach();
}

} // namespace skgpu
