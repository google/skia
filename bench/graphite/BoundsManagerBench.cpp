/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "bench/Benchmark.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkRandom.h"
#include "src/gpu/graphite/geom/BoundsManager.h"
#include "tools/ToolUtils.h"
#include "tools/flags/CommandLineFlags.h"

#if defined(SK_ENABLE_SVG)
#include "tools/SvgPathExtractor.h"
#endif

using namespace skia_private;

static DEFINE_string(boundsManagerFile, "",
                     "svg or skp for the BoundsManager bench to sniff paths from.");

#define PRINT_DRAWSET_COUNT 0 // set to 1 to display number of CompressedPaintersOrder groups

namespace skgpu::graphite {

class BoundsManagerBench : public Benchmark {
public:
    BoundsManagerBench(std::unique_ptr<BoundsManager> manager) : fManager(std::move(manager)) {}

protected:
    virtual void gatherRects(TArray<SkRect>* rects) = 0;

    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

    const char* onGetName() final { return fName.c_str(); }

    void onDelayedSetup() final {
        TArray<SkRect> rects;
        this->gatherRects(&rects);

        fRectCount = rects.size();
        fRects = fAlignedAllocator.makeArray<Rect>(fRectCount);
        for (int i = 0; i < fRectCount; ++i) {
            fRects[i] = rects[i];
        }
    }

    void onDraw(int loops, SkCanvas*) final {
        for (int i = 0; i < loops; ++i) {
            this->doBench();
        }
    }

    void onPerCanvasPostDraw(SkCanvas*) override {
#if PRINT_DRAWSET_COUNT
        SkDebugf("%s >> grouped %d draws into %d sets <<\n",
                 fName.c_str(), fRectCount, fMaxRead.bits());
#endif
    }

    void doBench() {
        CompressedPaintersOrder maxRead = CompressedPaintersOrder::First();
        for (int i = 0; i < fRectCount; ++i) {
            const Rect& drawBounds = fRects[i];
            CompressedPaintersOrder order = fManager->getMostRecentDraw(drawBounds).next();
            fManager->recordDraw(drawBounds, order);
            if (order > maxRead) {
                maxRead = order;
            }
        }

        fMaxRead = maxRead;
        fManager->reset();
    }

    std::unique_ptr<BoundsManager> fManager;
    SkString fName;
    SkArenaAlloc fAlignedAllocator{0};
    int fRectCount;
    Rect* fRects;

    CompressedPaintersOrder fMaxRead;
};

class RandomBoundsManagerBench : public BoundsManagerBench {
public:
    RandomBoundsManagerBench(std::unique_ptr<BoundsManager> manager,
                             const char* managerName,
                             int numRandomRects)
            : BoundsManagerBench(std::move(manager))
            , fNumRandomRects(numRandomRects) {
        fName.printf("BoundsManager_rand_%i_%s", numRandomRects, managerName);
    }

private:
    void gatherRects(TArray<SkRect>* rects) override {
        SkRandom rand;
        for (int i = 0; i < fNumRandomRects; ++i) {
            rects->push_back(SkRect::MakeXYWH(rand.nextRangeF(0, 2000),
                                              rand.nextRangeF(0, 2000),
                                              rand.nextRangeF(0, 70),
                                              rand.nextRangeF(0, 70)));
        }
    }

    int fNumRandomRects;
};

class FileBoundsManagerBench : public BoundsManagerBench {
public:
    FileBoundsManagerBench(std::unique_ptr<BoundsManager> manager,
                           const char* managerName)
            : BoundsManagerBench(std::move(manager)) {
        if (!FLAGS_boundsManagerFile.isEmpty()) {
            const char* filename = strrchr(FLAGS_boundsManagerFile[0], '/');
            if (filename) {
                ++filename;
            } else {
                filename = FLAGS_boundsManagerFile[0];
            }
            fName.printf("BoundsManager_file_%s_%s", filename, managerName);
        }
    }

private:
    bool isSuitableFor(Backend backend) final {
        if (FLAGS_boundsManagerFile.isEmpty()) {
            return false;
        }
        return BoundsManagerBench::isSuitableFor(backend);
    }

    void gatherRects(TArray<SkRect>* rects) override {
        if (FLAGS_boundsManagerFile.isEmpty()) {
            return;
        }
        SkRect fileBounds = SkRect::MakeEmpty();
        auto callback = [&](const SkMatrix& matrix,
                            const SkPath& path,
                            const SkPaint& paint) {
            if (!paint.canComputeFastBounds() || path.isInverseFillType()) {
                // These would pessimistically cover the entire canvas, but we don't have enough
                // info in the benchmark to handle that, so just skip these draws.
                return;
            }

            SkRect bounds = path.getBounds();
            SkRect drawBounds = matrix.mapRect(paint.computeFastBounds(bounds, &bounds));
            rects->push_back(drawBounds);

            fileBounds.join(drawBounds);
        };

        const char* path = FLAGS_boundsManagerFile[0];
        if (const char* ext = strrchr(path, '.'); ext && !strcmp(ext, ".svg")) {
#if defined(SK_ENABLE_SVG)
            ToolUtils::ExtractPathsFromSVG(path, callback);
#else
            SK_ABORT("must compile with svg backend to process svgs");
#endif
        } else {
            ToolUtils::ExtractPathsFromSKP(path, callback);
        }

#if PRINT_DRAWSET_COUNT
        SkDebugf("%s bounds are [%f %f %f %f]\n",
            FLAGS_boundsManagerFile[0],
            fileBounds.fLeft, fileBounds.fTop, fileBounds.fRight, fileBounds.fBottom);
#endif
    }

};

}  // namespace skgpu::graphite

#define DEF_BOUNDS_MANAGER_BENCH_SET(manager, name) \
    DEF_BENCH( return new skgpu::graphite::RandomBoundsManagerBench(manager, name, 100); ) \
    DEF_BENCH( return new skgpu::graphite::RandomBoundsManagerBench(manager, name, 500); ) \
    DEF_BENCH( return new skgpu::graphite::RandomBoundsManagerBench(manager, name, 1000); ) \
    DEF_BENCH( return new skgpu::graphite::RandomBoundsManagerBench(manager, name, 10000); ) \
    DEF_BENCH( return new skgpu::graphite::FileBoundsManagerBench(manager, name); )


DEF_BOUNDS_MANAGER_BENCH_SET(std::make_unique<skgpu::graphite::NaiveBoundsManager>(),      "naive")
DEF_BOUNDS_MANAGER_BENCH_SET(std::make_unique<skgpu::graphite::BruteForceBoundsManager>(), "brute")
DEF_BOUNDS_MANAGER_BENCH_SET(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 128), "grid128")
DEF_BOUNDS_MANAGER_BENCH_SET(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 512), "grid512")
DEF_BOUNDS_MANAGER_BENCH_SET(std::make_unique<skgpu::graphite::HybridBoundsManager>(SkISize{1800, 1800}, 16, 64), "hybrid16x16n128")
DEF_BOUNDS_MANAGER_BENCH_SET(std::make_unique<skgpu::graphite::HybridBoundsManager>(SkISize{1800, 1800}, 16, 128), "hybrid16x16n256")
// Uncomment and adjust device size to match reported bounds from --boundsManagerFile
// DEF_BOUNDS_MANAGER_BENCH_SET(skgpu::graphite::GridBoundsManager::MakeRes({w, h}, 8), "gridRes8")

#undef DEF_BOUNDS_MANAGER_BENCH_SET
