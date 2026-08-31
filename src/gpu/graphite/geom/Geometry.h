/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_Geometry_DEFINED
#define skgpu_graphite_geom_Geometry_DEFINED

#include "include/core/SkMesh.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkVertices.h"
#include "include/private/SkAssert.h"
#include "src/gpu/graphite/geom/AnalyticBlurMask.h"
#include "src/gpu/graphite/geom/CoverageMaskShape.h"
#include "src/gpu/graphite/geom/EdgeAAQuad.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Shape.h"
#include "src/gpu/graphite/geom/SubRunData.h"

#if defined(SK_ENABLE_SPARSE_STRIPS)
#include "src/gpu/graphite/geom/EndCaps.h"
#include "src/gpu/graphite/geom/WideTiles.h"
#endif

#include <cstdint>
#include <new>
#include <type_traits>
#include <utility>

namespace skgpu::graphite {

/**
 * Geometry is a container that can house Shapes, SkVertices, text SubRuns, and per-edge AA quads.
 * TODO - Add unit tests for Geometry.
 */
class Geometry {
public:
    enum class Type : uint8_t {
        kEmpty,
        kShape,
        kVertices,
        kMesh,
        kSubRun,
        kEdgeAAQuad,
        kCoverageMaskShape,
        kAnalyticBlur,
#if defined(SK_ENABLE_SPARSE_STRIPS)
        kWideTiles,
        kEndCaps,
#endif
    };

    Geometry() {}
    Geometry(Geometry&& geom) { *this = std::move(geom); }
    Geometry(const Geometry& geom) { *this = geom; }

    explicit Geometry(const Shape& shape) { this->setShape(shape); }
    explicit Geometry(const SubRunData& subrun) { this->setSubRun(subrun); }
    explicit Geometry(sk_sp<SkVertices> vertices) { this->setVertices(std::move(vertices)); }
    explicit Geometry(const EdgeAAQuad& edgeAAQuad) { this->setEdgeAAQuad(edgeAAQuad); }
    explicit Geometry(const SkMesh& mesh) { this->setMesh(mesh); }
    explicit Geometry(const CoverageMaskShape& mask) { this->setCoverageMaskShape(mask); }
    explicit Geometry(const AnalyticBlurMask& blur) { this->setAnalyticBlur(blur); }
#if defined(SK_ENABLE_SPARSE_STRIPS)
    explicit Geometry(WideTiles&& wideTiles) {
        this->setWideTiles(std::move(wideTiles));
    }
    explicit Geometry(const WideTiles& wideTiles) {
        this->setWideTiles(wideTiles);
    }
    explicit Geometry(EndCaps&& endCaps) {
        this->setEndCaps(std::move(endCaps));
    }
    explicit Geometry(const EndCaps& endCaps) {
        this->setEndCaps(endCaps);
    }
#endif

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
                case Type::kMesh:
                    this->setMesh(geom.fMesh);
                    geom.setType(Type::kEmpty);
                    break;
                case Type::kSubRun:
                    this->setSubRun(geom.subRunData());
                    geom.setType(Type::kEmpty);
                    break;
                case Type::kEdgeAAQuad:
                    this->setEdgeAAQuad(geom.edgeAAQuad());
                    geom.setType(Type::kEmpty);
                    break;
                case Type::kCoverageMaskShape:
                    this->setCoverageMaskShape(geom.coverageMaskShape());
                    geom.setType(Type::kEmpty);
                    break;
                case Type::kAnalyticBlur:
                    this->setAnalyticBlur(geom.analyticBlurMask());
                    geom.setType(Type::kEmpty);
                    break;
#if defined(SK_ENABLE_SPARSE_STRIPS)
                case Type::kWideTiles:
                    this->setWideTiles(std::move(geom.fWideTiles));
                    geom.setType(Type::kEmpty);
                    break;
                case Type::kEndCaps:
                    this->setEndCaps(std::move(geom.fEndCaps));
                    geom.setType(Type::kEmpty);
                    break;
#endif
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
            case Type::kMesh: this->setMesh(geom.fMesh); break;
            case Type::kEdgeAAQuad: this->setEdgeAAQuad(geom.edgeAAQuad()); break;
            case Type::kCoverageMaskShape:
                    this->setCoverageMaskShape(geom.coverageMaskShape()); break;
            case Type::kAnalyticBlur: this->setAnalyticBlur(geom.analyticBlurMask()); break;
#if defined(SK_ENABLE_SPARSE_STRIPS)
            case Type::kWideTiles: this->setWideTiles(geom.wideTiles()); break;
            case Type::kEndCaps: this->setEndCaps(geom.endCaps()); break;
#endif
            default: break;
        }
        return *this;
    }

    Type type() const { return fType; }

    bool isShape() const { return fType == Type::kShape; }
    bool isVertices() const { return fType == Type::kVertices; }
    bool isMesh() const { return fType == Type::kMesh; }
    bool isSubRun() const { return fType == Type::kSubRun; }
    bool isEdgeAAQuad() const { return fType == Type::kEdgeAAQuad; }
    bool isCoverageMaskShape() const { return fType == Type::kCoverageMaskShape; }
    bool isAnalyticBlur() const { return fType == Type::kAnalyticBlur; }
