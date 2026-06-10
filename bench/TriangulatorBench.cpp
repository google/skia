/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "bench/Tiger.h"
#include "include/core/SkPath.h"
#include "src/core/SkArenaAlloc.h"
#include "src/gpu/ganesh/GrEagerVertexAllocator.h"
#include "src/gpu/ganesh/geometry/GrInnerFanTriangulator.h"
#include "src/gpu/ganesh/geometry/GrTriangulator.h"
#include <vector>

using namespace skia_private;

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)

constexpr float kTigerTolerance = 0.728769f;

class TriangulatorBenchmark : public Benchmark, public GrEagerVertexAllocator {
public:
    TriangulatorBenchmark(const char* name) {
        fName.printf("triangulator_%s", name);
    }

    const char* onGetName() override { return fName.c_str(); }
    bool isSuitableFor(Backend backend) final { return backend == Backend::kNonRendering; }

protected:
    void onDelayedSetup() override {
        fPaths = Tiger::GetTigerPaths();
    }

    void onDraw(int loops, SkCanvas*) final {
        for (int i = 0; i < loops; ++i) {
            this->doLoop();
        }
    }

    // GrEagerVertexAllocator.
    void* lock(size_t stride, int eagerCount) override {
        size_t allocSize = eagerCount * stride;
        if (allocSize > fVertexAllocSize) {
            fVertexData.reset(allocSize);
        }
        return fVertexData;
    }

    void unlock(int) override {}

    virtual void doLoop() = 0;

    SkString fName;
    std::vector<SkPath> fPaths;
    AutoTMalloc<char> fVertexData;
    size_t fVertexAllocSize = 0;
    SkArenaAllocWithReset fArena{GrTriangulator::kArenaDefaultChunkSize};
};

class PathToTrianglesBench : public TriangulatorBenchmark {
public:
    PathToTrianglesBench() : TriangulatorBenchmark("PathToTriangles") {}

    void doLoop() override {
        for (const SkPath& path : fPaths) {
            bool isLinear;
            GrTriangulator::PathToTriangles(path, kTigerTolerance, SkRect::MakeEmpty(), this,
                                            &isLinear);
        }
    }
};

DEF_BENCH( return new PathToTrianglesBench(); )

class TriangulateInnerFanBench : public TriangulatorBenchmark {
public:
    TriangulateInnerFanBench() : TriangulatorBenchmark("TriangulateInnerFan") {}

    void doLoop() override {
        bool isLinear;
        for (const SkPath& path : fPaths) {
            GrInnerFanTriangulator::BreadcrumbTriangleList breadcrumbList;
            GrInnerFanTriangulator(path, &fArena).pathToTriangles(this, &breadcrumbList, &isLinear);
        }
        fArena.reset();
    }
};

DEF_BENCH( return new TriangulateInnerFanBench(); )

#if 0
#include "src/gpu/tessellate/GrMiddleOutPolygonTriangulator.h"

class MiddleOutComparisonBench : public TriangulatorBenchmark {
public:
    MiddleOutComparisonBench() : TriangulatorBenchmark("MiddleOutComparison") {}
    void doLoop() override {
        for (const SkPath& path : fPaths) {
            int maxInnerTriangles = path.countVerbs() - 1;
            auto* data = this->GrEagerVertexAllocator::lock<SkPoint>(maxInnerTriangles * 3);
            int vertexCount = GrMiddleOutPolygonTriangulator::WritePathInnerFan(
                    data, 3/*perTriangleVertexAdvance*/, path) * 3;
            this->unlock(vertexCount);
        }
    }
};

DEF_BENCH( return new MiddleOutComparisonBench(); );
#endif

#endif // SK_ENABLE_OPTIMIZE_SIZE
