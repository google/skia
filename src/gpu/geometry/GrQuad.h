/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrQuad_DEFINED
#define GrQuad_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "include/private/SkVx.h"
#include "src/gpu/GrVertexWriter.h"

enum class GrQuadAAFlags;

/**
 * GrQuad is a collection of 4 points which can be used to represent an arbitrary quadrilateral. The
 * points make a triangle strip with CCW triangles (top-left, bottom-left, top-right, bottom-right).
 */
class GrQuad {
public:
    // Quadrilaterals can be classified in several useful ways that assist AA tessellation and other
    // analysis when drawing, in particular, knowing if it was originally a rectangle transformed by
    // certain types of matrices:
    enum class Type {
        // The 4 points remain an axis-aligned rectangle; their logical indices may not respect
        // TL, BL, TR, BR ordering if the transform was a 90 degree rotation or mirror.
        kAxisAligned,
        // The 4 points represent a rectangle subjected to a rotation, its corners are right angles.
        kRectilinear,
        // Arbitrary 2D quadrilateral; may have been a rectangle transformed with skew or some
        // clipped polygon. Its w coordinates will all be 1.
        kGeneral,
        // Even more general-purpose than kGeneral, this allows the w coordinates to be non-unity.
        kPerspective,
        kLast = kPerspective
    };
    static const int kTypeCount = static_cast<int>(Type::kLast) + 1;

    // This enforces W == 1 for non-perspective quads, but does not initialize X or Y.
    GrQuad() = default;
    GrQuad(const GrQuad&) = default;

    explicit GrQuad(const SkRect& rect)
            : fX{rect.fLeft, rect.fLeft, rect.fRight, rect.fRight}
            , fY{rect.fTop, rect.fBottom, rect.fTop, rect.fBottom} {}

    static GrQuad MakeFromRect(const SkRect&, const SkMatrix&);

    // Creates a GrQuad from the quadrilateral 'pts', transformed by the matrix. The input
    // points array is arranged as per SkRect::toQuad (top-left, top-right, bottom-right,
    // bottom-left). The returned instance's point order will still be CCW tri-strip order.
    static GrQuad MakeFromSkQuad(const SkPoint pts[4], const SkMatrix&);

    GrQuad& operator=(const GrQuad&) = default;

    SkPoint3 point3(int i) const { return {fX[i], fY[i], fW[i]}; }

    SkPoint point(int i) const {
        if (fType == Type::kPerspective) {
            return {fX[i] / fW[i], fY[i] / fW[i]};
        } else {
            return {fX[i], fY[i]};
        }
    }

    void writeVertex(int cornerIdx, GrVertexWriter& w) const {
        w << this->point(cornerIdx);
    }

    SkRect bounds() const {
        if (fType == GrQuad::Type::kPerspective) {
            return this->projectedBounds();
        }
        // Calculate min/max directly on the 4 floats, instead of loading/unloading into SIMD. Since
        // there's no horizontal min/max, it's not worth it. Defining non-perspective case in header
        // also leads to substantial performance boost due to inlining.
        auto min = [](const float c[4]) { return std::min(std::min(c[0], c[1]),
                                                          std::min(c[2], c[3]));};
        auto max = [](const float c[4]) { return std::max(std::max(c[0], c[1]),
                                                          std::max(c[2], c[3]));};
        return { min(fX), min(fY), max(fX), max(fY) };
    }

    bool isFinite() const {
        // If any coordinate is infinity or NaN, then multiplying it with 0 will make accum NaN
        float accum = 0;
        for (int i = 0; i < 4; ++i) {
            accum *= fX[i];
            accum *= fY[i];
            accum *= fW[i];
        }
        SkASSERT(0 == accum || SkScalarIsNaN(accum));
        return !SkScalarIsNaN(accum);
    }

    float x(int i) const { return fX[i]; }
    float y(int i) const { return fY[i]; }
    float w(int i) const { return fW[i]; }
    float iw(int i) const { return sk_ieee_float_divide(1.f, fW[i]); }

    skvx::Vec<4, float> x4f() const { return skvx::Vec<4, float>::Load(fX); }
    skvx::Vec<4, float> y4f() const { return skvx::Vec<4, float>::Load(fY); }
    skvx::Vec<4, float> w4f() const { return skvx::Vec<4, float>::Load(fW); }
    skvx::Vec<4, float> iw4f() const { return 1.f / this->w4f(); }

    Type quadType() const { return fType; }

    bool hasPerspective() const { return fType == Type::kPerspective; }

    // True if anti-aliasing affects this quad. Only valid when quadType == kAxisAligned
    bool aaHasEffectOnRect() const;

    // True if this quad is axis-aligned and still has its top-left corner at v0. Equivalently,
    // quad == GrQuad(quad->bounds()). Axis-aligned quads with flips and rotations may exactly
    // fill their bounds, but their vertex order will not match TL BL TR BR anymore.
    bool asRect(SkRect* rect) const;

    // The non-const pointers are provided to support modifying a GrQuad in-place, but care must be
    // taken to keep its quad type aligned with the geometric nature of the new coordinates.
    const float* xs() const { return fX; }
    float* xs() { return fX; }
    const float* ys() const { return fY; }
    float* ys() { return fY; }
    const float* ws() const { return fW; }
    float* ws() { return fW; }

    // Automatically ensures ws are 1 if new type is not perspective.
    void setQuadType(Type newType) {
        if (newType != Type::kPerspective && fType == Type::kPerspective) {
            fW[0] = fW[1] = fW[2] = fW[3] = 1.f;
        }
        SkASSERT(newType == Type::kPerspective ||
                 (SkScalarNearlyEqual(fW[0], 1.f) && SkScalarNearlyEqual(fW[1], 1.f) &&
                  SkScalarNearlyEqual(fW[2], 1.f) && SkScalarNearlyEqual(fW[3], 1.f)));

        fType = newType;
    }
private:
    template<typename T>
    friend class GrQuadListBase; // for access to fX, fY, fW

    GrQuad(const skvx::Vec<4, float>& xs, const skvx::Vec<4, float>& ys, Type type)
            : fType(type) {
        SkASSERT(type != Type::kPerspective);
        xs.store(fX);
        ys.store(fY);
    }

    GrQuad(const skvx::Vec<4, float>& xs, const skvx::Vec<4, float>& ys,
           const skvx::Vec<4, float>& ws, Type type)
            : fW{} // Include fW in member initializer to avoid redundant default initializer
            , fType(type) {
        xs.store(fX);
        ys.store(fY);
        ws.store(fW);
    }

    // Defined in GrQuadUtils.cpp to share the coord clipping code
    SkRect projectedBounds() const;

    float fX[4];
    float fY[4];
    float fW[4] = {1.f, 1.f, 1.f, 1.f};

    Type fType = Type::kAxisAligned;
};

template<> struct GrVertexWriter::is_quad<GrQuad> : std::true_type {};

// A simple struct representing the common work unit of a pair of device and local coordinates, as
// well as the edge flags controlling anti-aliasing for the quadrilateral when drawn.
struct DrawQuad {
    GrQuad        fDevice;
    GrQuad        fLocal;
    GrQuadAAFlags fEdgeFlags;
};

#endif
