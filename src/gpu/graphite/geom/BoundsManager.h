/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_BoundsManager_DEFINED
#define skgpu_graphite_geom_BoundsManager_DEFINED

#include "include/core/SkSize.h"
#include "include/private/base/SkTemplates.h"

#include "src/base/SkTBlockList.h"
#include "src/base/SkVx.h"
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
            if (r.intersects(boundsComplement) && max < *orderIter) {
                max = *orderIter;
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

    int count() const { return fRects.count(); }

    void replayDraws(BoundsManager* manager) const {
        auto orderIter = fOrders.items().begin();
        for (const Rect& r : fRects.items()) {
            manager->recordDraw(r, *orderIter);
            ++orderIter;
        }
    }

private:
    // fRects and fOrders are parallel, but kept separate to avoid wasting padding since Rect is
    // an over-aligned type.
    SkTBlockList<Rect, 16> fRects{SkBlockAllocator::GrowthPolicy::kFibonacci};
    SkTBlockList<CompressedPaintersOrder, 16> fOrders{SkBlockAllocator::GrowthPolicy::kFibonacci};
};

// A BoundsManager that tracks highest CompressedPaintersOrder over a uniform spatial grid.
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

    static std::unique_ptr<GridBoundsManager> MakeRes(SkISize deviceSize,
                                                      int gridCellSize,
                                                      int maxGridSize=0) {
        SkASSERT(deviceSize.width() > 0 && deviceSize.height() > 0);
        SkASSERT(gridCellSize >= 1);

        int gridWidth = SkScalarCeilToInt(deviceSize.width() / (float) gridCellSize);
        if (maxGridSize > 0 && gridWidth > maxGridSize) {
            // We'd have too many sizes so clamp the grid resolution, leave the device size alone
            // since the grid cell size can't be preserved anyways.
            gridWidth = maxGridSize;
        } else {
             // Pad out the device size to keep cell size the same
            deviceSize.fWidth = gridWidth * gridCellSize;
        }

        int gridHeight = SkScalarCeilToInt(deviceSize.height() / (float) gridCellSize);
        if (maxGridSize > 0 && gridHeight > maxGridSize) {
            gridHeight = maxGridSize;
        } else {
            deviceSize.fHeight = gridHeight * gridCellSize;
        }
        return Make(deviceSize, {gridWidth, gridHeight});
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

    skia_private::AutoTMalloc<CompressedPaintersOrder> fNodes;
};

// A BoundsManager that first relies on BruteForceBoundsManager for N draw calls, and then switches
// to the GridBoundsManager if it exceeds its limit. For low N, the brute force approach is
// surprisingly efficient, has the highest accuracy, and very low memory overhead. Once the draw
// call count is large enough, the grid's lower performance complexity outweigh its memory cost and
// reduced accuracy.
class HybridBoundsManager : public BoundsManager {
public:
    HybridBoundsManager(const SkISize& deviceSize,
                        int gridCellSize,
                        int maxBruteForceN,
                        int maxGridSize=0)
            : fDeviceSize(deviceSize)
            , fGridCellSize(gridCellSize)
            , fMaxBruteForceN(maxBruteForceN)
            , fMaxGridSize(maxGridSize)
            , fCurrentManager(&fBruteForceManager) {
        SkASSERT(deviceSize.width() >= 1 && deviceSize.height() >= 1 &&
                 gridCellSize >= 1 && maxBruteForceN >= 1);
    }

    CompressedPaintersOrder getMostRecentDraw(const Rect& bounds) const override {
        return fCurrentManager->getMostRecentDraw(bounds);
    }

    void recordDraw(const Rect& bounds, CompressedPaintersOrder order) override {
        this->updateCurrentManagerIfNeeded();
        fCurrentManager->recordDraw(bounds, order);
    }

    void reset() override {
        const bool usedGrid = fCurrentManager == fGridManager.get();
        if (usedGrid) {
            // Reset the grid manager so it's ready to use next frame, but don't delete it.
            fGridManager->reset();
            // Assume brute force manager was reset when we swapped to the grid originally
            fCurrentManager = &fBruteForceManager;
        } else {
            if (fGridManager) {
                // Clean up the grid manager that was created over a frame ago without being used.
                // This could lead to re-allocating the grid every-other frame, but it's a simple
                // way to ensure we don't hold onto the grid in perpetuity if it's not needed.
                fGridManager = nullptr;
            }
            fBruteForceManager.reset();
            SkASSERT(fCurrentManager == &fBruteForceManager);
        }
    }

private:
    const SkISize fDeviceSize;
    const int     fGridCellSize;
    const int     fMaxBruteForceN;
    const int     fMaxGridSize;

    BoundsManager* fCurrentManager;

    BruteForceBoundsManager                  fBruteForceManager;

    // The grid manager starts out null and is created the first time we exceed fMaxBruteForceN.
    // However, even if we reset back to the brute force manager, we keep the grid around under the
    // assumption that the owning Device will have similar frame-to-frame draw counts and will need
    // to upgrade to the grid manager again.
    std::unique_ptr<GridBoundsManager>       fGridManager;

    void updateCurrentManagerIfNeeded() {
        if (fCurrentManager == fGridManager.get() ||
            fBruteForceManager.count() < fMaxBruteForceN) {
            // Already using the grid or the about-to-be-recorded draw will not cause us to exceed
            // the brute force limit, so no need to change the current manager implementation.
            return;
        }
        // Else we need to switch from the brute force manager to the grid manager
        if (!fGridManager) {
            fGridManager = GridBoundsManager::MakeRes(fDeviceSize, fGridCellSize, fMaxGridSize);
        }
        fCurrentManager = fGridManager.get();

        // Fill out the grid manager with the recorded draws in the brute force manager
        fBruteForceManager.replayDraws(fCurrentManager);
        fBruteForceManager.reset();
    }
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_BoundsManager_DEFINED
