/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/tessellate/GrMiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/GrPathTessellateOp.h"
#include "src/gpu/tessellate/GrResolveLevelCounter.h"
#include "src/gpu/tessellate/GrStrokePatchBuilder.h"
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
    BenchmarkTarget() {
        GrMockOptions mockOptions;
        mockOptions.fDrawInstancedSupport = true;
        mockOptions.fMaxTessellationSegments = 64;
        mockOptions.fMapBufferFlags = GrCaps::kCanMap_MapFlag;
        mockOptions.fConfigOptions[(int)GrColorType::kAlpha_8].fRenderability =
                GrMockOptions::ConfigOptions::Renderability::kMSAA;
        mockOptions.fConfigOptions[(int)GrColorType::kAlpha_8].fTexturable = true;
        mockOptions.fIntegerSupport = true;

        GrContextOptions ctxOptions;
        ctxOptions.fGpuPathRenderers = GpuPathRenderers::kTessellation;

        fMockContext = GrDirectContext::MakeMock(&mockOptions, ctxOptions);
    }
    const GrDirectContext* mockContext() const { return fMockContext.get(); }
    const GrCaps& caps() const override { return *fMockContext->priv().caps(); }
    GrResourceProvider* resourceProvider() const override {
        return fMockContext->priv().resourceProvider();
    }
    GrSmallPathAtlasMgr* smallPathAtlasManager() const override { return nullptr; }
    void resetAllocator() { fAllocator.reset(); }
    SkArenaAlloc* allocator() override { return &fAllocator; }
    void putBackVertices(int vertices, size_t vertexStride) override { /* no-op */ }

    void* makeVertexSpace(size_t vertexSize, int vertexCount, sk_sp<const GrBuffer>*,
                          int* startVertex) override {
        if (vertexSize * vertexCount > sizeof(fStaticVertexData)) {
            SK_ABORT("FATAL: wanted %zu bytes of static vertex data; only have %zu.\n",
                     vertexSize * vertexCount, sizeof(fStaticVertexData));
        }
        *startVertex = 0;
        return fStaticVertexData;
    }

    void* makeVertexSpaceAtLeast(size_t vertexSize, int minVertexCount, int fallbackVertexCount,
                                 sk_sp<const GrBuffer>*, int* startVertex,
                                 int* actualVertexCount) override {
        if (vertexSize * minVertexCount > sizeof(fStaticVertexData)) {
            SK_ABORT("FATAL: wanted %zu bytes of static vertex data; only have %zu.\n",
                     vertexSize * minVertexCount, sizeof(fStaticVertexData));
        }
        *startVertex = 0;
        *actualVertexCount = sizeof(fStaticVertexData) / vertexSize;
        return fStaticVertexData;
    }

    GrDrawIndexedIndirectCommand* makeDrawIndexedIndirectSpace(
            int drawCount, sk_sp<const GrBuffer>* buffer, size_t* offsetInBytes) override {
        int staticBufferCount = (int)SK_ARRAY_COUNT(fStaticDrawIndexedIndirectData);
        if (drawCount > staticBufferCount) {
            SK_ABORT("FATAL: wanted %i static drawIndexedIndirect elements; only have %i.\n",
                     drawCount, staticBufferCount);
        }
        return fStaticDrawIndexedIndirectData;
    }

#define UNIMPL(...) __VA_ARGS__ override { SK_ABORT("unimplemented."); }
    UNIMPL(void recordDraw(const GrGeometryProcessor*, const GrSimpleMesh[], int,
                           const GrSurfaceProxy* const[], GrPrimitiveType))
    UNIMPL(uint16_t* makeIndexSpace(int, sk_sp<const GrBuffer>*, int*))
    UNIMPL(uint16_t* makeIndexSpaceAtLeast(int, int, sk_sp<const GrBuffer>*, int*, int*))
    UNIMPL(GrDrawIndirectCommand* makeDrawIndirectSpace(int, sk_sp<const GrBuffer>*, size_t*))
    UNIMPL(void putBackIndices(int))
    UNIMPL(GrRenderTargetProxy* proxy() const)
    UNIMPL(const GrSurfaceProxyView* writeView() const)
    UNIMPL(const GrAppliedClip* appliedClip() const)
    UNIMPL(GrAppliedClip detachAppliedClip())
    UNIMPL(const GrXferProcessor::DstProxyView& dstProxyView() const)
    UNIMPL(GrXferBarrierFlags renderPassBarriers() const)
    UNIMPL(GrStrikeCache* strikeCache() const)
    UNIMPL(GrAtlasManager* atlasManager() const)
    UNIMPL(SkTArray<GrSurfaceProxy*, true>* sampledProxyArray())
    UNIMPL(GrDeferredUploadTarget* deferredUploadTarget())
