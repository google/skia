/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureOp.h"

#include "GrAppliedClip.h"
#include "GrCaps.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrDrawOpTest.h"
#include "GrGeometryProcessor.h"
#include "GrMemoryPool.h"
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
#include "SkMatrixPriv.h"
#include "SkPoint.h"
#include "SkPoint3.h"
#include "SkTo.h"
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"
#include <new>

namespace {

enum class MultiTexture : bool { kNo = false, kYes = true };

enum class Domain : bool { kNo = false, kYes = true };

/**
 * Geometry Processor that draws a texture modulated by a vertex color (though, this is meant to be
 * the same value across all vertices of a quad and uses flat interpolation when available). This is
 * used by TextureOp below.
 */
class TextureGeometryProcessor : public GrGeometryProcessor {
public:
    template <typename Pos> struct VertexCommon {
        using Position = Pos;
        Position fPosition;
        GrColor fColor;
        SkPoint fTextureCoords;
    };

    template <typename Pos, MultiTexture MT> struct OptionalMultiTextureVertex;
    template <typename Pos>
    struct OptionalMultiTextureVertex<Pos, MultiTexture::kNo> : VertexCommon<Pos> {
        static constexpr MultiTexture kMultiTexture = MultiTexture::kNo;
    };
    template <typename Pos>
    struct OptionalMultiTextureVertex<Pos, MultiTexture::kYes> : VertexCommon<Pos> {
        static constexpr MultiTexture kMultiTexture = MultiTexture::kYes;
        int fTextureIdx;
    };

    template <typename Pos, MultiTexture MT, Domain D> struct OptionalDomainVertex;
    template <typename Pos, MultiTexture MT>
    struct OptionalDomainVertex<Pos, MT, Domain::kNo> : OptionalMultiTextureVertex<Pos, MT> {
        static constexpr Domain kDomain = Domain::kNo;
    };
    template <typename Pos, MultiTexture MT>
    struct OptionalDomainVertex<Pos, MT, Domain::kYes> : OptionalMultiTextureVertex<Pos, MT> {
        static constexpr Domain kDomain = Domain::kYes;
        SkRect fTextureDomain;
    };

    template <typename Pos, MultiTexture MT, Domain D, GrAA> struct OptionalAAVertex;
    template <typename Pos, MultiTexture MT, Domain D>
    struct OptionalAAVertex<Pos, MT, D, GrAA::kNo> : OptionalDomainVertex<Pos, MT, D> {
        static constexpr GrAA kAA = GrAA::kNo;
    };
    template <typename Pos, MultiTexture MT, Domain D>
    struct OptionalAAVertex<Pos, MT, D, GrAA::kYes> : OptionalDomainVertex<Pos, MT, D> {
        static constexpr GrAA kAA = GrAA::kYes;
        SkPoint3 fEdges[4];
    };

