/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_BoundsManager_DEFINED
#define skgpu_graphite_geom_BoundsManager_DEFINED

#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/geom/Rect.h"

#include "src/core/SkTBlockList.h"

#include <cstdint>

namespace skgpu::graphite {

/**
 * BoundsManager is an acceleration structure for device-space related pixel bounds queries.
 * The BoundsManager tracks a single ordinal value per bounds: the CompressedPaintersOrder of a draw
 * The CompressedPaintersOrder enforces a specific submission order of draws to the GPU but can
 * re-arrange draws out of their original painter's order if the GREATER depth test and the draw's Z
 * value resolve out-of-order rendering.
 *
 * It supports querying the most recent draw intersecting a bounding rect (represented as a
 * CompressedPaintersOrder value), and recording a (bounds, CompressedPaintersOrder) pair.
 */
class BoundsManager {
public:
    virtual ~BoundsManager() {}

    virtual CompressedPaintersOrder getMostRecentDraw(const Rect& bounds) const = 0;

    virtual void recordDraw(const Rect& bounds, CompressedPaintersOrder order) = 0;

    virtual void reset() = 0;
};

// TODO: Select one most-effective BoundsManager implementation, make it the only option, and remove
// virtual-ness. For now, this seems useful for correctness testing by comparing against trivial
// implementations and for identifying how much "smarts" are actually worthwhile.

// A BoundsManager that produces exact painter's order and assumes nothing is occluded.
class NaiveBoundsManager final : public BoundsManager {
public:
    ~NaiveBoundsManager() override {}

    CompressedPaintersOrder getMostRecentDraw(const Rect& bounds) const override {
        return fLatestDraw;
    }


    void recordDraw(const Rect& bounds, CompressedPaintersOrder order) override {
        if (fLatestDraw < order) {
            fLatestDraw = order;
        }
    }

    void reset() override {
        fLatestDraw = CompressedPaintersOrder::First();
    }

private:
    CompressedPaintersOrder fLatestDraw = CompressedPaintersOrder::First();
};

// A BoundsManager that tracks every draw and can exactly determine all queries
// using a brute force search.
class BruteForceBoundsManager : public BoundsManager {
public:
    ~BruteForceBoundsManager() override {}

    CompressedPaintersOrder getMostRecentDraw(const Rect& bounds) const override {
        SkASSERT(fRects.count() == fOrders.count());

        Rect::ComplementRect boundsComplement(bounds);
        CompressedPaintersOrder max = CompressedPaintersOrder::First();
        auto orderIter = fOrders.items().begin();
        for (const Rect& r : fRects.items()) {
            if (r.intersects(boundsComplement)) {
                if (max < *orderIter) {
                    max = *orderIter;
                }
            }
            ++orderIter;
        }
        return max;
    }

    void recordDraw(const Rect& bounds, CompressedPaintersOrder order) override {
        fRects.push_back(bounds);
        fOrders.push_back(order);
    }

    void reset() override {
        fRects.reset();
        fOrders.reset();
    }

private:
    // fRects and fOrders are parallel, but kept separate to avoid wasting padding since Rect is
    // an over-aligned type.
    SkTBlockList<Rect> fRects{16, SkBlockAllocator::GrowthPolicy::kFibonacci};
    SkTBlockList<CompressedPaintersOrder> fOrders{16, SkBlockAllocator::GrowthPolicy::kFibonacci};
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_BoundsManager_DEFINED
