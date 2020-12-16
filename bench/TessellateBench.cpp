/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/mock/GrMockOpTarget.h"
#include "src/gpu/tessellate/GrMiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/GrPathTessellateOp.h"
#include "src/gpu/tessellate/GrResolveLevelCounter.h"
#include "src/gpu/tessellate/GrStrokeIndirectOp.h"
#include "src/gpu/tessellate/GrStrokeTessellateOp.h"
#include "src/gpu/tessellate/GrWangsFormula.h"
#include "tools/ToolUtils.h"
#include <vector>

// This is the number of cubics in desk_chalkboard.skp. (There are no quadratics in the chalkboard.)
constexpr static int kNumCubicsInChalkboard = 47182;

static sk_sp<GrDirectContext> make_mock_context() {
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
    ctxOptions.fSuppressTessellationShaders = false;

    return GrDirectContext::MakeMock(&mockOptions, ctxOptions);
}

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
    void onDelayedSetup() override {
        fTarget = std::make_unique<GrMockOpTarget>(make_mock_context());
    }

    void onDraw(int loops, SkCanvas*) final {
        if (!fTarget->mockContext()) {
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
            this->runBench(fTarget.get(), &fOp);
            fTarget->resetAllocator();
        }
    }

    virtual void runBench(GrMeshDrawOp::Target*, GrPathTessellateOp*) = 0;

    GrPathTessellateOp fOp;
    std::unique_ptr<GrMockOpTarget> fTarget;
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

class GrStrokeTessellateOp::TestingOnly_Benchmark : public Benchmark {
public:
    TestingOnly_Benchmark(float matrixScale, const char* suffix) : fMatrixScale(matrixScale) {
        fName.printf("tessellate_GrStrokeTessellateOp_prepare%s", suffix);
    }

private:
    const char* onGetName() override { return fName.c_str(); }
    bool isSuitableFor(Backend backend) final { return backend == kNonRendering_Backend; }

    void onDelayedSetup() override {
        fTarget = std::make_unique<GrMockOpTarget>(make_mock_context());
        fPath.reset().moveTo(0, 0);
        for (int i = 0; i < kNumCubicsInChalkboard/2; ++i) {
            fPath.cubicTo(100, 0, 50, 100, 100, 100);
            fPath.cubicTo(0, -100, 200, 100, 0, 0);
        }
        fStrokeRec.setStrokeStyle(8);
        fStrokeRec.setStrokeParams(SkPaint::kButt_Cap, SkPaint::kMiter_Join, 4);
    }

    void onDraw(int loops, SkCanvas*) final {
        if (!fTarget->mockContext()) {
            SkDebugf("ERROR: could not create mock context.");
            return;
        }
        for (int i = 0; i < loops; ++i) {
            GrStrokeTessellateOp op(GrAAType::kMSAA, SkMatrix::Scale(fMatrixScale, fMatrixScale),
                                    fStrokeRec, fPath, GrPaint());
            op.fTarget = fTarget.get();
            op.prepareBuffers();
        }
    }

    const float fMatrixScale;
    SkString fName;
    std::unique_ptr<GrMockOpTarget> fTarget;
    SkPath fPath;
    SkStrokeRec fStrokeRec = SkStrokeRec(SkStrokeRec::kFill_InitStyle);
};

DEF_BENCH( return new GrStrokeTessellateOp::TestingOnly_Benchmark(1, ""); )
DEF_BENCH( return new GrStrokeTessellateOp::TestingOnly_Benchmark(5, "_one_chop"); )

class GrStrokeIndirectOp::Benchmark : public ::Benchmark {
public:
    Benchmark(const char* nameSuffix, SkPaint::Join join, std::vector<SkPoint> pts)
            : fJoin(join), fPts(std::move(pts)) {
        fName.printf("tessellate_GrStrokeIndirectOpBench%s", nameSuffix);
    }

private:
    const char* onGetName() override { return fName.c_str(); }
    bool isSuitableFor(Backend backend) final { return backend == kNonRendering_Backend; }

    void onDelayedSetup() override {
        fTarget = std::make_unique<GrMockOpTarget>(make_mock_context());
        if (fJoin == SkPaint::kRound_Join) {
            fPath.reset().moveTo(fPts.back());
            for (size_t i = 0; i < kNumCubicsInChalkboard/fPts.size(); ++i) {
                for (size_t j = 0; j < fPts.size(); ++j) {
                    fPath.lineTo(fPts[j]);
                }
            }
        } else {
            fPath.reset().moveTo(fPts[0]);
            for (int i = 0; i < kNumCubicsInChalkboard/2; ++i) {
                if (fPts.size() == 4) {
                    fPath.cubicTo(fPts[1], fPts[2], fPts[3]);
                    fPath.cubicTo(fPts[2], fPts[1], fPts[0]);
                } else {
                    SkASSERT(fPts.size() == 3);
                    fPath.quadTo(fPts[1], fPts[2]);
                    fPath.quadTo(fPts[2], fPts[1]);
                }
            }
        }
        fStrokeRec.setStrokeStyle(8);
        fStrokeRec.setStrokeParams(SkPaint::kButt_Cap, fJoin, 4);
    }

    void onDraw(int loops, SkCanvas*) final {
        if (!fTarget->mockContext()) {
            SkDebugf("ERROR: could not create mock context.");
            return;
        }
        for (int i = 0; i < loops; ++i) {
            GrStrokeIndirectOp op(GrAAType::kMSAA, SkMatrix::I(), fPath, fStrokeRec, GrPaint());
            op.prePrepareResolveLevels(fTarget->allocator());
            op.prepareBuffers(fTarget.get());
        }
    }

    SkString fName;
    SkPaint::Join fJoin;
    std::vector<SkPoint> fPts;
    std::unique_ptr<GrMockOpTarget> fTarget;
    SkPath fPath;
    SkStrokeRec fStrokeRec = SkStrokeRec(SkStrokeRec::kFill_InitStyle);
};

DEF_BENCH( return new GrStrokeIndirectOp::Benchmark(
        "_inflect1", SkPaint::kBevel_Join, {{0,0}, {100,0}, {0,100}, {100,100}}); )

DEF_BENCH( return new GrStrokeIndirectOp::Benchmark(
        "_inflect2", SkPaint::kBevel_Join, {{37,162}, {412,160}, {249,65}, {112,360}}); )

DEF_BENCH( return new GrStrokeIndirectOp::Benchmark(
        "_loop", SkPaint::kBevel_Join, {{0,0}, {100,0}, {0,100}, {0,0}}); )

DEF_BENCH( return new GrStrokeIndirectOp::Benchmark(
        "_nochop", SkPaint::kBevel_Join, {{0,0}, {50,0}, {100,50}, {100,100}}); )

DEF_BENCH( return new GrStrokeIndirectOp::Benchmark(
        "_quad", SkPaint::kBevel_Join, {{0,0}, {50,100}, {100,0}}); )

DEF_BENCH( return new GrStrokeIndirectOp::Benchmark(
        "_roundjoin", SkPaint::kRound_Join, {{0,0}, {50,100}, {100,0}}); )
