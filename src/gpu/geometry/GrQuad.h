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

enum class GrAAType : unsigned;
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
        // TL, BL, TR, BR ordering if the transform was a 90 degre rotation or mirror.
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

    GrQuad() = default;

    explicit GrQuad(const SkRect& rect)
            : fX{rect.fLeft, rect.fLeft, rect.fRight, rect.fRight}
            , fY{rect.fTop, rect.fBottom, rect.fTop, rect.fBottom}
            , fW{1.f, 1.f, 1.f, 1.f}
            , fType(Type::kAxisAligned) {}

    GrQuad(const skvx::Vec<4, float>& xs, const skvx::Vec<4, float>& ys, Type type)
            : fType(type) {
        SkASSERT(type != Type::kPerspective);
        xs.store(fX);
        ys.store(fY);
        fW[0] = fW[1] = fW[2] = fW[3] = 1.f;
    }

    GrQuad(const skvx::Vec<4, float>& xs, const skvx::Vec<4, float>& ys,
           const skvx::Vec<4, float>& ws, Type type)
            : fType(type) {
        xs.store(fX);
        ys.store(fY);
        ws.store(fW);
    }

    // Copy 4 values from each of the arrays into the quad's components
    GrQuad(const float xs[4], const float ys[4], const float ws[4], Type type)
            : fType(type) {
        memcpy(fX, xs, 4 * sizeof(float));
        memcpy(fY, ys, 4 * sizeof(float));
        memcpy(fW, ws, 4 * sizeof(float));
    }

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

    SkRect bounds() const {
        auto x = this->x4f();
        auto y = this->y4f();
        if (fType == Type::kPerspective) {
            auto iw = this->iw4f();
            x *= iw;
            y *= iw;
        }

        return {min(x), min(y), max(x), max(y)};
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

private:
    template<typename T>
    friend class GrQuadListBase; // for access to fX, fY, fW

    float fX[4];
    float fY[4];
    float fW[4];

    Type fType;
};

// Resolve disagreements between the overall requested AA type and the per-edge quad AA flags.
// Both outAAType and outEdgeFlags will be updated.
void GrResolveAATypeForQuad(GrAAType requestedAAType, GrQuadAAFlags requestedEdgeFlags,
                            const GrQuad& quad, GrAAType* outAAtype, GrQuadAAFlags* outEdgeFlags);

#endif