    template <typename Pos, MultiTexture MT, Domain D, GrAA AA>
    using Vertex = OptionalAAVertex<Pos, MT, D, AA>;

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
                                           bool perspective, Domain domain,
                                           const GrSamplerState::Filter filters[],
                                           const GrShaderCaps& caps) {
        // We use placement new to avoid always allocating space for kMaxTextures TextureSampler
        // instances.
        int samplerCnt = NumSamplersToUse(proxyCnt, caps);
        size_t size = sizeof(TextureGeometryProcessor) + sizeof(TextureSampler) * (samplerCnt - 1);
        void* mem = GrGeometryProcessor::operator new(size);
        return sk_sp<TextureGeometryProcessor>(
                new (mem) TextureGeometryProcessor(proxies, proxyCnt, samplerCnt, std::move(csxf),
                                                   coverageAA, perspective, domain, filters, caps));
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
        uint32_t x = this->usesCoverageEdgeAA() ? 0 : 1;
        x |= kFloat3_GrVertexAttribType == fPositions.type() ? 0 : 2;
        x |= fDomain.isInitialized() ? 4 : 0;
        b->add32(x);
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps& caps) const override {
        class GLSLProcessor : public GrGLSLGeometryProcessor {
        public:
            void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& proc,
                         FPCoordTransformIter&& transformIter) override {
                const auto& textureGP = proc.cast<TextureGeometryProcessor>();
                this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
                fColorSpaceXformHelper.setData(pdman, textureGP.fColorSpaceXform.get());
            }

        private:
            void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
                using Interpolation = GrGLSLVaryingHandler::Interpolation;
                const auto& textureGP = args.fGP.cast<TextureGeometryProcessor>();
                fColorSpaceXformHelper.emitCode(
                        args.fUniformHandler, textureGP.fColorSpaceXform.get());
                if (kFloat2_GrVertexAttribType == textureGP.fPositions.type()) {
                    args.fVaryingHandler->setNoPerspective();
                }
                args.fVaryingHandler->emitAttributes(textureGP);
                gpArgs->fPositionVar = textureGP.fPositions.asShaderVar();

                this->emitTransforms(args.fVertBuilder,
                                     args.fVaryingHandler,
                                     args.fUniformHandler,
                                     textureGP.fTextureCoords.asShaderVar(),
                                     args.fFPCoordTransformHandler);
                args.fVaryingHandler->addPassThroughAttribute(
                        textureGP.fColors, args.fOutputColor, Interpolation::kCanBeFlat);
                args.fFragBuilder->codeAppend("float2 texCoord;");
                args.fVaryingHandler->addPassThroughAttribute(textureGP.fTextureCoords, "texCoord");
                if (textureGP.fDomain.isInitialized()) {
                    args.fFragBuilder->codeAppend("float4 domain;");
                    args.fVaryingHandler->addPassThroughAttribute(
                            textureGP.fDomain, "domain",
                            GrGLSLVaryingHandler::Interpolation::kCanBeFlat);
                    args.fFragBuilder->codeAppend(
                            "texCoord = clamp(texCoord, domain.xy, domain.zw);");
                }
                if (textureGP.numTextureSamplers() > 1) {
                    // If this changes to float, reconsider Interpolation::kMustBeFlat.
                    SkASSERT(kInt_GrVertexAttribType == textureGP.fTextureIdx.type());
                    SkASSERT(args.fShaderCaps->integerSupport());
                    args.fFragBuilder->codeAppend("int texIdx;");
                    args.fVaryingHandler->addPassThroughAttribute(textureGP.fTextureIdx, "texIdx",
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
                    bool mulByFragCoordW = false;
                    GrGLSLVarying aaDistVarying(kFloat4_GrSLType,
                                                GrGLSLVarying::Scope::kVertToFrag);
                    if (kFloat3_GrVertexAttribType == textureGP.fPositions.type()) {
                        args.fVaryingHandler->addVarying("aaDists", &aaDistVarying);
                        // The distance from edge equation e to homogenous point p=sk_Position
                        // is e.x*p.x/p.wx + e.y*p.y/p.w + e.z. However, we want screen space
                        // interpolation of this distance. We can do this by multiplying the
                        // varying in the VS by p.w and then multiplying by sk_FragCoord.w in
                        // the FS. So we output e.x*p.x + e.y*p.y + e.z * p.w
                        args.fVertBuilder->codeAppendf(
                                R"(%s = float4(dot(aaEdge0, %s), dot(aaEdge1, %s),
                                               dot(aaEdge2, %s), dot(aaEdge3, %s));)",
                                aaDistVarying.vsOut(), textureGP.fPositions.name(),
                                textureGP.fPositions.name(), textureGP.fPositions.name(),
                                textureGP.fPositions.name());
                        mulByFragCoordW = true;
                    } else {
                        args.fVaryingHandler->addVarying("aaDists", &aaDistVarying);
                        args.fVertBuilder->codeAppendf(
                                R"(%s = float4(dot(aaEdge0.xy, %s.xy) + aaEdge0.z,
                                               dot(aaEdge1.xy, %s.xy) + aaEdge1.z,
                                               dot(aaEdge2.xy, %s.xy) + aaEdge2.z,
                                               dot(aaEdge3.xy, %s.xy) + aaEdge3.z);)",
                                aaDistVarying.vsOut(), textureGP.fPositions.name(),
                                textureGP.fPositions.name(), textureGP.fPositions.name(),
                                textureGP.fPositions.name());
                    }
                    args.fFragBuilder->codeAppendf(
                            "float mindist = min(min(%s.x, %s.y), min(%s.z, %s.w));",
                            aaDistVarying.fsIn(), aaDistVarying.fsIn(), aaDistVarying.fsIn(),
                            aaDistVarying.fsIn());
                    if (mulByFragCoordW) {
                        args.fFragBuilder->codeAppend("mindist *= sk_FragCoord.w;");
                    }
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
                             sk_sp<GrColorSpaceXform> csxf, bool coverageAA, bool perspective,
                             Domain domain, const GrSamplerState::Filter filters[],
                             const GrShaderCaps& caps)
            : INHERITED(kTextureGeometryProcessor_ClassID), fColorSpaceXform(std::move(csxf)) {
        SkASSERT(proxyCnt > 0 && samplerCnt >= proxyCnt);
        fSamplers[0].reset(std::move(proxies[0]), filters[0]);
        this->addTextureSampler(&fSamplers[0]);
        for (int i = 1; i < proxyCnt; ++i) {
            // This class has one sampler built in, the rest come from memory this processor was
            // placement-newed into and so haven't been constructed.
            new (&fSamplers[i]) TextureSampler(std::move(proxies[i]), filters[i]);
            this->addTextureSampler(&fSamplers[i]);
        }

        if (perspective) {
            fPositions = {"position", kFloat3_GrVertexAttribType};
        } else {
            fPositions = {"position", kFloat2_GrVertexAttribType};
        }
        fColors = {"color", kUByte4_norm_GrVertexAttribType};
        fTextureCoords = {"textureCoords", kFloat2_GrVertexAttribType};
        int vertexAttributeCnt = 3;

        if (samplerCnt > 1) {
            // Here we initialize any extra samplers by repeating the last one samplerCnt - proxyCnt
            // times.
            GrTextureProxy* dupeProxy = fSamplers[proxyCnt - 1].proxy();
            for (int i = proxyCnt; i < samplerCnt; ++i) {
                new (&fSamplers[i]) TextureSampler(sk_ref_sp(dupeProxy), filters[proxyCnt - 1]);
                this->addTextureSampler(&fSamplers[i]);
            }
            SkASSERT(caps.integerSupport());
            fTextureIdx = {"textureIdx", kInt_GrVertexAttribType};
            ++vertexAttributeCnt;
        }
        if (domain == Domain::kYes) {
            fDomain = {"domain", kFloat4_GrVertexAttribType};
            ++vertexAttributeCnt;
        }
        if (coverageAA) {
            fAAEdges[0] = {"aaEdge0", kFloat3_GrVertexAttribType};
            fAAEdges[1] = {"aaEdge1", kFloat3_GrVertexAttribType};
            fAAEdges[2] = {"aaEdge2", kFloat3_GrVertexAttribType};
            fAAEdges[3] = {"aaEdge3", kFloat3_GrVertexAttribType};
            vertexAttributeCnt += 4;
        }
        this->setVertexAttributeCnt(vertexAttributeCnt);
    }

    const Attribute& onVertexAttribute(int i) const override {
        return IthInitializedAttribute(i, fPositions, fColors, fTextureCoords, fTextureIdx, fDomain,
                                       fAAEdges[0], fAAEdges[1], fAAEdges[2], fAAEdges[3]);
    }

    Attribute fPositions;
    Attribute fColors;
    Attribute fTextureCoords;
    Attribute fTextureIdx;
    Attribute fDomain;
    Attribute fAAEdges[4];
    sk_sp<GrColorSpaceXform> fColorSpaceXform;
    TextureSampler fSamplers[1];

    typedef GrGeometryProcessor INHERITED;
};

