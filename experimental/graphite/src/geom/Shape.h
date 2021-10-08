/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_geom_Shape_DEFINED
#define skgpu_geom_Shape_DEFINED

#include "include/core/SkM44.h"
#include "include/core/SkPath.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkSpan.h"

#include <array>

namespace skgpu::geom {

/**
 * Shape is effectively a std::variant over different geometric shapes, with the most complex
 * being an SkPath. It provides a consistent way to query geometric properties, such as convexity,
 * point containment, or iteration.
 */
class Shape {
public:
    enum class Type : uint8_t {
        kEmpty, kRect, kRRect, kPath
    };
    inline static constexpr int kTypeCount = static_cast<int>(Type::kPath) + 1;

    Shape() {}
    Shape(const Shape& shape)            { *this = shape; }
    Shape(Shape&&) = delete;

    explicit Shape(const SkRect& rect)   { this->setRect(rect); }
    explicit Shape(const SkRRect& rrect) { this->setRRect(rrect); }
    explicit Shape(const SkPath& path)   { this->setPath(path); }

    ~Shape() { this->reset(); }

    // NOTE: None of the geometry types benefit from move semantics, so we don't bother
    // defining a move assignment operator for Shape.
    Shape& operator=(Shape&&) = delete;
    Shape& operator=(const Shape&);

    // Return the type of the data last stored in the Shape, which does not incorporate any possible
    // simplifications that could be applied to it (e.g. a degenerate round rect with 0 radius
    // corners is kRRect and not kRect).
    Type type() const { return fType; }

    bool isEmpty()    const { return fType == Type::kEmpty; }
    bool isRect()     const { return fType == Type::kRect; }
    bool isRRect()    const { return fType == Type::kRRect; }
    bool isPath()     const { return fType == Type::kPath; }

    bool inverted() const {
        SkASSERT(fType != Type::kPath || fInverted == fPath.isInverseFillType());
        return fInverted;
    }

    void setInverted(bool inverted) {
        if (fType == Type::kPath && inverted != fPath.isInverseFillType()) {
            fPath.toggleInverseFillType();
        }
        fInverted = inverted;
    }

    SkPathFillType fillType() const {
        if (fType == Type::kPath) {
            return fPath.getFillType(); // already incorporates invertedness
        } else {
            return fInverted ? SkPathFillType::kInverseEvenOdd : SkPathFillType::kEvenOdd;
        }
    }

    // True if the given bounding box is completely inside the shape, if it's conservatively treated
    // as a filled, closed shape.
    bool conservativeContains(const SkRect& rect) const;
    bool conservativeContains(const SkV2& point) const;

    // True if the underlying geometry represents a closed shape, without the need for an
    // implicit close.
    bool closed() const;

    // True if the underlying shape is known to be convex, assuming no other styles. If 'simpleFill'
    // is true, it is assumed the contours will be implicitly closed when drawn or used.
    bool convex(bool simpleFill = true) const;

    // The bounding box of the shape.
    SkRect bounds() const;

    // Convert the shape into a path that describes the same geometry.
    SkPath asPath() const;

    // Access the actual geometric description of the shape. May only access the appropriate type
    // based on what was last set.
    const SkRect&      rect()        const { SkASSERT(this->isRect()); return fRect; }
    const SkRRect&     rrect()       const { SkASSERT(this->isRRect()); return fRRect; }
    const SkPath&      path()        const { SkASSERT(this->isPath()); return fPath; }

    // Update the geometry stored in the Shape and update its associated type to match. This
    // performs no simplification, so calling setRRect() with a round rect that has isRect() return
    // true will still be considered an rrect by Shape.
    //
    // These reset inversion to the default for the geometric type.
    void setRect(const SkRect& rect) {
        this->setType(Type::kRect);
        fRect = rect;
        fInverted = false;
    }
    void setRRect(const SkRRect& rrect) {
        this->setType(Type::kRRect);
        fRRect = rrect;
        fInverted = false;
    }
    void setPath(const SkPath& path) {
        if (fType == Type::kPath) {
            // Assign directly
            fPath = path;
        } else {
            // In-place initialize
            this->setType(Type::kPath);
            new (&fPath) SkPath(path);
        }
        fInverted = path.isInverseFillType();
    }

    void reset() {
        this->setType(Type::kEmpty);
        fInverted = false;
    }

private:
    void setType(Type type) {
        if (this->isPath() && type != Type::kPath) {
            fPath.~SkPath();
        }
        fType = type;
    }

    union {
        SkRect  fRect;
        SkRRect fRRect;
        SkPath  fPath;
    };

    Type    fType        = Type::kEmpty;
    bool    fInverted    = false;
};

} // namespace skgpu::geom

#endif // skgpu_geom_Shape_DEFINED
