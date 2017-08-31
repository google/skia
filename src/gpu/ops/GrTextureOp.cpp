/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureOp.h"
#include "GrAppliedClip.h"
#include "GrDrawOpTest.h"
#include "GrGeometryProcessor.h"
#include "GrMeshDrawOp.h"
#include "GrOpFlushState.h"
#include "GrQuad.h"
#include "GrResourceProvider.h"
#include "GrShaderCaps.h"
#include "GrTexture.h"
#include "GrTextureProxy.h"
#include "SkGr.h"
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLVarying.h"

namespace {

/**
 * Geometry Processor that draws a texture modulated by a vertex color (though, this is meant to be
 * the same value across all vertices of a quad and uses flat interpolation when available). This is
 * used by TextureOp below.
 */
class TextureGeometryProcessor : public GrGeometryProcessor {
public:
    struct Vertex {
        SkPoint fPosition;
        SkPoint fTextureCoords;
        GrColor fColor;
    };
    static sk_sp<GrGeometryProcessor> Make(sk_sp<GrTextureProxy> proxy,
                                           sk_sp<GrColorSpaceXform> csxf,
                                           GrSamplerParams::FilterMode filter) {
        return sk_sp<TextureGeometryProcessor>(
                new TextureGeometryProcessor(std::move(proxy), std::move(csxf), filter));
    }

    const char* name() const override { return "TextureGeometryProcessor"; }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        b->add32(GrColorSpaceXform::XformKey(fColorSpaceXform.get()));
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps& caps) const override {
        class GLSLProcessor : public GrGLSLGeometryProcessor {
        public:
            void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& proc,
                         FPCoordTransformIter&& transformIter) override {
                const auto& textureGP = proc.cast<TextureGeometryProcessor>();
                this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
                if (fColorSpaceXformHelper.isValid()) {
                    fColorSpaceXformHelper.setData(pdman, textureGP.fColorSpaceXform.get());
                }
            }

        private:
            void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
                const auto& textureGP = args.fGP.cast<TextureGeometryProcessor>();
                fColorSpaceXformHelper.emitCode(
                        args.fUniformHandler, textureGP.fColorSpaceXform.get());
                args.fVaryingHandler->setNoPerspective();
                args.fVaryingHandler->emitAttributes(textureGP);
                this->writeOutputPosition(args.fVertBuilder, gpArgs, textureGP.fPositions.fName);
                this->emitTransforms(args.fVertBuilder,
                                     args.fVaryingHandler,
                                     args.fUniformHandler,
                                     gpArgs->fPositionVar,
                                     textureGP.fTextureCoords.fName,
                                     args.fFPCoordTransformHandler);
                if (args.fShaderCaps->flatInterpolationSupport()) {
                    args.fVaryingHandler->addFlatPassThroughAttribute(&textureGP.fColors,
                                                                      args.fOutputColor);
                } else {
                    args.fVaryingHandler->addPassThroughAttribute(&textureGP.fColors,
                                                                  args.fOutputColor);
                }
                args.fFragBuilder->codeAppend("highp float2 texCoord;");
                args.fVaryingHandler->addPassThroughAttribute(&textureGP.fTextureCoords, "texCoord",
                                                              kHigh_GrSLPrecision);
                args.fFragBuilder->codeAppendf("%s = ", args.fOutputColor);
                args.fFragBuilder->appendTextureLookupAndModulate(args.fOutputColor,
                                                                  args.fTexSamplers[0],
                                                                  "texCoord",
                                                                  kVec2f_GrSLType,
                                                                  &fColorSpaceXformHelper);
                args.fFragBuilder->codeAppend(";");
                args.fFragBuilder->codeAppendf("%s = float4(1);", args.fOutputCoverage);
            }
            GrGLSLColorSpaceXformHelper fColorSpaceXformHelper;
        };
        return new GLSLProcessor;
    }

