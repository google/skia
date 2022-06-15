/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_Geometry_DEFINED
#define skgpu_graphite_geom_Geometry_DEFINED

#include "include/core/SkVertices.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Shape.h"

namespace skgpu::graphite {

/**
 * Geometry is a container that can house Shapes, SkVertices, and eventually text subruns.
 * TODO - Add text subruns & vertices functionality. As of right now, this class is really just a
 * wrapper for Shape.
 */
class Geometry {
public:
    enum class Type : uint8_t {
        kEmpty, kShape, kVertices
    };

    Geometry() {}
    explicit Geometry(const Shape& shape) { this->setShape(shape); }
    ~Geometry() { this->setType(Type::kEmpty); }

    Geometry& operator=(Geometry&& geom) {
        if (this != &geom) {
            switch (geom.type()) {
                case Type::kEmpty: this->setType(Type::kEmpty); break;
                case Type::kShape: this->setShape(geom.shape()); geom.setType(Type::kEmpty); break;
                default: break;
            }
        }
        return *this;
    }
    Geometry& operator=(const Geometry& geom) {
        switch (geom.type()) {
            case Type::kEmpty: this->setType(Type::kEmpty); break;
            case Type::kShape: this->setShape(geom.shape()); break;
            default: break;
        }
        return *this;
    }

    Geometry(const Geometry& geom) {
        *this = geom;
    }

    void setShape(const Shape& shape) {
        if (fType == Type::kShape) {
            fShape = shape;
        } else {
            this->setType(Type::kShape);
            new (&fShape) Shape(shape);
        }
    }
    // void setVertices(const SkVertices&);

    const Shape& shape() const { SkASSERT(this->isShape()); return fShape; }
    // const SkVertices& vertices() const;
    Rect bounds() const {
        switch (fType) {
            case Type::kShape: return fShape.bounds();
            default: return Rect(0, 0, 0, 0);
        }
    }
    Type type() const { return fType; }

    bool isShape() const { return fType == Type::kShape; }
    bool isVertices() const { return fType == Type::kVertices; }
    bool isEmpty() const {
        return fType == (Type::kEmpty) || (this->isShape() && this->shape().isEmpty());
    }

private:
    void setType(Type type) {
        if (this->isShape() && type != Type::kShape) {
            fShape.~Shape();
        }
        fType = type;
    }

    Type fType = Type::kEmpty;
    union {
        Shape fShape;
        // SkVertices fVertices;
    };

};
} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_Geometry_DEFINED
