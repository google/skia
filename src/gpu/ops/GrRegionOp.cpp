/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRegionOp.h"
#include <GrDrawOpTest.h>
#include "GrDefaultGeoProcFactory.h"
#include "GrMeshDrawOp.h"
#include "GrOpFlushState.h"
#include "GrResourceProvider.h"
#include "GrSimpleMeshDrawOpHelper.h"
#include "SkMatrixPriv.h"
#include "SkRegion.h"

static const int kVertsPerInstance = 4;
static const int kIndicesPerInstance = 6;

static sk_sp<GrGeometryProcessor> make_gp(const SkMatrix& viewMatrix) {
    using namespace GrDefaultGeoProcFactory;
    return GrDefaultGeoProcFactory::Make(Color::kPremulGrColorAttribute_Type, Coverage::kSolid_Type,
                                         LocalCoords::kUsePosition_Type, viewMatrix);
}

static void tesselate_region(intptr_t vertices,
                             size_t vertexStride,
                             GrColor color,
                             const SkRegion& region) {
    SkRegion::Iterator iter(region);

    intptr_t verts = vertices;
    while (!iter.done()) {
        SkRect rect = SkRect::Make(iter.rect());
        SkPoint* position = (SkPoint*)verts;
        position->setRectFan(rect.fLeft, rect.fTop, rect.fRight, rect.fBottom, vertexStride);

        static const int kColorOffset = sizeof(SkPoint);
        GrColor* vertColor = reinterpret_cast<GrColor*>(verts + kColorOffset);
        for (int i = 0; i < kVertsPerInstance; i++) {
            *vertColor = color;
            vertColor = (GrColor*)((intptr_t)vertColor + vertexStride);
        }

        verts += vertexStride * kVertsPerInstance;
        iter.next();
    }
}

namespace {

class RegionOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelper;

public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(GrPaint&& paint, const SkMatrix& viewMatrix,
                                          const SkRegion& region, GrAAType aaType) {
        return Helper::FactoryHelper<RegionOp>(std::move(paint), viewMatrix, region, aaType);
    }

    RegionOp(const Helper::MakeArgs& helperArgs, GrColor color, const SkMatrix& viewMatrix,
             const SkRegion& region, GrAAType aaType)
            : INHERITED(ClassID()), fHelper(helperArgs, aaType), fViewMatrix(viewMatrix) {
        RegionInfo& info = fRegions.push_back();
        info.fColor = color;
        info.fRegion = region;

        SkRect bounds = SkRect::Make(region.getBounds());
        this->setTransformedBounds(bounds, viewMatrix, HasAABloat::kNo, IsZeroArea::kNo);
    }

    const char* name() const override { return "GrRegionOp"; }

    SkString dumpInfo() const override {
        SkString str;
        str.appendf("# combined: %d\n", fRegions.count());
        for (int i = 0; i < fRegions.count(); ++i) {
            const RegionInfo& info = fRegions[i];
            str.appendf("%d: Color: 0x%08x, Region with %d rects\n", i, info.fColor,
                        info.fRegion.computeRegionComplexity());
        }
        str += fHelper.dumpInfo();
        str += INHERITED::dumpInfo();
        return str;
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override {
        return fHelper.xpRequiresDstTexture(caps, clip, GrProcessorAnalysisCoverage::kNone,
                                            &fRegions[0].fColor);
    }

private:
    void onPrepareDraws(Target* target) const override {
        sk_sp<GrGeometryProcessor> gp = make_gp(fViewMatrix);
        if (!gp) {
            SkDebugf("Couldn't create GrGeometryProcessor\n");
            return;
        }
        SkASSERT(gp->getVertexStride() == sizeof(GrDefaultGeoProcFactory::PositionColorAttr));

        int numRegions = fRegions.count();
        int numRects = 0;
        for (int i = 0; i < numRegions; i++) {
            numRects += fRegions[i].fRegion.computeRegionComplexity();
        }

        if (!numRects) {
            return;
        }
        size_t vertexStride = gp->getVertexStride();
        sk_sp<const GrBuffer> indexBuffer(target->resourceProvider()->refQuadIndexBuffer());
        PatternHelper helper(GrPrimitiveType::kTriangles);
        void* vertices =
                helper.init(target, vertexStride, indexBuffer.get(), kVertsPerInstance,
                            kIndicesPerInstance, numRects);
        if (!vertices || !indexBuffer) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        intptr_t verts = reinterpret_cast<intptr_t>(vertices);
        for (int i = 0; i < numRegions; i++) {
            tesselate_region(verts, vertexStride, fRegions[i].fColor, fRegions[i].fRegion);
            int numRectsInRegion = fRegions[i].fRegion.computeRegionComplexity();
            verts += numRectsInRegion * kVertsPerInstance * vertexStride;
        }
        helper.recordDraw(target, gp.get(), fHelper.makePipeline(target));
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        RegionOp* that = t->cast<RegionOp>();
        if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
            return false;
        }

        if (fViewMatrix != that->fViewMatrix) {
            return false;
        }

        fRegions.push_back_n(that->fRegions.count(), that->fRegions.begin());
        this->joinBounds(*that);
        return true;
    }

    struct RegionInfo {
        GrColor fColor;
        SkRegion fRegion;
    };

    Helper fHelper;
    SkMatrix fViewMatrix;
    SkSTArray<1, RegionInfo, true> fRegions;

    typedef GrMeshDrawOp INHERITED;
};

}  // anonymous namespace

namespace GrRegionOp {

std::unique_ptr<GrDrawOp> Make(GrPaint&& paint, const SkMatrix& viewMatrix, const SkRegion& region,
                               GrAAType aaType) {
    if (aaType != GrAAType::kNone && aaType != GrAAType::kMSAA) {
        return nullptr;
    }
    return RegionOp::Make(std::move(paint), viewMatrix, region, aaType);
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
    return RegionOp::Make(std::move(paint), viewMatrix, region, aaType);
}

#endif