private:
    TextureGeometryProcessor(sk_sp<GrTextureProxy> proxy, sk_sp<GrColorSpaceXform> csxf,
                             GrSamplerParams::FilterMode filter)
            : fSampler(std::move(proxy), filter), fColorSpaceXform(std::move(csxf)) {
        this->initClassID<TextureGeometryProcessor>();
        fPositions =
                this->addVertexAttrib("position", kVec2f_GrVertexAttribType, kHigh_GrSLPrecision);
        fTextureCoords = this->addVertexAttrib("textureCoords", kVec2f_GrVertexAttribType,
                                               kHigh_GrSLPrecision);
        fColors = this->addVertexAttrib("color", kVec4ub_GrVertexAttribType);
        this->addTextureSampler(&fSampler);
    }

    Attribute fPositions;
    Attribute fTextureCoords;
    Attribute fColors;
    TextureSampler fSampler;
    sk_sp<GrColorSpaceXform> fColorSpaceXform;
};

/**
 * Op that implements GrTextureOp::Make. It draws textured quads. Each quad can modulate against a
 * the texture by color. The blend with the destination is always src-over. The edges are non-AA.
 */
class TextureOp final : public GrMeshDrawOp {
public:
    static std::unique_ptr<GrDrawOp> Make(sk_sp<GrTextureProxy> proxy,
                                          GrSamplerParams::FilterMode filter, GrColor color,
                                          const SkRect srcRect, const SkRect dstRect,
                                          const SkMatrix& viewMatrix, sk_sp<GrColorSpaceXform> csxf,
                                          bool allowSRBInputs) {
        return std::unique_ptr<GrDrawOp>(new TextureOp(std::move(proxy), filter, color, srcRect,
                                                       dstRect, viewMatrix, std::move(csxf),
                                                       allowSRBInputs));
    }

    ~TextureOp() override { fFinalized ? fProxy->completedRead() : fProxy->unref(); }

    const char* name() const override { return "TextureOp"; }

    SkString dumpInfo() const override {
        SkString str;
        str.appendf("Filter: %d AllowSRGBInputs: %d\n", fFilter, fAllowSRGBInputs);
        str.appendf("# draws: %d\n", fDraws.count());
        for (int i = 0; i < fDraws.count(); ++i) {
            const Draw& draw = fDraws[i];
            str.appendf(
                    "%d: Color: 0x%08x, TexRect [L: %.2f, T: %.2f, R: %.2f, B: %.2f] Quad [(%.2f, "
                    "%.2f), (%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f)]\n",
                    i, draw.fColor, draw.fSrcRect.fLeft, draw.fSrcRect.fTop, draw.fSrcRect.fRight,
                    draw.fSrcRect.fBottom, draw.fQuad.points()[0].fX, draw.fQuad.points()[0].fY,
                    draw.fQuad.points()[1].fX, draw.fQuad.points()[1].fY, draw.fQuad.points()[2].fX,
                    draw.fQuad.points()[2].fY, draw.fQuad.points()[3].fX,
                    draw.fQuad.points()[3].fY);
        }
        str += INHERITED::dumpInfo();
        return str;
    }

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override {
        SkASSERT(!fFinalized);
        fFinalized = true;
        fProxy->addPendingRead();
        fProxy->unref();
        return RequiresDstTexture::kNo;
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }

    DEFINE_OP_CLASS_ID

private:
    TextureOp(sk_sp<GrTextureProxy> proxy, GrSamplerParams::FilterMode filter, GrColor color,
              const SkRect& srcRect, const SkRect& dstRect, const SkMatrix& viewMatrix,
              sk_sp<GrColorSpaceXform> csxf, bool allowSRGBInputs)
            : INHERITED(ClassID())
            , fProxy(proxy.release())
            , fFilter(filter)
            , fColorSpaceXform(std::move(csxf))
            , fFinalized(false)
            , fAllowSRGBInputs(allowSRGBInputs) {
        Draw& draw = fDraws.push_back();
        draw.fSrcRect = srcRect;
        draw.fColor = color;
        draw.fQuad.setFromMappedRect(dstRect, viewMatrix);
        SkRect bounds;
        bounds.setBounds(draw.fQuad.points(), 4);
        this->setBounds(bounds, HasAABloat::kNo, IsZeroArea::kNo);
    }

