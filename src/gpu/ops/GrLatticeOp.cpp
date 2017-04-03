/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrLatticeOp.h"

#include "GrDefaultGeoProcFactory.h"
#include "GrMeshDrawOp.h"
#include "GrOpFlushState.h"
#include "GrResourceProvider.h"
#include "SkBitmap.h"
#include "SkLatticeIter.h"
#include "SkRect.h"

static sk_sp<GrGeometryProcessor> create_gp() {
    using namespace GrDefaultGeoProcFactory;
    return GrDefaultGeoProcFactory::Make(Color::kPremulGrColorAttribute_Type, Coverage::kSolid_Type,
                                         LocalCoords::kHasExplicit_Type, SkMatrix::I());
}

class NonAALatticeOp final : public GrLegacyMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static const int kVertsPerRect = 4;
    static const int kIndicesPerRect = 6;

    NonAALatticeOp(GrColor color, const SkMatrix& viewMatrix, int imageWidth, int imageHeight,
                   std::unique_ptr<SkLatticeIter> iter, const SkRect& dst)
            : INHERITED(ClassID()) {
        Patch& patch = fPatches.push_back();
        patch.fViewMatrix = viewMatrix;
        patch.fColor = color;
        patch.fIter = std::move(iter);
        patch.fDst = dst;

        fImageWidth = imageWidth;
        fImageHeight = imageHeight;

        // setup bounds
        this->setTransformedBounds(patch.fDst, viewMatrix, HasAABloat::kNo, IsZeroArea::kNo);
    }

    const char* name() const override { return "NonAALatticeOp"; }

    SkString dumpInfo() const override {
        SkString str;

        for (int i = 0; i < fPatches.count(); ++i) {
            str.appendf("%d: Color: 0x%08x Dst [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n", i,
                        fPatches[i].fColor, fPatches[i].fDst.fLeft, fPatches[i].fDst.fTop,
                        fPatches[i].fDst.fRight, fPatches[i].fDst.fBottom);
        }

        str.append(DumpPipelineInfo(*this->pipeline()));
        str.append(INHERITED::dumpInfo());
        return str;
    }

private:
    void getProcessorAnalysisInputs(GrProcessorAnalysisColor* color,
                                    GrProcessorAnalysisCoverage* coverage) const override {
        color->setToUnknown();
        *coverage = GrProcessorAnalysisCoverage::kNone;
    }

    void applyPipelineOptimizations(const PipelineOptimizations& analysioptimizations) override {
        analysioptimizations.getOverrideColorIfSet(&fPatches[0].fColor);
    }

    void onPrepareDraws(Target* target) const override {
        sk_sp<GrGeometryProcessor> gp(create_gp());
        if (!gp) {
            SkDebugf("Couldn't create GrGeometryProcessor\n");
            return;
        }

        size_t vertexStride = gp->getVertexStride();
        int patchCnt = fPatches.count();
        int numRects = 0;
        for (int i = 0; i < patchCnt; i++) {
            numRects += fPatches[i].fIter->numRectsToDraw();
        }

        sk_sp<const GrBuffer> indexBuffer(target->resourceProvider()->refQuadIndexBuffer());
        InstancedHelper helper;
        void* vertices = helper.init(target, kTriangles_GrPrimitiveType, vertexStride,
                                     indexBuffer.get(), kVertsPerRect, kIndicesPerRect, numRects);
        if (!vertices || !indexBuffer) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        intptr_t verts = reinterpret_cast<intptr_t>(vertices);
        for (int i = 0; i < patchCnt; i++) {
            const Patch& patch = fPatches[i];

            // Apply the view matrix here if it is scale-translate.  Otherwise, we need to
            // wait until we've created the dst rects.
            bool isScaleTranslate = patch.fViewMatrix.isScaleTranslate();
            if (isScaleTranslate) {
                patch.fIter->mapDstScaleTranslate(patch.fViewMatrix);
            }

            SkRect srcR, dstR;
            intptr_t patchVerts = verts;
            while (patch.fIter->next(&srcR, &dstR)) {
                SkPoint* positions = reinterpret_cast<SkPoint*>(verts);
                positions->setRectFan(dstR.fLeft, dstR.fTop, dstR.fRight, dstR.fBottom,
                                      vertexStride);

                // Setup local coords
                static const int kLocalOffset = sizeof(SkPoint) + sizeof(GrColor);
                SkPoint* coords = reinterpret_cast<SkPoint*>(verts + kLocalOffset);
                coords->setRectFan(srcR.fLeft, srcR.fTop, srcR.fRight, srcR.fBottom, vertexStride);

                static const int kColorOffset = sizeof(SkPoint);
                GrColor* vertColor = reinterpret_cast<GrColor*>(verts + kColorOffset);
                for (int j = 0; j < 4; ++j) {
                    *vertColor = patch.fColor;
                    vertColor = (GrColor*)((intptr_t)vertColor + vertexStride);
                }
                verts += kVertsPerRect * vertexStride;
            }

            // If we didn't handle it above, apply the matrix here.
            if (!isScaleTranslate) {
                SkPoint* positions = reinterpret_cast<SkPoint*>(patchVerts);
                patch.fViewMatrix.mapPointsWithStride(
                        positions, vertexStride, kVertsPerRect * patch.fIter->numRectsToDraw());
            }
        }
        helper.recordDraw(target, gp.get(), this->pipeline());
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        NonAALatticeOp* that = t->cast<NonAALatticeOp>();
        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                    that->bounds(), caps)) {
            return false;
        }

        SkASSERT(this->fImageWidth == that->fImageWidth &&
                 this->fImageHeight == that->fImageHeight);

        fPatches.move_back_n(that->fPatches.count(), that->fPatches.begin());
        this->joinBounds(*that);
        return true;
    }

    struct Patch {
        SkMatrix fViewMatrix;
        std::unique_ptr<SkLatticeIter> fIter;
        SkRect fDst;
        GrColor fColor;
    };

    int fImageWidth;
    int fImageHeight;
    SkSTArray<1, Patch, true> fPatches;

    typedef GrLegacyMeshDrawOp INHERITED;
};

namespace GrLatticeOp {
std::unique_ptr<GrLegacyMeshDrawOp> MakeNonAA(GrColor color, const SkMatrix& viewMatrix,
                                              int imageWidth, int imageHeight,
                                              std::unique_ptr<SkLatticeIter> iter,
                                              const SkRect& dst) {
    return std::unique_ptr<GrLegacyMeshDrawOp>(
            new NonAALatticeOp(color, viewMatrix, imageWidth, imageHeight, std::move(iter), dst));
}
};
