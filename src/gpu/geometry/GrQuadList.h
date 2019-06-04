/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrQuadList_DEFINED
#define GrQuadList_DEFINED

#include "include/private/SkTArray.h"
#include "src/gpu/geometry/GrQuad.h"

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

    void reserve(int count, bool needsPerspective) {
        fXYs.reserve(count);
        if (needsPerspective || fType == GrQuadType::kPerspective) {
            fWs.reserve(4 * count);
        }
    }

    GrPerspQuad operator[] (int i) const {
        SkASSERT(i < this->count());
        SkASSERT(i >= 0);

        const QuadData<T>& item = fXYs[i];
        if (fType == GrQuadType::kPerspective) {
            // Read the explicit ws
            return GrPerspQuad(item.fX, item.fY, fWs.begin() + 4 * i, fType);
        } else {
            // Ws are implicitly 1s.
            static constexpr float kNoPerspectiveWs[4] = {1.f, 1.f, 1.f, 1.f};
            return GrPerspQuad(item.fX, item.fY, kNoPerspectiveWs, fType);
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
    QuadData<T>& pushBackImpl(const GrQuad& quad) {
        this->upgradeType(quad.quadType());
        QuadData<T>& item = fXYs.push_back();
        memcpy(item.fX, quad.fX, 4 * sizeof(float));
        memcpy(item.fY, quad.fY, 4 * sizeof(float));
        if (fType == GrQuadType::kPerspective) {
            fWs.push_back_n(4, 1.f);
        }
        return item;
    }

    QuadData<T>& pushBackImpl(const GrPerspQuad& quad) {
        this->upgradeType(quad.quadType());
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

    void push_back(const GrQuad& quad) {
        this->pushBackImpl(quad);
    }

    void push_back(const GrPerspQuad& quad) {
        this->pushBackImpl(quad);
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
    void push_back(const GrQuad& quad, T&& metadata) {
        QuadData<T>& item = this->pushBackImpl(quad);
        item.fMetadata = std::move(metadata);
    }

    void push_back(const GrPerspQuad& quad, T&& metadata) {
        QuadData<T>& item = this->pushBackImpl(quad);
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
