/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkRect.h"
#include "src/core/SkLatticeIter.h"
#include "src/core/SkMatrixPriv.h"
#include "src/gpu/GrDefaultGeoProcFactory.h"
#include "src/gpu/GrDrawOpTest.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrResourceProviderPriv.h"
#include "src/gpu/GrVertexWriter.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/glsl/GrGLSLColorSpaceXformHelper.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/ops/GrLatticeOp.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"

namespace {

class LatticeGP : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Make(SkArenaAlloc* arena,
                                     const GrSurfaceProxyView& view,
                                     sk_sp<GrColorSpaceXform> csxf,
                                     GrSamplerState::Filter filter,
                                     bool wideColor) {
        return arena->make<LatticeGP>(view, std::move(csxf), filter, wideColor);
    }

    const char* name() const override { return "LatticeGP"; }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        b->add32(GrColorSpaceXform::XformKey(fColorSpaceXform.get()));
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps& caps) const override {
        class GLSLProcessor : public GrGLSLGeometryProcessor {
        public:
            void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& proc,
                         const CoordTransformRange& transformRange) override {
                const auto& latticeGP = proc.cast<LatticeGP>();
                this->setTransformDataHelper(SkMatrix::I(), pdman, transformRange);
                fColorSpaceXformHelper.setData(pdman, latticeGP.fColorSpaceXform.get());
            }

        private:
            void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
                using Interpolation = GrGLSLVaryingHandler::Interpolation;
                const auto& latticeGP = args.fGP.cast<LatticeGP>();
                fColorSpaceXformHelper.emitCode(args.fUniformHandler,
                                                latticeGP.fColorSpaceXform.get());

                args.fVaryingHandler->emitAttributes(latticeGP);
                this->writeOutputPosition(args.fVertBuilder, gpArgs, latticeGP.fInPosition.name());
                this->emitTransforms(args.fVertBuilder,
                                     args.fVaryingHandler,
                                     args.fUniformHandler,
                                     latticeGP.fInTextureCoords.asShaderVar(),
                                     args.fFPCoordTransformHandler);
                args.fFragBuilder->codeAppend("float2 textureCoords;");
                args.fVaryingHandler->addPassThroughAttribute(latticeGP.fInTextureCoords,
                                                              "textureCoords");
                args.fFragBuilder->codeAppend("float4 textureDomain;");
                args.fVaryingHandler->addPassThroughAttribute(
                        latticeGP.fInTextureDomain, "textureDomain", Interpolation::kCanBeFlat);
                args.fVaryingHandler->addPassThroughAttribute(latticeGP.fInColor,
                                                              args.fOutputColor,
                                                              Interpolation::kCanBeFlat);
                args.fFragBuilder->codeAppendf("%s = ", args.fOutputColor);
                args.fFragBuilder->appendTextureLookupAndModulate(
                        args.fOutputColor,
                        args.fTexSamplers[0],
                        "clamp(textureCoords, textureDomain.xy, textureDomain.zw)",
                        kFloat2_GrSLType,
                        &fColorSpaceXformHelper);
                args.fFragBuilder->codeAppend(";");
                args.fFragBuilder->codeAppendf("%s = half4(1);", args.fOutputCoverage);
            }
            GrGLSLColorSpaceXformHelper fColorSpaceXformHelper;
        };
        return new GLSLProcessor;
    }

