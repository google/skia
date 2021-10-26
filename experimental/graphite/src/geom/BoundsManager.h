/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_geom_BoundsManager_DEFINED
#define skgpu_geom_BoundsManager_DEFINED

#include "experimental/graphite/src/DrawOrder.h"
#include "experimental/graphite/src/geom/Rect.h"

#include "src/core/SkTBlockList.h"

#include <cstdint>

namespace skgpu {

/**
 * BoundsManager is an acceleration structure for device-space related pixel bounds queries.
 * The BoundsManager relies on two related ordinal values: the CompressedPaintersOrder of a draw
 * and the Z/depth value of the draw. The CompressedPaintersOrder enforces a specific submission
 * order of draws to the GPU but can re-arrange draws out of their original painter's order if the
 * GREATER depth test and the draw's Z value resolve out-of-order rendering.
 *
 * It supports querying the most recent draw intersecting a bounding rect (represented as a
 * CompressedPaintersOrder value), recording a (bounds, CompressedPaintersOrder, and Z value) tuple,
 * and querying if a (bounds, Z value) pair is fully occluded by another draw.
 */
class BoundsManager {
public:
    virtual ~BoundsManager() {}

    virtual CompressedPaintersOrder getMostRecentDraw(const Rect& bounds) const = 0;

    virtual bool isOccluded(const Rect& bounds, PaintersDepth z) const = 0;

    virtual void recordDraw(const Rect& bounds,
                            CompressedPaintersOrder order,
                            PaintersDepth z,
                            bool fullyOpaque=false) = 0;
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

    bool isOccluded(const Rect& bounds, PaintersDepth z) const override { return false; }

    void recordDraw(const Rect& bounds, CompressedPaintersOrder order, PaintersDepth z,
                    bool fullyOpaque=false) override {
        if (fLatestDraw < order) {
            fLatestDraw = order;
        }
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
        CompressedPaintersOrder max = CompressedPaintersOrder::First();
        for (const Record& r : fRects.items()) {
            if (max < r.fOrder && r.fBounds.intersects(bounds)) {
                max = r.fOrder;
            }
        }
        return max;
    }

    bool isOccluded(const Rect& bounds, PaintersDepth z) const override {
        // Iterate in reverse since the records were likely recorded in increasing Z
        for (const Record& r : fRects.ritems()) {
            if (r.fOpaque && z < r.fZ && r.fBounds.contains(bounds)) {
                return true;
            }
        }
        return false;
    }

    void recordDraw(const Rect& bounds, CompressedPaintersOrder order, PaintersDepth z,
                    bool fullyOpaque=false) override {
        fRects.push_back({bounds, order, z, fullyOpaque});
    }

private:
    struct Record {
        Rect fBounds;
        CompressedPaintersOrder fOrder;
        PaintersDepth fZ;
        bool fOpaque;
    };

    SkTBlockList<Record> fRects{16, SkBlockAllocator::GrowthPolicy::kFibonacci};
};

} // namespace skgpu

#endif // skgpu_geom_BoundsManager_DEFINED
