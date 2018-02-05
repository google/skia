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
#include "SkPoint.h"
#include "SkPoint3.h"
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"

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
    struct AAVertex {
        SkPoint fPosition;
        SkPoint fTextureCoords;
        SkPoint3 fEdges[4];
        GrColor fColor;
    };
    struct MultiTextureVertex {
        SkPoint fPosition;
        int fTextureIdx;
        SkPoint fTextureCoords;
        GrColor fColor;
    };
    struct AAMultiTextureVertex {
        SkPoint fPosition;
        int fTextureIdx;
        SkPoint fTextureCoords;
        SkPoint3 fEdges[4];
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
        return caps.integerSupport() && caps.maxFragmentSamplers() > 1;
    }

    static sk_sp<GrGeometryProcessor> Make(sk_sp<GrTextureProxy> proxies[], int proxyCnt,
                                           sk_sp<GrColorSpaceXform> csxf, bool coverageAA,
                                           const GrSamplerState::Filter filters[],
                                           const GrShaderCaps& caps) {
        // We use placement new to avoid always allocating space for kMaxTextures TextureSampler
        // instances.
        int samplerCnt = NumSamplersToUse(proxyCnt, caps);
        size_t size = sizeof(TextureGeometryProcessor) + sizeof(TextureSampler) * (samplerCnt - 1);
        void* mem = GrGeometryProcessor::operator new(size);
        return sk_sp<TextureGeometryProcessor>(new (mem) TextureGeometryProcessor(
                proxies, proxyCnt, samplerCnt, std::move(csxf), coverageAA, filters, caps));
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
        b->add32(static_cast<uint32_t>(this->usesCoverageEdgeAA()));
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
                using Interpolation = GrGLSLVaryingHandler::Interpolation;
                const auto& textureGP = args.fGP.cast<TextureGeometryProcessor>();
                fColorSpaceXformHelper.emitCode(
                        args.fUniformHandler, textureGP.fColorSpaceXform.get());
                args.fVaryingHandler->setNoPerspective();
                args.fVaryingHandler->emitAttributes(textureGP);
                this->writeOutputPosition(args.fVertBuilder, gpArgs, textureGP.fPositions.fName);
                this->emitTransforms(args.fVertBuilder,
                                     args.fVaryingHandler,
                                     args.fUniformHandler,
                                     textureGP.fTextureCoords.asShaderVar(),
                                     args.fFPCoordTransformHandler);
                args.fVaryingHandler->addPassThroughAttribute(&textureGP.fColors,
                                                              args.fOutputColor,
                                                              Interpolation::kCanBeFlat);
                args.fFragBuilder->codeAppend("float2 texCoord;");
                args.fVaryingHandler->addPassThroughAttribute(&textureGP.fTextureCoords,
                                                              "texCoord");
                if (textureGP.numTextureSamplers() > 1) {
                    // If this changes to float, reconsider Interpolation::kMustBeFlat.
                    SkASSERT(kInt_GrVertexAttribType == textureGP.fTextureIdx.fType);
                    SkASSERT(args.fShaderCaps->integerSupport());
                    args.fFragBuilder->codeAppend("int texIdx;");
                    args.fVaryingHandler->addPassThroughAttribute(&textureGP.fTextureIdx, "texIdx",
                                                                  Interpolation::kMustBeFlat);
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
                if (textureGP.usesCoverageEdgeAA()) {
                    const char* aaDistName = nullptr;
                    // When interpolation is innacurate we perform the evaluation of the edge
                    // equations in the fragment shader rather than interpolating values computed
                    // in the vertex shader.
                    if (!args.fShaderCaps->interpolantsAreInaccurate()) {
                        GrGLSLVarying aaDistVarying(kFloat4_GrSLType,
                                                    GrGLSLVarying::Scope::kVertToFrag);
                        args.fVaryingHandler->addVarying("aaDists", &aaDistVarying);
                        args.fVertBuilder->codeAppendf(
                                R"(%s = float4(dot(aaEdge0.xy, %s.xy) + aaEdge0.z,
                                               dot(aaEdge1.xy, %s.xy) + aaEdge1.z,
                                               dot(aaEdge2.xy, %s.xy) + aaEdge2.z,
                                               dot(aaEdge3.xy, %s.xy) + aaEdge3.z);)",
                                aaDistVarying.vsOut(), textureGP.fPositions.fName,
                                textureGP.fPositions.fName, textureGP.fPositions.fName,
                                textureGP.fPositions.fName);
                        aaDistName = aaDistVarying.fsIn();
                    } else {
                        GrGLSLVarying aaEdgeVarying[4]{
                                {kFloat3_GrSLType, GrGLSLVarying::Scope::kVertToFrag},
                                {kFloat3_GrSLType, GrGLSLVarying::Scope::kVertToFrag},
                                {kFloat3_GrSLType, GrGLSLVarying::Scope::kVertToFrag},
                                {kFloat3_GrSLType, GrGLSLVarying::Scope::kVertToFrag}
                        };
                        for (int i = 0; i < 4; ++i) {
                            SkString name;
                            name.printf("aaEdge%d", i);
                            args.fVaryingHandler->addVarying(name.c_str(), &aaEdgeVarying[i],
                                                             Interpolation::kCanBeFlat);
                            args.fVertBuilder->codeAppendf(
                                    "%s = aaEdge%d;", aaEdgeVarying[i].vsOut(), i);
                        }
                        args.fFragBuilder->codeAppendf(
                                R"(float4 aaDists = float4(dot(%s.xy, sk_FragCoord.xy) + %s.z,
                                                           dot(%s.xy, sk_FragCoord.xy) + %s.z,
                                                           dot(%s.xy, sk_FragCoord.xy) + %s.z,
                                                           dot(%s.xy, sk_FragCoord.xy) + %s.z);)",
                        aaEdgeVarying[0].fsIn(), aaEdgeVarying[0].fsIn(),
                        aaEdgeVarying[1].fsIn(), aaEdgeVarying[1].fsIn(),
                        aaEdgeVarying[2].fsIn(), aaEdgeVarying[2].fsIn(),
                        aaEdgeVarying[3].fsIn(), aaEdgeVarying[3].fsIn());
                        aaDistName = "aaDists";
                    }
                    args.fFragBuilder->codeAppendf(
                            "float mindist = min(min(%s.x, %s.y), min(%s.z, %s.w));",
                            aaDistName, aaDistName, aaDistName, aaDistName);
                    args.fFragBuilder->codeAppendf("%s = float4(clamp(mindist, 0, 1));",
                                                   args.fOutputCoverage);
                } else {
                    args.fFragBuilder->codeAppendf("%s = float4(1);", args.fOutputCoverage);
                }
            }
            GrGLSLColorSpaceXformHelper fColorSpaceXformHelper;
        };
        return new GLSLProcessor;
    }

    bool usesCoverageEdgeAA() const { return SkToBool(fAAEdges[0].isInitialized()); }

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
                             sk_sp<GrColorSpaceXform> csxf, bool coverageAA,
                             const GrSamplerState::Filter filters[], const GrShaderCaps& caps)
            : INHERITED(kTextureGeometryProcessor_ClassID), fColorSpaceXform(std::move(csxf)) {
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
        if (coverageAA) {
            fAAEdges[0] = this->addVertexAttrib("aaEdge0", kFloat3_GrVertexAttribType);
            fAAEdges[1] = this->addVertexAttrib("aaEdge1", kFloat3_GrVertexAttribType);
            fAAEdges[2] = this->addVertexAttrib("aaEdge2", kFloat3_GrVertexAttribType);
            fAAEdges[3] = this->addVertexAttrib("aaEdge3", kFloat3_GrVertexAttribType);
        }
        fColors = this->addVertexAttrib("color", kUByte4_norm_GrVertexAttribType);
    }

    Attribute fPositions;
    Attribute fTextureIdx;
    Attribute fTextureCoords;
    Attribute fColors;
    Attribute fAAEdges[4];
    sk_sp<GrColorSpaceXform> fColorSpaceXform;
    TextureSampler fSamplers[1];

    typedef GrGeometryProcessor INHERITED;
};

