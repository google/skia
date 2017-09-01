/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureOp.h"
#include "GrAppliedClip.h"
#include "GrCaps.h"
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
    struct MultiTextureVertex {
        SkPoint fPosition;
        int fTextureIdx;
        SkPoint fTextureCoords;
        GrColor fColor;
    };

    static constexpr int kMaxTextures = 16;
    static int SupportsMultitexture(const GrShaderCaps& caps) { return caps.integerSupport(); }

    static sk_sp<GrGeometryProcessor> Make(sk_sp<GrTextureProxy> proxies[kMaxTextures],
                                           sk_sp<GrColorSpaceXform>
                                                   csxf,
                                           GrSamplerParams::FilterMode filters[kMaxTextures],
                                           const GrShaderCaps& caps) {
        return sk_sp<TextureGeometryProcessor>(
                new TextureGeometryProcessor(proxies, std::move(csxf), filters, caps));
    }

    const char* name() const override { return "TextureGeometryProcessor"; }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        b->add32(GrColorSpaceXform::XformKey(fColorSpaceXform.get()));
        int n = 0;
        for (; n < kMaxTextures && fSamplers[n].isInitialized(); ++n) {
        }
        b->add32(n);
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
                bool multitexture = SkToBool(textureGP.fSamplers[1].isInitialized());
                if (multitexture) {
                    SkASSERT(args.fShaderCaps->integerSupport());
                    args.fFragBuilder->codeAppend("int texIdx;");
                    if (args.fShaderCaps->flatInterpolationSupport()) {
                        args.fVaryingHandler->addFlatPassThroughAttribute(&textureGP.fTextureIdx,
                                                                          "texIdx");
                    } else {
                        args.fVaryingHandler->addPassThroughAttribute(&textureGP.fTextureIdx,
                                                                      "texIdx");
                    }
                    args.fFragBuilder->codeAppend("switch (texIdx) {");
                    for (int i = 0; i < kMaxTextures && textureGP.fSamplers[i].isInitialized();
                         ++i) {
                        args.fFragBuilder->codeAppendf("case %d: %s = ", i, args.fOutputColor);
                        args.fFragBuilder->appendTextureLookupAndModulate(args.fOutputColor,
                                                                          args.fTexSamplers[i],
                                                                          "texCoord",
                                                                          kVec2f_GrSLType,
                                                                          &fColorSpaceXformHelper);
                        args.fFragBuilder->codeAppend("; break;");
                    }
                    args.fFragBuilder->codeAppend("}");
                } else {
                    args.fFragBuilder->codeAppendf("%s = ", args.fOutputColor);
                    args.fFragBuilder->appendTextureLookupAndModulate(args.fOutputColor,
                                                                      args.fTexSamplers[0],
                                                                      "texCoord",
                                                                      kVec2f_GrSLType,
                                                                      &fColorSpaceXformHelper);
                }
                args.fFragBuilder->codeAppend(";");
                args.fFragBuilder->codeAppendf("%s = float4(1);", args.fOutputCoverage);
            }
            GrGLSLColorSpaceXformHelper fColorSpaceXformHelper;
        };
        return new GLSLProcessor;
    }

