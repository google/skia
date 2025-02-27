/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/ops/RegionOp.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRegion.h"
#include "include/core/SkString.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTArray.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkSafeMath.h"
#include "src/core/SkColorData.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/ganesh/GrAppliedClip.h"
#include "src/gpu/ganesh/GrDefaultGeoProcFactory.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/GrProcessorAnalysis.h"
#include "src/gpu/ganesh/GrProcessorSet.h"
#include "src/gpu/ganesh/GrProgramInfo.h"
#include "src/gpu/ganesh/ops/GrMeshDrawOp.h"
#include "src/gpu/ganesh/ops/GrSimpleMeshDrawOpHelperWithStencil.h"

#if defined(GPU_TEST_UTILS)
#include "src/base/SkRandom.h"
#include "src/gpu/ganesh/GrDrawOpTest.h"
#include "src/gpu/ganesh/GrTestUtils.h"
#endif

#include <utility>

class GrCaps;
class GrDstProxyView;
class GrMeshDrawTarget;
class GrSurfaceProxyView;
class SkArenaAlloc;
enum class GrXferBarrierFlags;
struct GrSimpleMesh;

namespace skgpu::ganesh {
class SurfaceDrawContext;
}

using namespace skia_private;

namespace skgpu::ganesh::RegionOp {

namespace {

GrGeometryProcessor* make_gp(SkArenaAlloc* arena,
                                    const SkMatrix& viewMatrix,
                                    bool wideColor) {
    using namespace GrDefaultGeoProcFactory;
    Color::Type colorType = wideColor ? Color::kPremulWideColorAttribute_Type
                                      : Color::kPremulGrColorAttribute_Type;
    return GrDefaultGeoProcFactory::Make(arena, colorType, Coverage::kSolid_Type,
                                         LocalCoords::kUsePosition_Type, viewMatrix);
}

class RegionOpImpl final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelperWithStencil;

public:
    DEFINE_OP_CLASS_ID

    static GrOp::Owner Make(GrRecordingContext* context,
                            GrPaint&& paint,
                            const SkMatrix& viewMatrix,
                            const SkRegion& region,
                            GrAAType aaType,
                            const GrUserStencilSettings* stencilSettings = nullptr) {
        return Helper::FactoryHelper<RegionOpImpl>(context, std::move(paint), viewMatrix, region,
                                                   aaType, stencilSettings);
    }

    RegionOpImpl(GrProcessorSet* processorSet, const SkPMColor4f& color,
                 const SkMatrix& viewMatrix, const SkRegion& region, GrAAType aaType,
                 const GrUserStencilSettings* stencilSettings)
            : INHERITED(ClassID())
            , fHelper(processorSet, aaType, stencilSettings)
            , fViewMatrix(viewMatrix) {
        RegionInfo& info = fRegions.push_back();
        info.fColor = color;
        info.fRegion = region;

        SkRect bounds = SkRect::Make(region.getBounds());
        this->setTransformedBounds(bounds, viewMatrix, HasAABloat::kNo, IsHairline::kNo);
    }

    const char* name() const override { return "GrRegionOp"; }

    void visitProxies(const GrVisitProxyFunc& func) const override {
        if (fProgramInfo) {
            fProgramInfo->visitFPProxies(func);
        } else {
            fHelper.visitProxies(func);
        }
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

    GrProcessorSet::Analysis finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                      GrClampType clampType) override {
        return fHelper.finalizeProcessors(caps, clip, clampType, GrProcessorAnalysisCoverage::kNone,
                                          &fRegions[0].fColor, &fWideColor);
    }

private:
    GrProgramInfo* programInfo() override { return fProgramInfo; }

    void onCreateProgramInfo(const GrCaps* caps,
                             SkArenaAlloc* arena,
                             const GrSurfaceProxyView& writeView,
                             bool usesMSAASurface,
                             GrAppliedClip&& appliedClip,
                             const GrDstProxyView& dstProxyView,
                             GrXferBarrierFlags renderPassXferBarriers,
                             GrLoadOp colorLoadOp) override {
        GrGeometryProcessor* gp = make_gp(arena, fViewMatrix, fWideColor);
        if (!gp) {
            SkDebugf("Couldn't create GrGeometryProcessor\n");
            return;
        }

        fProgramInfo = fHelper.createProgramInfoWithStencil(caps, arena, writeView, usesMSAASurface,
                                                            std::move(appliedClip), dstProxyView,
                                                            gp, GrPrimitiveType::kTriangles,
                                                            renderPassXferBarriers, colorLoadOp);
    }