private:
    friend class ::SkArenaAlloc; // for access to ctor

    LatticeGP(const GrSurfaceProxyView& view, sk_sp<GrColorSpaceXform> csxf,
              GrSamplerState::Filter filter, bool wideColor)
            : INHERITED(kLatticeGP_ClassID)
            , fColorSpaceXform(std::move(csxf)) {

        fSampler.reset(GrSamplerState(GrSamplerState::WrapMode::kClamp, filter),
                       view.proxy()->backendFormat(), view.swizzle());
        this->setTextureSamplerCnt(1);
        fInPosition = {"position", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
        fInTextureCoords = {"textureCoords", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
        fInTextureDomain = {"textureDomain", kFloat4_GrVertexAttribType, kFloat4_GrSLType};
        fInColor = MakeColorAttribute("color", wideColor);
        this->setVertexAttributes(&fInPosition, 4);
    }

    const TextureSampler& onTextureSampler(int) const override { return fSampler; }

    Attribute fInPosition;
    Attribute fInTextureCoords;
    Attribute fInTextureDomain;
    Attribute fInColor;

    sk_sp<GrColorSpaceXform> fColorSpaceXform;
    TextureSampler fSampler;

    typedef GrGeometryProcessor INHERITED;
};

class NonAALatticeOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelper;

public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                          GrPaint&& paint,
                                          const SkMatrix& viewMatrix,
                                          GrSurfaceProxyView view,
                                          GrColorType srcColorType,
                                          sk_sp<GrColorSpaceXform> colorSpaceXForm,
                                          GrSamplerState::Filter filter,
                                          std::unique_ptr<SkLatticeIter> iter,
                                          const SkRect& dst) {
        SkASSERT(view.proxy());
        return Helper::FactoryHelper<NonAALatticeOp>(context, std::move(paint), viewMatrix,
                                                     std::move(view), srcColorType,
                                                     std::move(colorSpaceXForm), filter,
                                                     std::move(iter), dst);
    }

    NonAALatticeOp(Helper::MakeArgs& helperArgs, const SkPMColor4f& color,
                   const SkMatrix& viewMatrix, GrSurfaceProxyView view,
                   GrColorType srcColorType, sk_sp<GrColorSpaceXform> colorSpaceXform,
                   GrSamplerState::Filter filter, std::unique_ptr<SkLatticeIter> iter,
                   const SkRect& dst)
            : INHERITED(ClassID())
            , fHelper(helperArgs, GrAAType::kNone)
            , fView(std::move(view))
            , fSrcColorType(srcColorType)
            , fColorSpaceXform(std::move(colorSpaceXform))
            , fFilter(filter) {
        Patch& patch = fPatches.push_back();
        patch.fViewMatrix = viewMatrix;
        patch.fColor = color;
        patch.fIter = std::move(iter);
        patch.fDst = dst;

        // setup bounds
        this->setTransformedBounds(patch.fDst, viewMatrix, HasAABloat::kNo, IsHairline::kNo);
    }

    const char* name() const override { return "NonAALatticeOp"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        bool mipped = (GrSamplerState::Filter::kMipMap == fFilter);
        func(fView.proxy(), GrMipMapped(mipped));
        fHelper.visitProxies(func);
    }

#ifdef SK_DEBUG
    SkString dumpInfo() const override {
        SkString str;

        for (int i = 0; i < fPatches.count(); ++i) {
            str.appendf("%d: Color: 0x%08x Dst [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n", i,
                        fPatches[i].fColor.toBytes_RGBA(), fPatches[i].fDst.fLeft,
                        fPatches[i].fDst.fTop, fPatches[i].fDst.fRight, fPatches[i].fDst.fBottom);
        }

        str += fHelper.dumpInfo();
        str += INHERITED::dumpInfo();
        return str;
    }
#endif

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

    GrProcessorSet::Analysis finalize(
            const GrCaps& caps, const GrAppliedClip* clip, bool hasMixedSampledCoverage,
            GrClampType clampType) override {
        auto opaque = fPatches[0].fColor.isOpaque() && !GrColorTypeHasAlpha(fSrcColorType)
                              ? GrProcessorAnalysisColor::Opaque::kYes
                              : GrProcessorAnalysisColor::Opaque::kNo;
        auto analysisColor = GrProcessorAnalysisColor(opaque);
        auto result = fHelper.finalizeProcessors(
                caps, clip, hasMixedSampledCoverage, clampType, GrProcessorAnalysisCoverage::kNone,
                &analysisColor);
        analysisColor.isConstant(&fPatches[0].fColor);
        fWideColor = !fPatches[0].fColor.fitsInBytes();
        return result;
    }

