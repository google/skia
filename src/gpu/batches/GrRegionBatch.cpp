/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRegionBatch.h"

#include "GrDefaultGeoProcFactory.h"
#include "GrBatchFlushState.h"
#include "GrResourceProvider.h"
#include "GrVertexBatch.h"
#include "SkMatrixPriv.h"
#include "SkRegion.h"

static const int kVertsPerInstance = 4;
static const int kIndicesPerInstance = 6;

static sk_sp<GrGeometryProcessor> make_gp(bool readsCoverage) {
    using namespace GrDefaultGeoProcFactory;
    Color color(Color::kAttribute_Type);
    Coverage coverage(readsCoverage ? Coverage::kSolid_Type : Coverage::kNone_Type);

    LocalCoords localCoords(LocalCoords::kHasExplicit_Type);
    return GrDefaultGeoProcFactory::Make(color, coverage, localCoords, SkMatrix::I());
}

static int tesselate_region(intptr_t vertices,
                            size_t vertexStride,
                            GrColor color,
                            const SkMatrix& viewMatrix,
                            const SkRegion& region) {
    SkRegion::Iterator iter(region);

    intptr_t verts = vertices;
    while (!iter.done()) {
        SkIRect rect = iter.rect();
        SkPoint* position = (SkPoint*) verts;
        position->setIRectFan(rect.fLeft, rect.fTop, rect.fRight, rect.fBottom, vertexStride);

        static const int kLocalOffset = sizeof(SkPoint) + sizeof(GrColor);
        SkPoint* localPosition = (SkPoint*) (verts + kLocalOffset);
        localPosition->setIRectFan(rect.fLeft, rect.fTop, rect.fRight, rect.fBottom, vertexStride);

        static const int kColorOffset = sizeof(SkPoint);
        GrColor* vertColor = reinterpret_cast<GrColor*>(verts + kColorOffset);
        for (int i = 0; i < kVertsPerInstance; i++) {
            *vertColor = color;
            vertColor = (GrColor*) ((intptr_t) vertColor + vertexStride);
        }

        verts += vertexStride * kVertsPerInstance;
        iter.next();
    }

    SkPoint* positions = reinterpret_cast<SkPoint*>(vertices);
    int numRects = region.computeRegionComplexity();
    SkMatrixPriv::MapPointsWithStride(viewMatrix, positions, vertexStride,
                                      numRects * kVertsPerInstance);

    return numRects;
}

class RegionBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID

    RegionBatch(GrColor color, const SkMatrix& viewMatrix, const SkRegion& region)
            : INHERITED(ClassID()) {

        RegionInfo& info = fRegions.push_back();
        info.fColor = color;
        info.fViewMatrix = viewMatrix;
        info.fRegion = region;

        SkRect bounds = SkRect::Make(region.getBounds());
        this->setTransformedBounds(bounds, viewMatrix, HasAABloat::kNo, IsZeroArea::kNo);
    }

    const char* name() const override { return "GrRegionBatch"; }

    SkString dumpInfo() const override {
        SkString str;
        str.appendf("# batched: %d\n", fRegions.count());
        for (int i = 0; i < fRegions.count(); ++i) {
            const RegionInfo& info = fRegions[i];
            str.appendf("%d: Color: 0x%08x, Region with %d rects\n",
                        i, info.fColor, info.fRegion.computeRegionComplexity());
        }
        str.append(INHERITED::dumpInfo());
        return str;
    }

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        // When this is called on a batch, there is only one region.
        color->setKnownFourComponents(fRegions[0].fColor);
        coverage->setKnownSingleComponent(0xff);
    }

    void initBatchTracker(const GrXPOverridesForBatch& overrides) override {
        overrides.getOverrideColorIfSet(&fRegions[0].fColor);
        fOverrides = overrides;
    }

private:

    void onPrepareDraws(Target* target) const override {
        sk_sp<GrGeometryProcessor> gp = make_gp(fOverrides.readsCoverage());
        if (!gp) {
            SkDebugf("Couldn't create GrGeometryProcessor\n");
            return;
        }
        SkASSERT(gp->getVertexStride() ==
                sizeof(GrDefaultGeoProcFactory::PositionColorLocalCoordAttr));

        int numRegions = fRegions.count();
        int numRects = 0;
        for (int i = 0; i < numRegions; i++) {
            numRects += fRegions[i].fRegion.computeRegionComplexity();
        }

        size_t vertexStride = gp->getVertexStride();
        SkAutoTUnref<const GrBuffer> indexBuffer(target->resourceProvider()->refQuadIndexBuffer());
        InstancedHelper helper;
        void* vertices = helper.init(target, kTriangles_GrPrimitiveType, vertexStride,
                                     indexBuffer, kVertsPerInstance, kIndicesPerInstance, numRects);
        if (!vertices || !indexBuffer) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        intptr_t verts = reinterpret_cast<intptr_t>(vertices);
        for (int i = 0; i < numRegions; i++) {
            int numRectsInRegion = tesselate_region(verts, vertexStride, fRegions[i].fColor,
                                                    fRegions[i].fViewMatrix, fRegions[i].fRegion);
            verts += numRectsInRegion * kVertsPerInstance * vertexStride;
        }
        helper.recordDraw(target, gp.get());
    }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override {
        RegionBatch* that = t->cast<RegionBatch>();
        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                    that->bounds(), caps)) {
            return false;
        }

        fRegions.push_back_n(that->fRegions.count(), that->fRegions.begin());
        this->joinBounds(*that);
        return true;
    }

    struct RegionInfo {
        GrColor fColor;
        SkMatrix fViewMatrix;
        SkRegion fRegion;
    };

    GrXPOverridesForBatch fOverrides;
    SkSTArray<1, RegionInfo, true> fRegions;

    typedef GrVertexBatch INHERITED;
};

namespace GrRegionBatch {

GrDrawBatch* Create(GrColor color,
                    const SkMatrix& viewMatrix,
                    const SkRegion& region) {
    return new RegionBatch(color, viewMatrix, region);
}

};