#undef UNIMPL

private:
    sk_sp<GrDirectContext> fMockContext;
    SkPoint fStaticVertexData[(kNumCubicsInChalkboard + 2) * 8];
    GrDrawIndexedIndirectCommand fStaticDrawIndexedIndirectData[32];
    SkSTArenaAllocWithReset<1024 * 1024> fAllocator;
};

// This serves as a base class for benchmarking individual methods on GrPathTessellateOp.
class GrPathTessellateOp::TestingOnly_Benchmark : public Benchmark {
public:
    TestingOnly_Benchmark(const char* subName, SkPath path, const SkMatrix& m)
            : fOp(m, path, GrPaint(), GrAAType::kMSAA, GrTessellationPathRenderer::OpFlags::kNone) {
        fName.printf("tessellate_%s", subName);
    }

    const char* onGetName() override { return fName.c_str(); }
    bool isSuitableFor(Backend backend) final { return backend == kNonRendering_Backend; }

    class prepareMiddleOutStencilGeometry;
    class prepareMiddleOutStencilGeometry_indirect;
    class prepareIndirectOuterCubics;
    class prepareTessellatedOuterCubics;
    class prepareTessellatedCubicWedges;
    class wangs_formula_cubic_log2;
    class wangs_formula_cubic_log2_scale;
    class wangs_formula_cubic_log2_affine;
    class middle_out_triangulation;

private:
    void onDraw(int loops, SkCanvas*) final {
        if (!fTarget.mockContext()) {
            SkDebugf("ERROR: could not create mock context.");
            return;
        }
        for (int i = 0; i < loops; ++i) {
            fOp.fTriangleBuffer.reset();
            fOp.fTriangleVertexCount = 0;
            fOp.fPipelineForStencils = nullptr;
            fOp.fPipelineForFills = nullptr;
            fOp.fStencilTrianglesProgram = nullptr;
            fOp.fFillTrianglesProgram = nullptr;
            fOp.fCubicBuffer.reset();
            fOp.fCubicVertexCount = 0;
            // Make fStencilCubicsProgram non-null to keep assertions happy.
            fOp.fStencilCubicsProgram = (GrProgramInfo*)-1;
            fOp.fFillPathProgram = nullptr;
            this->runBench(&fTarget, &fOp);
            fTarget.resetAllocator();
        }
    }

    virtual void runBench(GrMeshDrawOp::Target*, GrPathTessellateOp*) = 0;

    GrPathTessellateOp fOp;
    BenchmarkTarget fTarget;
    SkString fName;
};

#define DEF_PATH_TESS_BENCH(NAME, PATH, MATRIX, TARGET, OP) \
    class GrPathTessellateOp::TestingOnly_Benchmark::NAME \
            : public GrPathTessellateOp::TestingOnly_Benchmark { \
    public: \
        NAME() : TestingOnly_Benchmark(#NAME, (PATH), (MATRIX)) {} \
        void runBench(GrMeshDrawOp::Target* target, GrPathTessellateOp* op) override; \
    }; \
    DEF_BENCH( return new GrPathTessellateOp::TestingOnly_Benchmark::NAME(); ); \
    void GrPathTessellateOp::TestingOnly_Benchmark::NAME::runBench( \
            GrMeshDrawOp::Target* TARGET, GrPathTessellateOp* op)

DEF_PATH_TESS_BENCH(prepareMiddleOutStencilGeometry, make_cubic_path(), SkMatrix::I(), target, op) {
    // Make fStencilTrianglesProgram non-null so we benchmark the tessellation path with separate
    // triangles.
    op->fStencilTrianglesProgram = (GrProgramInfo*)-1;
    op->prepareMiddleOutTrianglesAndCubics(target);
}

DEF_PATH_TESS_BENCH(prepareMiddleOutStencilGeometry_indirect, make_cubic_path(), SkMatrix::I(),
                    target, op) {
    GrResolveLevelCounter resolveLevelCounter;
    op->prepareMiddleOutTrianglesAndCubics(target, &resolveLevelCounter);
}

DEF_PATH_TESS_BENCH(prepareIndirectOuterCubics, make_cubic_path(), SkMatrix::I(), target, op) {
    GrResolveLevelCounter resolveLevelCounter;
    resolveLevelCounter.reset(op->fPath, SkMatrix::I(), 4);
    op->prepareIndirectOuterCubics(target, resolveLevelCounter);
}

DEF_PATH_TESS_BENCH(prepareTessellatedOuterCubics, make_cubic_path(), SkMatrix::I(), target, op) {
    op->prepareTessellatedOuterCubics(target, kNumCubicsInChalkboard);
}

DEF_PATH_TESS_BENCH(prepareTessellatedCubicWedges, make_cubic_path(), SkMatrix::I(), target, op) {
    op->prepareTessellatedCubicWedges(target);
}

static void benchmark_wangs_formula_cubic_log2(const SkMatrix& matrix, const SkPath& path) {
    int sum = 0;
    GrVectorXform xform(matrix);
    for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
        if (verb == SkPathVerb::kCubic) {
            sum += GrWangsFormula::cubic_log2(4, pts, xform);
        }
    }
    // Don't let the compiler optimize away GrWangsFormula::cubic_log2.
    if (sum <= 0) {
        SK_ABORT("sum should be > 0.");
    }
}