// This computes the four edge equations for a quad, then outsets them and computes a new quad
// as the intersection points of the outset edges. 'x' and 'y' contain the original points as input
// and the outset points as output. 'a', 'b', and 'c' are the edge equation coefficients on output.
static void compute_quad_edges_and_outset_vertices(Sk4f* x, Sk4f* y, Sk4f* a, Sk4f* b, Sk4f* c) {
    static constexpr auto fma = SkNx_fma<4, float>;
    // These rotate the points/edge values either clockwise or counterclockwise assuming tri strip
    // order.
    auto nextCW  = [](const Sk4f& v) { return SkNx_shuffle<2, 0, 3, 1>(v); };
    auto nextCCW = [](const Sk4f& v) { return SkNx_shuffle<1, 3, 0, 2>(v); };

    auto xnext = nextCCW(*x);
    auto ynext = nextCCW(*y);
    *a = ynext - *y;
    *b = *x - xnext;
    *c = fma(xnext, *y,  -ynext * *x);
    Sk4f invNormLengths = (*a * *a + *b * *b).rsqrt();
    // Make sure the edge equations have their normals facing into the quad in device space.
    auto test = fma(*a, nextCW(*x), fma(*b, nextCW(*y), *c));
    if ((test < Sk4f(0)).anyTrue()) {
        invNormLengths = -invNormLengths;
    }
    *a *= invNormLengths;
    *b *= invNormLengths;
    *c *= invNormLengths;

    // Here is the outset. This makes our edge equations compute coverage without requiring a
    // half pixel offset and is also used to compute the bloated quad that will cover all
    // pixels.
    *c += Sk4f(0.5f);

    // Reverse the process to compute the points of the bloated quad from the edge equations.
    // This time the inputs don't have 1s as their third coord and we want to homogenize rather
    // than normalize.
    auto anext = nextCW(*a);
    auto bnext = nextCW(*b);
    auto cnext = nextCW(*c);
    *x = fma(bnext, *c, -*b * cnext);
    *y = fma(*a, cnext, -anext * *c);
    auto ic = (fma(anext, *b, -bnext * *a)).invert();
    *x *= ic;
    *y *= ic;
}

