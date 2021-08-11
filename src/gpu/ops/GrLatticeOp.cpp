/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrLatticeOp.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkRect.h"
#include "src/core/SkLatticeIter.h"
#include "src/core/SkMatrixPriv.h"
#include "src/gpu/GrDrawOpTest.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrResourceProviderPriv.h"
#include "src/gpu/GrVertexWriter.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/glsl/GrGLSLColorSpaceXformHelper.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
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
        return arena->make([&](void* ptr) {
            return new (ptr) LatticeGP(view, std::move(csxf), filter, wideColor);
        });
    }

    const char* name() const override { return "LatticeGP"; }

    void addToKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        b->add32(GrColorSpaceXform::XformKey(fColorSpaceXform.get()));
    }

    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const override {
        class Impl : public ProgramImpl {
        public:
            void setData(const GrGLSLProgramDataManager& pdman,
                         const GrShaderCaps&,
                         const GrGeometryProcessor& geomProc) override {
                const auto& latticeGP = geomProc.cast<LatticeGP>();
                fColorSpaceXformHelper.setData(pdman, latticeGP.fColorSpaceXform.get());
            }

        private:
            void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
                using Interpolation = GrGLSLVaryingHandler::Interpolation;
                const auto& latticeGP = args.fGeomProc.cast<LatticeGP>();
                fColorSpaceXformHelper.emitCode(args.fUniformHandler,
                                                latticeGP.fColorSpaceXform.get());

                args.fVaryingHandler->emitAttributes(latticeGP);
                WriteOutputPosition(args.fVertBuilder, gpArgs, latticeGP.fInPosition.name());
                gpArgs->fLocalCoordVar = latticeGP.fInTextureCoords.asShaderVar();

                args.fFragBuilder->codeAppend("float2 textureCoords;");
                args.fVaryingHandler->addPassThroughAttribute(
                        latticeGP.fInTextureCoords.asShaderVar(),
                        "textureCoords");
                args.fFragBuilder->codeAppend("float4 textureDomain;");
                args.fVaryingHandler->addPassThroughAttribute(
                        latticeGP.fInTextureDomain.asShaderVar(),
                        "textureDomain",
                        Interpolation::kCanBeFlat);
                args.fFragBuilder->codeAppendf("half4 %s;", args.fOutputColor);
                args.fVaryingHandler->addPassThroughAttribute(latticeGP.fInColor.asShaderVar(),
                                                              args.fOutputColor,
                                                              Interpolation::kCanBeFlat);
                args.fFragBuilder->codeAppendf("%s = ", args.fOutputColor);
                args.fFragBuilder->appendTextureLookupAndBlend(
                        args.fOutputColor,
                        SkBlendMode::kModulate,
                        args.fTexSamplers[0],
                        "clamp(textureCoords, textureDomain.xy, textureDomain.zw)",
                        &fColorSpaceXformHelper);
                args.fFragBuilder->codeAppend(";");
                args.fFragBuilder->codeAppendf("const half4 %s = half4(1);", args.fOutputCoverage);
            }

            GrGLSLColorSpaceXformHelper fColorSpaceXformHelper;
        };

        return std::make_unique<Impl>();
    }

private:
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

    using INHERITED = GrGeometryProcessor;
};

class NonAALatticeOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelper;

