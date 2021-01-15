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
class PathTessellateBenchmark : public Benchmark {
public:
    PathTessellateBenchmark(const char* subName, const SkPath& p, const SkMatrix& m)
            : fPath(p), fMatrix(m) {
        fName.printf("tessellate_%s", subName);
    }

    const char* onGetName() override { return fName.c_str(); }
    bool isSuitableFor(Backend backend) final { return backend == kNonRendering_Backend; }

protected:
    void onDelayedSetup() override {
        fTarget = std::make_unique<GrMockOpTarget>(make_mock_context());
    }

    void onDraw(int loops, SkCanvas*) final {
        if (!fTarget->mockContext()) {
            SkDebugf("ERROR: could not create mock context.");
            return;
        }
        for (int i = 0; i < loops; ++i) {
            this->runBench();
            fTarget->resetAllocator();
        }
    }

    virtual void runBench() = 0;

    SkString fName;
    std::unique_ptr<GrMockOpTarget> fTarget;
    const SkPath fPath;
    const SkMatrix& fMatrix;
};

#define DEF_PATH_TESS_BENCH(NAME, PATH, MATRIX) \
    class PathTessellateBenchmark_##NAME : public PathTessellateBenchmark { \
    public: \
        PathTessellateBenchmark_##NAME() : PathTessellateBenchmark(#NAME, (PATH), (MATRIX)) {} \
        void runBench() override; \
    }; \
    DEF_BENCH( return new PathTessellateBenchmark_##NAME(); ); \
    void PathTessellateBenchmark_##NAME::runBench()

DEF_PATH_TESS_BENCH(GrPathIndirectTessellator, make_cubic_path(), SkMatrix::I()) {
    GrPathIndirectTessellator tess(fMatrix, fPath, GrPathIndirectTessellator::DrawInnerFan::kNo);
    tess.prepare(fTarget.get(), fMatrix, fPath);
}

DEF_PATH_TESS_BENCH(GrPathOuterCurveTessellator, make_cubic_path(), SkMatrix::I()) {
    GrPathOuterCurveTessellator tess;
    tess.prepare(fTarget.get(), fMatrix, fPath);
}

DEF_PATH_TESS_BENCH(GrPathWedgeTessellator, make_cubic_path(), SkMatrix::I()) {
    GrPathWedgeTessellator tess;
    tess.prepare(fTarget.get(), fMatrix, fPath);
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

DEF_PATH_TESS_BENCH(wangs_formula_cubic_log2, make_cubic_path(), SkMatrix::I()) {
    benchmark_wangs_formula_cubic_log2(fMatrix, fPath);
}

DEF_PATH_TESS_BENCH(wangs_formula_cubic_log2_scale, make_cubic_path(),
                    SkMatrix::Scale(1.1f, 0.9f)) {
    benchmark_wangs_formula_cubic_log2(fMatrix, fPath);
}

DEF_PATH_TESS_BENCH(wangs_formula_cubic_log2_affine, make_cubic_path(),
                    SkMatrix::MakeAll(.9f,0.9f,0,  1.1f,1.1f,0, 0,0,1)) {
    benchmark_wangs_formula_cubic_log2(fMatrix, fPath);
}

DEF_PATH_TESS_BENCH(middle_out_triangulation,
                    ToolUtils::make_star(SkRect::MakeWH(500, 500), kNumCubicsInChalkboard),
                    SkMatrix::I()) {
    int baseVertex;
    auto vertexData = static_cast<SkPoint*>(fTarget->makeVertexSpace(
            sizeof(SkPoint), kNumCubicsInChalkboard, nullptr, &baseVertex));
    GrMiddleOutPolygonTriangulator::WritePathInnerFan(vertexData, 3, fPath);
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
protected:
    Benchmark(const char* nameSuffix, SkPaint::Join join) : fJoin(join) {
        fName.printf("tessellate_GrStrokeIndirectOpBench%s", nameSuffix);
    }

    const SkPaint::Join fJoin;

private:
    const char* onGetName() final { return fName.c_str(); }
    bool isSuitableFor(Backend backend) final { return backend == kNonRendering_Backend; }
    void onDelayedSetup() final {
        fTarget = std::make_unique<GrMockOpTarget>(make_mock_context());
        fStrokeRec.setStrokeStyle(8);
        fStrokeRec.setStrokeParams(SkPaint::kButt_Cap, fJoin, 4);
        this->setupPaths(&fPaths);
    }
    void onDraw(int loops, SkCanvas*) final {
        if (!fTarget->mockContext()) {
            SkDebugf("ERROR: could not create mock context.");
            return;
        }
        for (int i = 0; i < loops; ++i) {
            for (const SkPath& path : fPaths) {
                GrStrokeIndirectOp op(GrAAType::kMSAA, SkMatrix::I(), path, fStrokeRec, GrPaint());
                op.prePrepareResolveLevels(fTarget->allocator());
                op.prepareBuffers(fTarget.get());
            }
            fTarget->resetAllocator();
        }
    }
    virtual void setupPaths(SkTArray<SkPath>*) = 0;

    SkString fName;
    std::unique_ptr<GrMockOpTarget> fTarget;
    SkTArray<SkPath> fPaths;
    SkStrokeRec fStrokeRec{SkStrokeRec::kHairline_InitStyle};
};

class StrokeIndirectBenchmark : public GrStrokeIndirectOp::Benchmark {
public:
    StrokeIndirectBenchmark(const char* nameSuffix, SkPaint::Join join, std::vector<SkPoint> pts)
            : Benchmark(nameSuffix, join), fPts(std::move(pts)) {}

private:
    void setupPaths(SkTArray<SkPath>* paths) final {
        SkPath& path = paths->push_back();
        if (fJoin == SkPaint::kRound_Join) {
            path.reset().moveTo(fPts.back());
            for (size_t i = 0; i < kNumCubicsInChalkboard/fPts.size(); ++i) {
                for (size_t j = 0; j < fPts.size(); ++j) {
                    path.lineTo(fPts[j]);
                }
            }
        } else {
            path.reset().moveTo(fPts[0]);
            for (int i = 0; i < kNumCubicsInChalkboard/2; ++i) {
                if (fPts.size() == 4) {
                    path.cubicTo(fPts[1], fPts[2], fPts[3]);
                    path.cubicTo(fPts[2], fPts[1], fPts[0]);
                } else {
                    SkASSERT(fPts.size() == 3);
                    path.quadTo(fPts[1], fPts[2]);
                    path.quadTo(fPts[2], fPts[1]);
                }
            }
        }
    }

    const std::vector<SkPoint> fPts;
};

DEF_BENCH( return new StrokeIndirectBenchmark(
        "_inflect1", SkPaint::kBevel_Join, {{0,0}, {100,0}, {0,100}, {100,100}}); )

DEF_BENCH( return new StrokeIndirectBenchmark(
        "_inflect2", SkPaint::kBevel_Join, {{37,162}, {412,160}, {249,65}, {112,360}}); )

DEF_BENCH( return new StrokeIndirectBenchmark(
        "_loop", SkPaint::kBevel_Join, {{0,0}, {100,0}, {0,100}, {0,0}}); )

DEF_BENCH( return new StrokeIndirectBenchmark(
        "_nochop", SkPaint::kBevel_Join, {{0,0}, {50,0}, {100,50}, {100,100}}); )

DEF_BENCH( return new StrokeIndirectBenchmark(
        "_quad", SkPaint::kBevel_Join, {{0,0}, {50,100}, {100,0}}); )

DEF_BENCH( return new StrokeIndirectBenchmark(
        "_roundjoin", SkPaint::kRound_Join, {{0,0}, {50,100}, {100,0}}); )

class SingleVerbStrokeIndirectBenchmark : public GrStrokeIndirectOp::Benchmark {
public:
    SingleVerbStrokeIndirectBenchmark(const char* nameSuffix, SkPathVerb verb)
            : Benchmark(nameSuffix, SkPaint::kBevel_Join), fVerb(verb) {}

private:
    void setupPaths(SkTArray<SkPath>* paths) override {
        SkRandom rand;
        for (int i = 0; i < kNumCubicsInChalkboard; ++i)   {
            switch (fVerb) {
                case SkPathVerb::kQuad:
                    paths->push_back().quadTo(rand.nextF(), rand.nextF(), rand.nextF(),
                                              rand.nextF());
                    break;
                case SkPathVerb::kCubic:
                    switch (i % 3) {
                        case 0:
                            paths->push_back().cubicTo(100, 0, 0, 100, 100, 100);  // 1 inflection.
                            break;
                        case 1:
                            paths->push_back().cubicTo(100, 0, 0, 100, 0, 0);  // loop.
                            break;
                        case 2:
                            paths->push_back().cubicTo(50, 0, 100, 50, 100, 100);  // no chop.
                            break;
                    }
                    break;
                default:
                    SkUNREACHABLE;
            }
        }
    }

    const SkPathVerb fVerb;
};

DEF_BENCH( return new SingleVerbStrokeIndirectBenchmark("_singlequads", SkPathVerb::kQuad); )
DEF_BENCH( return new SingleVerbStrokeIndirectBenchmark("_singlecubics", SkPathVerb::kCubic); )