namespace {
// This is a class soley so it can be partially specialized (functions cannot be).
template <typename V, GrAA AA = V::kAA, typename Position = typename V::Position>
class VertexAAHandler;

template<typename V> class VertexAAHandler<V, GrAA::kNo, SkPoint> {
public:
    static void AssignPositionsAndTexCoords(V* vertices, const GrPerspQuad& quad,
                                            const SkRect& texRect) {
        SkASSERT((quad.w4f() == Sk4f(1.f)).allTrue());
        SkPointPriv::SetRectTriStrip(&vertices[0].fTextureCoords, texRect, sizeof(V));
        for (int i = 0; i < 4; ++i) {
            vertices[i].fPosition = {quad.x(i), quad.y(i)};
        }
    }
};

template<typename V> class VertexAAHandler<V, GrAA::kNo, SkPoint3> {
public:
    static void AssignPositionsAndTexCoords(V* vertices, const GrPerspQuad& quad,
                                            const SkRect& texRect) {
        SkPointPriv::SetRectTriStrip(&vertices[0].fTextureCoords, texRect, sizeof(V));
        for (int i = 0; i < 4; ++i) {
            vertices[i].fPosition = quad.point(i);
        }
    }
};

template<typename V> class VertexAAHandler<V, GrAA::kYes, SkPoint> {
public:
    static void AssignPositionsAndTexCoords(V* vertices, const GrPerspQuad& quad,
                                            const SkRect& texRect) {
        SkASSERT((quad.w4f() == Sk4f(1.f)).allTrue());
        auto x = quad.x4f();
        auto y = quad.y4f();
        Sk4f a, b, c;
        compute_quad_edges_and_outset_vertices(&x, &y, &a, &b, &c);

        for (int i = 0; i < 4; ++i) {
            vertices[i].fPosition = {x[i], y[i]};
            for (int j = 0; j < 4; ++j) {
                vertices[i].fEdges[j]  = {a[j], b[j], c[j]};
            }
        }

        AssignTexCoords(vertices, quad, texRect);
    }

private:
    static void AssignTexCoords(V* vertices, const GrPerspQuad& quad, const SkRect& tex) {
        SkMatrix q = SkMatrix::MakeAll(quad.x(0), quad.x(1), quad.x(2),
                                       quad.y(0), quad.y(1), quad.y(2),
                                             1.f,       1.f,       1.f);
        SkMatrix qinv;
        if (!q.invert(&qinv)) {
            return;
        }
        SkMatrix t = SkMatrix::MakeAll(tex.fLeft,    tex.fLeft, tex.fRight,
                                        tex.fTop,  tex.fBottom,   tex.fTop,
                                             1.f,          1.f,        1.f);
        SkMatrix map;
        map.setConcat(t, qinv);
        SkMatrixPriv::MapPointsWithStride(map, &vertices[0].fTextureCoords, sizeof(V),
                                          &vertices[0].fPosition, sizeof(V), 4);
    }
};

template<typename V> class VertexAAHandler<V, GrAA::kYes, SkPoint3> {
public:
    static void AssignPositionsAndTexCoords(V* vertices, const GrPerspQuad& quad,
                                            const SkRect& texRect) {
        auto x = quad.x4f();
        auto y = quad.y4f();
        auto iw = quad.iw4f();
        x *= iw;
        y *= iw;

        // Get an equation for w from device space coords.
        SkMatrix P;
        P.setAll(x[0], y[0], 1, x[1], y[1], 1, x[2], y[2], 1);
        SkAssertResult(P.invert(&P));
        SkPoint3 weq{quad.w(0), quad.w(1), quad.w(2)};
        P.mapHomogeneousPoints(&weq, &weq, 1);

        Sk4f a, b, c;
        compute_quad_edges_and_outset_vertices(&x, &y, &a, &b, &c);

        // Compute new w values for the output vertices;
        auto w = Sk4f(weq.fX) * x + Sk4f(weq.fY) * y + Sk4f(weq.fZ);
        x *= w;
        y *= w;

        for (int i = 0; i < 4; ++i) {
            vertices[i].fPosition = {x[i], y[i], w[i]};
            for (int j = 0; j < 4; ++j) {
                vertices[i].fEdges[j] = {a[j], b[j], c[j]};
            }
        }

        AssignTexCoords(vertices, quad, texRect);
    }

private:
    static void AssignTexCoords(V* vertices, const GrPerspQuad& quad, const SkRect& tex) {
        SkMatrix q = SkMatrix::MakeAll(quad.x(0), quad.x(1), quad.x(2),
                                       quad.y(0), quad.y(1), quad.y(2),
                                       quad.w(0), quad.w(1), quad.w(2));
        SkMatrix qinv;
        if (!q.invert(&qinv)) {
            return;
        }
        SkMatrix t = SkMatrix::MakeAll(tex.fLeft, tex.fLeft,   tex.fRight,
                                       tex.fTop,  tex.fBottom, tex.fTop,
                                       1.f,       1.f,         1.f);
        SkMatrix map;
        map.setConcat(t, qinv);
        SkPoint3 tempTexCoords[4];
        SkMatrixPriv::MapHomogeneousPointsWithStride(map, tempTexCoords, sizeof(SkPoint3),
                                                     &vertices[0].fPosition, sizeof(V), 4);
        for (int i = 0; i < 4; ++i) {
            auto invW = 1.f / tempTexCoords[i].fZ;
            vertices[i].fTextureCoords.fX = tempTexCoords[i].fX * invW;
            vertices[i].fTextureCoords.fY = tempTexCoords[i].fY * invW;
        }
    }
};

template <typename V, MultiTexture MT = V::kMultiTexture> struct TexIdAssigner;

template <typename V> struct TexIdAssigner<V, MultiTexture::kYes> {
    static void Assign(V* vertices, int textureIdx) {
        for (int i = 0; i < 4; ++i) {
            vertices[i].fTextureIdx = textureIdx;
        }
    }
};

template <typename V> struct TexIdAssigner<V, MultiTexture::kNo> {
    static void Assign(V* vertices, int textureIdx) {}
};

template <typename V, Domain D = V::kDomain> struct DomainAssigner;

template <typename V> struct DomainAssigner<V, Domain::kYes> {
    static void Assign(V* vertices, Domain domain, GrSamplerState::Filter filter,
                       const SkRect& srcRect, GrSurfaceOrigin origin, float iw, float ih) {
        static constexpr SkRect kLargeRect = {-2, -2, 2, 2};
        SkRect domainRect;
        if (domain == Domain::kYes) {
            auto ltrb = Sk4f::Load(&srcRect);
            if (filter == GrSamplerState::Filter::kBilerp) {
                auto rblt = SkNx_shuffle<2, 3, 0, 1>(ltrb);
                auto whwh = (rblt - ltrb).abs();
                auto c = (rblt + ltrb) * 0.5f;
                static const Sk4f kOffsets = {0.5f, 0.5f, -0.5f, -0.5f};
                ltrb = (whwh < 1.f).thenElse(c, ltrb + kOffsets);
            }
            ltrb *= Sk4f(iw, ih, iw, ih);
            if (origin == kBottomLeft_GrSurfaceOrigin) {
                static const Sk4f kMul = {1.f, -1.f, 1.f, -1.f};
                static const Sk4f kAdd = {0.f, 1.f, 0.f, 1.f};
                ltrb = SkNx_shuffle<0, 3, 2, 1>(kMul * ltrb + kAdd);
            }
            ltrb.store(&domainRect);
        } else {
            domainRect = kLargeRect;
        }
        for (int i = 0; i < 4; ++i) {
            vertices[i].fTextureDomain = domainRect;
        }
    }
};

template <typename V> struct DomainAssigner<V, Domain::kNo> {
    static void Assign(V*, Domain domain, GrSamplerState::Filter, const SkRect&, GrSurfaceOrigin,
                       float iw, float ih) {
        SkASSERT(domain == Domain::kNo);
    }
};

}  // anonymous namespace