namespace {
// This is a class soley so it can be partially specialized (functions cannot be).
template<GrAA, typename Vertex> class VertexAAHandler;

template<typename Vertex> class VertexAAHandler<GrAA::kNo, Vertex> {
public:
    static void AssignPositionsAndTexCoords(Vertex* vertices, const GrQuad& quad,
                                            const SkRect& texRect) {
        vertices[0].fPosition = quad.point(0);
        vertices[0].fTextureCoords = {texRect.fLeft, texRect.fTop};
        vertices[1].fPosition = quad.point(1);
        vertices[1].fTextureCoords = {texRect.fLeft, texRect.fBottom};
        vertices[2].fPosition = quad.point(2);
        vertices[2].fTextureCoords = {texRect.fRight, texRect.fTop};
        vertices[3].fPosition = quad.point(3);
        vertices[3].fTextureCoords = {texRect.fRight, texRect.fBottom};
    }
};

template<typename Vertex> class VertexAAHandler<GrAA::kYes, Vertex> {
public:
    static void AssignPositionsAndTexCoords(Vertex* vertices, const GrQuad& quad,
                                            const SkRect& texRect) {
        // We compute the four edge equations for quad, then outset them and compute a new quad
        // as the intersection points of the outset edges.

        // GrQuad is in tristip order but we want the points to be in a fan order, so swap 2 and 3.
        Sk4f xs(quad.point(0).fX, quad.point(1).fX, quad.point(3).fX, quad.point(2).fX);
        Sk4f ys(quad.point(0).fY, quad.point(1).fY, quad.point(3).fY, quad.point(2).fY);
        Sk4f xsrot = SkNx_shuffle<1, 2, 3, 0>(xs);
        Sk4f ysrot = SkNx_shuffle<1, 2, 3, 0>(ys);
        Sk4f normXs = ysrot - ys;
        Sk4f normYs = xs - xsrot;
        Sk4f ds = xsrot * ys - ysrot * xs;
        Sk4f invNormLengths = (normXs * normXs + normYs * normYs).rsqrt();
        float test = normXs[0] * xs[2] + normYs[0] * ys[2] + ds[0];
        // Make sure the edge equations have their normals facing into the quad in device space
        if (test < 0) {
            invNormLengths = -invNormLengths;
        }
        normXs *= invNormLengths;
        normYs *= invNormLengths;
        ds *= invNormLengths;

        // Here is the bloat. This makes our edge equations compute coverage without requiring a
        // half pixel offset and is also used to compute the bloated quad that will cover all
        // pixels.
        ds += Sk4f(0.5f);

        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                vertices[j].fEdges[i].fX = normXs[i];
                vertices[j].fEdges[i].fY = normYs[i];
                vertices[j].fEdges[i].fZ = ds[i];
            }
        }

