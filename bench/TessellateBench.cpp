/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkRectPriv.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/mock/GrMockOpTarget.h"
#include "src/gpu/ganesh/tessellate/PathTessellator.h"
#include "src/gpu/ganesh/tessellate/StrokeTessellator.h"
#include "src/gpu/tessellate/AffineMatrix.h"
#include "src/gpu/tessellate/MiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/WangsFormula.h"
#include "tools/ToolUtils.h"
#include <vector>

namespace skgpu::ganesh {

// This is the number of cubics in desk_chalkboard.skp. (There are no quadratics in the chalkboard.)
constexpr static int kNumCubicsInChalkboard = 47182;

static sk_sp<GrDirectContext> make_mock_context() {
    GrMockOptions mockOptions;
    mockOptions.fDrawInstancedSupport = true;
    mockOptions.fMapBufferFlags = GrCaps::kCanMap_MapFlag;
    mockOptions.fConfigOptions[(int)GrColorType::kAlpha_8].fRenderability =
            GrMockOptions::ConfigOptions::Renderability::kMSAA;
    mockOptions.fConfigOptions[(int)GrColorType::kAlpha_8].fTexturable = true;
    mockOptions.fIntegerSupport = true;

    GrContextOptions ctxOptions;
    ctxOptions.fGpuPathRenderers = GpuPathRenderers::kTessellation;

    return GrDirectContext::MakeMock(&mockOptions, ctxOptions);
}

static SkPath make_cubic_path(int maxPow2) {
    SkRandom rand;
    SkPath path;
    for (int i = 0; i < kNumCubicsInChalkboard/2; ++i) {
        float x = std::ldexp(rand.nextF(), (i % maxPow2)) / 1e3f;
        path.cubicTo(111.625f*x, 308.188f*x, 764.62f*x, -435.688f*x, 742.63f*x, 85.187f*x);
        path.cubicTo(764.62f*x, -435.688f*x, 111.625f*x, 308.188f*x, 0, 0);
    }
    return path;
}

static SkPath make_conic_path() {
    SkRandom rand;
    SkPath path;
    for (int i = 0; i < kNumCubicsInChalkboard / 40; ++i) {
        for (int j = -10; j <= 10; j++) {
            const float x = std::ldexp(rand.nextF(), (i % 18)) / 1e3f;
            const float w = std::ldexp(1 + rand.nextF(), j);
            path.conicTo(111.625f * x, 308.188f * x, 764.62f * x, -435.688f * x, w);
        }
    }
    return path;
}

[[maybe_unused]] static SkPath make_quad_path(int maxPow2) {
    SkRandom rand;
    SkPath path;
    for (int i = 0; i < kNumCubicsInChalkboard; ++i) {
        float x = std::ldexp(rand.nextF(), (i % maxPow2)) / 1e3f;
        path.quadTo(111.625f * x, 308.188f * x, 764.62f * x, -435.688f * x);
    }
    return path;
}

[[maybe_unused]] static SkPath make_line_path(int maxPow2) {
    SkRandom rand;
    SkPath path;
    for (int i = 0; i < kNumCubicsInChalkboard; ++i) {
        float x = std::ldexp(rand.nextF(), (i % maxPow2)) / 1e3f;
        path.lineTo(764.62f * x, -435.688f * x);
    }
    return path;
}

// This serves as a base class for benchmarking individual methods on PathTessellateOp.
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
    const SkMatrix fMatrix;
};

#define DEF_PATH_TESS_BENCH(NAME, PATH, MATRIX) \
    class PathTessellateBenchmark_##NAME : public PathTessellateBenchmark { \
    public: \
        PathTessellateBenchmark_##NAME() : PathTessellateBenchmark(#NAME, (PATH), (MATRIX)) {} \
        void runBench() override; \
    }; \
    DEF_BENCH( return new PathTessellateBenchmark_##NAME(); ); \
    void PathTessellateBenchmark_##NAME::runBench()

static const SkMatrix gAlmostIdentity = SkMatrix::MakeAll(
        1.0001f, 0.0001f, 0.0001f,
        -.0001f, 0.9999f, -.0001f,
              0,       0,       1);

DEF_PATH_TESS_BENCH(GrPathCurveTessellator, make_cubic_path(8), SkMatrix::I()) {
    SkArenaAlloc arena(1024);
    GrPipeline noVaryingsPipeline(GrScissorTest::kDisabled, SkBlendMode::kSrcOver,
                                  skgpu::Swizzle::RGBA());
    auto tess = PathCurveTessellator::Make(&arena,
                                           fTarget->caps().shaderCaps()->fInfinitySupport);
    tess->prepare(fTarget.get(),
                  fMatrix,
                  {gAlmostIdentity, fPath, SK_PMColor4fTRANSPARENT},
                  fPath.countVerbs());
}

DEF_PATH_TESS_BENCH(GrPathWedgeTessellator, make_cubic_path(8), SkMatrix::I()) {
    SkArenaAlloc arena(1024);
    GrPipeline noVaryingsPipeline(GrScissorTest::kDisabled, SkBlendMode::kSrcOver,
                                  skgpu::Swizzle::RGBA());
    auto tess = PathWedgeTessellator::Make(&arena,
                                           fTarget->caps().shaderCaps()->fInfinitySupport);
    tess->prepare(fTarget.get(),
                  fMatrix,
                  {gAlmostIdentity, fPath, SK_PMColor4fTRANSPARENT},
                  fPath.countVerbs());
}

static void benchmark_wangs_formula_cubic_log2(const SkMatrix& matrix, const SkPath& path) {
    int sum = 0;
    wangs_formula::VectorXform xform(matrix);
    for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
        if (verb == SkPathVerb::kCubic) {
            sum += wangs_formula::cubic_log2(4, pts, xform);
        }
    }
    // Don't let the compiler optimize away wangs_formula::cubic_log2.
    if (sum <= 0) {
        SK_ABORT("sum should be > 0.");
    }
}

