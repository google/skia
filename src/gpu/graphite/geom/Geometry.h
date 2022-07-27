/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_Geometry_DEFINED
#define skgpu_graphite_geom_Geometry_DEFINED

#include "include/core/SkVertices.h"
#include "src/core/SkVerticesPriv.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Shape.h"
#include "src/gpu/graphite/geom/SubRunData.h"

namespace skgpu::graphite {

/**
 * Geometry is a container that can house Shapes, SkVertices, and text SubRuns.
 * TODO - Add unit tests for Geometry.
 */
class Geometry {
public:
    enum class Type : uint8_t {
        kEmpty, kShape, kVertices, kSubRun
    };

    Geometry() {}
    explicit Geometry(const Shape& shape) { this->setShape(shape); }
    explicit Geometry(const SubRunData& subrun) { this->setSubRun(subrun); }
    explicit Geometry(sk_sp<SkVertices> vertices) { this->setVertices(vertices); }
    ~Geometry() { this->setType(Type::kEmpty); }

    Geometry& operator=(Geometry&& geom) {
        if (this != &geom) {
            switch (geom.type()) {
                case Type::kEmpty:
                    this->setType(Type::kEmpty);
                    break;
                case Type::kShape:
                    this->setShape(geom.shape());
                    geom.setType(Type::kEmpty);
                    break;
                case Type::kVertices:
                    this->setVertices(std::move(geom.fVertices));
                    geom.setType(Type::kEmpty);
                    break;
                case Type::kSubRun:
                    this->setSubRun(geom.subRunData());
                    geom.setType(Type::kEmpty);
                    break;
            }
        }
        return *this;
    }
    Geometry& operator=(const Geometry& geom) {
        switch (geom.type()) {
            case Type::kEmpty: this->setType(Type::kEmpty); break;
            case Type::kShape: this->setShape(geom.shape()); break;
            case Type::kSubRun: this->setSubRun(geom.subRunData()); break;
            case Type::kVertices: this->setVertices(geom.fVertices); break;
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
    void setSubRun(const SubRunData& subRun) {
        if (fType == Type::kSubRun) {
            fSubRunData = subRun;
        } else {
            this->setType(Type::kSubRun);
            new (&fSubRunData) SubRunData(subRun);
        }
    }
    void setVertices(sk_sp<SkVertices> vertices) {
        if (fType == Type::kVertices) {
            fVertices = std::move(vertices);
        } else {
            this->setType(Type::kVertices);
            new (&fVertices) sk_sp<SkVertices>(std::move(vertices));
        }
    }

    const Shape& shape() const { SkASSERT(this->isShape()); return fShape; }
    const SubRunData& subRunData() const { SkASSERT(this->isSubRun()); return fSubRunData; }
    const SkVertices* vertices() const { SkASSERT(this->isVertices()); return fVertices.get(); }
    sk_sp<SkVertices> refVertices() const {
        SkASSERT(this->isVertices());
        return fVertices;
    }
    Rect bounds() const {
        switch (fType) {
            case Type::kEmpty: return Rect(0, 0, 0, 0);
            case Type::kShape: return fShape.bounds();
            case Type::kVertices: return fVertices->bounds();
            case Type::kSubRun: return fSubRunData.bounds();
        }
        SkUNREACHABLE;
    }
    Type type() const { return fType; }

    bool isShape() const { return fType == Type::kShape; }
    bool isVertices() const { return fType == Type::kVertices; }
    bool isSubRun() const { return fType == Type::kSubRun; }
    bool isEmpty() const {
        return fType == (Type::kEmpty) || (this->isShape() && this->shape().isEmpty());
    }

private:
    void setType(Type type) {
        if (this->isShape() && type != Type::kShape) {
            fShape.~Shape();
        } else if (this->isSubRun() && type != Type::kSubRun) {
            fSubRunData.~SubRunData();
        } else if (this->isVertices() && type != Type::kVertices) {
            fVertices.~sk_sp<SkVertices>();
        }
        fType = type;
    }

    Type fType = Type::kEmpty;
    union {
        Shape fShape;
        SubRunData fSubRunData;
        sk_sp<SkVertices> fVertices;
    };
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_Geometry_DEFINED