    void onPrepareDraws(GrMeshDrawTarget* target) override {
        if (!fProgramInfo) {
            this->createProgramInfo(target);
            if (!fProgramInfo) {
                return;
            }
        }

        int numRegions = fRegions.size();
        int numRects = 0;

        SkSafeMath safeMath;
        for (int i = 0; i < numRegions; i++) {
            numRects = safeMath.addInt(numRects, fRegions[i].fRegion.computeRegionComplexity());
        }

        if (!numRects || !safeMath) {
            return;
        }

        QuadHelper helper(target, fProgramInfo->geomProc().vertexStride(), numRects);

        VertexWriter vertices{helper.vertices()};
        if (!vertices) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        for (int i = 0; i < numRegions; i++) {
            VertexColor color(fRegions[i].fColor, fWideColor);
            SkRegion::Iterator iter(fRegions[i].fRegion);
            while (!iter.done()) {
                SkRect rect = SkRect::Make(iter.rect());
                vertices.writeQuad(VertexWriter::TriStripFromRect(rect), color);
                iter.next();
            }
        }

        fMesh = helper.mesh();
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        if (!fProgramInfo || !fMesh) {
            return;
        }

        flushState->bindPipelineAndScissorClip(*fProgramInfo, chainBounds);
        flushState->bindTextures(fProgramInfo->geomProc(), nullptr, fProgramInfo->pipeline());
        flushState->drawMesh(*fMesh);
    }

    CombineResult onCombineIfPossible(GrOp* t, SkArenaAlloc*, const GrCaps& caps) override {
        auto that = t->cast<RegionOpImpl>();
        if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
            return CombineResult::kCannotCombine;
        }

        if (fViewMatrix != that->fViewMatrix) {
            return CombineResult::kCannotCombine;
        }

        fRegions.push_back_n(that->fRegions.size(), that->fRegions.begin());
        fWideColor |= that->fWideColor;
        return CombineResult::kMerged;
    }

#if defined(GPU_TEST_UTILS)
    SkString onDumpInfo() const override {
        SkString str = SkStringPrintf("# combined: %d\n", fRegions.size());
        for (int i = 0; i < fRegions.size(); ++i) {
            const RegionInfo& info = fRegions[i];
            str.appendf("%d: Color: 0x%08x, Region with %d rects\n", i, info.fColor.toBytes_RGBA(),
                        info.fRegion.computeRegionComplexity());
        }
        str += fHelper.dumpInfo();
        return str;
    }
#endif

    struct RegionInfo {
        SkPMColor4f fColor;
        SkRegion fRegion;
    };

    Helper fHelper;
    SkMatrix fViewMatrix;
    STArray<1, RegionInfo, true> fRegions;
    bool fWideColor;

    GrSimpleMesh*  fMesh = nullptr;
    GrProgramInfo* fProgramInfo = nullptr;

    using INHERITED = GrMeshDrawOp;
};

}  // anonymous namespace

GrOp::Owner Make(GrRecordingContext* context,
                 GrPaint&& paint,
                 const SkMatrix& viewMatrix,
                 const SkRegion& region,
                 GrAAType aaType,
                 const GrUserStencilSettings* stencilSettings) {
    if (aaType != GrAAType::kNone && aaType != GrAAType::kMSAA) {
        return nullptr;
    }
    return RegionOpImpl::Make(context, std::move(paint), viewMatrix, region, aaType,
                              stencilSettings);
}

}  // namespace skgpu::ganesh::RegionOp

#if defined(GPU_TEST_UTILS)

GR_DRAW_OP_TEST_DEFINE(RegionOp) {
    SkRegion region;
    int n = random->nextULessThan(200);
    for (int i = 0; i < n; ++i) {
        SkIPoint center;
        center.fX = random->nextULessThan(1000);
        center.fY = random->nextULessThan(1000);
        int w = random->nextRangeU(10, 1000);
        int h = random->nextRangeU(10, 1000);
        SkIRect rect = {center.fX - w / 2, center.fY - h / 2, center.fX + w / 2, center.fY + h / 2};
        SkRegion::Op op;
        if (i == 0) {
            op = SkRegion::kReplace_Op;
        } else {
            // Pick an other than replace.
            static_assert(SkRegion::kLastOp == SkRegion::kReplace_Op);
            op = (SkRegion::Op)random->nextULessThan(SkRegion::kLastOp);
        }
        region.op(rect, op);
    }
    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    GrAAType aaType = GrAAType::kNone;
    if (numSamples > 1 && random->nextBool()) {
        aaType = GrAAType::kMSAA;
    }
    return skgpu::ganesh::RegionOp::RegionOpImpl::Make(context,
                                                       std::move(paint),
                                                       viewMatrix,
                                                       region,
                                                       aaType,
                                                       GrGetRandomStencil(random, context));
}

#endif // defined(GPU_TEST_UTILS)