DEF_PATH_TESS_BENCH(wangs_formula_cubic_log2, make_cubic_path(18), SkMatrix::I()) {
    benchmark_wangs_formula_cubic_log2(fMatrix, fPath);
}

DEF_PATH_TESS_BENCH(wangs_formula_cubic_log2_scale, make_cubic_path(18),
                    SkMatrix::Scale(1.1f, 0.9f)) {
    benchmark_wangs_formula_cubic_log2(fMatrix, fPath);
}

DEF_PATH_TESS_BENCH(wangs_formula_cubic_log2_affine, make_cubic_path(18),
                    SkMatrix::MakeAll(.9f,0.9f,0,  1.1f,1.1f,0, 0,0,1)) {
    benchmark_wangs_formula_cubic_log2(fMatrix, fPath);
}

static void benchmark_wangs_formula_conic(const SkMatrix& matrix, const SkPath& path) {
    int sum = 0;
    wangs_formula::VectorXform xform(matrix);
    for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
        if (verb == SkPathVerb::kConic) {
            sum += wangs_formula::conic(4, pts, *w, xform);
        }
    }
    // Don't let the compiler optimize away wangs_formula::conic.
    if (sum <= 0) {
        SK_ABORT("sum should be > 0.");
    }
}

static void benchmark_wangs_formula_conic_log2(const SkMatrix& matrix, const SkPath& path) {
    int sum = 0;
    wangs_formula::VectorXform xform(matrix);
    for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
        if (verb == SkPathVerb::kConic) {
            sum += wangs_formula::conic_log2(4, pts, *w, xform);
        }
    }
    // Don't let the compiler optimize away wangs_formula::conic.
    if (sum <= 0) {
        SK_ABORT("sum should be > 0.");
    }
}

DEF_PATH_TESS_BENCH(wangs_formula_conic, make_conic_path(), SkMatrix::I()) {
    benchmark_wangs_formula_conic(fMatrix, fPath);
}

DEF_PATH_TESS_BENCH(wangs_formula_conic_log2, make_conic_path(), SkMatrix::I()) {
    benchmark_wangs_formula_conic_log2(fMatrix, fPath);
}

DEF_PATH_TESS_BENCH(middle_out_triangulation,
                    ToolUtils::make_star(SkRect::MakeWH(500, 500), kNumCubicsInChalkboard),
                    SkMatrix::I()) {
    // Conservative estimate of triangulation (see PathStencilCoverOp)
    const int maxVerts = 3 * (kNumCubicsInChalkboard - 2);

    sk_sp<const GrBuffer> buffer;
    int baseVertex;
    VertexWriter vertexWriter = fTarget->makeVertexWriter(
            sizeof(SkPoint), maxVerts, &buffer, &baseVertex);
    tess::AffineMatrix m(gAlmostIdentity);
    for (tess::PathMiddleOutFanIter it(fPath); !it.done();) {
        for (auto [p0, p1, p2] : it.nextStack()) {
            vertexWriter << m.map2Points(p0, p1) << m.mapPoint(p2);
        }
    }
}

using PathStrokeList = StrokeTessellator::PathStrokeList;
using MakePathStrokesFn = std::vector<PathStrokeList>(*)();

static std::vector<PathStrokeList> make_simple_cubic_path() {
    auto path = SkPath().moveTo(0, 0);
    for (int i = 0; i < kNumCubicsInChalkboard/2; ++i) {
        path.cubicTo(100, 0, 50, 100, 100, 100);
        path.cubicTo(0, -100, 200, 100, 0, 0);
    }
    SkStrokeRec stroke(SkStrokeRec::kFill_InitStyle);
    stroke.setStrokeStyle(8);
    stroke.setStrokeParams(SkPaint::kButt_Cap, SkPaint::kMiter_Join, 4);
    return {{path, stroke, SK_PMColor4fWHITE}};
}

