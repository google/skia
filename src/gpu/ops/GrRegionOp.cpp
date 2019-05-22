/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrRegionOp.h"

#include "include/core/SkRegion.h"
#include "src/core/SkMatrixPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDefaultGeoProcFactory.h"
#include "src/gpu/GrDrawOpTest.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrVertexWriter.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"

static const int kVertsPerInstance = 4;
static const int kIndicesPerInstance = 6;

static sk_sp<GrGeometryProcessor> make_gp(const GrShaderCaps* shaderCaps,
                                          const SkMatrix& viewMatrix,
                                          bool wideColor) {
    using namespace GrDefaultGeoProcFactory;
    Color::Type colorType =
        wideColor ? Color::kPremulWideColorAttribute_Type : Color::kPremulGrColorAttribute_Type;
    return GrDefaultGeoProcFactory::Make(shaderCaps, colorType, Coverage::kSolid_Type,
                                         LocalCoords::kUsePosition_Type, viewMatrix);
}

namespace {

class RegionOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelperWithStencil;

public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                          GrPaint&& paint,
                                          const SkMatrix& viewMatrix,
                                          const SkRegion& region,
                                          GrAAType aaType,
                                          const GrUserStencilSettings* stencilSettings = nullptr) {
        return Helper::FactoryHelper<RegionOp>(context, std::move(paint), viewMatrix, region,
                                               aaType, stencilSettings);
    }

    RegionOp(const Helper::MakeArgs& helperArgs, const SkPMColor4f& color,
             const SkMatrix& viewMatrix, const SkRegion& region, GrAAType aaType,
             const GrUserStencilSettings* stencilSettings)
            : INHERITED(ClassID())
            , fHelper(helperArgs, aaType, stencilSettings)
            , fViewMatrix(viewMatrix) {
        RegionInfo& info = fRegions.push_back();
        info.fColor = color;
        info.fRegion = region;

        SkRect bounds = SkRect::Make(region.getBounds());
        this->setTransformedBounds(bounds, viewMatrix, HasAABloat::kNo, IsZeroArea::kNo);
    }

    const char* name() const override { return "GrRegionOp"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        fHelper.visitProxies(func);
    }

#ifdef SK_DEBUG
    SkString dumpInfo() const override {
        SkString str;
        str.appendf("# combined: %d\n", fRegions.count());
        for (int i = 0; i < fRegions.count(); ++i) {
            const RegionInfo& info = fRegions[i];
            str.appendf("%d: Color: 0x%08x, Region with %d rects\n", i, info.fColor.toBytes_RGBA(),
                        info.fRegion.computeRegionComplexity());
        }
        str += fHelper.dumpInfo();
        str += INHERITED::dumpInfo();
        return str;
    }
#endif

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

    GrProcessorSet::Analysis finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                      GrFSAAType fsaaType, GrClampType clampType) override {
        return fHelper.finalizeProcessors(caps, clip, fsaaType, clampType,
                                          GrProcessorAnalysisCoverage::kNone, &fRegions[0].fColor,
                                          &fWideColor);
    }

private:
    void onPrepareDraws(Target* target) override {
        sk_sp<GrGeometryProcessor> gp = make_gp(target->caps().shaderCaps(), fViewMatrix,
                                                fWideColor);
        if (!gp) {
            SkDebugf("Couldn't create GrGeometryProcessor\n");
            return;
        }

        int numRegions = fRegions.count();
        int numRects = 0;
        for (int i = 0; i < numRegions; i++) {
            numRects += fRegions[i].fRegion.computeRegionComplexity();
        }

        if (!numRects) {
            return;
        }
        sk_sp<const GrGpuBuffer> indexBuffer = target->resourceProvider()->refQuadIndexBuffer();
        if (!indexBuffer) {
            SkDebugf("Could not allocate indices\n");
            return;
        }
        PatternHelper helper(target, GrPrimitiveType::kTriangles, gp->vertexStride(),
                             std::move(indexBuffer), kVertsPerInstance, kIndicesPerInstance,
                             numRects);
        GrVertexWriter vertices{helper.vertices()};
        if (!vertices.fPtr) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        for (int i = 0; i < numRegions; i++) {
            GrVertexColor color(fRegions[i].fColor, fWideColor);
            SkRegion::Iterator iter(fRegions[i].fRegion);
            while (!iter.done()) {
                SkRect rect = SkRect::Make(iter.rect());
                vertices.writeQuad(GrVertexWriter::TriStripFromRect(rect), color);
                iter.next();
            }
        }
        helper.recordDraw(target, std::move(gp));
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        fHelper.executeDrawsAndUploads(this, flushState, chainBounds);
    }

    CombineResult onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        RegionOp* that = t->cast<RegionOp>();
        if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
            return CombineResult::kCannotCombine;
        }

        if (fViewMatrix != that->fViewMatrix) {
            return CombineResult::kCannotCombine;
        }

        fRegions.push_back_n(that->fRegions.count(), that->fRegions.begin());
        fWideColor |= that->fWideColor;
        return CombineResult::kMerged;
    }

    struct RegionInfo {
        SkPMColor4f fColor;
        SkRegion fRegion;
    };

    Helper fHelper;
    SkMatrix fViewMatrix;
    SkSTArray<1, RegionInfo, true> fRegions;
    bool fWideColor;

    typedef GrMeshDrawOp INHERITED;
};

}  // anonymous namespace

namespace GrRegionOp {

std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                               GrPaint&& paint,
                               const SkMatrix& viewMatrix,
                               const SkRegion& region,
                               GrAAType aaType,
                               const GrUserStencilSettings* stencilSettings) {
    if (aaType != GrAAType::kNone && aaType != GrAAType::kMSAA) {
        return nullptr;
    }
    return RegionOp::Make(context, std::move(paint), viewMatrix, region, aaType, stencilSettings);
}
}

#if GR_TEST_UTILS

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
            GR_STATIC_ASSERT(SkRegion::kLastOp == SkRegion::kReplace_Op);
            op = (SkRegion::Op)random->nextULessThan(SkRegion::kLastOp);
        }
        region.op(rect, op);
    }
    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    GrAAType aaType = GrAAType::kNone;
    if (GrFSAAType::kUnifiedMSAA == fsaaType && random->nextBool()) {
        aaType = GrAAType::kMSAA;
    }
    return RegionOp::Make(context, std::move(paint), viewMatrix, region, aaType,
                          GrGetRandomStencil(random, context));
}

#endif
