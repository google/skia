/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/gpu/GrContext.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/tessellate/GrTessellatePathOp.h"
#include "src/gpu/tessellate/GrWangsFormula.h"
#include "tools/ToolUtils.h"

// This is the number of cubics in desk_chalkboard.skp. (There are no quadratics in the chalkboard.)
constexpr static int kNumCubicsInChalkboard = 47182;

static SkPath make_cubic_path() {
    SkRandom rand;
    SkPath path;
    for (int i = 0; i < kNumCubicsInChalkboard/2; ++i) {
                float x = std::ldexp(rand.nextF(), (i % 18)) / 1e3f;
        path.cubicTo(111.625f*x, 308.188f*x, 764.62f*x, -435.688f*x, 742.63f*x, 85.187f*x);
        path.cubicTo(764.62f*x, -435.688f*x, 111.625f*x, 308.188f*x, 0, 0);
    }
    return path;
}

static sk_sp<GrContext> make_tessellation_mock_context() {
    GrMockOptions mockOptions;
    mockOptions.fDrawInstancedSupport = true;
    mockOptions.fTessellationSupport = true;
    mockOptions.fMapBufferFlags = GrCaps::kCanMap_MapFlag;
    return GrContext::MakeMock(&mockOptions);
}

// This serves as a base class for benchmarking individual methods on GrTessellatePathOp.
class GrTessellatePathOp::TestingOnly_Benchmark : public Benchmark {
public:
    TestingOnly_Benchmark(const char* subName, SkPath path, const SkMatrix& m)
            : fCtx(make_tessellation_mock_context())
            , fOp(m, path, GrPaint(), GrAAType::kMSAA)
            , fFlushState(fCtx->priv().getGpu(), fCtx->priv().resourceProvider(), nullptr,
                          GrBufferAllocPool::CpuBufferCache::Make(6)) {
        fName.printf("tessellate_%s", subName);
    }

    const char* onGetName() override { return fName.c_str(); }
    bool isSuitableFor(Backend backend) final { return backend == kNonRendering_Backend; }

    class MiddleOutInnerTrianglesBench;
    class OuterCubicsBench;
    class CubicWedgesBench;
    class WangsFormulaBench;

private:
    void onDraw(int loops, SkCanvas*) final {
        for (int i = 0; i < loops; ++i) {
            fOp.fTriangleBuffer.reset();
            fOp.fDoStencilTriangleBuffer = false;
            fOp.fDoFillTriangleBuffer = false;
            fOp.fCubicBuffer.reset();
            fOp.fStencilCubicsShader = nullptr;
            this->runBench(&fFlushState, &fOp);
        }
    }

    virtual void runBench(GrOpFlushState*, GrTessellatePathOp*) = 0;

    sk_sp<GrContext> fCtx;
    GrTessellatePathOp fOp;
    GrOpFlushState fFlushState;
    SkString fName;
};

class GrTessellatePathOp::TestingOnly_Benchmark::MiddleOutInnerTrianglesBench
        : public GrTessellatePathOp::TestingOnly_Benchmark {
public:
    MiddleOutInnerTrianglesBench()
            : TestingOnly_Benchmark("prepareMiddleOutInnerTriangles",
                                    ToolUtils::make_star(SkRect::MakeWH(100, 100),
                                                         kNumCubicsInChalkboard),
                                    SkMatrix::I()) {
    }
    void runBench(GrOpFlushState* flushState, GrTessellatePathOp* op) override {
        int numBeziers;
        op->prepareMiddleOutInnerTriangles(flushState, &numBeziers);
    }
};

DEF_BENCH( return new GrTessellatePathOp::TestingOnly_Benchmark::MiddleOutInnerTrianglesBench(); );

class GrTessellatePathOp::TestingOnly_Benchmark::OuterCubicsBench
        : public GrTessellatePathOp::TestingOnly_Benchmark {
public:
    OuterCubicsBench()
            : TestingOnly_Benchmark("prepareOuterCubics", make_cubic_path(), SkMatrix::I()) {
    }
    void runBench(GrOpFlushState* flushState, GrTessellatePathOp* op) override {
        op->prepareOuterCubics(flushState, kNumCubicsInChalkboard,
                               CubicDataAlignment::kVertexBoundary);
    }
};

DEF_BENCH( return new GrTessellatePathOp::TestingOnly_Benchmark::OuterCubicsBench(); );

class GrTessellatePathOp::TestingOnly_Benchmark::CubicWedgesBench
        : public GrTessellatePathOp::TestingOnly_Benchmark {
public:
    CubicWedgesBench()
            : TestingOnly_Benchmark("prepareCubicWedges", make_cubic_path(), SkMatrix::I()) {
    }
    void runBench(GrOpFlushState* flushState, GrTessellatePathOp* op) override {
        op->prepareCubicWedges(flushState);
    }
};

DEF_BENCH( return new GrTessellatePathOp::TestingOnly_Benchmark::CubicWedgesBench(););

class GrTessellatePathOp::TestingOnly_Benchmark::WangsFormulaBench
        : public GrTessellatePathOp::TestingOnly_Benchmark {
public:
    WangsFormulaBench(const char* suffix, const SkMatrix& matrix)
            : TestingOnly_Benchmark(SkStringPrintf("wangs_formula_cubic_log2%s", suffix).c_str(),
                                    make_cubic_path(), SkMatrix::I())
            , fMatrix(matrix) {
    }
    void runBench(GrOpFlushState* flushState, GrTessellatePathOp* op) override {
        int sum = 0;
        GrVectorXform xform(fMatrix);
        for (auto [verb, pts, w] : SkPathPriv::Iterate(op->fPath)) {
            if (verb == SkPathVerb::kCubic) {
                sum += GrWangsFormula::cubic_log2(4, pts, xform);
            }
        }
        // Don't let the compiler optimize away GrWangsFormula::cubic_log2.
        if (sum <= 0) {
            SK_ABORT("sum should be > 0.");
        }
    }
private:
    SkMatrix fMatrix;
};

DEF_BENCH(
    return new GrTessellatePathOp::TestingOnly_Benchmark::WangsFormulaBench("", SkMatrix::I());
);
DEF_BENCH(
    return new GrTessellatePathOp::TestingOnly_Benchmark::WangsFormulaBench(
            "_scale", SkMatrix::MakeScale(1.1f, 0.9f));
);
DEF_BENCH(
    return new GrTessellatePathOp::TestingOnly_Benchmark::WangsFormulaBench(
            "_affine", SkMatrix::MakeAll(.9f,0.9f,0,  1.1f,1.1f,0, 0,0,1));
);