// Generates a list of paths that resemble the MotionMark benchmark.
static std::vector<PathStrokeList> make_motionmark_paths() {
    std::vector<PathStrokeList> pathStrokes;
    SkRandom rand;
    for (int i = 0; i < 8702; ++i) {
        // The number of paths with a given number of verbs in the MotionMark bench gets cut in half
        // every time the number of verbs increases by 1.
        int numVerbs = 28 - SkNextLog2(rand.nextRangeU(0, (1 << 27) - 1));
        SkPath path;
        for (int j = 0; j < numVerbs; ++j) {
            switch (rand.nextU() & 3) {
                case 0:
                case 1:
                    path.lineTo(rand.nextRangeF(0, 150), rand.nextRangeF(0, 150));
                    break;
                case 2:
                    if (rand.nextULessThan(10) == 0) {
                        // Cusp.
                        auto [x, y] = (path.isEmpty())
                                ? SkPoint{0,0}
                                : SkPathPriv::PointData(path)[path.countPoints() - 1];
                        path.quadTo(x + rand.nextRangeF(0, 150), y, x - rand.nextRangeF(0, 150), y);
                    } else {
                        path.quadTo(rand.nextRangeF(0, 150), rand.nextRangeF(0, 150),
                                    rand.nextRangeF(0, 150), rand.nextRangeF(0, 150));
                    }
                    break;
                case 3:
                    if (rand.nextULessThan(10) == 0) {
                        // Cusp.
                        float y = (path.isEmpty())
                                ? 0 : SkPathPriv::PointData(path)[path.countPoints() - 1].fY;
                        path.cubicTo(rand.nextRangeF(0, 150), y, rand.nextRangeF(0, 150), y,
                                     rand.nextRangeF(0, 150), y);
                    } else {
                        path.cubicTo(rand.nextRangeF(0, 150), rand.nextRangeF(0, 150),
                                     rand.nextRangeF(0, 150), rand.nextRangeF(0, 150),
                                     rand.nextRangeF(0, 150), rand.nextRangeF(0, 150));
                    }
                    break;
            }
        }
        SkStrokeRec stroke(SkStrokeRec::kFill_InitStyle);
        // The number of paths with a given stroke width in the MotionMark bench gets cut in half
        // every time the stroke width increases by 1.
        float strokeWidth = 21 - log2f(rand.nextRangeF(0, 1 << 20));
        stroke.setStrokeStyle(strokeWidth);
        stroke.setStrokeParams(SkPaint::kButt_Cap, SkPaint::kBevel_Join, 0);
        pathStrokes.emplace_back(path, stroke, SK_PMColor4fWHITE);
    }
    return pathStrokes;
}

using PatchAttribs = tess::PatchAttribs;

class TessPrepareBench : public Benchmark {
public:
    TessPrepareBench(MakePathStrokesFn makePathStrokesFn,
                     PatchAttribs attribs,
                     float matrixScale,
                     const char* suffix)
            : fMakePathStrokesFn(makePathStrokesFn)
            , fPatchAttribs(attribs)
            , fMatrixScale(matrixScale) {
        fName.printf("tessellate_%s", suffix);
    }

private:
    const char* onGetName() override { return fName.c_str(); }
    bool isSuitableFor(Backend backend) final { return backend == kNonRendering_Backend; }

    void onDelayedSetup() override {
        fTarget = std::make_unique<GrMockOpTarget>(make_mock_context());
        if (!fTarget->mockContext()) {
            SkDebugf("ERROR: could not create mock context.");
            return;
        }

        fPathStrokes = fMakePathStrokesFn();
        for (size_t i = 0; i < fPathStrokes.size(); ++i) {
            if (i + 1 < fPathStrokes.size()) {
                fPathStrokes[i].fNext = &fPathStrokes[i + 1];
            }
            fTotalVerbCount += fPathStrokes[i].fPath.countVerbs();
        }

        fTessellator = std::make_unique<StrokeTessellator>(fPatchAttribs);
    }

    void onDraw(int loops, SkCanvas*) final {
        for (int i = 0; i < loops; ++i) {
            fTessellator->prepare(fTarget.get(),
                                  SkMatrix::Scale(fMatrixScale, fMatrixScale),
                                  fPathStrokes.data(),
                                  fTotalVerbCount);
            fTarget->resetAllocator();
        }
    }

    SkString fName;
    MakePathStrokesFn fMakePathStrokesFn;
    const PatchAttribs fPatchAttribs;
    float fMatrixScale;
    std::unique_ptr<GrMockOpTarget> fTarget;
    std::vector<PathStrokeList> fPathStrokes;
    std::unique_ptr<StrokeTessellator> fTessellator;
    SkArenaAlloc fPersistentArena{1024};
    int fTotalVerbCount = 0;
};

DEF_BENCH(return new TessPrepareBench(
        make_simple_cubic_path, PatchAttribs::kNone, 1,
        "GrStrokeFixedCountTessellator");
)

DEF_BENCH(return new TessPrepareBench(
        make_simple_cubic_path, PatchAttribs::kNone, 5,
        "GrStrokeFixedCountTessellator_one_chop");
)

DEF_BENCH(return new TessPrepareBench(
        make_motionmark_paths, PatchAttribs::kStrokeParams, 1,
        "GrStrokeFixedCountTessellator_motionmark");
)

}  // namespace skgpu::ganesh