        // Reverse the process to compute the points of the bloated quad from the edge equations.
        // This time the inputs don't have 1s as their third coord and we want to homogenize rather
        // than normalize the output since we need a GrQuad with 2D points.
        xsrot = SkNx_shuffle<3, 0, 1, 2>(normXs);
        ysrot = SkNx_shuffle<3, 0, 1, 2>(normYs);
        Sk4f dsrot = SkNx_shuffle<3, 0, 1, 2>(ds);
        xs = ysrot * ds - normYs * dsrot;
        ys = normXs * dsrot - xsrot * ds;
        ds = xsrot * normYs - ysrot * normXs;
        ds = ds.invert();
        xs *= ds;
        ys *= ds;

        // Go back to tri strip order when writing out the bloated quad to vertex positions.
        vertices[0].fPosition = {xs[0], ys[0]};
        vertices[1].fPosition = {xs[1], ys[1]};
        vertices[3].fPosition = {xs[2], ys[2]};
        vertices[2].fPosition = {xs[3], ys[3]};

        AssignTexCoords(vertices, quad, texRect);
    }

private:
    static void AssignTexCoords(Vertex* vertices, const GrQuad& quad, const SkRect& tex) {
        SkMatrix q = SkMatrix::MakeAll(quad.point(0).fX, quad.point(1).fX, quad.point(2).fX,
                                       quad.point(0).fY, quad.point(1).fY, quad.point(2).fY,
                                                    1.f,              1.f,              1.f);
        SkMatrix qinv;
        if (!q.invert(&qinv)) {
            return;
        }
        SkMatrix t = SkMatrix::MakeAll(tex.fLeft,    tex.fLeft, tex.fRight,
                                        tex.fTop,  tex.fBottom,   tex.fTop,
                                             1.f,          1.f,        1.f);
        SkMatrix map;
        map.setConcat(t, qinv);
        SkMatrixPriv::MapPointsWithStride(map, &vertices[0].fTextureCoords, sizeof(Vertex),
                                          &vertices[0].fPosition, sizeof(Vertex), 4);
    }
};

template <typename Vertex, bool IsMultiTex> struct TexIdAssigner;

