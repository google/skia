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
#include "include/private/SkTArray.h"
#include "include/private/SkVx.h"

enum class GrAAType : unsigned;
enum class GrQuadAAFlags;

// Rectangles transformed by matrices (view or local) can be classified in three ways:
//  1. Stays a rectangle - the matrix rectStaysRect() is true, or x(0) == x(1) && x(2) == x(3)
//     and y(0) == y(2) && y(1) == y(3). Or under mirrors, x(0) == x(2) && x(1) == x(3) and
//     y(0) == y(1) && y(2) == y(3).
//  2. Is rectilinear - the matrix does not have skew or perspective, but may rotate (unlike #1)
//  3. Is a quadrilateral - the matrix does not have perspective, but may rotate or skew, or
//     ws() == all ones.
//  4. Is a perspective quad - the matrix has perspective, subsuming all previous quad types.
enum class GrQuadType {
    kRect,
    kRectilinear,
    kStandard,
    kPerspective,
    kLast = kPerspective
};
static const int kGrQuadTypeCount = static_cast<int>(GrQuadType::kLast) + 1;

// If an SkRect is transformed by this matrix, what class of quad is required to represent it.
GrQuadType GrQuadTypeForTransformedRect(const SkMatrix& matrix);
// Perform minimal analysis of 'pts' (which are suitable for MakeFromSkQuad), and determine a
// quad type that will be as minimally general as possible.
GrQuadType GrQuadTypeForPoints(const SkPoint pts[4], const SkMatrix& matrix);

// Resolve disagreements between the overall requested AA type and the per-edge quad AA flags.
// knownQuadType must have come from GrQuadTypeForTransformedRect with the matrix that created the
// provided quad. Both outAAType and outEdgeFlags will be updated.
template <typename Q>
void GrResolveAATypeForQuad(GrAAType requestedAAType, GrQuadAAFlags requestedEdgeFlags,
                            const Q& quad, GrQuadType knownQuadType,
                            GrAAType* outAAtype, GrQuadAAFlags* outEdgeFlags);

/**
 * GrQuad is a collection of 4 points which can be used to represent an arbitrary quadrilateral. The
 * points make a triangle strip with CCW triangles (top-left, bottom-left, top-right, bottom-right).
 */
class GrQuad {
public:
    GrQuad() = default;

    GrQuad(const GrQuad& that) = default;

    explicit GrQuad(const SkRect& rect)
            : fX{rect.fLeft, rect.fLeft, rect.fRight, rect.fRight}
            , fY{rect.fTop, rect.fBottom, rect.fTop, rect.fBottom} {}

    GrQuad(const skvx::Vec<4, float>& xs, const skvx::Vec<4, float>& ys) {
        xs.store(fX);
        ys.store(fY);
    }

    explicit GrQuad(const SkPoint pts[4])
            : fX{pts[0].fX, pts[1].fX, pts[2].fX, pts[3].fX}
            , fY{pts[0].fY, pts[1].fY, pts[2].fY, pts[3].fY} {}

    /** Sets the quad to the rect as transformed by the matrix. */
    static GrQuad MakeFromRect(const SkRect&, const SkMatrix&);

    // Creates a GrQuad from the quadrilateral 'pts', transformed by the matrix. Unlike the explicit
    // constructor, the input points array is arranged as per SkRect::toQuad (top-left, top-right,
    // bottom-right, bottom-left). The returned instance's point order will still be CCW tri-strip
    // order.
    static GrQuad MakeFromSkQuad(const SkPoint pts[4], const SkMatrix&);

    GrQuad& operator=(const GrQuad& that) = default;

    SkPoint point(int i) const { return {fX[i], fY[i]}; }

    SkRect bounds() const {
        auto x = this->x4f(), y = this->y4f();
        return {min(x), min(y), max(x), max(y)};
    }

    float x(int i) const { return fX[i]; }
    float y(int i) const { return fY[i]; }

    skvx::Vec<4, float> x4f() const { return skvx::Vec<4, float>::Load(fX); }
    skvx::Vec<4, float> y4f() const { return skvx::Vec<4, float>::Load(fY); }

