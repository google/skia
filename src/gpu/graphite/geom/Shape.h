/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_Shape_DEFINED
#define skgpu_graphite_geom_Shape_DEFINED

#include "include/core/SkArc.h"
#include "include/core/SkM44.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/private/base/SkAssert.h"
#include "src/base/SkVx.h"
#include "src/gpu/graphite/geom/Rect.h"

#include <cstdint>
#include <new>

struct SkRect;

namespace skgpu::graphite {

/**
 * Shape is effectively a std::variant over different geometric shapes, with the most complex
 * being an SkPath. It provides a consistent way to query geometric properties, such as convexity,
 * point containment, or iteration.
 */
class Shape {
public:
    enum class Type : uint8_t {
        kEmpty, kLine, kRect, kRRect, kArc, kPath
    };
    inline static constexpr int kTypeCount = static_cast<int>(Type::kPath) + 1;

    Shape() {}
    Shape(const Shape& shape)               { *this = shape; }
    Shape(Shape&&) = delete;

    Shape(SkPoint p0, SkPoint p1)           { this->setLine(p0, p1); }
    Shape(SkV2 p0, SkV2 p1)                 { this->setLine(p0, p1); }
    Shape(skvx::float2 p0, skvx::float2 p1) { this->setLine(p0, p1); }
    explicit Shape(const Rect& rect)        { this->setRect(rect);   }
    explicit Shape(const SkRect& rect)      { this->setRect(rect);   }
    explicit Shape(const SkRRect& rrect)    { this->setRRect(rrect); }
    explicit Shape(const SkArc& arc)        { this->setArc(arc);     }
    explicit Shape(const SkPath& path)      { this->setPath(path);   }

    ~Shape() { this->reset(); }

    // NOTE: None of the geometry types benefit from move semantics, so we don't bother
    // defining a move assignment operator for Shape.
    Shape& operator=(Shape&&) = delete;
    Shape& operator=(const Shape&);

    // Return the type of the data last stored in the Shape, which does not incorporate any possible
    // simplifications that could be applied to it (e.g. a degenerate round rect with 0 radius
    // corners is kRRect and not kRect).
    Type type() const { return fType; }

    bool isEmpty() const { return fType == Type::kEmpty; }
    bool isLine()  const { return fType == Type::kLine;  }
    bool isRect()  const { return fType == Type::kRect;  }
    bool isRRect() const { return fType == Type::kRRect; }
    bool isArc()   const { return fType == Type::kArc;   }
    bool isPath()  const { return fType == Type::kPath;  }

    bool isVolatilePath() const {
        return fType == Type::kPath && this->path().isVolatile();
    }

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
    bool conservativeContains(const Rect& rect) const;
    bool conservativeContains(skvx::float2 point) const;

    // True if the underlying shape is known to be convex, assuming no other styles. If 'simpleFill'
    // is true, it is assumed the contours will be implicitly closed when drawn or used.
    bool convex(bool simpleFill = true) const;

    // The bounding box of the shape.
    Rect bounds() const;

    // Convert the shape into a path that describes the same geometry.
    SkPath asPath() const;

    // Access the actual geometric description of the shape. May only access the appropriate type
    // based on what was last set.
    skvx::float2   p0()    const { SkASSERT(this->isLine());  return fRect.topLeft();  }
    skvx::float2   p1()    const { SkASSERT(this->isLine());  return fRect.botRight(); }
    skvx::float4   line()  const { SkASSERT(this->isLine());  return fRect.ltrb();     }
    const Rect&    rect()  const { SkASSERT(this->isRect());  return fRect;            }
    const SkRRect& rrect() const { SkASSERT(this->isRRect()); return fRRect;           }
    const SkArc&   arc()   const { SkASSERT(this->isArc());   return fArc;             }
    const SkPath&  path()  const { SkASSERT(this->isPath());  return fPath;            }

    // Update the geometry stored in the Shape and update its associated type to match. This
    // performs no simplification, so calling setRRect() with a round rect that has isRect() return
    // true will still be considered an rrect by Shape.
    //
    // These reset inversion to the default for the geometric type.
    void setLine(SkPoint p0, SkPoint p1) {
        this->setLine(skvx::float2{p0.fX, p0.fY}, skvx::float2{p1.fX, p1.fY});
    }
    void setLine(SkV2 p0, SkV2 p1) {
        this->setLine(skvx::float2{p0.x, p0.y}, skvx::float2{p1.x, p1.y});
    }
    void setLine(skvx::float2 p0, skvx::float2 p1) {
        this->setType(Type::kLine);
        fRect = Rect(p0, p1);
        fInverted = false;
    }
    void setRect(const SkRect& rect) { this->setRect(Rect(rect)); }
    void setRect(const Rect& rect) {
        this->setType(Type::kRect);
        fRect = rect;
        fInverted = false;
    }
    void setRRect(const SkRRect& rrect) {
        this->setType(Type::kRRect);
        fRRect = rrect;
        fInverted = false;
    }
    void setArc(const SkArc& arc) {
        this->setType(Type::kArc);
        fArc = arc;
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

    /**
     * Gets the size of the key for the shape represented by this Shape.
     */
    int keySize() const;

    /**
     * Writes keySize() bytes into the provided pointer. Assumes that there is enough
     * space allocated for the key. If includeInverted is false, non-inverted state will
     * be written into the key regardless of the Shape's state.
     */
    void writeKey(uint32_t* key, bool includeInverted) const;

private:
    void setType(Type type) {
        if (this->isPath() && type != Type::kPath) {
            fPath.~SkPath();
        }
        fType = type;
    }

    /**
     * Key for the state data in the shape. This includes path fill type,
     * and any tracked inversion, as well as the class of geometry.
     * If includeInverted is false, non-inverted state will be written into
     * the key regardless of the Shape's state.
     */
    uint32_t stateKey(bool includeInverted) const;

    union {
        Rect    fRect; // p0 = top-left, p1 = bot-right if type is kLine (may be unsorted)
        SkRRect fRRect;
        SkArc   fArc;
        SkPath  fPath;
    };

    Type    fType     = Type::kEmpty;
    bool    fInverted = false;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_Shape_DEFINED