public:
    DEFINE_OP_CLASS_ID

    static GrOp::Owner Make(GrRecordingContext* context,
                            GrPaint&& paint,
                            const SkMatrix& viewMatrix,
                            GrSurfaceProxyView view,
                            SkAlphaType alphaType,
                            sk_sp<GrColorSpaceXform> colorSpaceXForm,
                            GrSamplerState::Filter filter,
                            std::unique_ptr<SkLatticeIter> iter,
                            const SkRect& dst) {
        SkASSERT(view.proxy());
        return Helper::FactoryHelper<NonAALatticeOp>(context, std::move(paint), viewMatrix,
                                                     std::move(view), alphaType,
                                                     std::move(colorSpaceXForm), filter,
                                                     std::move(iter), dst);
    }

    NonAALatticeOp(GrProcessorSet* processorSet, const SkPMColor4f& color,
                   const SkMatrix& viewMatrix, GrSurfaceProxyView view,
                   SkAlphaType alphaType, sk_sp<GrColorSpaceXform> colorSpaceXform,
                   GrSamplerState::Filter filter, std::unique_ptr<SkLatticeIter> iter,
                   const SkRect& dst)
            : INHERITED(ClassID())
            , fHelper(processorSet, GrAAType::kNone)
            , fView(std::move(view))
            , fAlphaType(alphaType)
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

    void visitProxies(const GrVisitProxyFunc& func) const override {
        func(fView.proxy(), GrMipmapped::kNo);
        if (fProgramInfo) {
            fProgramInfo->visitFPProxies(func);
        } else {
            fHelper.visitProxies(func);
        }
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

    GrProcessorSet::Analysis finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                      GrClampType clampType) override {
        auto opaque = fPatches[0].fColor.isOpaque() && fAlphaType == kOpaque_SkAlphaType
                              ? GrProcessorAnalysisColor::Opaque::kYes
                              : GrProcessorAnalysisColor::Opaque::kNo;
        auto analysisColor = GrProcessorAnalysisColor(opaque);
        auto result = fHelper.finalizeProcessors(caps, clip, clampType,
                                                 GrProcessorAnalysisCoverage::kNone,
                                                 &analysisColor);
        analysisColor.isConstant(&fPatches[0].fColor);
        fWideColor = !fPatches[0].fColor.fitsInBytes();
        return result;
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

        auto gp = LatticeGP::Make(arena, fView, fColorSpaceXform, fFilter, fWideColor);
        if (!gp) {
            return;
        }

        fProgramInfo = GrSimpleMeshDrawOpHelper::CreateProgramInfo(caps, arena, writeView,
                                                                   std::move(appliedClip),
                                                                   dstProxyView, gp,
                                                                   fHelper.detachProcessorSet(),
                                                                   GrPrimitiveType::kTriangles,
                                                                   renderPassXferBarriers,
                                                                   colorLoadOp,
                                                                   fHelper.pipelineFlags(),
                                                                   &GrUserStencilSettings::kUnused);
    }

    void onPrepareDraws(GrMeshDrawTarget* target) override {
        if (!fProgramInfo) {
            this->createProgramInfo(target);
            if (!fProgramInfo) {
                return;
            }
        }

        int patchCnt = fPatches.count();
        int numRects = 0;
        for (int i = 0; i < patchCnt; i++) {
            numRects += fPatches[i].fIter->numRectsToDraw();
        }

        if (!numRects) {
            return;
        }

        const size_t kVertexStride = fProgramInfo->geomProc().vertexStride();

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

                if (isScaleTranslate) {
                    vertices.writeQuad(GrVertexWriter::TriStripFromRect(dstR),
                                       GrVertexWriter::TriStripFromRect(texCoords),
                                       texDomain,
                                       patchColor);
                } else {
                    SkPoint mappedPts[4];
                    patch.fViewMatrix.mapRectToQuad(mappedPts, dstR);
                    // In the above if statement, writeQuad writes the corners as:
                    // left-top, left-bottom, right-top, right-bottom.
                    // However, mapRectToQuad returns them in the order:
                    // left-top, right-top, right-bottom, left-bottom
                    // Thus we write out the vertices to match the writeQuad path.
                    vertices.write(mappedPts[0],
                                   SkPoint::Make(texCoords.fLeft, texCoords.fTop),
                                   texDomain,
                                   patchColor);
                    vertices.write(mappedPts[3],
                                   SkPoint::Make(texCoords.fLeft, texCoords.fBottom),
                                   texDomain,
                                   patchColor);
                    vertices.write(mappedPts[1],
                                   SkPoint::Make(texCoords.fRight, texCoords.fTop),
                                   texDomain,
                                   patchColor);
                    vertices.write(mappedPts[2],
                                   SkPoint::Make(texCoords.fRight, texCoords.fBottom),
                                   texDomain,
                                   patchColor);
                }
            }
        }

        fMesh = helper.mesh();
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        if (!fProgramInfo || !fMesh) {
            return;
        }

        flushState->bindPipelineAndScissorClip(*fProgramInfo, chainBounds);
        flushState->bindTextures(fProgramInfo->geomProc(),
                                 *fView.proxy(),
                                 fProgramInfo->pipeline());
        flushState->drawMesh(*fMesh);
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

