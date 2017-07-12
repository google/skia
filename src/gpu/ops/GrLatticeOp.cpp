/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrLatticeOp.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrDrawOpTest.h"
#include "GrMeshDrawOp.h"
#include "GrOpFlushState.h"
#include "GrResourceProvider.h"
#include "GrSimpleMeshDrawOpHelper.h"
#include "SkBitmap.h"
#include "SkLatticeIter.h"
#include "SkRect.h"

static sk_sp<GrGeometryProcessor> create_gp() {
    using namespace GrDefaultGeoProcFactory;
    return GrDefaultGeoProcFactory::Make(Color::kPremulGrColorAttribute_Type, Coverage::kSolid_Type,
                                         LocalCoords::kHasExplicit_Type, SkMatrix::I());
}

namespace {

class NonAALatticeOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelper;

public:
    DEFINE_OP_CLASS_ID

    static const int kVertsPerRect = 4;
    static const int kIndicesPerRect = 6;

    static std::unique_ptr<GrDrawOp> Make(GrPaint&& paint, const SkMatrix& viewMatrix,
                                          int imageWidth, int imageHeight,
                                          std::unique_ptr<SkLatticeIter> iter, const SkRect& dst) {
        return Helper::FactoryHelper<NonAALatticeOp>(std::move(paint), viewMatrix, imageWidth,
                                                     imageHeight, std::move(iter), dst);
    }

    NonAALatticeOp(Helper::MakeArgs& helperArgs, GrColor color, const SkMatrix& viewMatrix,
                   int imageWidth, int imageHeight, std::unique_ptr<SkLatticeIter> iter,
                   const SkRect& dst)
            : INHERITED(ClassID()), fHelper(helperArgs, GrAAType::kNone) {
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

        str += fHelper.dumpInfo();
        str += INHERITED::dumpInfo();
        return str;
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override {
        return fHelper.xpRequiresDstTexture(caps, clip, GrProcessorAnalysisCoverage::kNone,
                                            &fPatches.front().fColor);
    }

private:
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

        if (!numRects) {
            return;
        }

        sk_sp<const GrBuffer> indexBuffer(target->resourceProvider()->refQuadIndexBuffer());
        PatternHelper helper(GrPrimitiveType::kTriangles);
        void* vertices = helper.init(target, vertexStride, indexBuffer.get(), kVertsPerRect,
                                     kIndicesPerRect, numRects);
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
        helper.recordDraw(target, gp.get(), fHelper.makePipeline(target));
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        NonAALatticeOp* that = t->cast<NonAALatticeOp>();
        if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
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

    Helper fHelper;
    SkSTArray<1, Patch, true> fPatches;
    int fImageWidth;
    int fImageHeight;

    typedef GrMeshDrawOp INHERITED;
};

}  // anonymous namespace

namespace GrLatticeOp {
std::unique_ptr<GrDrawOp> MakeNonAA(GrPaint&& paint, const SkMatrix& viewMatrix, int imageWidth,
                                    int imageHeight, std::unique_ptr<SkLatticeIter> iter,
                                    const SkRect& dst) {
    return NonAALatticeOp::Make(std::move(paint), viewMatrix, imageWidth, imageHeight,
                                std::move(iter), dst);
}
};

#if GR_TEST_UTILS