private:
    void onPrepareDraws(Target* target) override {
        auto gp = LatticeGP::Make(target->allocator(), fView, fColorSpaceXform,
                                  fFilter, fWideColor);
        if (!gp) {
            SkDebugf("Couldn't create GrGeometryProcessor\n");
            return;
        }

        int patchCnt = fPatches.count();
        int numRects = 0;
        for (int i = 0; i < patchCnt; i++) {
            numRects += fPatches[i].fIter->numRectsToDraw();
        }

        if (!numRects) {
            return;
        }

        const size_t kVertexStride = gp->vertexStride();

        QuadHelper helper(target, kVertexStride, numRects);

        GrVertexWriter vertices{helper.vertices()};
        if (!vertices.fPtr) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        for (int i = 0; i < patchCnt; i++) {
            const Patch& patch = fPatches[i];

            GrVertexColor patchColor(patch.fColor, fWideColor);

            // Apply the view matrix here if it is scale-translate.  Otherwise, we need to
            // wait until we've created the dst rects.
            bool isScaleTranslate = patch.fViewMatrix.isScaleTranslate();
            if (isScaleTranslate) {
                patch.fIter->mapDstScaleTranslate(patch.fViewMatrix);
            }

            SkIRect srcR;
            SkRect dstR;
            SkPoint* patchPositions = reinterpret_cast<SkPoint*>(vertices.fPtr);
            Sk4f scales(1.f / fView.proxy()->width(), 1.f / fView.proxy()->height(),
                        1.f / fView.proxy()->width(), 1.f / fView.proxy()->height());
            static const Sk4f kDomainOffsets(0.5f, 0.5f, -0.5f, -0.5f);
            static const Sk4f kFlipOffsets(0.f, 1.f, 0.f, 1.f);
            static const Sk4f kFlipMuls(1.f, -1.f, 1.f, -1.f);
            while (patch.fIter->next(&srcR, &dstR)) {
                Sk4f coords(SkIntToScalar(srcR.fLeft), SkIntToScalar(srcR.fTop),
                            SkIntToScalar(srcR.fRight), SkIntToScalar(srcR.fBottom));
                Sk4f domain = coords + kDomainOffsets;
                coords *= scales;
                domain *= scales;
                if (fView.origin() == kBottomLeft_GrSurfaceOrigin) {
                    coords = kFlipMuls * coords + kFlipOffsets;
                    domain = SkNx_shuffle<0, 3, 2, 1>(kFlipMuls * domain + kFlipOffsets);
                }
                SkRect texDomain;
                SkRect texCoords;
                domain.store(&texDomain);
                coords.store(&texCoords);

                vertices.writeQuad(GrVertexWriter::TriStripFromRect(dstR),
                                   GrVertexWriter::TriStripFromRect(texCoords),
                                   texDomain,
                                   patchColor);
            }

            // If we didn't handle it above, apply the matrix here.
            if (!isScaleTranslate) {
                SkMatrixPriv::MapPointsWithStride(
                    patch.fViewMatrix, patchPositions, kVertexStride,
                    GrResourceProvider::NumVertsPerNonAAQuad() * patch.fIter->numRectsToDraw());
            }
        }
        auto fixedDynamicState = target->makeFixedDynamicState(1);
        fixedDynamicState->fPrimitiveProcessorTextures[0] = fView.proxy();
        helper.recordDraw(target, gp, fixedDynamicState);
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        auto pipeline = GrSimpleMeshDrawOpHelper::CreatePipeline(flushState,
                                                                 fHelper.detachProcessorSet(),
                                                                 fHelper.pipelineFlags());

        flushState->executeDrawsAndUploadsForMeshDrawOp(this, chainBounds, pipeline);
    }

    CombineResult onCombineIfPossible(GrOp* t, SkArenaAlloc*, const GrCaps& caps) override {
        NonAALatticeOp* that = t->cast<NonAALatticeOp>();
        if (fView != that->fView) {
            return CombineResult::kCannotCombine;
        }
        if (fFilter != that->fFilter) {
            return CombineResult::kCannotCombine;
        }
        if (GrColorSpaceXform::Equals(fColorSpaceXform.get(), that->fColorSpaceXform.get())) {
            return CombineResult::kCannotCombine;
        }
        if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
            return CombineResult::kCannotCombine;
        }

        fPatches.move_back_n(that->fPatches.count(), that->fPatches.begin());
        fWideColor |= that->fWideColor;
        return CombineResult::kMerged;
    }

    struct Patch {
        SkMatrix fViewMatrix;
        std::unique_ptr<SkLatticeIter> fIter;
        SkRect fDst;
        SkPMColor4f fColor;
    };

    Helper fHelper;
    SkSTArray<1, Patch, true> fPatches;
    GrSurfaceProxyView fView;
    GrColorType fSrcColorType;
    sk_sp<GrColorSpaceXform> fColorSpaceXform;
    GrSamplerState::Filter fFilter;
    bool fWideColor;

    typedef GrMeshDrawOp INHERITED;
};

}  // anonymous namespace