template <typename Vertex> struct TexIdAssigner<Vertex, true> {
    static void Assign(Vertex* vertices, int textureIdx) {
        vertices[0].fTextureIdx = textureIdx;
        vertices[1].fTextureIdx = textureIdx;
        vertices[2].fTextureIdx = textureIdx;
        vertices[3].fTextureIdx = textureIdx;
    }
};

template <typename Vertex> struct TexIdAssigner<Vertex, false> {
    static void Assign(Vertex* vertices, int textureIdx) {}
};
}  // anonymous namespace

template <typename Vertex, bool IsMultiTex, GrAA AA>
static void tessellate_quad(const GrQuad& devQuad, const SkRect& srcRect, GrColor color,
                            GrSurfaceOrigin origin, Vertex* vertices, SkScalar iw, SkScalar ih,
                            int textureIdx) {
    SkRect texRect = {
            iw * srcRect.fLeft,
            ih * srcRect.fTop,
            iw * srcRect.fRight,
            ih * srcRect.fBottom
    };
    if (origin == kBottomLeft_GrSurfaceOrigin) {
        texRect.fTop = 1.f - texRect.fTop;
        texRect.fBottom = 1.f - texRect.fBottom;
    }
    VertexAAHandler<AA, Vertex>::AssignPositionsAndTexCoords(vertices, devQuad, texRect);
    vertices[0].fColor = color;
    vertices[1].fColor = color;
    vertices[2].fColor = color;
    vertices[3].fColor = color;
    TexIdAssigner<Vertex, IsMultiTex>::Assign(vertices, textureIdx);
}
/**
 * Op that implements GrTextureOp::Make. It draws textured quads. Each quad can modulate against a
 * the texture by color. The blend with the destination is always src-over. The edges are non-AA.
 */