/** Randomly divides subset into count divs. */
static void init_random_divs(int divs[], int count, int subsetStart, int subsetStop,
                             SkRandom* random) {
    // Rules for lattice divs: Must be strictly increasing and in the range
    // [subsetStart, subsetStop).
    // Not terribly efficient alg for generating random divs:
    // 1) Start with minimum legal pixels between each div.
    // 2) Randomly assign the remaining pixels of the subset to divs.
    // 3) Convert from pixel counts to div offsets.

    // 1) Initially each divs[i] represents the number of pixels between
    // div i-1 and i. The initial div is allowed to be at subsetStart. There
    // must be one pixel spacing between subsequent divs.
    divs[0] = 0;
    for (int i = 1; i < count; ++i) {
        divs[i] = 1;
    }
    // 2) Assign the remaining subset pixels to fall
    int subsetLength = subsetStop - subsetStart;
    for (int i = 0; i < subsetLength - count; ++i) {
        // +1 because count divs means count+1 intervals.
        int entry = random->nextULessThan(count + 1);
        // We don't have an entry to  to store the count after the last div
        if (entry < count) {
            divs[entry]++;
        }
    }
    // 3) Now convert the counts between divs to pixel indices, incorporating the subset's offset.
    int offset = subsetStart;
    for (int i = 0; i < count; ++i) {
        divs[i] += offset;
        offset = divs[i];
    }
}

GR_DRAW_OP_TEST_DEFINE(NonAALatticeOp) {
    SkCanvas::Lattice lattice;
    int imgW, imgH;
    // We loop because our random lattice code can produce an invalid lattice in the case where
    // there is a single div separator in both x and y and both are aligned with the left and top
    // edge of the image subset, respectively.
    std::unique_ptr<int[]> xdivs;
    std::unique_ptr<int[]> ydivs;
    std::unique_ptr<SkCanvas::Lattice::Flags[]> flags;
    SkIRect subset;
    do {
        imgW = random->nextRangeU(1, 1000);
        imgH = random->nextRangeU(1, 1000);
        if (random->nextBool()) {
            subset.fLeft = random->nextULessThan(imgW);
            subset.fRight = random->nextRangeU(subset.fLeft + 1, imgW);
            subset.fTop = random->nextULessThan(imgH);
            subset.fBottom = random->nextRangeU(subset.fTop + 1, imgH);
        } else {
            subset.setXYWH(0, 0, imgW, imgH);
        }
        // SkCanvas::Lattice allows bounds to be null. However, SkCanvas creates a temp Lattice with a
        // non-null bounds before creating a SkLatticeIter since SkLatticeIter requires a bounds.
        lattice.fBounds = &subset;
        lattice.fXCount = random->nextRangeU(1, subset.width());
        lattice.fYCount = random->nextRangeU(1, subset.height());
        xdivs.reset(new int[lattice.fXCount]);
        ydivs.reset(new int[lattice.fYCount]);
        init_random_divs(xdivs.get(), lattice.fXCount, subset.fLeft, subset.fRight, random);
        init_random_divs(ydivs.get(), lattice.fYCount, subset.fTop, subset.fBottom, random);
        lattice.fXDivs = xdivs.get();
        lattice.fYDivs = ydivs.get();
        bool hasFlags = random->nextBool();
        if (hasFlags) {
            int n = (lattice.fXCount + 1) * (lattice.fYCount + 1);
            flags.reset(new SkCanvas::Lattice::Flags[n]);
            for (int i = 0; i < n; ++i) {
                flags[i] = random->nextBool() ? SkCanvas::Lattice::kTransparent_Flags
                                              : (SkCanvas::Lattice::Flags)0;
            }
            lattice.fFlags = flags.get();
        } else {
            lattice.fFlags = nullptr;
        }
    } while (!SkLatticeIter::Valid(imgW, imgH, lattice));

    SkRect dst;
    dst.fLeft = random->nextRangeScalar(-2000.5f, 1000.f);
    dst.fTop = random->nextRangeScalar(-2000.5f, 1000.f);
    dst.fRight = dst.fLeft + random->nextRangeScalar(0.5f, 1000.f);
    dst.fBottom = dst.fTop + random->nextRangeScalar(0.5f, 1000.f);
    std::unique_ptr<SkLatticeIter> iter(new SkLatticeIter(lattice, dst));
    SkMatrix viewMatrix = GrTest::TestMatrixPreservesRightAngles(random);
    return NonAALatticeOp::Make(std::move(paint), viewMatrix, imgW, imgH, std::move(iter), dst);
}

#endif
