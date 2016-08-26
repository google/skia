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

static sk_sp<GrGeometryProcessor> make_gp(bool readsCoverage, const SkMatrix& viewMatrix) {
    using namespace GrDefaultGeoProcFactory;
    Color color(Color::kAttribute_Type);
    Coverage coverage(readsCoverage ? Coverage::kSolid_Type : Coverage::kNone_Type);

    LocalCoords localCoords(LocalCoords::kUsePosition_Type);
    return GrDefaultGeoProcFactory::Make(color, coverage, localCoords, viewMatrix);
}

static void tesselate_region(intptr_t vertices,
                            size_t vertexStride,
                            GrColor color,
                            const SkRegion& region) {
    SkRegion::Iterator iter(region);

    intptr_t verts = vertices;
    while (!iter.done()) {
        SkRect rect = SkRect::Make(iter.rect());
        SkPoint* position = (SkPoint*) verts;
        position->setRectFan(rect.fLeft, rect.fTop, rect.fRight, rect.fBottom, vertexStride);

        static const int kColorOffset = sizeof(SkPoint);
        GrColor* vertColor = reinterpret_cast<GrColor*>(verts + kColorOffset);
        for (int i = 0; i < kVertsPerInstance; i++) {
            *vertColor = color;
            vertColor = (GrColor*) ((intptr_t) vertColor + vertexStride);
        }

        verts += vertexStride * kVertsPerInstance;
        iter.next();
    }
}

class RegionBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID

    RegionBatch(GrColor color, const SkMatrix& viewMatrix, const SkRegion& region)
            : INHERITED(ClassID())
            , fViewMatrix(viewMatrix)
    {
        RegionInfo& info = fRegions.push_back();
        info.fColor = color;
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
        sk_sp<GrGeometryProcessor> gp = make_gp(fOverrides.readsCoverage(), fViewMatrix);
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
            tesselate_region(verts, vertexStride, fRegions[i].fColor, fRegions[i].fRegion);
            int numRectsInRegion = fRegions[i].fRegion.computeRegionComplexity();
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