    void onPrepareDraws(Target* target) override {
        if (!fProxy->instantiate(target->resourceProvider())) {
            return;
        }
        sk_sp<GrGeometryProcessor> gp = TextureGeometryProcessor::Make(
                sk_ref_sp(fProxy), std::move(fColorSpaceXform), fFilter);
        GrPipeline::InitArgs args;
        args.fProxy = target->proxy();
        args.fCaps = &target->caps();
        args.fResourceProvider = target->resourceProvider();
        args.fFlags = fAllowSRGBInputs ? GrPipeline::kAllowSRGBInputs_Flag : 0;
        const GrPipeline* pipeline = target->allocPipeline(args, GrProcessorSet::MakeEmptySet(),
                                                           target->detachAppliedClip());

        using Vertex = TextureGeometryProcessor::Vertex;
        SkASSERT(gp->getVertexStride() == sizeof(Vertex));

        int vstart;
        const GrBuffer* vbuffer;
        auto vertices = (Vertex*)target->makeVertexSpace(sizeof(Vertex), 4 * fDraws.count(),
                                                         &vbuffer, &vstart);
        if (!vertices) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }
        sk_sp<const GrBuffer> ibuffer;
        if (fDraws.count() > 1) {
            ibuffer.reset(target->resourceProvider()->refQuadIndexBuffer());
            if (!ibuffer) {
                SkDebugf("Could not allocate quad indices\n");
                return;
            }
        }
        GrTexture* texture = fProxy->priv().peekTexture();
        float iw = 1.f / texture->width();
        float ih = 1.f / texture->height();
        if (fDraws.count() > 1) {
            for (int i = 0; i < fDraws.count(); ++i) {
                float tl = iw * fDraws[i].fSrcRect.fLeft;
                float tr = iw * fDraws[i].fSrcRect.fRight;
                float tt = ih * fDraws[i].fSrcRect.fTop;
                float tb = ih * fDraws[i].fSrcRect.fBottom;
                if (fProxy->origin() == kBottomLeft_GrSurfaceOrigin) {
                    tt = 1.f - tt;
                    tb = 1.f - tb;
                }
                vertices[0 + 4 * i].fPosition = fDraws[i].fQuad.points()[0];
                vertices[0 + 4 * i].fTextureCoords = {tl, tt};
                vertices[0 + 4 * i].fColor = fDraws[i].fColor;
                vertices[1 + 4 * i].fPosition = fDraws[i].fQuad.points()[1];
                vertices[1 + 4 * i].fTextureCoords = {tl, tb};
                vertices[1 + 4 * i].fColor = fDraws[i].fColor;
                vertices[2 + 4 * i].fPosition = fDraws[i].fQuad.points()[2];
                vertices[2 + 4 * i].fTextureCoords = {tr, tb};
                vertices[2 + 4 * i].fColor = fDraws[i].fColor;
                vertices[3 + 4 * i].fPosition = fDraws[i].fQuad.points()[3];
                vertices[3 + 4 * i].fTextureCoords = {tr, tt};
                vertices[3 + 4 * i].fColor = fDraws[i].fColor;
            }
            GrMesh mesh(GrPrimitiveType::kTriangles);
            mesh.setIndexedPatterned(ibuffer.get(), 6, 4, fDraws.count(),
                                     GrResourceProvider::QuadCountOfQuadBuffer());
            mesh.setVertexData(vbuffer, vstart);
            target->draw(gp.get(), pipeline, mesh);
        } else {
            float tl = iw * fDraws[0].fSrcRect.fLeft;
            float tr = iw * fDraws[0].fSrcRect.fRight;
            float tt = ih * fDraws[0].fSrcRect.fTop;
            float tb = ih * fDraws[0].fSrcRect.fBottom;
            if (fProxy->origin() == kBottomLeft_GrSurfaceOrigin) {
                tt = 1.f - tt;
                tb = 1.f - tb;
            }
            vertices[0].fPosition = fDraws[0].fQuad.points()[0];
            vertices[0].fTextureCoords = {tl, tt};
            vertices[0].fColor = fDraws[0].fColor;
            vertices[1].fPosition = fDraws[0].fQuad.points()[3];
            vertices[1].fTextureCoords = {tr, tt};
            vertices[1].fColor = fDraws[0].fColor;
            vertices[2].fPosition = fDraws[0].fQuad.points()[1];
            vertices[2].fTextureCoords = {tl, tb};
            vertices[2].fColor = fDraws[0].fColor;
            vertices[3].fPosition = fDraws[0].fQuad.points()[2];
            vertices[3].fTextureCoords = {tr, tb};
            vertices[3].fColor = fDraws[0].fColor;
            GrMesh mesh(GrPrimitiveType::kTriangleStrip);
            mesh.setNonIndexedNonInstanced(4);
            mesh.setVertexData(vbuffer, vstart);
            target->draw(gp.get(), pipeline, mesh);
        }
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        const auto* that = t->cast<TextureOp>();
        if (fProxy->uniqueID() != that->fProxy->uniqueID() || fFilter != that->fFilter ||
            !GrColorSpaceXform::Equals(fColorSpaceXform.get(), that->fColorSpaceXform.get())) {
            return false;
        }
        fDraws.push_back_n(that->fDraws.count(), that->fDraws.begin());
        this->joinBounds(*that);
        return true;
    }

    struct Draw {
        SkRect fSrcRect;
        GrQuad fQuad;
        GrColor fColor;
    };
    SkSTArray<1, Draw, true> fDraws;
    GrTextureProxy* fProxy;
    GrSamplerParams::FilterMode fFilter;
    sk_sp<GrColorSpaceXform> fColorSpaceXform;
    // Used to track whether fProxy is ref'ed or has a pending IO after finalize() is called.
    bool fFinalized : 1;
    bool fAllowSRGBInputs : 1;
    typedef GrMeshDrawOp INHERITED;
};

}  // anonymous namespace

