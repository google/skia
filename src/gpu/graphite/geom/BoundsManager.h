/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_BoundsManager_DEFINED
#define skgpu_graphite_geom_BoundsManager_DEFINED

#include "include/core/SkSize.h"
#include "include/private/SkTemplates.h"

#include "src/core/SkTBlockList.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/geom/Rect.h"

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

class GridBoundsManager : public BoundsManager {
public:
    // 'gridSize' is the number of cells in the X and Y directions, splitting the pixels from [0,0]
    // to 'deviceSize' into uniformly-sized cells.
    static std::unique_ptr<GridBoundsManager> Make(const SkISize& deviceSize,
                                                   const SkISize& gridSize) {
        SkASSERT(deviceSize.width() > 0 && deviceSize.height() > 0);
        SkASSERT(gridSize.width() >= 1 && gridSize.height() >= 1);

        return std::unique_ptr<GridBoundsManager>(new GridBoundsManager(deviceSize, gridSize));
    }

    static std::unique_ptr<GridBoundsManager> Make(const SkISize& deviceSize, int gridSize) {
        return Make(deviceSize, {gridSize, gridSize});
    }

    static std::unique_ptr<GridBoundsManager> MakeRes(const SkISize& deviceSize, int gridCellSize) {
        SkASSERT(deviceSize.width() > 0 && deviceSize.height() > 0);
        SkASSERT(gridCellSize >= 1);

        int gridWidth = SkScalarCeilToInt(deviceSize.width() / (float) gridCellSize);
        int gridHeight = SkScalarCeilToInt(deviceSize.height() / (float) gridCellSize);

        // This keeps the grid cells exactly at the requested resolution, but pads the right and
        // bottom edges out to a multiple of the cell size. The alternative is pass in the unpadded
        // device size, which would mean the actual cell size will be smaller than the requested
        // (by (deviceSize % gridCellSize)/gridDims).
        SkISize paddedDeviceSize = {gridWidth * gridCellSize, gridHeight * gridCellSize};
        return Make(paddedDeviceSize, {gridWidth, gridHeight});
    }

    ~GridBoundsManager() override {}


    CompressedPaintersOrder getMostRecentDraw(const Rect& bounds) const override {
        SkASSERT(!bounds.isEmptyNegativeOrNaN());

        auto ltrb = this->getGridCoords(bounds);
        const CompressedPaintersOrder* p = fNodes.data() + ltrb[1] * fGridWidth + ltrb[0];
        int h = ltrb[3] - ltrb[1];
        int w = ltrb[2] - ltrb[0];

        CompressedPaintersOrder max = CompressedPaintersOrder::First();
        for (int y = 0; y <= h; ++y ) {
            for (int x = 0; x <= w; ++x) {
                CompressedPaintersOrder v = *(p + x);
                if (v > max) {
                    max = v;
                }
            }
            p = p + fGridWidth;
        }

        return max;
    }

    void recordDraw(const Rect& bounds, CompressedPaintersOrder order) override {
        SkASSERT(!bounds.isEmptyNegativeOrNaN());

        auto ltrb = this->getGridCoords(bounds);
        CompressedPaintersOrder* p = fNodes.data() + ltrb[1] * fGridWidth + ltrb[0];
        int h = ltrb[3] - ltrb[1];
        int w = ltrb[2] - ltrb[0];

        for (int y = 0; y <= h; ++y) {
            for (int x = 0; x <= w; ++x) {
                CompressedPaintersOrder v = *(p + x);
                if (order > v) {
                    *(p + x) = order;
                }
            }
            p = p + fGridWidth;
        }
    }

    void reset() override {
        memset(fNodes.data(), 0, sizeof(CompressedPaintersOrder) * fGridWidth * fGridHeight);
    }

private:
    GridBoundsManager(const SkISize& deviceSize, const SkISize& gridSize)
            : fScaleX(gridSize.width() / (float) deviceSize.width())
            , fScaleY(gridSize.height() / (float) deviceSize.height())
            , fGridWidth(gridSize.width())
            , fGridHeight(gridSize.height())
            , fNodes((size_t) fGridWidth * fGridHeight) {
        // Reset is needed to zero-out the uninitialized fNodes values.
        this->reset();
    }

    skvx::int4 getGridCoords(const Rect& bounds) const {
        // Normalize bounds by 1/wh of device bounds, then scale up to number of cells per side.
        // fScaleXY includes both 1/wh and the grid dimension scaling, then clamp to [0, gridDim-1].
        return pin(skvx::cast<int32_t>(bounds.ltrb() * skvx::float2(fScaleX, fScaleY).xyxy()),
                   skvx::int4(0),
                   skvx::int2(fGridWidth, fGridHeight).xyxy() - 1);
    }

    const float fScaleX;
    const float fScaleY;

    const int   fGridWidth;
    const int   fGridHeight;

    SkAutoTMalloc<CompressedPaintersOrder> fNodes;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_BoundsManager_DEFINED