namespace GrLatticeOp {
std::unique_ptr<GrDrawOp> MakeNonAA(GrRecordingContext* context,
                                    GrPaint&& paint,
                                    const SkMatrix& viewMatrix,
                                    GrSurfaceProxyView view,
                                    GrColorType srcColorType,
                                    sk_sp<GrColorSpaceXform> colorSpaceXform,
                                    GrSamplerState::Filter filter,
                                    std::unique_ptr<SkLatticeIter> iter,
                                    const SkRect& dst) {
    return NonAALatticeOp::Make(context, std::move(paint), viewMatrix, std::move(view),
                                srcColorType, std::move(colorSpaceXform), filter, std::move(iter),
                                dst);
}
};

#if GR_TEST_UTILS
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"

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
    // We loop because our random lattice code can produce an invalid lattice in the case where
    // there is a single div separator in both x and y and both are aligned with the left and top
    // edge of the image subset, respectively.
    std::unique_ptr<int[]> xdivs;
    std::unique_ptr<int[]> ydivs;
    std::unique_ptr<SkCanvas::Lattice::RectType[]> flags;
    std::unique_ptr<SkColor[]> colors;
    SkIRect subset;
    GrSurfaceDesc desc;
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    desc.fWidth = random->nextRangeU(1, 1000);
    desc.fHeight = random->nextRangeU(1, 1000);
    GrSurfaceOrigin origin = random->nextBool() ? kTopLeft_GrSurfaceOrigin
                                                : kBottomLeft_GrSurfaceOrigin;
    const GrBackendFormat format =
            context->priv().caps()->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                            GrRenderable::kNo);
    auto proxy = context->priv().proxyProvider()->createProxy(format,
                                                              desc,
                                                              GrRenderable::kNo,
                                                              1,
                                                              origin,
                                                              GrMipMapped::kNo,
                                                              SkBackingFit::kExact,
                                                              SkBudgeted::kYes,
                                                              GrProtected::kNo);

    do {
        if (random->nextBool()) {
            subset.fLeft = random->nextULessThan(desc.fWidth);
            subset.fRight = random->nextRangeU(subset.fLeft + 1, desc.fWidth);
            subset.fTop = random->nextULessThan(desc.fHeight);
            subset.fBottom = random->nextRangeU(subset.fTop + 1, desc.fHeight);
        } else {
            subset.setXYWH(0, 0, desc.fWidth, desc.fHeight);
        }
        // SkCanvas::Lattice allows bounds to be null. However, SkCanvas creates a temp Lattice with
        // a non-null bounds before creating a SkLatticeIter since SkLatticeIter requires a bounds.
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
            flags.reset(new SkCanvas::Lattice::RectType[n]);
            colors.reset(new SkColor[n]);
            for (int i = 0; i < n; ++i) {
                flags[i] = random->nextBool() ? SkCanvas::Lattice::kTransparent
                                              : SkCanvas::Lattice::kDefault;
            }
            lattice.fRectTypes = flags.get();
            lattice.fColors = colors.get();
        } else {
            lattice.fRectTypes = nullptr;
            lattice.fColors = nullptr;
        }
    } while (!SkLatticeIter::Valid(desc.fWidth, desc.fHeight, lattice));
    SkRect dst;
    dst.fLeft = random->nextRangeScalar(-2000.5f, 1000.f);
    dst.fTop = random->nextRangeScalar(-2000.5f, 1000.f);
    dst.fRight = dst.fLeft + random->nextRangeScalar(0.5f, 1000.f);
    dst.fBottom = dst.fTop + random->nextRangeScalar(0.5f, 1000.f);
    std::unique_ptr<SkLatticeIter> iter(new SkLatticeIter(lattice, dst));
    SkMatrix viewMatrix = GrTest::TestMatrixPreservesRightAngles(random);
    auto csxf = GrTest::TestColorXform(random);
    GrSamplerState::Filter filter =
            random->nextBool() ? GrSamplerState::Filter::kNearest : GrSamplerState::Filter::kBilerp;

    GrSurfaceProxyView view(
            std::move(proxy), origin,
            context->priv().caps()->getTextureSwizzle(format, GrColorType::kRGBA_8888));

    return NonAALatticeOp::Make(context, std::move(paint), viewMatrix, std::move(view),
                                GrColorType::kRGBA_8888, std::move(csxf), filter, std::move(iter),
                                dst);
}

#endif
