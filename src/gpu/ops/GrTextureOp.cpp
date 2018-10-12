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

    template <typename Pos, Domain D> struct OptionalDomainVertex;
    template <typename Pos>
    struct OptionalDomainVertex<Pos, Domain::kNo> : VertexCommon<Pos> {
        static constexpr Domain kDomain = Domain::kNo;
    };
    template <typename Pos>
    struct OptionalDomainVertex<Pos, Domain::kYes> : VertexCommon<Pos> {
        static constexpr Domain kDomain = Domain::kYes;
        SkRect fTextureDomain;
    };

    template <typename Pos, Domain D, GrAA> struct OptionalAAVertex;
    template <typename Pos, Domain D>
    struct OptionalAAVertex<Pos, D, GrAA::kNo> : OptionalDomainVertex<Pos, D> {
        static constexpr GrAA kAA = GrAA::kNo;
    };
    template <typename Pos, Domain D>
    struct OptionalAAVertex<Pos, D, GrAA::kYes> : OptionalDomainVertex<Pos, D> {
        static constexpr GrAA kAA = GrAA::kYes;
        SkPoint3 fEdges[4];
    };

    template <typename Pos, Domain D, GrAA AA>
    using Vertex = OptionalAAVertex<Pos, D, AA>;

    static sk_sp<GrGeometryProcessor> Make(GrTextureType textureType, GrPixelConfig textureConfig,
                                           const GrSamplerState::Filter filter,
                                           sk_sp<GrColorSpaceXform> textureColorSpaceXform,
                                           sk_sp<GrColorSpaceXform> paintColorSpaceXform,
                                           bool coverageAA, bool perspective, Domain domain,
                                           const GrShaderCaps& caps) {
        return sk_sp<TextureGeometryProcessor>(new TextureGeometryProcessor(
                textureType, textureConfig, filter, std::move(textureColorSpaceXform),
                std::move(paintColorSpaceXform), coverageAA, perspective, domain, caps));
    }

    const char* name() const override { return "TextureGeometryProcessor"; }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        b->add32(GrColorSpaceXform::XformKey(fTextureColorSpaceXform.get()));
        b->add32(GrColorSpaceXform::XformKey(fPaintColorSpaceXform.get()));
        uint32_t x = this->usesCoverageEdgeAA() ? 0 : 1;
        x |= kFloat3_GrVertexAttribType == fPositions.cpuType() ? 0 : 2;
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
                fTextureColorSpaceXformHelper.setData(
                        pdman, textureGP.fTextureColorSpaceXform.get());
                fPaintColorSpaceXformHelper.setData(pdman, textureGP.fPaintColorSpaceXform.get());
            }

        private:
            void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
                using Interpolation = GrGLSLVaryingHandler::Interpolation;
                const auto& textureGP = args.fGP.cast<TextureGeometryProcessor>();
                fTextureColorSpaceXformHelper.emitCode(
                        args.fUniformHandler, textureGP.fTextureColorSpaceXform.get());
                fPaintColorSpaceXformHelper.emitCode(
                        args.fUniformHandler, textureGP.fPaintColorSpaceXform.get(),
                        kVertex_GrShaderFlag);
                if (kFloat2_GrVertexAttribType == textureGP.fPositions.cpuType()) {
                    args.fVaryingHandler->setNoPerspective();
                }
                args.fVaryingHandler->emitAttributes(textureGP);
                gpArgs->fPositionVar = textureGP.fPositions.asShaderVar();

                this->emitTransforms(args.fVertBuilder,
                                     args.fVaryingHandler,
                                     args.fUniformHandler,
                                     textureGP.fTextureCoords.asShaderVar(),
                                     args.fFPCoordTransformHandler);
                if (fPaintColorSpaceXformHelper.isNoop()) {
                    args.fVaryingHandler->addPassThroughAttribute(
                            textureGP.fColors, args.fOutputColor, Interpolation::kCanBeFlat);
                } else {
                    GrGLSLVarying varying(kHalf4_GrSLType);
                    args.fVaryingHandler->addVarying("color", &varying);
                    args.fVertBuilder->codeAppend("half4 color = ");
                    args.fVertBuilder->appendColorGamutXform(textureGP.fColors.name(),
                                                             &fPaintColorSpaceXformHelper);
                    args.fVertBuilder->codeAppend(";");
                    args.fVertBuilder->codeAppendf("%s = half4(color.rgb * color.a, color.a);",
                                                   varying.vsOut());
                    args.fFragBuilder->codeAppendf("%s = %s;", args.fOutputColor, varying.fsIn());
                }
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
                args.fFragBuilder->codeAppendf("%s = ", args.fOutputColor);
                args.fFragBuilder->appendTextureLookupAndModulate(
                        args.fOutputColor, args.fTexSamplers[0], "texCoord", kFloat2_GrSLType,
                        &fTextureColorSpaceXformHelper);
                args.fFragBuilder->codeAppend(";");
                if (textureGP.usesCoverageEdgeAA()) {
                    bool mulByFragCoordW = false;
                    GrGLSLVarying aaDistVarying(kFloat4_GrSLType,
                                                GrGLSLVarying::Scope::kVertToFrag);
                    if (kFloat3_GrVertexAttribType == textureGP.fPositions.cpuType()) {
                        args.fVaryingHandler->addVarying("aaDists", &aaDistVarying);
                        // The distance from edge equation e to homogeneous point p=sk_Position
                        // is e.x*p.x/p.w + e.y*p.y/p.w + e.z. However, we want screen space
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
                    args.fFragBuilder->codeAppendf("%s = float4(saturate(mindist));",
                                                   args.fOutputCoverage);
                } else {
                    args.fFragBuilder->codeAppendf("%s = float4(1);", args.fOutputCoverage);
                }
            }
            GrGLSLColorSpaceXformHelper fTextureColorSpaceXformHelper;
            GrGLSLColorSpaceXformHelper fPaintColorSpaceXformHelper;
        };
        return new GLSLProcessor;
    }

    bool usesCoverageEdgeAA() const { return SkToBool(fAAEdges[0].isInitialized()); }