private:
    TextureGeometryProcessor(sk_sp<GrTextureProxy> proxies[kMaxTextures],
                             sk_sp<GrColorSpaceXform> csxf,
                             GrSamplerParams::FilterMode filters[kMaxTextures],
                             const GrShaderCaps& caps)
            : fColorSpaceXform(std::move(csxf)) {
        fSamplers[0].reset(std::move(proxies[0]), filters[0]);
        this->addTextureSampler(&fSamplers[0]);
        bool multitexture = false;
        for (int i = 1; i < kMaxTextures && proxies[i]; ++i) {
            fSamplers[i].reset(std::move(proxies[i]), filters[i]);
            this->addTextureSampler(&fSamplers[i]);
            multitexture = true;
            SkASSERT(i < caps.maxFragmentSamplers());
        }
        // We always expect at least one texture to sample.
        SkASSERT(fSamplers[0].proxy());
        this->initClassID<TextureGeometryProcessor>();
        fPositions =
                this->addVertexAttrib("position", kVec2f_GrVertexAttribType, kHigh_GrSLPrecision);
        if (multitexture) {
            SkASSERT(caps.integerSupport());
            fTextureIdx = this->addVertexAttrib("textureIdx", kInt_GrVertexAttribType);
        }
        fTextureCoords = this->addVertexAttrib("textureCoords", kVec2f_GrVertexAttribType,
                                               kHigh_GrSLPrecision);
        fColors = this->addVertexAttrib("color", kVec4ub_GrVertexAttribType);
    }

    Attribute fPositions;
    Attribute fTextureIdx;
    Attribute fTextureCoords;
    Attribute fColors;
    TextureSampler fSamplers[kMaxTextures];
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

    ~TextureOp() override {
        if (fFinalized) {
            for (int i = 0; i < kMaxTextures && fProxies[i]; ++i) {
                fProxies[i]->completedRead();
            }
        } else {
            SkASSERT(!fProxies[1]);
            fProxies[0]->unref();
        }
    }

    const char* name() const override { return "TextureOp"; }

    SkString dumpInfo() const override {
        SkString str;
        str.appendf("AllowSRGBInputs: %d\n", fAllowSRGBInputs);
        str.appendf("# draws: %d\n", fDraws.count());
        for (int i = 0; i < kMaxTextures && fProxies[i]; ++i) {
            str.appendf("Proxy ID %d: %d, Filter: %d\n", i, fProxies[i]->uniqueID().asUInt(),
                        fFilters[i]);
        }
        for (int i = 0; i < fDraws.count(); ++i) {
            const Draw& draw = fDraws[i];
            str.appendf(
                    "%d: Color: 0x%08x, ProxyIdx: %d, TexRect [L: %.2f, T: %.2f, R: %.2f, B: %.2f] "
                    "Quad [(%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f)]\n",
                    i, draw.fColor, draw.fTextureIdx, draw.fSrcRect.fLeft, draw.fSrcRect.fTop,
                    draw.fSrcRect.fRight, draw.fSrcRect.fBottom, draw.fQuad.points()[0].fX,
                    draw.fQuad.points()[0].fY, draw.fQuad.points()[1].fX, draw.fQuad.points()[1].fY,
                    draw.fQuad.points()[2].fX, draw.fQuad.points()[2].fY, draw.fQuad.points()[3].fX,
                    draw.fQuad.points()[3].fY);
        }
        str += INHERITED::dumpInfo();
        return str;
    }

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override {
        SkASSERT(!fFinalized);
        SkASSERT(!fProxies[1]);
        fFinalized = true;
        fProxies[0]->addPendingRead();
        fProxies[0]->unref();
        return RequiresDstTexture::kNo;
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }

    DEFINE_OP_CLASS_ID

