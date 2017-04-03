/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRegionOp.h"

#include "GrDefaultGeoProcFactory.h"
#include "GrMeshDrawOp.h"
#include "GrOpFlushState.h"
#include "GrResourceProvider.h"
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

class RegionOp final : public GrLegacyMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    RegionOp(GrColor color, const SkMatrix& viewMatrix, const SkRegion& region)
            : INHERITED(ClassID()), fViewMatrix(viewMatrix) {
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
        str.append(DumpPipelineInfo(*this->pipeline()));
        str.append(INHERITED::dumpInfo());
        return str;
    }

private:
    void getProcessorAnalysisInputs(GrProcessorAnalysisColor* color,
                                    GrProcessorAnalysisCoverage* coverage) const override {
        color->setToConstant(fRegions[0].fColor);
        *coverage = GrProcessorAnalysisCoverage::kNone;
    }

    void applyPipelineOptimizations(const PipelineOptimizations& optimizations) override {
        optimizations.getOverrideColorIfSet(&fRegions[0].fColor);
    }

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

        size_t vertexStride = gp->getVertexStride();
        sk_sp<const GrBuffer> indexBuffer(target->resourceProvider()->refQuadIndexBuffer());
        InstancedHelper helper;
        void* vertices =
                helper.init(target, kTriangles_GrPrimitiveType, vertexStride, indexBuffer.get(),
                            kVertsPerInstance, kIndicesPerInstance, numRects);
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
        helper.recordDraw(target, gp.get(), this->pipeline());
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        RegionOp* that = t->cast<RegionOp>();
        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                    that->bounds(), caps)) {
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

    SkMatrix fViewMatrix;
    SkSTArray<1, RegionInfo, true> fRegions;

    typedef GrLegacyMeshDrawOp INHERITED;
};

namespace GrRegionOp {

std::unique_ptr<GrLegacyMeshDrawOp> Make(GrColor color, const SkMatrix& viewMatrix,
                                         const SkRegion& region) {
    return std::unique_ptr<GrLegacyMeshDrawOp>(new RegionOp(color, viewMatrix, region));
}
}