DEF_PATH_TESS_BENCH(wangs_formula_cubic_log2, make_cubic_path(), SkMatrix::I(), target, op) {
    benchmark_wangs_formula_cubic_log2(op->fViewMatrix, op->fPath);
}

DEF_PATH_TESS_BENCH(wangs_formula_cubic_log2_scale, make_cubic_path(), SkMatrix::Scale(1.1f, 0.9f),
                    target, op) {
    benchmark_wangs_formula_cubic_log2(op->fViewMatrix, op->fPath);
}

DEF_PATH_TESS_BENCH(wangs_formula_cubic_log2_affine, make_cubic_path(),
                    SkMatrix::MakeAll(.9f,0.9f,0,  1.1f,1.1f,0, 0,0,1), target, op) {
    benchmark_wangs_formula_cubic_log2(op->fViewMatrix, op->fPath);
}

DEF_PATH_TESS_BENCH(middle_out_triangulation,
                    ToolUtils::make_star(SkRect::MakeWH(500, 500), kNumCubicsInChalkboard),
                    SkMatrix::I(), target, op) {
    int baseVertex;
    auto vertexData = static_cast<SkPoint*>(target->makeVertexSpace(
            sizeof(SkPoint), kNumCubicsInChalkboard, nullptr, &baseVertex));
    GrMiddleOutPolygonTriangulator middleOut(vertexData, 3, kNumCubicsInChalkboard + 2);
    for (auto [verb, pts, w] : SkPathPriv::Iterate(op->fPath)) {
        switch (verb) {
            case SkPathVerb::kMove:
                middleOut.closeAndMove(pts[0]);
                break;
            case SkPathVerb::kLine:
                middleOut.pushVertex(pts[1]);
                break;
            case SkPathVerb::kClose:
                middleOut.close();
                break;
            case SkPathVerb::kQuad:
            case SkPathVerb::kConic:
            case SkPathVerb::kCubic:
                SkUNREACHABLE;
        }
        middleOut.closeAndMove(pts[0]);
    }
}

class StrokePatchBuilderBench : public Benchmark {
    const char* onGetName() override { return "tessellate_StrokePatchBuilder"; }
    bool isSuitableFor(Backend backend) final { return backend == kNonRendering_Backend; }

    void onDelayedSetup() override {
        fPath.reset().moveTo(0, 0);
        for (int i = 0; i < kNumCubicsInChalkboard/2; ++i) {
            fPath.cubicTo(100, 0, 0, 100, 100, 100);
            fPath.cubicTo(100, 0, 0, 100, 0, 0);
        }
        fStrokeRec.setStrokeStyle(8);
        fStrokeRec.setStrokeParams(SkPaint::kButt_Cap, SkPaint::kMiter_Join, 4);
    }

    void onDraw(int loops, SkCanvas*) final {
        if (!fTarget.mockContext()) {
            SkDebugf("ERROR: could not create mock context.");
            return;
        }
        for (int i = 0; i < loops; ++i) {
            fPatchChunks.reset();
            GrStrokePatchBuilder builder(&fTarget, &fPatchChunks, 1, fStrokeRec,
                                         fPath.countVerbs());
            builder.addPath(fPath);
        }
    }

    BenchmarkTarget fTarget;
    SkPath fPath;
    SkStrokeRec fStrokeRec = SkStrokeRec(SkStrokeRec::kFill_InitStyle);
    SkSTArray<8, GrStrokePatchBuilder::PatchChunk> fPatchChunks;
};

DEF_BENCH( return new StrokePatchBuilderBench(); )