    // True if anti-aliasing affects this quad. Only valid when quadType == kRect_QuadType
    bool aaHasEffectOnRect() const;

private:
    template<typename T>
    friend class GrQuadListBase;

    float fX[4];
    float fY[4];
};

class GrPerspQuad {
public:
    GrPerspQuad() = default;

    explicit GrPerspQuad(const SkRect& rect)
            : fX{rect.fLeft, rect.fLeft, rect.fRight, rect.fRight}
            , fY{rect.fTop, rect.fBottom, rect.fTop, rect.fBottom}
            , fW{1.f, 1.f, 1.f, 1.f} {}

    GrPerspQuad(const skvx::Vec<4, float>& xs, const skvx::Vec<4, float>& ys) {
        xs.store(fX);
        ys.store(fY);
        fW[0] = fW[1] = fW[2] = fW[3] = 1.f;
    }

    GrPerspQuad(const skvx::Vec<4, float>& xs, const skvx::Vec<4, float>& ys,
                const skvx::Vec<4, float>& ws) {
        xs.store(fX);
        ys.store(fY);
        ws.store(fW);
    }

    static GrPerspQuad MakeFromRect(const SkRect&, const SkMatrix&);

    // Creates a GrPerspQuad from the quadrilateral 'pts', transformed by the matrix. The input
    // points array is arranged as per SkRect::toQuad (top-left, top-right, bottom-right,
    // bottom-left). The returned instance's point order will still be CCW tri-strip order.
    static GrPerspQuad MakeFromSkQuad(const SkPoint pts[4], const SkMatrix&);

    GrPerspQuad& operator=(const GrPerspQuad&) = default;

    SkPoint3 point(int i) const { return {fX[i], fY[i], fW[i]}; }