#if defined(SK_ENABLE_SPARSE_STRIPS)
    bool isWideTiles() const { return fType == Type::kWideTiles; }
    bool isEndCaps() const { return fType == Type::kEndCaps; }
#endif
    bool isEmpty() const {
        return fType == (Type::kEmpty) || (this->isShape() &&
                                           this->shape().isEmpty() &&
                                           !this->shape().inverted());
    }

    const Shape& shape() const { SkASSERT(this->isShape()); return fShape; }
    Shape& shape() { SkASSERT(this->isShape()); return fShape; }

    const SubRunData& subRunData() const { SkASSERT(this->isSubRun()); return fSubRunData; }
    const EdgeAAQuad& edgeAAQuad() const { SkASSERT(this->isEdgeAAQuad()); return fEdgeAAQuad; }
    const CoverageMaskShape& coverageMaskShape() const {
        SkASSERT(this->isCoverageMaskShape()); return fCoverageMaskShape;
    }
    const AnalyticBlurMask& analyticBlurMask() const {
        SkASSERT(this->isAnalyticBlur()); return fAnalyticBlurMask;
    }
    const SkVertices* vertices() const { SkASSERT(this->isVertices()); return fVertices.get(); }
    sk_sp<SkVertices> refVertices() const {
        SkASSERT(this->isVertices());
        return fVertices;
    }
#if defined(SK_ENABLE_SPARSE_STRIPS)
    const WideTiles& wideTiles() const {
        SkASSERT(this->isWideTiles()); return fWideTiles;
    }
    const EndCaps& endCaps() const {
        SkASSERT(this->isEndCaps()); return fEndCaps;
    }
#endif
    const SkMesh& mesh() const { SkASSERT(this->isMesh()); return fMesh; }

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

    void setMesh(const SkMesh& mesh) {
        if (fType == Type::kMesh) {
            fMesh = mesh;
        } else {
            this->setType(Type::kMesh);
            new (&fMesh) SkMesh(mesh);
        }
    }

    void setEdgeAAQuad(const EdgeAAQuad& edgeAAQuad) {
        if (fType == Type::kEdgeAAQuad) {
            fEdgeAAQuad = edgeAAQuad;
        } else {
            this->setType(Type::kEdgeAAQuad);
            new (&fEdgeAAQuad) EdgeAAQuad(edgeAAQuad);
        }
    }

    void setCoverageMaskShape(const CoverageMaskShape& maskShape) {
        if (fType == Type::kCoverageMaskShape) {
            fCoverageMaskShape = maskShape;
        } else {
            this->setType(Type::kCoverageMaskShape);
            new (&fCoverageMaskShape) CoverageMaskShape(maskShape);
        }
    }

    void setAnalyticBlur(const AnalyticBlurMask& blur) {
        if (fType == Type::kAnalyticBlur) {
            fAnalyticBlurMask = blur;
        } else {
            this->setType(Type::kAnalyticBlur);
            new (&fAnalyticBlurMask) AnalyticBlurMask(blur);
        }
    }

#if defined(SK_ENABLE_SPARSE_STRIPS)
    void setWideTiles(WideTiles&& wideTiles) {
        if (fType == Type::kWideTiles) {
            fWideTiles = std::move(wideTiles);
        } else {
            this->setType(Type::kWideTiles);
            new (&fWideTiles) WideTiles(std::move(wideTiles));
        }
    }

    void setWideTiles(const WideTiles& wideTiles) {
        if (fType == Type::kWideTiles) {
            fWideTiles = wideTiles;
        } else {
            this->setType(Type::kWideTiles);
            new (&fWideTiles) WideTiles(wideTiles);
        }
    }

    void setEndCaps(EndCaps&& endCaps) {
        if (fType == Type::kEndCaps) {
            fEndCaps = std::move(endCaps);
        } else {
            this->setType(Type::kEndCaps);
            new (&fEndCaps) EndCaps(std::move(endCaps));
        }
    }

    void setEndCaps(const EndCaps& endCaps) {
        if (fType == Type::kEndCaps) {
            fEndCaps = endCaps;
        } else {
            this->setType(Type::kEndCaps);
            new (&fEndCaps) EndCaps(endCaps);
        }
    }