private:
    TextureGeometryProcessor(GrTextureType textureType, GrPixelConfig textureConfig,
                             GrSamplerState::Filter filter,
                             sk_sp<GrColorSpaceXform> textureColorSpaceXform,
                             sk_sp<GrColorSpaceXform> paintColorSpaceXform, bool coverageAA,
                             bool perspective, Domain domain, const GrShaderCaps& caps)
            : INHERITED(kTextureGeometryProcessor_ClassID)
            , fTextureColorSpaceXform(std::move(textureColorSpaceXform))
            , fPaintColorSpaceXform(std::move(paintColorSpaceXform))
            , fSampler(textureType, textureConfig, filter) {
        this->setTextureSamplerCnt(1);

        if (perspective) {
            fPositions = {"position", kFloat3_GrVertexAttribType, kFloat3_GrSLType};
        } else {
            fPositions = {"position", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
        }
        fColors = {"color", kUByte4_norm_GrVertexAttribType, kHalf4_GrSLType};
        fTextureCoords = {"textureCoords", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
        int vertexAttributeCnt = 3;

        if (domain == Domain::kYes) {
            fDomain = {"domain", kFloat4_GrVertexAttribType, kFloat4_GrSLType};
            ++vertexAttributeCnt;
        }
        if (coverageAA) {
            fAAEdges[0] = {"aaEdge0", kFloat3_GrVertexAttribType, kFloat3_GrSLType};
            fAAEdges[1] = {"aaEdge1", kFloat3_GrVertexAttribType, kFloat3_GrSLType};
            fAAEdges[2] = {"aaEdge2", kFloat3_GrVertexAttribType, kFloat3_GrSLType};
            fAAEdges[3] = {"aaEdge3", kFloat3_GrVertexAttribType, kFloat3_GrSLType};
            vertexAttributeCnt += 4;
        }
        this->setVertexAttributeCnt(vertexAttributeCnt);
    }

    const Attribute& onVertexAttribute(int i) const override {
        return IthInitializedAttribute(i, fPositions, fColors, fTextureCoords, fDomain, fAAEdges[0],
                                       fAAEdges[1], fAAEdges[2], fAAEdges[3]);
    }

    const TextureSampler& onTextureSampler(int) const override { return fSampler; }

    Attribute fPositions;
    Attribute fColors;
    Attribute fTextureCoords;
    Attribute fDomain;
    Attribute fAAEdges[4];
    sk_sp<GrColorSpaceXform> fTextureColorSpaceXform;
    sk_sp<GrColorSpaceXform> fPaintColorSpaceXform;
    TextureSampler fSampler;

    typedef GrGeometryProcessor INHERITED;
};

// This computes the four edge equations for a quad, then outsets them and optionally computes a new
// quad as the intersection points of the outset edges. 'x' and 'y' contain the original points as
// input and the outset points as output. 'a', 'b', and 'c' are the edge equation coefficients on
// output.
static void compute_quad_edges_and_outset_vertices(GrQuadAAFlags aaFlags, Sk4f* x, Sk4f* y, Sk4f* a,
                                                   Sk4f* b, Sk4f* c, bool outsetCorners) {
    SkASSERT(GrQuadAAFlags::kNone != aaFlags);

    static constexpr auto fma = SkNx_fma<4, float>;
    // These rotate the points/edge values either clockwise or counterclockwise assuming tri strip
    // order.
    auto nextCW  = [](const Sk4f& v) { return SkNx_shuffle<2, 0, 3, 1>(v); };
    auto nextCCW = [](const Sk4f& v) { return SkNx_shuffle<1, 3, 0, 2>(v); };

    // Compute edge equations for the quad.
    auto xnext = nextCCW(*x);
    auto ynext = nextCCW(*y);
    // xdiff and ydiff will comprise the normalized vectors pointing along each quad edge.
    auto xdiff = xnext - *x;
    auto ydiff = ynext - *y;
    auto invLengths = fma(xdiff, xdiff, ydiff * ydiff).rsqrt();
    xdiff *= invLengths;
    ydiff *= invLengths;

    // Use above vectors to compute edge equations.
    *c = fma(xnext, *y,  -ynext * *x) * invLengths;
    // Make sure the edge equations have their normals facing into the quad in device space.
    auto test = fma(ydiff, nextCW(*x), fma(-xdiff, nextCW(*y), *c));
    if ((test < Sk4f(0)).anyTrue()) {
        *a = -ydiff;
        *b = xdiff;
        *c = -*c;
    } else {
        *a = ydiff;
        *b = -xdiff;
    }
    // Outset the edge equations so aa coverage evaluates to zero half a pixel away from the
    // original quad edge.
    *c += 0.5f;

    if (aaFlags != GrQuadAAFlags::kAll) {
        // This order is the same order the edges appear in xdiff/ydiff and therefore as the
        // edges in a/b/c.
        auto mask = Sk4f(GrQuadAAFlags::kLeft & aaFlags ? 1.f : 0.f,
                         GrQuadAAFlags::kBottom & aaFlags ? 1.f : 0.f,
                         GrQuadAAFlags::kTop & aaFlags ? 1.f : 0.f,
                         GrQuadAAFlags::kRight & aaFlags ? 1.f : 0.f);
        // Outset edge equations for masked out edges another pixel so that they always evaluate
        // >= 1.
        *c += (1.f - mask);
        if (outsetCorners) {
            // Do the vertex outset.
            mask *= 0.5f;
            auto maskCW = nextCW(mask);
            *x += maskCW * -xdiff + mask * nextCW(xdiff);
            *y += maskCW * -ydiff + mask * nextCW(ydiff);
        }
    } else if (outsetCorners) {
        *x += 0.5f * (-xdiff + nextCW(xdiff));
        *y += 0.5f * (-ydiff + nextCW(ydiff));
    }
}

namespace {
// This is a class soley so it can be partially specialized (functions cannot be).
template <typename V, GrAA AA = V::kAA, typename Position = typename V::Position>
class VertexAAHandler;

template<typename V> class VertexAAHandler<V, GrAA::kNo, SkPoint> {
public:
    static void AssignPositionsAndTexCoords(V* vertices, const GrPerspQuad& quad,
                                            GrQuadAAFlags aaFlags, const SkRect& texRect) {
        // Should be kNone for non-AA and kAll for MSAA.
        SkASSERT(aaFlags == GrQuadAAFlags::kNone || aaFlags == GrQuadAAFlags::kAll);
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
                                            GrQuadAAFlags aaFlags, const SkRect& texRect) {
        // Should be kNone for non-AA and kAll for MSAA.
        SkASSERT(aaFlags == GrQuadAAFlags::kNone || aaFlags == GrQuadAAFlags::kAll);
        SkPointPriv::SetRectTriStrip(&vertices[0].fTextureCoords, texRect, sizeof(V));
        for (int i = 0; i < 4; ++i) {
            vertices[i].fPosition = quad.point(i);
        }
    }
};

template<typename V> class VertexAAHandler<V, GrAA::kYes, SkPoint> {
public:
    static void AssignPositionsAndTexCoords(V* vertices, const GrPerspQuad& quad,
                                            GrQuadAAFlags aaFlags, const SkRect& texRect) {
        SkASSERT((quad.w4f() == Sk4f(1.f)).allTrue());
        auto x = quad.x4f();
        auto y = quad.y4f();
        Sk4f a, b, c;
        compute_quad_edges_and_outset_vertices(aaFlags, &x, &y, &a, &b, &c, true);

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
                                            GrQuadAAFlags aaFlags, const SkRect& texRect) {
        auto x = quad.x4f();
        auto y = quad.y4f();
        auto iw = quad.iw4f();

        Sk4f a, b, c;
        auto x2d = x * iw;
        auto y2d = y * iw;
        compute_quad_edges_and_outset_vertices(aaFlags, &x2d, &y2d, &a, &b, &c, false);
        auto w = quad.w4f();
        static const float kOutset = 0.5f;
        if ((GrQuadAAFlags::kLeft | GrQuadAAFlags::kRight) & aaFlags) {
            // For each entry in x the equivalent entry in opX is the left/right opposite and so on.
            Sk4f opX = SkNx_shuffle<2, 3, 0, 1>(x);
            Sk4f opW = SkNx_shuffle<2, 3, 0, 1>(w);
            Sk4f opY = SkNx_shuffle<2, 3, 0, 1>(y);
            // vx/vy holds the device space left-to-right vectors along top and bottom of the quad.
            Sk2f vx = SkNx_shuffle<2, 3>(x2d) - SkNx_shuffle<0, 1>(x2d);
            Sk2f vy = SkNx_shuffle<2, 3>(y2d) - SkNx_shuffle<0, 1>(y2d);
            Sk2f len = SkNx_fma(vx, vx, vy * vy).sqrt();
            // For each device space corner, devP, label its left/right opposite device space point
            // opDevPt. The new device space point is opDevPt + s (devPt - opDevPt) where s is
            // (length(devPt - opDevPt) + 0.5) / length(devPt - opDevPt);
            Sk4f s = SkNx_shuffle<0, 1, 0, 1>((len + kOutset) / len);
            // Compute t in homogeneous space from s using similar triangles so that we can produce
            // homogeneous outset vertices for perspective-correct interpolation.
            Sk4f sOpW = s * opW;
            Sk4f t = sOpW / (sOpW + (1.f - s) * w);
            // mask is used to make the t values be 1 when the left/right side is not antialiased.
            Sk4f mask(GrQuadAAFlags::kLeft & aaFlags  ? 1.f : 0.f,
                      GrQuadAAFlags::kLeft & aaFlags  ? 1.f : 0.f,
                      GrQuadAAFlags::kRight & aaFlags ? 1.f : 0.f,
                      GrQuadAAFlags::kRight & aaFlags ? 1.f : 0.f);
            t = t * mask + (1.f - mask);
            x = opX + t * (x - opX);
            y = opY + t * (y - opY);
            w = opW + t * (w - opW);
            if ((GrQuadAAFlags::kTop | GrQuadAAFlags::kBottom) & aaFlags) {
                // Update the 2D points for the top/bottom calculation.
                iw = w.invert();
                x2d = x * iw;
                y2d = y * iw;
            }
        }

        if ((GrQuadAAFlags::kTop | GrQuadAAFlags::kBottom) & aaFlags) {
            // This operates the same as above but for top/bottom rather than left/right.
            Sk4f opX = SkNx_shuffle<1, 0, 3, 2>(x);
            Sk4f opW = SkNx_shuffle<1, 0, 3, 2>(w);
            Sk4f opY = SkNx_shuffle<1, 0, 3, 2>(y);

            Sk2f vx = SkNx_shuffle<1, 3>(x2d) - SkNx_shuffle<0, 2>(x2d);
            Sk2f vy = SkNx_shuffle<1, 3>(y2d) - SkNx_shuffle<0, 2>(y2d);
            Sk2f len = SkNx_fma(vx, vx, vy * vy).sqrt();

            Sk4f s = SkNx_shuffle<0, 0, 1, 1>((len + kOutset) / len);

            Sk4f sOpW = s * opW;
            Sk4f t = sOpW / (sOpW + (1.f - s) * w);

            Sk4f mask(GrQuadAAFlags::kTop    & aaFlags ? 1.f : 0.f,
                      GrQuadAAFlags::kBottom & aaFlags ? 1.f : 0.f,
                      GrQuadAAFlags::kTop    & aaFlags ? 1.f : 0.f,
                      GrQuadAAFlags::kBottom & aaFlags ? 1.f : 0.f);
            t = t * mask + (1.f - mask);
            x = opX + t * (x - opX);
            y = opY + t * (y - opY);
            w = opW + t * (w - opW);
        }
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
static void tessellate_quad(const GrPerspQuad& devQuad, GrQuadAAFlags aaFlags,
                            const SkRect& srcRect, GrColor color, GrSurfaceOrigin origin,
                            GrSamplerState::Filter filter, V* vertices, SkScalar iw, SkScalar ih,
                            Domain domain) {
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
    VertexAAHandler<V>::AssignPositionsAndTexCoords(vertices, devQuad, aaFlags, texRect);
    vertices[0].fColor = color;
    vertices[1].fColor = color;
    vertices[2].fColor = color;
    vertices[3].fColor = color;
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
                                          GrQuadAAFlags aaFlags,
                                          SkCanvas::SrcRectConstraint constraint,
                                          const SkMatrix& viewMatrix,
                                          sk_sp<GrColorSpaceXform> textureColorSpaceXform,
                                          sk_sp<GrColorSpaceXform> paintColorSpaceXform) {
        GrOpMemoryPool* pool = context->contextPriv().opMemoryPool();

        return pool->allocate<TextureOp>(
                std::move(proxy), filter, color, srcRect, dstRect, aaType, aaFlags, constraint,
                viewMatrix, std::move(textureColorSpaceXform), std::move(paintColorSpaceXform));
    }

    ~TextureOp() override {
        if (fFinalized) {
            fProxy->completedRead();
        } else {
            fProxy->unref();
        }
    }

    const char* name() const override { return "TextureOp"; }

    void visitProxies(const VisitProxyFunc& func) const override { func(fProxy); }

    SkString dumpInfo() const override {
        SkString str;
        str.appendf("# draws: %d\n", fDraws.count());
        str.appendf("Proxy ID: %d, Filter: %d\n", fProxy->uniqueID().asUInt(),
                    static_cast<int>(fFilter));
        for (int i = 0; i < fDraws.count(); ++i) {
            const Draw& draw = fDraws[i];
            str.appendf(
                    "%d: Color: 0x%08x, TexRect [L: %.2f, T: %.2f, R: %.2f, B: %.2f] "
                    "Quad [(%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f)]\n",
                    i, draw.color(), draw.srcRect().fLeft, draw.srcRect().fTop,
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
        fFinalized = true;
        fProxy->addPendingRead();
        fProxy->unref();
        return RequiresDstTexture::kNo;
    }

    FixedFunctionFlags fixedFunctionFlags() const override {
        return this->aaType() == GrAAType::kMSAA ? FixedFunctionFlags::kUsesHWAA
                                                 : FixedFunctionFlags::kNone;
    }

    DEFINE_OP_CLASS_ID

private:
    friend class ::GrOpMemoryPool;

    TextureOp(sk_sp<GrTextureProxy> proxy, GrSamplerState::Filter filter, GrColor color,
              const SkRect& srcRect, const SkRect& dstRect, GrAAType aaType, GrQuadAAFlags aaFlags,
              SkCanvas::SrcRectConstraint constraint, const SkMatrix& viewMatrix,
              sk_sp<GrColorSpaceXform> textureColorSpaceXform,
              sk_sp<GrColorSpaceXform> paintColorSpaceXform)
            : INHERITED(ClassID())
            , fTextureColorSpaceXform(std::move(textureColorSpaceXform))
            , fPaintColorSpaceXform(std::move(paintColorSpaceXform))
            , fProxy(proxy.release())
            , fFilter(filter)
            , fAAType(static_cast<unsigned>(aaType))
            , fFinalized(0) {
        switch (aaType) {
            case GrAAType::kNone:
                aaFlags = GrQuadAAFlags::kNone;
                break;
            case GrAAType::kCoverage:
                if (aaFlags == GrQuadAAFlags::kNone) {
                    fAAType = static_cast<unsigned>(GrAAType::kNone);
                }
                break;
            case GrAAType::kMSAA:
                aaFlags = GrQuadAAFlags::kAll;
                break;
            case GrAAType::kMixedSamples:
                SK_ABORT("Should not use mixed sample AA");
                break;
        }
        fPerspective = viewMatrix.hasPerspective();
        auto quad = GrPerspQuad(dstRect, viewMatrix);
        auto bounds = quad.bounds();
        // We expect our caller to have already caught this optimization.
        SkASSERT(!srcRect.contains(fProxy->getWorstCaseBoundsRect()) ||
                 constraint == SkCanvas::kFast_SrcRectConstraint);
        if (viewMatrix.rectStaysRect()) {
            // Disable filtering when there is no scaling or fractional translation.
            // Disable coverage AA when rect falls on integers in device space.
            if (SkScalarIsInt(bounds.fLeft) && SkScalarIsInt(bounds.fTop) &&
                SkScalarIsInt(bounds.fRight) && SkScalarIsInt(bounds.fBottom)) {
                if (viewMatrix.isScaleTranslate()) {
                    if (bounds.width() == srcRect.width() && bounds.height() == srcRect.height()) {
                        fFilter = GrSamplerState::Filter::kNearest;
                    }
                } else {
                    if (bounds.width() == srcRect.height() && bounds.height() == srcRect.width()) {
                        fFilter = GrSamplerState::Filter::kNearest;
                    }
                }
                if (GrAAType::kCoverage == this->aaType()) {
                    fAAType = static_cast<unsigned>(GrAAType::kNone);
                    aaFlags = GrQuadAAFlags::kNone;
                }
            }
        }
        // We may have had a strict constraint with nearest filter solely due to possible AA bloat.
        // If we don't have (or determined we don't need) coverage AA then we can skip using a
        // domain.
        if (constraint == SkCanvas::kStrict_SrcRectConstraint &&
            fFilter == GrSamplerState::Filter::kNearest &&
            this->aaType() != GrAAType::kCoverage) {
            constraint = SkCanvas::kFast_SrcRectConstraint;
        }
        const auto& draw = fDraws.emplace_back(srcRect, quad, aaFlags, constraint, color);
        this->setBounds(bounds, HasAABloat::kNo, IsZeroArea::kNo);
        fDomain = static_cast<bool>(draw.domain());
    }

    template <typename Pos, Domain D, GrAA AA>
    void tess(void* v, const GrGeometryProcessor* gp) const {
        using Vertex = TextureGeometryProcessor::Vertex<Pos, D, AA>;
        SkASSERT(gp->debugOnly_vertexStride() == sizeof(Vertex));
        auto vertices = static_cast<Vertex*>(v);
        auto origin = fProxy->origin();
        const auto* texture = fProxy->peekTexture();
        float iw = 1.f / texture->width();
        float ih = 1.f / texture->height();

        for (const auto& draw : fDraws) {
            tessellate_quad<Vertex>(draw.quad(), draw.aaFlags(), draw.srcRect(), draw.color(),
                                    origin, fFilter, vertices, iw, ih, draw.domain());
            vertices += 4;
        }
    }

    void onPrepareDraws(Target* target) override {
        bool hasPerspective = false;
        Domain domain = Domain::kNo;
        int numOps = 0;
        for (const auto& op : ChainRange<TextureOp>(this)) {
            ++numOps;
            hasPerspective |= op.fPerspective;
            if (op.fDomain) {
                domain = Domain::kYes;
            }
            if (!op.fProxy->instantiate(target->resourceProvider())) {
                return;
            }
        }

        bool coverageAA = GrAAType::kCoverage == this->aaType();
        sk_sp<GrGeometryProcessor> gp = TextureGeometryProcessor::Make(
                fProxy->textureType(), fProxy->config(), fFilter,
                std::move(fTextureColorSpaceXform), std::move(fPaintColorSpaceXform), coverageAA,
                hasPerspective, domain, *target->caps().shaderCaps());
        GrPipeline::InitArgs args;
        args.fProxy = target->proxy();
        args.fCaps = &target->caps();
        args.fResourceProvider = target->resourceProvider();
        args.fFlags = 0;
        if (GrAAType::kMSAA == this->aaType()) {
            args.fFlags |= GrPipeline::kHWAntialias_Flag;
        }

        auto clip = target->detachAppliedClip();
        // We'll use a dynamic state array for the GP textures when there are multiple ops.
        // Otherwise, we use fixed dynamic state to specify the single op's proxy.
        GrPipeline::DynamicStateArrays* dynamicStateArrays = nullptr;
        GrPipeline::FixedDynamicState* fixedDynamicState;
        if (numOps > 1) {
            dynamicStateArrays = target->allocDynamicStateArrays(numOps, 1, false);
            fixedDynamicState = target->allocFixedDynamicState(clip.scissorState().rect(), 0);
        } else {
            fixedDynamicState = target->allocFixedDynamicState(clip.scissorState().rect(), 1);
            fixedDynamicState->fPrimitiveProcessorTextures[0] = fProxy;
        }
        const auto* pipeline =
                target->allocPipeline(args, GrProcessorSet::MakeEmptySet(), std::move(clip));
        using TessFn = decltype(&TextureOp::tess<SkPoint, Domain::kNo, GrAA::kNo>);
#define TESS_FN_AND_VERTEX_SIZE(Point, Domain, AA)                          \
    {                                                                       \
        &TextureOp::tess<Point, Domain, AA>,                                \
                sizeof(TextureGeometryProcessor::Vertex<Point, Domain, AA>) \
    }
        static constexpr struct {
            TessFn fTessFn;
            size_t fVertexSize;
        } kTessFnsAndVertexSizes[] = {
                TESS_FN_AND_VERTEX_SIZE(SkPoint,  Domain::kNo,  GrAA::kNo),
                TESS_FN_AND_VERTEX_SIZE(SkPoint,  Domain::kNo,  GrAA::kYes),
                TESS_FN_AND_VERTEX_SIZE(SkPoint,  Domain::kYes, GrAA::kNo),
                TESS_FN_AND_VERTEX_SIZE(SkPoint,  Domain::kYes, GrAA::kYes),
                TESS_FN_AND_VERTEX_SIZE(SkPoint3, Domain::kNo,  GrAA::kNo),
                TESS_FN_AND_VERTEX_SIZE(SkPoint3, Domain::kNo,  GrAA::kYes),
                TESS_FN_AND_VERTEX_SIZE(SkPoint3, Domain::kYes, GrAA::kNo),
                TESS_FN_AND_VERTEX_SIZE(SkPoint3, Domain::kYes, GrAA::kYes),
        };
#undef TESS_FN_AND_VERTEX_SIZE
        int tessFnIdx = 0;
        tessFnIdx |= coverageAA               ? 0x1 : 0x0;
        tessFnIdx |= (domain == Domain::kYes) ? 0x2 : 0x0;
        tessFnIdx |= hasPerspective           ? 0x4 : 0x0;

        SkASSERT(kTessFnsAndVertexSizes[tessFnIdx].fVertexSize == gp->debugOnly_vertexStride());

        GrMesh* meshes = target->allocMeshes(numOps);
        int i = 0;
        for (const auto& op : ChainRange<TextureOp>(this)) {
            int vstart;
            const GrBuffer* vbuffer;
            void* vdata = target->makeVertexSpace(kTessFnsAndVertexSizes[tessFnIdx].fVertexSize,
                                                  4 * op.fDraws.count(), &vbuffer, &vstart);
            if (!vdata) {
                SkDebugf("Could not allocate vertices\n");
                return;
            }

            (op.*(kTessFnsAndVertexSizes[tessFnIdx].fTessFn))(vdata, gp.get());

            if (op.fDraws.count() > 1) {
                meshes[i].setPrimitiveType(GrPrimitiveType::kTriangles);
                sk_sp<const GrBuffer> ibuffer = target->resourceProvider()->refQuadIndexBuffer();
                if (!ibuffer) {
                    SkDebugf("Could not allocate quad indices\n");
                    return;
                }
                meshes[i].setIndexedPatterned(ibuffer.get(), 6, 4, op.fDraws.count(),
                                              GrResourceProvider::QuadCountOfQuadBuffer());
            } else {
                meshes[i].setPrimitiveType(GrPrimitiveType::kTriangleStrip);
                meshes[i].setNonIndexedNonInstanced(4);
            }
            meshes[i].setVertexData(vbuffer, vstart);
            if (dynamicStateArrays) {
                dynamicStateArrays->fPrimitiveProcessorTextures[i] = op.fProxy;
            }
            ++i;
        }
        target->draw(std::move(gp), pipeline, fixedDynamicState, dynamicStateArrays, meshes,
                     numOps);
    }

    CombineResult onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        const auto* that = t->cast<TextureOp>();
        if (!GrColorSpaceXform::Equals(fTextureColorSpaceXform.get(),
                                       that->fTextureColorSpaceXform.get())) {
            return CombineResult::kCannotCombine;
        }
        if (!GrColorSpaceXform::Equals(fPaintColorSpaceXform.get(),
                                       that->fPaintColorSpaceXform.get())) {
            return CombineResult::kCannotCombine;
        }
        // TODO: Should we allow kNone and kCoverage to merge by upgrading kNone to kCoverage?
        // If we allowed chaining the head op would have to pre-iterate to determine the aa-type.
        if (this->aaType() != that->aaType()) {
            return CombineResult::kCannotCombine;
        }
        if (fFilter != that->fFilter) {
            return CombineResult::kCannotCombine;
        }
        if (fProxy->uniqueID() != that->fProxy->uniqueID() || that->isChained()) {
            // We can't merge across different proxies (and we're disallowed from merging when
            // 'that' is chained. Check if we can be chained with 'that'.
            if (fProxy->config() == that->fProxy->config() &&
                fProxy->textureType() == that->fProxy->textureType() &&
                caps.dynamicStateArrayGeometryProcessorTextureSupport()) {
                return CombineResult::kMayChain;
            }
            return CombineResult::kCannotCombine;
        }
        fDraws.push_back_n(that->fDraws.count(), that->fDraws.begin());
        this->joinBounds(*that);
        fPerspective |= that->fPerspective;
        fDomain |= that->fDomain;
        return CombineResult::kMerged;
    }

    GrAAType aaType() const { return static_cast<GrAAType>(fAAType); }

    class Draw {
    public:
        Draw(const SkRect& srcRect, const GrPerspQuad& quad, GrQuadAAFlags aaFlags,
             SkCanvas::SrcRectConstraint constraint, GrColor color)
                : fSrcRect(srcRect)
                , fQuad(quad)
                , fColor(color)
                , fHasDomain(constraint == SkCanvas::kStrict_SrcRectConstraint)
                , fAAFlags(static_cast<unsigned>(aaFlags)) {
            SkASSERT(fAAFlags == static_cast<unsigned>(aaFlags));
        }
        const GrPerspQuad& quad() const { return fQuad; }
        const SkRect& srcRect() const { return fSrcRect; }
        GrColor color() const { return fColor; }
        Domain domain() const { return Domain(fHasDomain); }
        GrQuadAAFlags aaFlags() const { return static_cast<GrQuadAAFlags>(fAAFlags); }

    private:
        SkRect fSrcRect;
        GrPerspQuad fQuad;
        GrColor fColor;
        unsigned fHasDomain : 1;
        unsigned fAAFlags : 4;
    };
    SkSTArray<1, Draw, true> fDraws;
    sk_sp<GrColorSpaceXform> fTextureColorSpaceXform;
    sk_sp<GrColorSpaceXform> fPaintColorSpaceXform;
    GrTextureProxy* fProxy;
    GrSamplerState::Filter fFilter;
    unsigned fAAType : 2;
    unsigned fPerspective : 1;
    unsigned fDomain : 1;
    // Used to track whether fProxy is ref'ed or has a pending IO after finalize() is called.
    unsigned fFinalized : 1;

    typedef GrMeshDrawOp INHERITED;
};

}  // anonymous namespace

namespace GrTextureOp {

std::unique_ptr<GrDrawOp> Make(GrContext* context,
                               sk_sp<GrTextureProxy> proxy,
                               GrSamplerState::Filter filter,
                               GrColor color,
                               const SkRect& srcRect,
                               const SkRect& dstRect,
                               GrAAType aaType,
                               GrQuadAAFlags aaFlags,
                               SkCanvas::SrcRectConstraint constraint,
                               const SkMatrix& viewMatrix,
                               sk_sp<GrColorSpaceXform> textureColorSpaceXform,
                               sk_sp<GrColorSpaceXform> paintColorSpaceXform) {
    return TextureOp::Make(context, std::move(proxy), filter, color, srcRect, dstRect, aaType,
                           aaFlags, constraint, viewMatrix, std::move(textureColorSpaceXform),
                           std::move(paintColorSpaceXform));
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
    auto texXform = GrTest::TestColorXform(random);
    auto paintXform = GrTest::TestColorXform(random);
    GrAAType aaType = GrAAType::kNone;
    if (random->nextBool()) {
        aaType = (fsaaType == GrFSAAType::kUnifiedMSAA) ? GrAAType::kMSAA : GrAAType::kCoverage;
    }
    GrQuadAAFlags aaFlags = GrQuadAAFlags::kNone;
    aaFlags |= random->nextBool() ? GrQuadAAFlags::kLeft : GrQuadAAFlags::kNone;
    aaFlags |= random->nextBool() ? GrQuadAAFlags::kTop : GrQuadAAFlags::kNone;
    aaFlags |= random->nextBool() ? GrQuadAAFlags::kRight : GrQuadAAFlags::kNone;
    aaFlags |= random->nextBool() ? GrQuadAAFlags::kBottom : GrQuadAAFlags::kNone;
    auto constraint = random->nextBool() ? SkCanvas::kStrict_SrcRectConstraint
                                         : SkCanvas::kFast_SrcRectConstraint;
    return GrTextureOp::Make(context, std::move(proxy), filter, color, srcRect, rect, aaType,
                             aaFlags, constraint, viewMatrix, std::move(texXform),
                             std::move(paintXform));
}

#endif