#if GR_TEST_UTILS
    SkString onDumpInfo() const override {
        SkString str;

        for (int i = 0; i < fPatches.count(); ++i) {
            str.appendf("%d: Color: 0x%08x Dst [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n", i,
                        fPatches[i].fColor.toBytes_RGBA(), fPatches[i].fDst.fLeft,
                        fPatches[i].fDst.fTop, fPatches[i].fDst.fRight, fPatches[i].fDst.fBottom);
        }

        str += fHelper.dumpInfo();
        return str;
    }
#endif

    struct Patch {
        SkMatrix fViewMatrix;
        std::unique_ptr<SkLatticeIter> fIter;
        SkRect fDst;
        SkPMColor4f fColor;
    };

    Helper fHelper;
    SkSTArray<1, Patch, true> fPatches;
    GrSurfaceProxyView fView;
    SkAlphaType fAlphaType;
    sk_sp<GrColorSpaceXform> fColorSpaceXform;
    GrSamplerState::Filter fFilter;
    bool fWideColor;

    GrSimpleMesh*  fMesh = nullptr;
    GrProgramInfo* fProgramInfo = nullptr;

    using INHERITED = GrMeshDrawOp;
};

}  // anonymous namespace

namespace GrLatticeOp {
GrOp::Owner MakeNonAA(GrRecordingContext* context,
                      GrPaint&& paint,
                      const SkMatrix& viewMatrix,
                      GrSurfaceProxyView view,
                      SkAlphaType alphaType,
                      sk_sp<GrColorSpaceXform> colorSpaceXform,
                      GrSamplerState::Filter filter,
                      std::unique_ptr<SkLatticeIter> iter,
                      const SkRect& dst) {
    return NonAALatticeOp::Make(context, std::move(paint), viewMatrix, std::move(view), alphaType,
                                std::move(colorSpaceXform), filter, std::move(iter), dst);
}
}  // namespace GrLatticeOp

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
    SkISize dims;
    dims.fWidth = random->nextRangeU(1, 1000);
    dims.fHeight = random->nextRangeU(1, 1000);
    GrSurfaceOrigin origin = random->nextBool() ? kTopLeft_GrSurfaceOrigin
                                                : kBottomLeft_GrSurfaceOrigin;
    const GrBackendFormat format =
            context->priv().caps()->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                            GrRenderable::kNo);

    auto proxy = context->priv().proxyProvider()->createProxy(format,
                                                              dims,
                                                              GrRenderable::kNo,
                                                              1,
                                                              GrMipmapped::kNo,
                                                              SkBackingFit::kExact,
                                                              SkBudgeted::kYes,
                                                              GrProtected::kNo);

    do {
        if (random->nextBool()) {
            subset.fLeft = random->nextULessThan(dims.fWidth);
            subset.fRight = random->nextRangeU(subset.fLeft + 1, dims.fWidth);
            subset.fTop = random->nextULessThan(dims.fHeight);
            subset.fBottom = random->nextRangeU(subset.fTop + 1, dims.fHeight);
        } else {
            subset.setXYWH(0, 0, dims.fWidth, dims.fHeight);
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
    } while (!SkLatticeIter::Valid(dims.fWidth, dims.fHeight, lattice));
    SkRect dst;
    dst.fLeft = random->nextRangeScalar(-2000.5f, 1000.f);
    dst.fTop = random->nextRangeScalar(-2000.5f, 1000.f);
    dst.fRight = dst.fLeft + random->nextRangeScalar(0.5f, 1000.f);
    dst.fBottom = dst.fTop + random->nextRangeScalar(0.5f, 1000.f);
    std::unique_ptr<SkLatticeIter> iter(new SkLatticeIter(lattice, dst));
    SkMatrix viewMatrix = GrTest::TestMatrixPreservesRightAngles(random);
    auto csxf = GrTest::TestColorXform(random);
    GrSamplerState::Filter filter =
            random->nextBool() ? GrSamplerState::Filter::kNearest : GrSamplerState::Filter::kLinear;

    GrSurfaceProxyView view(
            std::move(proxy), origin,
            context->priv().caps()->getReadSwizzle(format, GrColorType::kRGBA_8888));

    return NonAALatticeOp::Make(context, std::move(paint), viewMatrix, std::move(view),
                                kPremul_SkAlphaType, std::move(csxf), filter, std::move(iter), dst);
}

#endif