class TextureOp final : public GrMeshDrawOp {
public:
    static std::unique_ptr<GrDrawOp> Make(sk_sp<GrTextureProxy> proxy,
                                          GrSamplerState::Filter filter, GrColor color,
                                          const SkRect& srcRect, const SkRect& dstRect,
                                          GrAAType aaType, const SkMatrix& viewMatrix,
                                          sk_sp<GrColorSpaceXform> csxf, bool allowSRBInputs) {
        return std::unique_ptr<GrDrawOp>(new TextureOp(std::move(proxy), filter, color, srcRect,
                                                       dstRect, aaType, viewMatrix, std::move(csxf),
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

    FixedFunctionFlags fixedFunctionFlags() const override {
        return this->aaType() == GrAAType::kMSAA ? FixedFunctionFlags::kUsesHWAA
                                                 : FixedFunctionFlags::kNone;
    }

    DEFINE_OP_CLASS_ID

private:

    // This is used in a heursitic for choosing a code path. We don't care what happens with
    // really large rects, infs, nans, etc.
#if defined(__clang__) && (__clang_major__ * 1000 + __clang_minor__) >= 3007
__attribute__((no_sanitize("float-cast-overflow")))
#endif
    size_t RectSizeAsSizeT(const SkRect& rect) {;
        return static_cast<size_t>(SkTMax(rect.width(), 1.f) * SkTMax(rect.height(), 1.f));
    }

    static constexpr int kMaxTextures = TextureGeometryProcessor::kMaxTextures;

    TextureOp(sk_sp<GrTextureProxy> proxy, GrSamplerState::Filter filter, GrColor color,
              const SkRect& srcRect, const SkRect& dstRect, GrAAType aaType,
              const SkMatrix& viewMatrix, sk_sp<GrColorSpaceXform> csxf, bool allowSRGBInputs)
            : INHERITED(ClassID())
            , fColorSpaceXform(std::move(csxf))
            , fProxy0(proxy.release())
            , fFilter0(filter)
            , fProxyCnt(1)
            , fAAType(static_cast<unsigned>(aaType))
            , fFinalized(0)
            , fAllowSRGBInputs(allowSRGBInputs ? 1 : 0) {
        SkASSERT(aaType != GrAAType::kMixedSamples);
        Draw& draw = fDraws.push_back();
        draw.fSrcRect = srcRect;
        draw.fTextureIdx = 0;
        draw.fColor = color;
        draw.fQuad.setFromMappedRect(dstRect, viewMatrix);
        SkRect bounds;
        bounds.setBounds(draw.fQuad.points(), 4);
        this->setBounds(bounds, HasAABloat::kNo, IsZeroArea::kNo);

        fMaxApproxDstPixelArea = RectSizeAsSizeT(bounds);
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

        bool coverageAA = GrAAType::kCoverage == this->aaType();
        sk_sp<GrGeometryProcessor> gp =
                TextureGeometryProcessor::Make(proxiesSPs, fProxyCnt, std::move(fColorSpaceXform),
                                               coverageAA, filters, *target->caps().shaderCaps());
        GrPipeline::InitArgs args;
        args.fProxy = target->proxy();
        args.fCaps = &target->caps();
        args.fResourceProvider = target->resourceProvider();
        args.fFlags = 0;
        if (fAllowSRGBInputs) {
            args.fFlags |= GrPipeline::kAllowSRGBInputs_Flag;
        }
        if (GrAAType::kMSAA == this->aaType()) {
            args.fFlags |= GrPipeline::kHWAntialias_Flag;
        }

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
        if (1 == fProxyCnt) {
            GrSurfaceOrigin origin = proxies[0]->origin();
            GrTexture* texture = proxies[0]->priv().peekTexture();
            float iw = 1.f / texture->width();
            float ih = 1.f / texture->height();
            if (coverageAA) {
                SkASSERT(gp->getVertexStride() == sizeof(TextureGeometryProcessor::AAVertex));
                auto vertices = static_cast<TextureGeometryProcessor::AAVertex*>(vdata);
                for (int i = 0; i < fDraws.count(); ++i) {
                    tessellate_quad<TextureGeometryProcessor::AAVertex, false, GrAA::kYes>(
                            fDraws[i].fQuad, fDraws[i].fSrcRect, fDraws[i].fColor, origin,
                            vertices + 4 * i, iw, ih, 0);
                }
            } else {
                SkASSERT(gp->getVertexStride() == sizeof(TextureGeometryProcessor::Vertex));
                auto vertices = static_cast<TextureGeometryProcessor::Vertex*>(vdata);
                for (int i = 0; i < fDraws.count(); ++i) {
                    tessellate_quad<TextureGeometryProcessor::Vertex, false, GrAA::kNo>(
                            fDraws[i].fQuad, fDraws[i].fSrcRect, fDraws[i].fColor, origin,
                            vertices + 4 * i, iw, ih, 0);
                }
            }
        } else {
            GrTexture* textures[kMaxTextures];
            float iw[kMaxTextures];
            float ih[kMaxTextures];
            for (int t = 0; t < fProxyCnt; ++t) {
                textures[t] = proxies[t]->priv().peekTexture();
                iw[t] = 1.f / textures[t]->width();
                ih[t] = 1.f / textures[t]->height();
            }
            if (coverageAA) {
                SkASSERT(gp->getVertexStride() ==
                         sizeof(TextureGeometryProcessor::AAMultiTextureVertex));
                auto vertices = static_cast<TextureGeometryProcessor::AAMultiTextureVertex*>(vdata);
                for (int i = 0; i < fDraws.count(); ++i) {
                    auto tidx = fDraws[i].fTextureIdx;
                    GrSurfaceOrigin origin = proxies[tidx]->origin();
                    tessellate_quad<TextureGeometryProcessor::AAMultiTextureVertex, true,
                                    GrAA::kYes>(fDraws[i].fQuad, fDraws[i].fSrcRect,
                                                fDraws[i].fColor, origin, vertices + 4 * i,
                                                iw[tidx], ih[tidx], tidx);
                }
            } else {
                SkASSERT(gp->getVertexStride() ==
                         sizeof(TextureGeometryProcessor::MultiTextureVertex));
                auto vertices = static_cast<TextureGeometryProcessor::MultiTextureVertex*>(vdata);
                for (int i = 0; i < fDraws.count(); ++i) {
                    auto tidx = fDraws[i].fTextureIdx;
                    GrSurfaceOrigin origin = proxies[tidx]->origin();
                    tessellate_quad<TextureGeometryProcessor::MultiTextureVertex, true, GrAA::kNo>(
                            fDraws[i].fQuad, fDraws[i].fSrcRect, fDraws[i].fColor, origin,
                            vertices + 4 * i, iw[tidx], ih[tidx], tidx);
                }
            }
        }
        GrPrimitiveType primitiveType =
                fDraws.count() > 1 ? GrPrimitiveType::kTriangles : GrPrimitiveType::kTriangleStrip;
        GrMesh mesh(primitiveType);
        if (fDraws.count() > 1) {
            sk_sp<const GrBuffer> ibuffer = target->resourceProvider()->refQuadIndexBuffer();
            if (!ibuffer) {
                SkDebugf("Could not allocate quad indices\n");
                return;
            }
            mesh.setIndexedPatterned(ibuffer.get(), 6, 4, fDraws.count(),
                                     GrResourceProvider::QuadCountOfQuadBuffer());
        } else {
            mesh.setNonIndexedNonInstanced(4);
        }
        mesh.setVertexData(vbuffer, vstart);
        target->draw(gp.get(), pipeline, mesh);
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        const auto* that = t->cast<TextureOp>();
        const auto& shaderCaps = *caps.shaderCaps();
        if (!GrColorSpaceXform::Equals(fColorSpaceXform.get(), that->fColorSpaceXform.get())) {
            return false;
        }
        if (this->aaType() != that->aaType()) {
            return false;
        }
        // Because of an issue where GrColorSpaceXform adds the same function every time it is used
        // in a texture lookup, we only allow multiple textures when there is no transform.
        if (TextureGeometryProcessor::SupportsMultitexture(shaderCaps) && !fColorSpaceXform &&
            fMaxApproxDstPixelArea <= shaderCaps.disableImageMultitexturingDstRectAreaThreshold() &&
            that->fMaxApproxDstPixelArea <=
                    shaderCaps.disableImageMultitexturingDstRectAreaThreshold()) {
            int map[kMaxTextures];
            int numNewProxies = this->mergeProxies(that, map, shaderCaps);
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
            // We can get here when one of the ops is already multitextured but the other cannot
            // be because of the dst rect size.
            if (fProxyCnt > 1 || that->fProxyCnt > 1) {
                return false;
            }
            if (fProxy0->uniqueID() != that->fProxy0->uniqueID() || fFilter0 != that->fFilter0) {
                return false;
            }
            fDraws.push_back_n(that->fDraws.count(), that->fDraws.begin());
        }
        this->joinBounds(*that);
        fMaxApproxDstPixelArea = SkTMax(that->fMaxApproxDstPixelArea, fMaxApproxDstPixelArea);
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
        int actualMaxTextures = SkTMin(caps.maxFragmentSamplers(), kMaxTextures);
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

    GrAAType aaType() const { return static_cast<GrAAType>(fAAType); }

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
    size_t fMaxApproxDstPixelArea;
    GrSamplerState::Filter fFilter0;
    uint8_t fProxyCnt;
    unsigned fAAType : 2;
    // Used to track whether fProxy is ref'ed or has a pending IO after finalize() is called.
    unsigned fFinalized : 1;
    unsigned fAllowSRGBInputs : 1;

    typedef GrMeshDrawOp INHERITED;
};

constexpr int TextureGeometryProcessor::kMaxTextures;
constexpr int TextureOp::kMaxTextures;

}  // anonymous namespace

namespace GrTextureOp {

std::unique_ptr<GrDrawOp> Make(sk_sp<GrTextureProxy> proxy, GrSamplerState::Filter filter,
                               GrColor color, const SkRect& srcRect, const SkRect& dstRect,
                               GrAAType aaType, const SkMatrix& viewMatrix,
                               sk_sp<GrColorSpaceXform> csxf, bool allowSRGBInputs) {
    SkASSERT(!viewMatrix.hasPerspective());
    return TextureOp::Make(std::move(proxy), filter, color, srcRect, dstRect, aaType, viewMatrix,
                           std::move(csxf), allowSRGBInputs);
}

}  // namespace GrTextureOp

#if GR_TEST_UTILS
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrProxyProvider.h"

GR_DRAW_OP_TEST_DEFINE(TextureOp) {
    GrSurfaceDesc desc;
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    desc.fHeight = random->nextULessThan(90) + 10;
    desc.fWidth = random->nextULessThan(90) + 10;
    desc.fOrigin = random->nextBool() ? kTopLeft_GrSurfaceOrigin : kBottomLeft_GrSurfaceOrigin;
    SkBackingFit fit = random->nextBool() ? SkBackingFit::kApprox : SkBackingFit::kExact;

    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
    sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(desc, fit, SkBudgeted::kNo);

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
    GrAAType aaType = GrAAType::kNone;
    if (random->nextBool()) {
        aaType = (fsaaType == GrFSAAType::kUnifiedMSAA) ? GrAAType::kMSAA : GrAAType::kCoverage;
    }
    return GrTextureOp::Make(std::move(proxy), filter, color, srcRect, rect, aaType, viewMatrix,
                             std::move(csxf), allowSRGBInputs);
}

#endif
