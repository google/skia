/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrLatticeOp.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrDrawOpTest.h"
#include "GrGpu.h"
#include "GrMeshDrawOp.h"
#include "GrOpFlushState.h"
#include "GrResourceProvider.h"
#include "GrResourceProviderPriv.h"
#include "GrSimpleMeshDrawOpHelper.h"
#include "GrVertexWriter.h"
#include "SkBitmap.h"
#include "SkLatticeIter.h"
#include "SkMatrixPriv.h"
#include "SkRect.h"
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLVarying.h"

namespace {

class LatticeGP : public GrGeometryProcessor {
public:
    static sk_sp<GrGeometryProcessor> Make(GrGpu* gpu,
                                           const GrTextureProxy* proxy,
                                           sk_sp<GrColorSpaceXform> csxf,
                                           GrSamplerState::Filter filter,
                                           bool wideColor) {
        return sk_sp<GrGeometryProcessor>(
                new LatticeGP(gpu, proxy, std::move(csxf), filter, wideColor));
    }

    const char* name() const override { return "LatticeGP"; }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        b->add32(GrColorSpaceXform::XformKey(fColorSpaceXform.get()));
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps& caps) const override {
        class GLSLProcessor : public GrGLSLGeometryProcessor {
        public:
            void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& proc,
                         FPCoordTransformIter&& transformIter) override {
                const auto& latticeGP = proc.cast<LatticeGP>();
                this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
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
    LatticeGP(GrGpu* gpu, const GrTextureProxy* proxy, sk_sp<GrColorSpaceXform> csxf,
              GrSamplerState::Filter filter, bool wideColor)
            : INHERITED(kLatticeGP_ClassID), fColorSpaceXform(std::move(csxf)) {

        GrSamplerState samplerState = GrSamplerState(GrSamplerState::WrapMode::kClamp,
                                                     filter);
        uint32_t extraSamplerKey = gpu->getExtraSamplerKeyForProgram(samplerState,
                                                                     proxy->backendFormat());

        fSampler.reset(proxy->textureType(), proxy->config(), samplerState,
                       extraSamplerKey);
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

    static const int kVertsPerRect = 4;
    static const int kIndicesPerRect = 6;

    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                          GrPaint&& paint,
                                          const SkMatrix& viewMatrix,
                                          sk_sp<GrTextureProxy> proxy,
                                          sk_sp<GrColorSpaceXform> colorSpaceXForm,
                                          GrSamplerState::Filter filter,
                                          std::unique_ptr<SkLatticeIter> iter,
                                          const SkRect& dst) {
        SkASSERT(proxy);
        return Helper::FactoryHelper<NonAALatticeOp>(context, std::move(paint), viewMatrix,
                                                     std::move(proxy),
                                                     std::move(colorSpaceXForm), filter,
                                                     std::move(iter), dst);
    }

    NonAALatticeOp(Helper::MakeArgs& helperArgs, const SkPMColor4f& color,
                   const SkMatrix& viewMatrix, sk_sp<GrTextureProxy> proxy,
                   sk_sp<GrColorSpaceXform> colorSpaceXform, GrSamplerState::Filter filter,
                   std::unique_ptr<SkLatticeIter> iter, const SkRect& dst)
            : INHERITED(ClassID())
            , fHelper(helperArgs, GrAAType::kNone)
            , fProxy(std::move(proxy))
            , fColorSpaceXform(std::move(colorSpaceXform))
            , fFilter(filter) {
        Patch& patch = fPatches.push_back();
        patch.fViewMatrix = viewMatrix;
        patch.fColor = color;
        patch.fIter = std::move(iter);
        patch.fDst = dst;

        // setup bounds
        this->setTransformedBounds(patch.fDst, viewMatrix, HasAABloat::kNo, IsZeroArea::kNo);
        fWideColor = !SkPMColor4fFitsInBytes(color);
    }

    const char* name() const override { return "NonAALatticeOp"; }

    void visitProxies(const VisitProxyFunc& func, VisitorType) const override {
        func(fProxy.get());
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

    GrProcessorSet::Analysis finalize(const GrCaps& caps, const GrAppliedClip* clip) override {
        auto opaque = fPatches[0].fColor.isOpaque() && GrPixelConfigIsOpaque(fProxy->config())
                              ? GrProcessorAnalysisColor::Opaque::kYes
                              : GrProcessorAnalysisColor::Opaque::kNo;
        auto analysisColor = GrProcessorAnalysisColor(opaque);
        auto result = fHelper.finalizeProcessors(
                caps, clip, GrProcessorAnalysisCoverage::kNone, &analysisColor);
        analysisColor.isConstant(&fPatches[0].fColor);
        return result;
    }

private:
    void onPrepareDraws(Target* target) override {
        GrGpu* gpu = target->resourceProvider()->priv().gpu();
        auto gp = LatticeGP::Make(gpu, fProxy.get(), fColorSpaceXform, fFilter, fWideColor);
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
        sk_sp<const GrBuffer> indexBuffer = target->resourceProvider()->refQuadIndexBuffer();
        if (!indexBuffer) {
            SkDebugf("Could not allocate indices\n");
            return;
        }
        PatternHelper helper(target, GrPrimitiveType::kTriangles, kVertexStride,
                             std::move(indexBuffer), kVertsPerRect, kIndicesPerRect, numRects);
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
            Sk4f scales(1.f / fProxy->width(), 1.f / fProxy->height(),
                        1.f / fProxy->width(), 1.f / fProxy->height());
            static const Sk4f kDomainOffsets(0.5f, 0.5f, -0.5f, -0.5f);
            static const Sk4f kFlipOffsets(0.f, 1.f, 0.f, 1.f);
            static const Sk4f kFlipMuls(1.f, -1.f, 1.f, -1.f);
            while (patch.fIter->next(&srcR, &dstR)) {
                Sk4f coords(SkIntToScalar(srcR.fLeft), SkIntToScalar(srcR.fTop),
                            SkIntToScalar(srcR.fRight), SkIntToScalar(srcR.fBottom));
                Sk4f domain = coords + kDomainOffsets;
                coords *= scales;
                domain *= scales;
                if (fProxy->origin() == kBottomLeft_GrSurfaceOrigin) {
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
                SkMatrixPriv::MapPointsWithStride(patch.fViewMatrix, patchPositions, kVertexStride,
                                                  kVertsPerRect * patch.fIter->numRectsToDraw());
            }
        }
        auto pipe = fHelper.makePipeline(target, 1);
        pipe.fFixedDynamicState->fPrimitiveProcessorTextures[0] = fProxy.get();
        helper.recordDraw(target, std::move(gp), pipe.fPipeline, pipe.fFixedDynamicState);
    }

    CombineResult onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        NonAALatticeOp* that = t->cast<NonAALatticeOp>();
        if (fProxy != that->fProxy) {
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
    sk_sp<GrTextureProxy> fProxy;
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
                                    sk_sp<GrTextureProxy> proxy,
                                    sk_sp<GrColorSpaceXform> colorSpaceXform,
                                    GrSamplerState::Filter filter,
                                    std::unique_ptr<SkLatticeIter> iter,
                                    const SkRect& dst) {
    return NonAALatticeOp::Make(context, std::move(paint), viewMatrix, std::move(proxy),
                                std::move(colorSpaceXform), filter, std::move(iter), dst);
}
};

#if GR_TEST_UTILS
#include "GrProxyProvider.h"
#include "GrRecordingContextPriv.h"

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
    GrSurfaceOrigin origin =
            random->nextBool() ? kTopLeft_GrSurfaceOrigin : kBottomLeft_GrSurfaceOrigin;
    const GrBackendFormat format =
            context->priv().caps()->getBackendFormatFromColorType(kRGBA_8888_SkColorType);
    auto proxy = context->priv().proxyProvider()->createProxy(
            format, desc, origin, SkBackingFit::kExact, SkBudgeted::kYes);

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
    return NonAALatticeOp::Make(context, std::move(paint), viewMatrix, std::move(proxy),
                                std::move(csxf), filter, std::move(iter), dst);
}

#endif
