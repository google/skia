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
#include "GrTexturePriv.h"
#include "GrTextureProxy.h"
#include "SkGr.h"
#include "SkMathPriv.h"
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

    // Maximum number of textures supported by this op. Must also be checked against the caps
    // limit. These numbers were based on some limited experiments on a HP Z840 and Pixel XL 2016
    // and could probably use more tuning.
#ifdef SK_BUILD_FOR_ANDROID
    static constexpr int kMaxTextures = 4;
#else
    static constexpr int kMaxTextures = 8;
#endif

    static int SupportsMultitexture(const GrShaderCaps& caps) {
        return caps.integerSupport() && !caps.disableImageMultitexturingSupport();
    }

    static sk_sp<GrGeometryProcessor> Make(sk_sp<GrTextureProxy> proxies[], int proxyCnt,
                                           sk_sp<GrColorSpaceXform> csxf,
                                           const GrSamplerState::Filter filters[],
                                           const GrShaderCaps& caps) {
        // We use placement new to avoid always allocating space for kMaxTextures TextureSampler
        // instances.
        int samplerCnt = NumSamplersToUse(proxyCnt, caps);
        size_t size = sizeof(TextureGeometryProcessor) + sizeof(TextureSampler) * (samplerCnt - 1);
        void* mem = GrGeometryProcessor::operator new(size);
        return sk_sp<TextureGeometryProcessor>(new (mem) TextureGeometryProcessor(
                proxies, proxyCnt, samplerCnt, std::move(csxf), filters, caps));
    }

    ~TextureGeometryProcessor() override {
        int cnt = this->numTextureSamplers();
        for (int i = 1; i < cnt; ++i) {
            fSamplers[i].~TextureSampler();
        }
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
                if (args.fShaderCaps->preferFlatInterpolation()) {
                    args.fVaryingHandler->addFlatPassThroughAttribute(&textureGP.fColors,
                                                                      args.fOutputColor);
                } else {
                    args.fVaryingHandler->addPassThroughAttribute(&textureGP.fColors,
                                                                  args.fOutputColor);
                }
                args.fFragBuilder->codeAppend("float2 texCoord;");
                args.fVaryingHandler->addPassThroughAttribute(&textureGP.fTextureCoords,
                                                              "texCoord");
                if (textureGP.numTextureSamplers() > 1) {
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
                    for (int i = 0; i < textureGP.numTextureSamplers(); ++i) {
                        args.fFragBuilder->codeAppendf("case %d: %s = ", i, args.fOutputColor);
                        args.fFragBuilder->appendTextureLookupAndModulate(args.fOutputColor,
                                                                          args.fTexSamplers[i],
                                                                          "texCoord",
                                                                          kFloat2_GrSLType,
                                                                          &fColorSpaceXformHelper);
                        args.fFragBuilder->codeAppend("; break;");
                    }
                    args.fFragBuilder->codeAppend("}");
                } else {
                    args.fFragBuilder->codeAppendf("%s = ", args.fOutputColor);
                    args.fFragBuilder->appendTextureLookupAndModulate(args.fOutputColor,
                                                                      args.fTexSamplers[0],
                                                                      "texCoord",
                                                                      kFloat2_GrSLType,
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
    // This exists to reduce the number of shaders generated. It does some rounding of sampler
    // counts.
    static int NumSamplersToUse(int numRealProxies, const GrShaderCaps& caps) {
        SkASSERT(numRealProxies > 0 && numRealProxies <= kMaxTextures &&
                 numRealProxies <= caps.maxFragmentSamplers());
        if (1 == numRealProxies) {
            return 1;
        }
        if (numRealProxies <= 4) {
            return 4;
        }
        // Round to the next power of 2 and then clamp to kMaxTextures and the max allowed by caps.
        return SkTMin(SkNextPow2(numRealProxies), SkTMin(kMaxTextures, caps.maxFragmentSamplers()));
    }

    TextureGeometryProcessor(sk_sp<GrTextureProxy> proxies[], int proxyCnt, int samplerCnt,
                             sk_sp<GrColorSpaceXform> csxf, const GrSamplerState::Filter filters[],
                             const GrShaderCaps& caps)
            : INHERITED(kTextureGeometryProcessor_ClassID)
            , fColorSpaceXform(std::move(csxf)) {
        SkASSERT(proxyCnt > 0 && samplerCnt >= proxyCnt);
        fPositions = this->addVertexAttrib("position", kFloat2_GrVertexAttribType);
        fSamplers[0].reset(std::move(proxies[0]), filters[0]);
        this->addTextureSampler(&fSamplers[0]);
        for (int i = 1; i < proxyCnt; ++i) {
            // This class has one sampler built in, the rest come from memory this processor was
            // placement-newed into and so haven't been constructed.
            new (&fSamplers[i]) TextureSampler(std::move(proxies[i]), filters[i]);
            this->addTextureSampler(&fSamplers[i]);
        }
        if (samplerCnt > 1) {
            // Here we initialize any extra samplers by repeating the last one samplerCnt - proxyCnt
            // times.
            GrTextureProxy* dupeProxy = fSamplers[proxyCnt - 1].proxy();
            for (int i = proxyCnt; i < samplerCnt; ++i) {
                new (&fSamplers[i]) TextureSampler(sk_ref_sp(dupeProxy), filters[proxyCnt - 1]);
                this->addTextureSampler(&fSamplers[i]);
            }
            SkASSERT(caps.integerSupport());
            fTextureIdx = this->addVertexAttrib("textureIdx", kInt_GrVertexAttribType);
        }

        fTextureCoords = this->addVertexAttrib("textureCoords", kFloat2_GrVertexAttribType);
        fColors = this->addVertexAttrib("color", kUByte4_norm_GrVertexAttribType);
    }

    Attribute fPositions;
    Attribute fTextureIdx;
    Attribute fTextureCoords;
    Attribute fColors;
    sk_sp<GrColorSpaceXform> fColorSpaceXform;
    TextureSampler fSamplers[1];

    typedef GrGeometryProcessor INHERITED;
};

/**
 * Op that implements GrTextureOp::Make. It draws textured quads. Each quad can modulate against a
 * the texture by color. The blend with the destination is always src-over. The edges are non-AA.
 */
class TextureOp final : public GrMeshDrawOp {
public:
    static std::unique_ptr<GrDrawOp> Make(sk_sp<GrTextureProxy> proxy,
                                          GrSamplerState::Filter filter, GrColor color,
                                          const SkRect srcRect, const SkRect dstRect,
                                          const SkMatrix& viewMatrix, sk_sp<GrColorSpaceXform> csxf,
                                          bool allowSRBInputs) {
        return std::unique_ptr<GrDrawOp>(new TextureOp(std::move(proxy), filter, color, srcRect,
                                                       dstRect, viewMatrix, std::move(csxf),
                                                       allowSRBInputs));
    }

    ~TextureOp() override {
        if (fFinalized) {
            auto proxies = this->proxies();
            for (int i = 0; i < fProxyCnt; ++i) {
                proxies[i]->completedRead();
            }
            if (fProxyCnt > 1) {
                delete[] reinterpret_cast<const char*>(proxies);
            }
        } else {
            SkASSERT(1 == fProxyCnt);
            fProxy0->unref();
        }
    }

    const char* name() const override { return "TextureOp"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        auto proxies = this->proxies();
        for (int i = 0; i < fProxyCnt; ++i) {
            func(proxies[i]);
        }
    }

    SkString dumpInfo() const override {
        SkString str;
        str.appendf("AllowSRGBInputs: %d\n", fAllowSRGBInputs);
        str.appendf("# draws: %d\n", fDraws.count());
        auto proxies = this->proxies();
        for (int i = 0; i < fProxyCnt; ++i) {
            str.appendf("Proxy ID %d: %d, Filter: %d\n", i, proxies[i]->uniqueID().asUInt(),
                        static_cast<int>(this->filters()[i]));
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

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                GrPixelConfigIsClamped dstIsClamped) override {
        SkASSERT(!fFinalized);
        SkASSERT(1 == fProxyCnt);
        fFinalized = true;
        fProxy0->addPendingRead();
        fProxy0->unref();
        return RequiresDstTexture::kNo;
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }

    DEFINE_OP_CLASS_ID

private:
    static constexpr int kMaxTextures = TextureGeometryProcessor::kMaxTextures;

    TextureOp(sk_sp<GrTextureProxy> proxy, GrSamplerState::Filter filter, GrColor color,
              const SkRect& srcRect, const SkRect& dstRect, const SkMatrix& viewMatrix,
              sk_sp<GrColorSpaceXform> csxf, bool allowSRGBInputs)
            : INHERITED(ClassID())
            , fColorSpaceXform(std::move(csxf))
            , fProxy0(proxy.release())
            , fFilter0(filter)
            , fProxyCnt(1)
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
        sk_sp<GrTextureProxy> proxiesSPs[kMaxTextures];
        auto proxies = this->proxies();
        auto filters = this->filters();
        for (int i = 0; i < fProxyCnt; ++i) {
            if (!proxies[i]->instantiate(target->resourceProvider())) {
                return;
            }
            proxiesSPs[i] = sk_ref_sp(proxies[i]);
        }

        sk_sp<GrGeometryProcessor> gp =
                TextureGeometryProcessor::Make(proxiesSPs, fProxyCnt, std::move(fColorSpaceXform),
                                               filters, *target->caps().shaderCaps());
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
            ibuffer = target->resourceProvider()->refQuadIndexBuffer();
            if (!ibuffer) {
                SkDebugf("Could not allocate quad indices\n");
                return;
            }
            if (1 == fProxyCnt) {
                SkASSERT(gp->getVertexStride() == sizeof(TextureGeometryProcessor::Vertex));
                for (int i = 0; i < fDraws.count(); ++i) {
                    auto vertices = static_cast<TextureGeometryProcessor::Vertex*>(vdata);
                    GrTexture* texture = proxies[0]->priv().peekTexture();
                    float iw = 1.f / texture->width();
                    float ih = 1.f / texture->height();
                    float tl = iw * fDraws[i].fSrcRect.fLeft;
                    float tr = iw * fDraws[i].fSrcRect.fRight;
                    float tt = ih * fDraws[i].fSrcRect.fTop;
                    float tb = ih * fDraws[i].fSrcRect.fBottom;
                    if (proxies[0]->origin() == kBottomLeft_GrSurfaceOrigin) {
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
                for (int t = 0; t < fProxyCnt; ++t) {
                    textures[t] = proxies[t]->priv().peekTexture();
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
                    if (proxies[t]->origin() == kBottomLeft_GrSurfaceOrigin) {
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
            SkASSERT(1 == fProxyCnt);
            SkASSERT(gp->getVertexStride() == sizeof(TextureGeometryProcessor::Vertex));
            auto vertices = static_cast<TextureGeometryProcessor::Vertex*>(vdata);
            GrTexture* texture = proxies[0]->priv().peekTexture();
            float iw = 1.f / texture->width();
            float ih = 1.f / texture->height();
            float tl = iw * fDraws[0].fSrcRect.fLeft;
            float tr = iw * fDraws[0].fSrcRect.fRight;
            float tt = ih * fDraws[0].fSrcRect.fTop;
            float tb = ih * fDraws[0].fSrcRect.fBottom;
            if (proxies[0]->origin() == kBottomLeft_GrSurfaceOrigin) {
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
        // Because of an issue where GrColorSpaceXform adds the same function every time it is used
        // in a texture lookup, we only allow multiple textures when there is no transform.
        if (TextureGeometryProcessor::SupportsMultitexture(*caps.shaderCaps()) &&
            !fColorSpaceXform) {
            int map[kMaxTextures];
            int numNewProxies = this->mergeProxies(that, map, *caps.shaderCaps());
            if (numNewProxies < 0) {
                return false;
            }
            if (1 == fProxyCnt && numNewProxies) {
                void* mem = new char[(sizeof(GrSamplerState::Filter) + sizeof(GrTextureProxy*)) *
                                     kMaxTextures];
                auto proxies = reinterpret_cast<GrTextureProxy**>(mem);
                auto filters = reinterpret_cast<GrSamplerState::Filter*>(proxies + kMaxTextures);
                proxies[0] = fProxy0;
                filters[0] = fFilter0;
                fProxyArray = proxies;
            }
            fProxyCnt += numNewProxies;
            auto thisProxies = fProxyArray;
            auto thatProxies = that->proxies();
            auto thatFilters = that->filters();
            auto thisFilters = reinterpret_cast<GrSamplerState::Filter*>(thisProxies +
                    kMaxTextures);
            for (int i = 0; i < that->fProxyCnt; ++i) {
                if (map[i] < 0) {
                    thatProxies[i]->addPendingRead();

                    thisProxies[-map[i]] = thatProxies[i];
                    thisFilters[-map[i]] = thatFilters[i];
                    map[i] = -map[i];
                }
            }
            int firstNewDraw = fDraws.count();
            fDraws.push_back_n(that->fDraws.count(), that->fDraws.begin());
            for (int i = firstNewDraw; i < fDraws.count(); ++i) {
                fDraws[i].fTextureIdx = map[fDraws[i].fTextureIdx];
            }
        } else {
            if (fProxy0->uniqueID() != that->fProxy0->uniqueID() || fFilter0 != that->fFilter0) {
                return false;
            }
            fDraws.push_back_n(that->fDraws.count(), that->fDraws.begin());
        }
        this->joinBounds(*that);
        return true;
    }

    /**
     * Determines a mapping of indices from that's proxy array to this's proxy array. A negative map
     * value means that's proxy should be added to this's proxy array at the absolute value of
     * the map entry. If it is determined that the ops shouldn't combine their proxies then a
     * negative value is returned. Otherwise, return value indicates the number of proxies that have
     * to be added to this op or, equivalently, the number of negative entries in map.
     */
    int mergeProxies(const TextureOp* that, int map[kMaxTextures], const GrShaderCaps& caps) const {
        std::fill_n(map, kMaxTextures, -kMaxTextures);
        int sharedProxyCnt = 0;
        auto thisProxies = this->proxies();
        auto thisFilters = this->filters();
        auto thatProxies = that->proxies();
        auto thatFilters = that->filters();
        for (int i = 0; i < fProxyCnt; ++i) {
            for (int j = 0; j < that->fProxyCnt; ++j) {
                if (thisProxies[i]->uniqueID() == thatProxies[j]->uniqueID()) {
                    if (thisFilters[i] != thatFilters[j]) {
                        // In GL we don't currently support using the same texture with different
                        // samplers. If we added support for sampler objects and a cap bit to know
                        // it's ok to use different filter modes then we could support this.
                        // Otherwise, we could also only allow a single filter mode for each op
                        // instance.
                        return -1;
                    }
                    map[j] = i;
                    ++sharedProxyCnt;
                    break;
                }
            }
        }
        int actualMaxTextures = SkTMin(caps.maxFragmentImageStorages(), kMaxTextures);
        int newProxyCnt = that->fProxyCnt - sharedProxyCnt;
        if (newProxyCnt + fProxyCnt > actualMaxTextures) {
            return -1;
        }
        GrPixelConfig config = thisProxies[0]->config();
        int nextSlot = fProxyCnt;
        for (int j = 0; j < that->fProxyCnt; ++j) {
            // We want to avoid making many shaders because of different permutations of shader
            // based swizzle and sampler types. The approach taken here is to require the configs to
            // be the same and to only allow already instantiated proxies that have the most
            // common sampler type. Otherwise we don't merge.
            if (thatProxies[j]->config() != config) {
                return -1;
            }
            if (GrTexture* tex = thatProxies[j]->priv().peekTexture()) {
                if (tex->texturePriv().samplerType() != kTexture2DSampler_GrSLType) {
                    return -1;
                }
            }
            if (map[j] < 0) {
                map[j] = -(nextSlot++);
            }
        }
        return newProxyCnt;
    }

    GrTextureProxy* const* proxies() const { return fProxyCnt > 1 ? fProxyArray : &fProxy0; }

    const GrSamplerState::Filter* filters() const {
        if (fProxyCnt > 1) {
            return reinterpret_cast<const GrSamplerState::Filter*>(fProxyArray + kMaxTextures);
        }
        return &fFilter0;
    }

    struct Draw {
        SkRect fSrcRect;
        int fTextureIdx;
        GrQuad fQuad;
        GrColor fColor;
    };
    SkSTArray<1, Draw, true> fDraws;
    sk_sp<GrColorSpaceXform> fColorSpaceXform;
    // Initially we store a single proxy ptr and a single filter. If we grow to have more than
    // one proxy we instead store pointers to dynamically allocated arrays of size kMaxTextures
    // followed by kMaxTextures filters.
    union {
        GrTextureProxy* fProxy0;
        GrTextureProxy** fProxyArray;
    };
    // The next four members should pack.
    GrSamplerState::Filter fFilter0;
    uint8_t fProxyCnt;
    // Used to track whether fProxy is ref'ed or has a pending IO after finalize() is called.
    uint8_t fFinalized;
    uint8_t fAllowSRGBInputs;

    typedef GrMeshDrawOp INHERITED;
};

constexpr int TextureGeometryProcessor::kMaxTextures;
constexpr int TextureOp::kMaxTextures;

}  // anonymous namespace

namespace GrTextureOp {

std::unique_ptr<GrDrawOp> Make(sk_sp<GrTextureProxy> proxy, GrSamplerState::Filter filter,
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
    GrSamplerState::Filter filter = (GrSamplerState::Filter)random->nextULessThan(
            static_cast<uint32_t>(GrSamplerState::Filter::kMipMap) + 1);
    auto csxf = GrTest::TestColorXform(random);
    bool allowSRGBInputs = random->nextBool();
    return GrTextureOp::Make(std::move(proxy), filter, color, srcRect, rect, viewMatrix,
                             std::move(csxf), allowSRGBInputs);
}

#endif