template <typename V>
static void tessellate_quad(const GrPerspQuad& devQuad, const SkRect& srcRect, GrColor color,
                            GrSurfaceOrigin origin, GrSamplerState::Filter filter, V* vertices,
                            SkScalar iw, SkScalar ih, int textureIdx, Domain domain) {
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
    VertexAAHandler<V>::AssignPositionsAndTexCoords(vertices, devQuad, texRect);
    vertices[0].fColor = color;
    vertices[1].fColor = color;
    vertices[2].fColor = color;
    vertices[3].fColor = color;
    TexIdAssigner<V>::Assign(vertices, textureIdx);
    DomainAssigner<V>::Assign(vertices, domain, filter, srcRect, origin, iw, ih);
}

/**
 * Op that implements GrTextureOp::Make. It draws textured quads. Each quad can modulate against a
 * the texture by color. The blend with the destination is always src-over. The edges are non-AA.
 */
class TextureOp final : public GrMeshDrawOp {
public:
    static std::unique_ptr<GrDrawOp> Make(GrContext* context,
                                          sk_sp<GrTextureProxy> proxy,
                                          GrSamplerState::Filter filter,
                                          GrColor color,
                                          const SkRect& srcRect,
                                          const SkRect& dstRect,
                                          GrAAType aaType,
                                          SkCanvas::SrcRectConstraint constraint,
                                          const SkMatrix& viewMatrix,
                                          sk_sp<GrColorSpaceXform> csxf) {
        GrOpMemoryPool* pool = context->contextPriv().opMemoryPool();

        return pool->allocate<TextureOp>(std::move(proxy), filter, color,
                                         srcRect, dstRect, aaType, constraint,
                                         viewMatrix, std::move(csxf));
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
                    i, draw.color(), draw.textureIdx(), draw.srcRect().fLeft, draw.srcRect().fTop,
                    draw.srcRect().fRight, draw.srcRect().fBottom, draw.quad().point(0).fX,
                    draw.quad().point(0).fY, draw.quad().point(1).fX, draw.quad().point(1).fY,
                    draw.quad().point(2).fX, draw.quad().point(2).fY, draw.quad().point(3).fX,
                    draw.quad().point(3).fY);
        }
        str += INHERITED::dumpInfo();
        return str;
    }

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override {
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
    friend class ::GrOpMemoryPool;

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
              SkCanvas::SrcRectConstraint constraint, const SkMatrix& viewMatrix,
              sk_sp<GrColorSpaceXform> csxf)
            : INHERITED(ClassID())
            , fColorSpaceXform(std::move(csxf))
            , fProxy0(proxy.release())
            , fFilter0(filter)
            , fProxyCnt(1)
            , fAAType(static_cast<unsigned>(aaType))
            , fFinalized(0) {
        SkASSERT(aaType != GrAAType::kMixedSamples);
        fPerspective = viewMatrix.hasPerspective();
        auto quad = GrPerspQuad(dstRect, viewMatrix);
        auto bounds = quad.bounds();
#ifndef SK_DONT_DROP_UNNECESSARY_AA_IN_TEXTURE_OP
        if (GrAAType::kCoverage == this->aaType() && viewMatrix.rectStaysRect()) {
            // Disable coverage AA when rect falls on integers in device space.
            auto is_int = [](float f) { return f == sk_float_floor(f); };
            if (is_int(bounds.fLeft) && is_int(bounds.fTop) && is_int(bounds.fRight) &&
                is_int(bounds.fBottom)) {
                fAAType = static_cast<unsigned>(GrAAType::kNone);
                // We may have had a strict constraint with nearest filter soley due to possible AA
                // bloat. In that case it's no longer necessary.
                if (constraint == SkCanvas::kStrict_SrcRectConstraint &&
                    filter == GrSamplerState::Filter::kNearest) {
                    constraint = SkCanvas::kFast_SrcRectConstraint;
                }
            }
        }
#endif
        const auto& draw = fDraws.emplace_back(srcRect, 0, quad, constraint, color);
        this->setBounds(bounds, HasAABloat::kNo, IsZeroArea::kNo);
        fDomain = static_cast<bool>(draw.domain());
        fMaxApproxDstPixelArea = RectSizeAsSizeT(bounds);
    }

    template <typename Pos, MultiTexture MT, Domain D, GrAA AA>
    void tess(void* v, const float iw[], const float ih[], const GrGeometryProcessor* gp) {
        using Vertex = TextureGeometryProcessor::Vertex<Pos, MT, D, AA>;
        SkASSERT(gp->debugOnly_vertexStride() == sizeof(Vertex));
        auto vertices = static_cast<Vertex*>(v);
        auto proxies = this->proxies();
        auto filters = this->filters();
        for (const auto& draw : fDraws) {
            auto textureIdx = draw.textureIdx();
            auto origin = proxies[textureIdx]->origin();
            tessellate_quad<Vertex>(draw.quad(), draw.srcRect(), draw.color(), origin,
                                    filters[textureIdx], vertices, iw[textureIdx], ih[textureIdx],
                                    textureIdx, draw.domain());
            vertices += 4;
        }
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

        Domain domain = fDomain ? Domain::kYes : Domain::kNo;
        bool coverageAA = GrAAType::kCoverage == this->aaType();
        sk_sp<GrGeometryProcessor> gp = TextureGeometryProcessor::Make(
                proxiesSPs, fProxyCnt, std::move(fColorSpaceXform), coverageAA, fPerspective,
                domain, filters, *target->caps().shaderCaps());
        GrPipeline::InitArgs args;
        args.fProxy = target->proxy();
        args.fCaps = &target->caps();
        args.fResourceProvider = target->resourceProvider();
        args.fFlags = 0;
        if (GrAAType::kMSAA == this->aaType()) {
            args.fFlags |= GrPipeline::kHWAntialias_Flag;
        }

        auto clip = target->detachAppliedClip();
        const auto* fixedDynamicState = target->allocFixedDynamicState(clip.scissorState().rect());
        const auto* pipeline =
                target->allocPipeline(args, GrProcessorSet::MakeEmptySet(), std::move(clip));
        using TessFn =
                decltype(&TextureOp::tess<SkPoint, MultiTexture::kNo, Domain::kNo, GrAA::kNo>);
#define TESS_FN_AND_VERTEX_SIZE(Point, MT, Domain, AA)                          \
    {                                                                           \
        &TextureOp::tess<Point, MT, Domain, AA>,                                \
                sizeof(TextureGeometryProcessor::Vertex<Point, MT, Domain, AA>) \
    }
        static constexpr struct {
            TessFn fTessFn;
            size_t fVertexSize;
        } kTessFnsAndVertexSizes[] = {
                TESS_FN_AND_VERTEX_SIZE(SkPoint, MultiTexture::kNo, Domain::kNo, GrAA::kNo),
                TESS_FN_AND_VERTEX_SIZE(SkPoint, MultiTexture::kNo, Domain::kNo, GrAA::kYes),
                TESS_FN_AND_VERTEX_SIZE(SkPoint, MultiTexture::kNo, Domain::kYes, GrAA::kNo),
                TESS_FN_AND_VERTEX_SIZE(SkPoint, MultiTexture::kNo, Domain::kYes, GrAA::kYes),
                TESS_FN_AND_VERTEX_SIZE(SkPoint, MultiTexture::kYes, Domain::kNo, GrAA::kNo),
                TESS_FN_AND_VERTEX_SIZE(SkPoint, MultiTexture::kYes, Domain::kNo, GrAA::kYes),
                TESS_FN_AND_VERTEX_SIZE(SkPoint, MultiTexture::kYes, Domain::kYes, GrAA::kNo),
                TESS_FN_AND_VERTEX_SIZE(SkPoint, MultiTexture::kYes, Domain::kYes, GrAA::kYes),
                TESS_FN_AND_VERTEX_SIZE(SkPoint3, MultiTexture::kNo, Domain::kNo, GrAA::kNo),
                TESS_FN_AND_VERTEX_SIZE(SkPoint3, MultiTexture::kNo, Domain::kNo, GrAA::kYes),
                TESS_FN_AND_VERTEX_SIZE(SkPoint3, MultiTexture::kNo, Domain::kYes, GrAA::kNo),
                TESS_FN_AND_VERTEX_SIZE(SkPoint3, MultiTexture::kNo, Domain::kYes, GrAA::kYes),
                TESS_FN_AND_VERTEX_SIZE(SkPoint3, MultiTexture::kYes, Domain::kNo, GrAA::kNo),
                TESS_FN_AND_VERTEX_SIZE(SkPoint3, MultiTexture::kYes, Domain::kNo, GrAA::kYes),
                TESS_FN_AND_VERTEX_SIZE(SkPoint3, MultiTexture::kYes, Domain::kYes, GrAA::kNo),
                TESS_FN_AND_VERTEX_SIZE(SkPoint3, MultiTexture::kYes, Domain::kYes, GrAA::kYes),
        };
#undef TESS_FN_AND_VERTEX_SIZE
        int tessFnIdx = 0;
        tessFnIdx |= coverageAA      ? 0x1 : 0x0;
        tessFnIdx |= fDomain         ? 0x2 : 0x0;
        tessFnIdx |= (fProxyCnt > 1) ? 0x4 : 0x0;
        tessFnIdx |= fPerspective    ? 0x8 : 0x0;

        SkASSERT(kTessFnsAndVertexSizes[tessFnIdx].fVertexSize == gp->debugOnly_vertexStride());

        int vstart;
        const GrBuffer* vbuffer;
        void* vdata = target->makeVertexSpace(kTessFnsAndVertexSizes[tessFnIdx].fVertexSize,
                                              4 * fDraws.count(), &vbuffer, &vstart);
        if (!vdata) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        float iw[kMaxTextures];
        float ih[kMaxTextures];
        for (int t = 0; t < fProxyCnt; ++t) {
            const auto* texture = proxies[t]->priv().peekTexture();
            iw[t] = 1.f / texture->width();
            ih[t] = 1.f / texture->height();
        }

        (this->*(kTessFnsAndVertexSizes[tessFnIdx].fTessFn))(vdata, iw, ih, gp.get());

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
        target->draw(gp.get(), pipeline, fixedDynamicState, mesh);
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
                fDraws[i].setTextureIdx(map[fDraws[i].textureIdx()]);
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
        fPerspective |= that->fPerspective;
        fDomain |= that->fDomain;
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

    class Draw {
    public:
        Draw(const SkRect& srcRect, int textureIdx, const GrPerspQuad& quad,
             SkCanvas::SrcRectConstraint constraint, GrColor color)
                : fSrcRect(srcRect)
                , fHasDomain(constraint == SkCanvas::kStrict_SrcRectConstraint)
                , fTextureIdx(SkToUInt(textureIdx))
                , fQuad(quad)
                , fColor(color) {}
        const GrPerspQuad& quad() const { return fQuad; }
        int textureIdx() const { return SkToInt(fTextureIdx); }
        const SkRect& srcRect() const { return fSrcRect; }
        GrColor color() const { return fColor; }
        Domain domain() const { return Domain(fHasDomain); }
        void setTextureIdx(int i) { fTextureIdx = SkToUInt(i); }

    private:
        SkRect fSrcRect;
        unsigned fHasDomain : 1;
        unsigned fTextureIdx : 31;
        GrPerspQuad fQuad;
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
    unsigned fPerspective : 1;
    unsigned fDomain : 1;
    // Used to track whether fProxy is ref'ed or has a pending IO after finalize() is called.
    unsigned fFinalized : 1;

    typedef GrMeshDrawOp INHERITED;
};

constexpr int TextureGeometryProcessor::kMaxTextures;
constexpr int TextureOp::kMaxTextures;

}  // anonymous namespace