namespace GrTextureOp {

std::unique_ptr<GrDrawOp> Make(sk_sp<GrTextureProxy> proxy, GrSamplerParams::FilterMode filter,
                               GrColor color, const SkRect& srcRect, const SkRect& dstRect,
                               const SkMatrix& viewMatrix, sk_sp<GrColorSpaceXform> csxf,
                               bool allowSRGBInputs) {
    SkASSERT(!viewMatrix.hasPerspective());
    return TextureOp::Make(std::move(proxy), filter, color, srcRect, dstRect, viewMatrix,
                           std::move(csxf), allowSRGBInputs);
}

}  // namespace GrTextureOp

#if GR_TEST_UTILS
#include "GrContext.h"

GR_DRAW_OP_TEST_DEFINE(TextureOp) {
    GrSurfaceDesc desc;
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    desc.fHeight = random->nextULessThan(90) + 10;
    desc.fWidth = random->nextULessThan(90) + 10;
    desc.fOrigin = random->nextBool() ? kTopLeft_GrSurfaceOrigin : kBottomLeft_GrSurfaceOrigin;
    SkBackingFit fit = random->nextBool() ? SkBackingFit::kApprox : SkBackingFit::kExact;
    auto proxy =
            GrSurfaceProxy::MakeDeferred(context->resourceProvider(), desc, fit, SkBudgeted::kNo);
    SkRect rect = GrTest::TestRect(random);
    SkRect srcRect;
    srcRect.fLeft = random->nextRangeScalar(0.f, proxy->width() / 2.f);
    srcRect.fRight = random->nextRangeScalar(0.f, proxy->width()) + proxy->width() / 2.f;
    srcRect.fTop = random->nextRangeScalar(0.f, proxy->height() / 2.f);
    srcRect.fBottom = random->nextRangeScalar(0.f, proxy->height()) + proxy->height() / 2.f;
    SkMatrix viewMatrix = GrTest::TestMatrixPreservesRightAngles(random);
    GrColor color = SkColorToPremulGrColor(random->nextU());
    GrSamplerParams::FilterMode filter = (GrSamplerParams::FilterMode)random->nextULessThan(
            GrSamplerParams::kMipMap_FilterMode + 1);
    auto csxf = GrTest::TestColorXform(random);
    bool allowSRGBInputs = random->nextBool();
    return GrTextureOp::Make(std::move(proxy), filter, color, srcRect, rect, viewMatrix,
                             std::move(csxf), allowSRGBInputs);
}

#endif