#endif

    // Bounds are relative to the mask coordinate space defined by maskToDevice(). If maskToDevice()
    // returns null, the bounds are relative to the original local-to-device transofrm of the draw.
    Rect bounds() const {
        switch (fType) {
            case Type::kEmpty: return Rect(0, 0, 0, 0);
            case Type::kShape: return fShape.bounds();
            case Type::kVertices: return fVertices->bounds();
            case Type::kMesh: return fMesh.bounds();
            case Type::kSubRun: return fSubRunData.bounds();
            case Type::kEdgeAAQuad: return fEdgeAAQuad.bounds();
            case Type::kCoverageMaskShape: return fCoverageMaskShape.bounds();
            case Type::kAnalyticBlur: return fAnalyticBlurMask.drawBounds();
#if defined(SK_ENABLE_SPARSE_STRIPS)
            case Type::kWideTiles: return Rect(0, 0, 0, 0); // These are unused, as the geometry
            case Type::kEndCaps: return Rect(0, 0, 0, 0);   // is generated after clipping
#endif
        }
        SkUNREACHABLE;
    }

    // Normally there are two coordinate spaces in play: local coords that parameters to drawX()
    // calls are defined in, and device coords representing the pixel coords of the SkDevice.
    // Some draws get mapped to an intermediate Geometry that can add a third coordinate space:
    // the mask space. This may differ from device coords by only an integer translation or could
    // include everything except perspective, etc.
    //
    // If this is non-null, the returned transform represents the transform to be applied by the
    // Renderer to the geometry but *not* the local coordinates. If null is returned, it is assumed
    // that the "mask" space is identical to the local coord space.
    const SkM44* maskToDevice() const {
        if (fType == Type::kCoverageMaskShape) {
            return &this->coverageMaskShape().maskToDevice();
        } else if (fType == Type::kSubRun) {
            return &this->subRunData().maskToDevice();
        } else {
            // Everything is defined relative to the local coordinate space.
            // TODO(michaelludwig): AnalyticBlur might by simplified using this instead of
            // deviceToScaledShape(), but it already tracks its original local bounds and not just
            // mask-space bounds so analytic blurs may be fine.
            return nullptr;
        }
    }

private:
    void setType(Type type) {
        static_assert(std::is_trivially_destructible<EdgeAAQuad>::value);
        if (this->isShape() && type != Type::kShape) {
            fShape.~Shape();
        } else if (this->isSubRun() && type != Type::kSubRun) {
            fSubRunData.~SubRunData();
        } else if (this->isVertices() && type != Type::kVertices) {
            fVertices.~sk_sp<SkVertices>();
        } else if (this->isMesh() && type != Type::kMesh) {
            fMesh.~SkMesh();
        } else if (this->isCoverageMaskShape() && type != Type::kCoverageMaskShape) {
            fCoverageMaskShape.~CoverageMaskShape();
        } else if (this->isAnalyticBlur() && type != Type::kAnalyticBlur) {
            fAnalyticBlurMask.~AnalyticBlurMask();
#if defined(SK_ENABLE_SPARSE_STRIPS)
        } else if (this->isWideTiles() && type != Type::kWideTiles) {
            fWideTiles.~WideTiles();
        } else if (this->isEndCaps() && type != Type::kEndCaps) {
            fEndCaps.~EndCaps();
#endif
        }
        fType = type;
    }

    Type fType = Type::kEmpty;
    union {
        Shape fShape;
        SubRunData fSubRunData;
        sk_sp<SkVertices> fVertices;
        SkMesh fMesh;
        EdgeAAQuad fEdgeAAQuad;
        CoverageMaskShape fCoverageMaskShape;
        AnalyticBlurMask fAnalyticBlurMask;
#if defined(SK_ENABLE_SPARSE_STRIPS)
        WideTiles fWideTiles;
        EndCaps fEndCaps;
#endif
    };
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_Geometry_DEFINED