private:
    static constexpr int kMaxTextures = TextureGeometryProcessor::kMaxTextures;

    TextureOp(sk_sp<GrTextureProxy> proxy, GrSamplerParams::FilterMode filter, GrColor color,
              const SkRect& srcRect, const SkRect& dstRect, const SkMatrix& viewMatrix,
              sk_sp<GrColorSpaceXform> csxf, bool allowSRGBInputs)
            : INHERITED(ClassID())
            , fProxies{proxy.release()}
            , fFilters{filter}
            , fColorSpaceXform(std::move(csxf))
            , fFinalized(false)
            , fAllowSRGBInputs(allowSRGBInputs) {
        Draw& draw = fDraws.push_back();
        draw.fSrcRect = srcRect;
        draw.fTextureIdx = 0;
        draw.fColor = color;
        draw.fQuad.setFromMappedRect(dstRect, viewMatrix);
        SkRect bounds;
        bounds.setBounds(draw.fQuad.points(), 4);
        this->setBounds(bounds, HasAABloat::kNo, IsZeroArea::kNo);
    }

    void onPrepareDraws(Target* target) override {
        int n;
        sk_sp<GrTextureProxy> proxies[kMaxTextures];
        for (n = 0; n < kMaxTextures && fProxies[n]; ++n) {
            if (!fProxies[n]->instantiate(target->resourceProvider())) {
                return;
            }
            proxies[n] = sk_ref_sp(fProxies[n]);
        }

        sk_sp<GrGeometryProcessor> gp = TextureGeometryProcessor::Make(
                proxies, std::move(fColorSpaceXform), fFilters, *target->caps().shaderCaps());
        GrPipeline::InitArgs args;
        args.fProxy = target->proxy();
        args.fCaps = &target->caps();
        args.fResourceProvider = target->resourceProvider();
        args.fFlags = fAllowSRGBInputs ? GrPipeline::kAllowSRGBInputs_Flag : 0;
        const GrPipeline* pipeline = target->allocPipeline(args, GrProcessorSet::MakeEmptySet(),
                                                           target->detachAppliedClip());
        int vstart;
        const GrBuffer* vbuffer;
        void* vdata = target->makeVertexSpace(gp->getVertexStride(), 4 * fDraws.count(), &vbuffer,
                                              &vstart);
        if (!vdata) {
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
        if (fDraws.count() > 1) {
            if (1 == n) {
                SkASSERT(gp->getVertexStride() == sizeof(TextureGeometryProcessor::Vertex));
                for (int i = 0; i < fDraws.count(); ++i) {
                    auto vertices = static_cast<TextureGeometryProcessor::Vertex*>(vdata);
                    GrTexture* texture = fProxies[0]->priv().peekTexture();
                    float iw = 1.f / texture->width();
                    float ih = 1.f / texture->height();
                    float tl = iw * fDraws[i].fSrcRect.fLeft;
                    float tr = iw * fDraws[i].fSrcRect.fRight;
                    float tt = ih * fDraws[i].fSrcRect.fTop;
                    float tb = ih * fDraws[i].fSrcRect.fBottom;
                    if (fProxies[0]->origin() == kBottomLeft_GrSurfaceOrigin) {
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
            } else {
                SkASSERT(gp->getVertexStride() ==
                         sizeof(TextureGeometryProcessor::MultiTextureVertex));
                GrTexture* textures[kMaxTextures];
                float iw[kMaxTextures];
                float ih[kMaxTextures];
                for (int t = 0; t < n; ++t) {
                    textures[t] = fProxies[t]->priv().peekTexture();
                    iw[t] = 1.f / textures[t]->width();
                    ih[t] = 1.f / textures[t]->height();
                }
                for (int i = 0; i < fDraws.count(); ++i) {
                    int t = fDraws[i].fTextureIdx;
                    auto vertices =
                            static_cast<TextureGeometryProcessor::MultiTextureVertex*>(vdata);
                    float tl = iw[t] * fDraws[i].fSrcRect.fLeft;
                    float tr = iw[t] * fDraws[i].fSrcRect.fRight;
                    float tt = ih[t] * fDraws[i].fSrcRect.fTop;
                    float tb = ih[t] * fDraws[i].fSrcRect.fBottom;
                    if (fProxies[t]->origin() == kBottomLeft_GrSurfaceOrigin) {
                        tt = 1.f - tt;
                        tb = 1.f - tb;
                    }
                    vertices[0 + 4 * i].fPosition = fDraws[i].fQuad.points()[0];
                    vertices[0 + 4 * i].fTextureIdx = t;
                    vertices[0 + 4 * i].fTextureCoords = {tl, tt};
                    vertices[0 + 4 * i].fColor = fDraws[i].fColor;
                    vertices[1 + 4 * i].fPosition = fDraws[i].fQuad.points()[1];
                    vertices[1 + 4 * i].fTextureIdx = t;
                    vertices[1 + 4 * i].fTextureCoords = {tl, tb};
                    vertices[1 + 4 * i].fColor = fDraws[i].fColor;
                    vertices[2 + 4 * i].fPosition = fDraws[i].fQuad.points()[2];
                    vertices[2 + 4 * i].fTextureIdx = t;
                    vertices[2 + 4 * i].fTextureCoords = {tr, tb};
                    vertices[2 + 4 * i].fColor = fDraws[i].fColor;
                    vertices[3 + 4 * i].fPosition = fDraws[i].fQuad.points()[3];
                    vertices[3 + 4 * i].fTextureIdx = t;
                    vertices[3 + 4 * i].fTextureCoords = {tr, tt};
                    vertices[3 + 4 * i].fColor = fDraws[i].fColor;
                }
            }
            GrMesh mesh(GrPrimitiveType::kTriangles);
            mesh.setIndexedPatterned(ibuffer.get(), 6, 4, fDraws.count(),
                                     GrResourceProvider::QuadCountOfQuadBuffer());
            mesh.setVertexData(vbuffer, vstart);
            target->draw(gp.get(), pipeline, mesh);
        } else {
            // If there is only one draw then there can only be one proxy.
            SkASSERT(1 == n);
            SkASSERT(gp->getVertexStride() == sizeof(TextureGeometryProcessor::Vertex));
            auto vertices = static_cast<TextureGeometryProcessor::Vertex*>(vdata);
            GrTexture* texture = fProxies[0]->priv().peekTexture();
            float iw = 1.f / texture->width();
            float ih = 1.f / texture->height();
            float tl = iw * fDraws[0].fSrcRect.fLeft;
            float tr = iw * fDraws[0].fSrcRect.fRight;
            float tt = ih * fDraws[0].fSrcRect.fTop;
            float tb = ih * fDraws[0].fSrcRect.fBottom;
            if (fProxies[0]->origin() == kBottomLeft_GrSurfaceOrigin) {
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
        if (!GrColorSpaceXform::Equals(fColorSpaceXform.get(), that->fColorSpaceXform.get())) {
            return false;
        }
        if (TextureGeometryProcessor::SupportsMultitexture(*caps.shaderCaps())) {
            int map[kMaxTextures];
            if (!this->canMergeProxies(*that, map, *caps.shaderCaps())) {
                return false;
            }
            for (int i = 0; i < kMaxTextures && that->fProxies[i]; ++i) {
                if (map[i] < 0) {
                    SkASSERT(!fProxies[-map[i]]);
                    that->fProxies[i]->addPendingRead();
                    fProxies[-map[i]] = that->fProxies[i];
                    fFilters[-map[i]] = that->fFilters[i];
                    map[i] = -map[i];
                }
            }
            int start = fDraws.count();
            fDraws.push_back_n(that->fDraws.count(), that->fDraws.begin());
            for (int i = start; i < fDraws.count(); ++i) {
                fDraws[i].fTextureIdx = map[fDraws[i].fTextureIdx];
            }
        } else {
            if (fProxies[0]->uniqueID() != that->fProxies[0]->uniqueID() ||
                fFilters[0] != that->fFilters[0]) {
                return false;
            }
            fDraws.push_back_n(that->fDraws.count(), that->fDraws.begin());
        }
        this->joinBounds(*that);
        return true;
    }

    /**
     * Determines a mapping from that's fProxies array to this's fProxies array. A negative value
     * means that's proxy should be added to this's fProxies array at the absolute value of the map
     * entry. Returns false if the sum of the proxies exceeds either kMaxTextures or the GPU's max
     * fragment shader texture units.
     */
    bool canMergeProxies(const TextureOp& that, int map[kMaxTextures],
                         const GrShaderCaps& caps) const {
        std::fill_n(map, kMaxTextures, -kMaxTextures);
        int i;
        int actualMaxTextures = SkTMin(caps.maxFragmentImageStorages(), kMaxTextures);
        for (i = 0; i < kMaxTextures && fProxies[i]; ++i) {
            for (int j = 0; j < kMaxTextures && that.fProxies[j]; ++j) {
                if (fProxies[i]->uniqueID() == that.fProxies[j]->uniqueID()) {
                    if (fFilters[i] != that.fFilters[j]) {
                        // In GL we don't currently support using the same texture with different
                        // samplers. If we added support for sampler objects and a cap bit to know
                        // it's ok to use different filter modes then we could support this.
                        // Othwerise, we could also only allow a single filter mode for each op
                        // instance.
                        return false;
                    }
                    map[j] = i;
                    break;
                }
            }
        }
        for (int j = 0; j < kMaxTextures && that.fProxies[j]; ++j) {
            if (map[j] < 0) {
                if (i == actualMaxTextures) {
                    return false;
                }
                map[j] = -(i++);
            }
        }
        return true;
    }

    struct Draw {
        SkRect fSrcRect;
        int fTextureIdx;
        GrQuad fQuad;
        GrColor fColor;
    };
    SkSTArray<1, Draw, true> fDraws;
    GrTextureProxy* fProxies[kMaxTextures];
    GrSamplerParams::FilterMode fFilters[kMaxTextures];
    sk_sp<GrColorSpaceXform> fColorSpaceXform;
    // Used to track whether fProxy is ref'ed or has a pending IO after finalize() is called.
    bool fFinalized : 1;
    bool fAllowSRGBInputs : 1;
    typedef GrMeshDrawOp INHERITED;
};

constexpr int TextureGeometryProcessor::kMaxTextures;
constexpr int TextureOp::kMaxTextures;

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
