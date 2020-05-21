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

// This is a dummy GrMeshDrawOp::Target implementation that just gives back pointers into
// pre-allocated CPU buffers, rather than allocating and mapping GPU buffers.
class BenchmarkTarget : public GrMeshDrawOp::Target {
public:
    void resetAllocator() { fAllocator.reset(); }
    SkArenaAlloc* allocator() override { return &fAllocator; }
    void putBackVertices(int vertices, size_t vertexStride) override { /* no-op */ }

    void* makeVertexSpace(size_t vertexSize, int vertexCount, sk_sp<const GrBuffer>*,
                          int* startVertex) override {
        if (vertexSize * vertexCount > sizeof(fStaticVertexData)) {
            SK_ABORT(SkStringPrintf(
                    "FATAL: wanted %zu bytes of static vertex data; only have %zu.\n",
                    vertexSize * vertexCount, SK_ARRAY_COUNT(fStaticVertexData)).c_str());
        }
        return fStaticVertexData;
    }

    GrDrawIndexedIndirectCommand* makeDrawIndexedIndirectSpace(
            int drawCount, sk_sp<const GrBuffer>* buffer, size_t* offsetInBytes) override {
        int staticBufferCount = (int)SK_ARRAY_COUNT(fStaticDrawIndexedIndirectData);
        if (drawCount > staticBufferCount) {
            SK_ABORT(SkStringPrintf(
                    "FATAL: wanted %i static drawIndexedIndirect elements; only have %i.\n",
                    drawCount, staticBufferCount).c_str());
        }
        return fStaticDrawIndexedIndirectData;
    }

#define UNIMPL(...) __VA_ARGS__ override { SK_ABORT("unimplemented."); }
    UNIMPL(void recordDraw(const GrGeometryProcessor*, const GrSimpleMesh[], int,
                           const GrSurfaceProxy* const[], GrPrimitiveType))
    UNIMPL(uint16_t* makeIndexSpace(int, sk_sp<const GrBuffer>*, int*))
    UNIMPL(void* makeVertexSpaceAtLeast(size_t, int, int, sk_sp<const GrBuffer>*, int*, int*))
    UNIMPL(uint16_t* makeIndexSpaceAtLeast(int, int, sk_sp<const GrBuffer>*, int*, int*))
    UNIMPL(GrDrawIndirectCommand* makeDrawIndirectSpace(int, sk_sp<const GrBuffer>*, size_t*))
    UNIMPL(void putBackIndices(int))
    UNIMPL(GrRenderTargetProxy* proxy() const)
    UNIMPL(const GrSurfaceProxyView* writeView() const)
    UNIMPL(const GrAppliedClip* appliedClip() const)
    UNIMPL(GrAppliedClip detachAppliedClip())
    UNIMPL(const GrXferProcessor::DstProxyView& dstProxyView() const)
    UNIMPL(GrResourceProvider* resourceProvider() const)
    UNIMPL(GrStrikeCache* strikeCache() const)
    UNIMPL(GrAtlasManager* atlasManager() const)
    UNIMPL(SkTArray<GrSurfaceProxy*, true>* sampledProxyArray())
    UNIMPL(const GrCaps& caps() const)
    UNIMPL(GrDeferredUploadTarget* deferredUploadTarget())
#undef UNIMPL

private:
    SkPoint fStaticVertexData[(kNumCubicsInChalkboard + 2) * 5];
    GrDrawIndexedIndirectCommand fStaticDrawIndexedIndirectData[32];
    SkSTArenaAlloc<1024 * 1024> fAllocator;
};

// This serves as a base class for benchmarking individual methods on GrTessellatePathOp.
class GrTessellatePathOp::TestingOnly_Benchmark : public Benchmark {
public:
    TestingOnly_Benchmark(const char* subName, SkPath path, const SkMatrix& m)
            : fOp(m, path, GrPaint(), GrAAType::kMSAA) {
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
            this->runBench(&fTarget, &fOp);
            fTarget.resetAllocator();
        }
    }

    virtual void runBench(GrMeshDrawOp::Target*, GrTessellatePathOp*) = 0;

    GrTessellatePathOp fOp;
    BenchmarkTarget fTarget;
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
    void runBench(GrMeshDrawOp::Target* target, GrTessellatePathOp* op) override {
        int numBeziers;
        op->prepareMiddleOutInnerTriangles(target, &numBeziers);
    }
};

DEF_BENCH( return new GrTessellatePathOp::TestingOnly_Benchmark::MiddleOutInnerTrianglesBench(); );

class GrTessellatePathOp::TestingOnly_Benchmark::OuterCubicsBench
        : public GrTessellatePathOp::TestingOnly_Benchmark {
public:
    OuterCubicsBench()
            : TestingOnly_Benchmark("prepareOuterCubics", make_cubic_path(), SkMatrix::I()) {
    }
    void runBench(GrMeshDrawOp::Target* target, GrTessellatePathOp* op) override {
        op->prepareOuterCubics(target, kNumCubicsInChalkboard,
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
    void runBench(GrMeshDrawOp::Target* target, GrTessellatePathOp* op) override {
        op->prepareCubicWedges(target);
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
    void runBench(GrMeshDrawOp::Target*, GrTessellatePathOp* op) override {
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