namespace GrTextureOp {

std::unique_ptr<GrDrawOp> Make(GrContext* context,
                               sk_sp<GrTextureProxy> proxy,
                               GrSamplerState::Filter filter,
                               GrColor color,
                               const SkRect& srcRect,
                               const SkRect& dstRect,
                               GrAAType aaType,
                               SkCanvas::SrcRectConstraint constraint,
                               const SkMatrix& viewMatrix,
                               sk_sp<GrColorSpaceXform> csxf) {
    return TextureOp::Make(context, std::move(proxy), filter, color, srcRect, dstRect, aaType,
                           constraint, viewMatrix, std::move(csxf));
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
    auto origin = random->nextBool() ? kTopLeft_GrSurfaceOrigin : kBottomLeft_GrSurfaceOrigin;
    GrMipMapped mipMapped = random->nextBool() ? GrMipMapped::kYes : GrMipMapped::kNo;
    SkBackingFit fit = SkBackingFit::kExact;
    if (mipMapped == GrMipMapped::kNo) {
        fit = random->nextBool() ? SkBackingFit::kApprox : SkBackingFit::kExact;
    }

    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
    sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(desc, origin, mipMapped, fit,
                                                             SkBudgeted::kNo,
                                                             GrInternalSurfaceFlags::kNone);

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
    while (mipMapped == GrMipMapped::kNo && filter == GrSamplerState::Filter::kMipMap) {
        filter = (GrSamplerState::Filter)random->nextULessThan(
                static_cast<uint32_t>(GrSamplerState::Filter::kMipMap) + 1);
    }
    auto csxf = GrTest::TestColorXform(random);
    GrAAType aaType = GrAAType::kNone;
    if (random->nextBool()) {
        aaType = (fsaaType == GrFSAAType::kUnifiedMSAA) ? GrAAType::kMSAA : GrAAType::kCoverage;
    }
    auto constraint = random->nextBool() ? SkCanvas::kStrict_SrcRectConstraint
                                         : SkCanvas::kFast_SrcRectConstraint;
    return GrTextureOp::Make(context, std::move(proxy), filter, color, srcRect, rect, aaType,
                             constraint, viewMatrix, std::move(csxf));
}

#endif
