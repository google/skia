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
#include "SkMatrixPriv.h"
#include "SkPointPriv.h"
#include "SkRect.h"
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLVarying.h"

namespace {

class LatticeGP : public GrGeometryProcessor {
public:
    struct Vertex {
        SkPoint fPosition;
        SkPoint fTextureCoords;
        SkRect fTextureDomain;
        GrColor fColor;
    };

    static sk_sp<GrGeometryProcessor> Make(sk_sp<GrTextureProxy> proxy,
                                           sk_sp<GrColorSpaceXform> csxf,
                                           GrSamplerState::Filter filter) {
        return sk_sp<GrGeometryProcessor>(new LatticeGP(std::move(proxy), std::move(csxf), filter));
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
                this->writeOutputPosition(args.fVertBuilder, gpArgs, latticeGP.kPositions.name());
                this->emitTransforms(args.fVertBuilder,
                                     args.fVaryingHandler,
                                     args.fUniformHandler,
                                     latticeGP.kTextureCoords.asShaderVar(),
                                     args.fFPCoordTransformHandler);
                args.fFragBuilder->codeAppend("float2 textureCoords;");
                args.fVaryingHandler->addPassThroughAttribute(latticeGP.kTextureCoords,
                                                              "textureCoords");
                args.fFragBuilder->codeAppend("float4 textureDomain;");
                args.fVaryingHandler->addPassThroughAttribute(
                        latticeGP.kTextureDomain, "textureDomain", Interpolation::kCanBeFlat);
                args.fVaryingHandler->addPassThroughAttribute(latticeGP.kColors, args.fOutputColor,
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
    LatticeGP(sk_sp<GrTextureProxy> proxy, sk_sp<GrColorSpaceXform> csxf,
              GrSamplerState::Filter filter)
            : INHERITED(kLatticeGP_ClassID), fColorSpaceXform(std::move(csxf)) {
        fSampler.reset(std::move(proxy), filter);
        this->addTextureSampler(&fSampler);
        this->setVertexAttributeCnt(4);
    }

    const Attribute& onVertexAttribute(int i) const override {
        return IthAttribute(i, kPositions, kTextureCoords, kTextureDomain, kColors);
    }

    static constexpr Attribute kPositions = {"position", kFloat2_GrVertexAttribType};
    static constexpr Attribute kTextureCoords = {"textureCoords", kFloat2_GrVertexAttribType};
    static constexpr Attribute kTextureDomain = {"textureDomain", kFloat4_GrVertexAttribType};
    static constexpr Attribute kColors = {"color", kUByte4_norm_GrVertexAttribType};

    sk_sp<GrColorSpaceXform> fColorSpaceXform;
    TextureSampler fSampler;

    typedef GrGeometryProcessor INHERITED;
};

constexpr GrPrimitiveProcessor::Attribute LatticeGP::kPositions;
constexpr GrPrimitiveProcessor::Attribute LatticeGP::kTextureCoords;
constexpr GrPrimitiveProcessor::Attribute LatticeGP::kTextureDomain;
constexpr GrPrimitiveProcessor::Attribute LatticeGP::kColors;

class NonAALatticeOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelper;

public:
    DEFINE_OP_CLASS_ID

    static const int kVertsPerRect = 4;
    static const int kIndicesPerRect = 6;

    static std::unique_ptr<GrDrawOp> Make(GrContext* context,
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

    NonAALatticeOp(Helper::MakeArgs& helperArgs, GrColor color, const SkMatrix& viewMatrix,
                   sk_sp<GrTextureProxy> proxy, sk_sp<GrColorSpaceXform> colorSpaceXform,
                   GrSamplerState::Filter filter, std::unique_ptr<SkLatticeIter> iter,
                   const SkRect& dst)
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
    }

    const char* name() const override { return "NonAALatticeOp"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        func(fProxy.get());
        fHelper.visitProxies(func);
    }

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
        auto opaque = GrColorIsOpaque(fPatches[0].fColor) && GrPixelConfigIsOpaque(fProxy->config())
                              ? GrProcessorAnalysisColor::Opaque::kYes
                              : GrProcessorAnalysisColor::Opaque::kNo;
        auto analysisColor = GrProcessorAnalysisColor(opaque);
        auto result = fHelper.xpRequiresDstTexture(
                caps, clip, GrProcessorAnalysisCoverage::kNone, &analysisColor);
        analysisColor.isConstant(&fPatches[0].fColor);
        return result;
    }

private:
    void onPrepareDraws(Target* target) override {
        auto gp = LatticeGP::Make(fProxy, fColorSpaceXform, fFilter);
        if (!gp) {
            SkDebugf("Couldn't create GrGeometryProcessor\n");
            return;
        }

        static constexpr size_t kVertexStide =
                sizeof(SkPoint) + sizeof(SkPoint) + sizeof(SkRect) + sizeof(uint32_t);
        SkASSERT(kVertexStide == gp->debugOnly_vertexStride());

        int patchCnt = fPatches.count();
        int numRects = 0;
        for (int i = 0; i < patchCnt; i++) {
            numRects += fPatches[i].fIter->numRectsToDraw();
        }

        if (!numRects) {
            return;
        }

        sk_sp<const GrBuffer> indexBuffer = target->resourceProvider()->refQuadIndexBuffer();
        PatternHelper helper(GrPrimitiveType::kTriangles);
        void* vertices = helper.init(target, kVertexStide, indexBuffer.get(), kVertsPerRect,
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

            SkIRect srcR;
            SkRect dstR;
            intptr_t patchVerts = verts;
            Sk4f scales(1.f / fProxy->width(), 1.f / fProxy->height(),
                        1.f / fProxy->width(), 1.f / fProxy->height());
            static const Sk4f kDomainOffsets(0.5f, 0.5f, -0.5f, -0.5f);
            static const Sk4f kFlipOffsets(0.f, 1, 0.f, 1.f);
            static const Sk4f kFlipMuls(1.f, -1.f, 1.f, -1.f);
            while (patch.fIter->next(&srcR, &dstR)) {
                auto vertices = reinterpret_cast<LatticeGP::Vertex*>(verts);
                SkPointPriv::SetRectTriStrip(&vertices->fPosition, dstR, kVertexStide);
                Sk4f coords(SkIntToScalar(srcR.fLeft), SkIntToScalar(srcR.fTop),
                            SkIntToScalar(srcR.fRight), SkIntToScalar(srcR.fBottom));
                Sk4f domain = coords + kDomainOffsets;
                coords *= scales;
                domain *= scales;
                if (fProxy->origin() == kBottomLeft_GrSurfaceOrigin) {
                    coords = kFlipMuls * coords + kFlipOffsets;
                    domain = SkNx_shuffle<0, 3, 2, 1>(kFlipMuls * domain + kFlipOffsets);
                }
                SkPointPriv::SetRectTriStrip(&vertices->fTextureCoords, coords[0], coords[1],
                                             coords[2], coords[3], kVertexStide);
                for (int j = 0; j < kVertsPerRect; ++j) {
                    vertices[j].fTextureDomain = {domain[0], domain[1], domain[2], domain[3]};
                }

                for (int j = 0; j < kVertsPerRect; ++j) {
                    vertices[j].fColor = patch.fColor;
                }
                verts += kVertsPerRect * kVertexStide;
            }

            // If we didn't handle it above, apply the matrix here.
            if (!isScaleTranslate) {
                SkPoint* positions = reinterpret_cast<SkPoint*>(patchVerts);
                SkMatrixPriv::MapPointsWithStride(patch.fViewMatrix, positions, kVertexStide,
                                                  kVertsPerRect * patch.fIter->numRectsToDraw());
            }
        }
        auto pipe = fHelper.makePipeline(target);
        helper.recordDraw(target, gp.get(), pipe.fPipeline, pipe.fFixedDynamicState);
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        NonAALatticeOp* that = t->cast<NonAALatticeOp>();
        if (fProxy != that->fProxy) {
            return false;
        }
        if (fFilter != that->fFilter) {
            return false;
        }
        if (GrColorSpaceXform::Equals(fColorSpaceXform.get(), that->fColorSpaceXform.get())) {
            return false;
        }
        if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
            return false;
        }

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
    sk_sp<GrTextureProxy> fProxy;
    sk_sp<GrColorSpaceXform> fColorSpaceXform;
    GrSamplerState::Filter fFilter;

    typedef GrMeshDrawOp INHERITED;
};

}  // anonymous namespace

namespace GrLatticeOp {
std::unique_ptr<GrDrawOp> MakeNonAA(GrContext* context,
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
#include "GrContextPriv.h"
#include "GrProxyProvider.h"

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
    auto proxy = context->contextPriv().proxyProvider()->createProxy(
            desc, origin, SkBackingFit::kExact, SkBudgeted::kYes);

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
