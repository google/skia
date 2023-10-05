/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "bench/Benchmark.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "src/base/SkMathPriv.h"
#include "src/base/SkRandom.h"
#include "src/gpu/graphite/geom/IntersectionTree.h"
#include "tools/ToolUtils.h"
#include "tools/flags/CommandLineFlags.h"

#if defined(SK_ENABLE_SVG)
#include "tools/SvgPathExtractor.h"
#endif

using namespace skia_private;

static DEFINE_string(intersectionTreeFile, "",
                     "svg or skp for the IntersectionTree bench to sniff paths from.");

namespace skgpu::graphite {

class IntersectionTreeBench : public Benchmark {
protected:
    const char* onGetName() final { return fName.c_str(); }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    void onDelayedSetup() final {
        TArray<SkRect> rects;
        this->gatherRects(&rects);
        fRectCount = rects.size();
        fRects = fAlignedAllocator.makeArray<Rect>(fRectCount);
        for (int i = 0; i < fRectCount; ++i) {
            fRects[i] = rects[i];
        }
        fRectBufferA = fAlignedAllocator.makeArray<Rect>(fRectCount);
        fRectBufferB = fAlignedAllocator.makeArray<Rect>(fRectCount);
    }

    virtual void gatherRects(TArray<SkRect>* rects) = 0;

    void onDraw(int loops, SkCanvas*) final {
        for (int i = 0; i < loops; ++i) {
            this->doBench();
        }
    }

    void doBench() {
        Rect* rects = fRects;
        Rect* collided = fRectBufferA;
        int rectCount = fRectCount;
        fNumTrees = 0;
        while (rectCount > 0) {
            IntersectionTree intersectionTree;
            int collidedCount = 0;
            for (int i = 0; i < rectCount; ++i) {
                if (!intersectionTree.add(rects[i])) {
                    collided[collidedCount++] = rects[i];
                }
            }
            std::swap(rects, collided);
            if (collided == fRects) {
                collided = fRectBufferB;
            }
            rectCount = collidedCount;
            ++fNumTrees;
        }
    }

    SkString fName;
    SkArenaAlloc fAlignedAllocator{0};
    int fRectCount;
    Rect* fRects;
    Rect* fRectBufferA;
    Rect* fRectBufferB;
    int fNumTrees = 0;
};

class RandomIntersectionBench : public IntersectionTreeBench {
public:
    RandomIntersectionBench(int numRandomRects) : fNumRandomRects(numRandomRects) {
        fName.printf("IntersectionTree_%i", numRandomRects);
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

    const int fNumRandomRects;
};

class FileIntersectionBench : public IntersectionTreeBench {
public:
    FileIntersectionBench() {
        if (FLAGS_intersectionTreeFile.isEmpty()) {
            return;
        }
        const char* filename = strrchr(FLAGS_intersectionTreeFile[0], '/');
        if (filename) {
            ++filename;
        } else {
            filename = FLAGS_intersectionTreeFile[0];
        }
        fName.printf("IntersectionTree_file_%s", filename);
    }

private:
    bool isSuitableFor(Backend backend) final {
        if (FLAGS_intersectionTreeFile.isEmpty()) {
            return false;
        }
        return IntersectionTreeBench::isSuitableFor(backend);
    }

    void gatherRects(TArray<SkRect>* rects) override {
        if (FLAGS_intersectionTreeFile.isEmpty()) {
            return;
        }
        auto callback = [&](const SkMatrix& matrix,
                            const SkPath& path,
                            const SkPaint& paint) {
            if (paint.getStyle() == SkPaint::kStroke_Style) {
                return;  // Goes to stroker.
            }
            if (path.isConvex()) {
                return;  // Goes to convex renderer.
            }
            int numVerbs = path.countVerbs();
            SkRect drawBounds = matrix.mapRect(path.getBounds());
            float gpuFragmentWork = drawBounds.height() * drawBounds.width();
            float cpuTessellationWork = numVerbs * SkNextLog2(numVerbs);  // N log N.
            constexpr static float kCpuWeight = 512;
            constexpr static float kMinNumPixelsToTriangulate = 256 * 256;
            if (cpuTessellationWork * kCpuWeight + kMinNumPixelsToTriangulate < gpuFragmentWork) {
                return;  // Goes to inner triangulator.
            }
            rects->push_back(drawBounds);
        };
        const char* path = FLAGS_intersectionTreeFile[0];
        if (const char* ext = strrchr(path, '.'); ext && !strcmp(ext, ".svg")) {
#if defined(SK_ENABLE_SVG)
            ToolUtils::ExtractPathsFromSVG(path, callback);
#else
            SK_ABORT("must compile with svg backend to process svgs");
#endif
        } else {
            ToolUtils::ExtractPathsFromSKP(path, callback);
        }
        SkDebugf(">> Found %i stencil/cover paths in %s <<\n",
                 rects->size(), FLAGS_intersectionTreeFile[0]);
    }

    void onPerCanvasPostDraw(SkCanvas*) override {
        if (FLAGS_intersectionTreeFile.isEmpty()) {
            return;
        }
        SkDebugf(">> Reordered %s into %i different stencil/cover draws <<\n",
                 FLAGS_intersectionTreeFile[0], fNumTrees);
    }
};

}  // namespace skgpu::graphite

DEF_BENCH( return new skgpu::graphite::RandomIntersectionBench(100); )
DEF_BENCH( return new skgpu::graphite::RandomIntersectionBench(500); )
DEF_BENCH( return new skgpu::graphite::RandomIntersectionBench(1000); )
DEF_BENCH( return new skgpu::graphite::RandomIntersectionBench(5000); )
DEF_BENCH( return new skgpu::graphite::RandomIntersectionBench(10000); )
DEF_BENCH( return new skgpu::graphite::FileIntersectionBench(); )  // Sniffs --intersectionTreeFile