    SkRect bounds(GrQuadType type) const {
        auto x = this->x4f();
        auto y = this->y4f();
        if (type == GrQuadType::kPerspective) {
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

    bool hasPerspective() const { return any(w4f() != 1.f); }

    // True if anti-aliasing affects this quad. Only valid when quadType == kRect_QuadType
    bool aaHasEffectOnRect() const;

private:
    template<typename T>
    friend class GrQuadListBase;

    // Copy 4 values from each of the arrays into the quad's components
    GrPerspQuad(const float xs[4], const float ys[4], const float ws[4]);

    float fX[4];
    float fY[4];
    float fW[4];
};

// Underlying data used by GrQuadListBase. It is defined outside of GrQuadListBase due to compiler
// issues related to specializing member types.
template<typename T>
struct QuadData {
    float fX[4];
    float fY[4];
    T fMetadata;
};

template<>
struct QuadData<void> {
    float fX[4];
    float fY[4];
};

// A dynamic list of (possibly) perspective quads that tracks the most general quad type of all
// added quads. It avoids storing the 3rd component if the quad type never becomes perspective.
// Use GrQuadList subclass when only storing quads. Use GrTQuadList subclass when storing quads
// and per-quad templated metadata (such as color or domain).
template<typename T>
class GrQuadListBase {
public:

    int count() const { return fXYs.count(); }

    GrQuadType quadType() const { return fType; }

    void reserve(int count, GrQuadType forType) {
        fXYs.reserve(count);
        if (forType == GrQuadType::kPerspective || fType == GrQuadType::kPerspective) {
            fWs.reserve(4 * count);
        }
    }

    GrPerspQuad operator[] (int i) const {
        SkASSERT(i < this->count());
        SkASSERT(i >= 0);

        const QuadData<T>& item = fXYs[i];
        if (fType == GrQuadType::kPerspective) {
            // Read the explicit ws
            return GrPerspQuad(item.fX, item.fY, fWs.begin() + 4 * i);
        } else {
            // Ws are implicitly 1s.
            static constexpr float kNoPerspectiveWs[4] = {1.f, 1.f, 1.f, 1.f};
            return GrPerspQuad(item.fX, item.fY, kNoPerspectiveWs);
        }
    }

    // Subclasses expose push_back(const GrQuad|GrPerspQuad&, GrQuadType, [const T&]), where
    // the metadata argument is only present in GrTQuadList's push_back definition.

protected:
    GrQuadListBase() : fType(GrQuadType::kRect) {}

    void concatImpl(const GrQuadListBase<T>& that) {
        this->upgradeType(that.fType);
        fXYs.push_back_n(that.fXYs.count(), that.fXYs.begin());
        if (fType == GrQuadType::kPerspective) {
            if (that.fType == GrQuadType::kPerspective) {
                // Copy the other's ws into the end of this list's data
                fWs.push_back_n(that.fWs.count(), that.fWs.begin());
            } else {
                // This list stores ws but the appended list had implicit 1s, so add explicit 1s to
                // fill out the total list
                fWs.push_back_n(4 * that.count(), 1.f);
            }
        }
    }

    // Returns the added item data so that its metadata can be initialized if T is not void
    QuadData<T>& pushBackImpl(const GrQuad& quad, GrQuadType type) {
        this->upgradeType(type);
        QuadData<T>& item = fXYs.push_back();
        memcpy(item.fX, quad.fX, 4 * sizeof(float));
        memcpy(item.fY, quad.fY, 4 * sizeof(float));
        if (fType == GrQuadType::kPerspective) {
            fWs.push_back_n(4, 1.f);
        }
        return item;
    }

    QuadData<T>& pushBackImpl(const GrPerspQuad& quad, GrQuadType type) {
        this->upgradeType(type);
        QuadData<T>& item = fXYs.push_back();
        memcpy(item.fX, quad.fX, 4 * sizeof(float));
        memcpy(item.fY, quad.fY, 4 * sizeof(float));
        if (fType == GrQuadType::kPerspective) {
            fWs.push_back_n(4, quad.fW);
        }
        return item;
    }

    const QuadData<T>& item(int i) const {
        return fXYs[i];
    }

    QuadData<T>& item(int i) {
        return fXYs[i];
    }

private:
    void upgradeType(GrQuadType type) {
        // Possibly upgrade the overall type tracked by the list
        if (type > fType) {
            fType = type;
            if (type == GrQuadType::kPerspective) {
                // All existing quads were 2D, so the ws array just needs to be filled with 1s
                fWs.push_back_n(4 * this->count(), 1.f);
            }
        }
    }

    // Interleaves xs, ys, and per-quad metadata so that all data for a single quad is together
    // (barring ws, which can be dropped entirely if the quad type allows it).
    SkSTArray<1, QuadData<T>, true> fXYs;
    // The w channel is kept separate so that it can remain empty when only dealing with 2D quads.
    SkTArray<float, true> fWs;

    GrQuadType fType;
};

// This list only stores the quad data itself.
class GrQuadList : public GrQuadListBase<void> {
public:
    GrQuadList() : INHERITED() {}

    void concat(const GrQuadList& that) {
        this->concatImpl(that);
    }

    void push_back(const GrQuad& quad, GrQuadType type) {
        this->pushBackImpl(quad, type);
    }

    void push_back(const GrPerspQuad& quad, GrQuadType type) {
        this->pushBackImpl(quad, type);
    }

private:
    typedef GrQuadListBase<void> INHERITED;
};

// This variant of the list allows simple metadata to be stored per quad as well, such as color
// or texture domain.
template<typename T>
class GrTQuadList : public GrQuadListBase<T> {
public:
    GrTQuadList() : INHERITED() {}

    void concat(const GrTQuadList<T>& that) {
        this->concatImpl(that);
    }

    // Adding to the list requires metadata
    void push_back(const GrQuad& quad, GrQuadType type, T&& metadata) {
        QuadData<T>& item = this->pushBackImpl(quad, type);
        item.fMetadata = std::move(metadata);
    }

    void push_back(const GrPerspQuad& quad, GrQuadType type, T&& metadata) {
        QuadData<T>& item = this->pushBackImpl(quad, type);
        item.fMetadata = std::move(metadata);
    }

    // And provide access to the metadata per quad
    const T& metadata(int i) const {
        return this->item(i).fMetadata;
    }

    T& metadata(int i) {
        return this->item(i).fMetadata;
    }

private:
    typedef GrQuadListBase<T> INHERITED;
};

#endif
