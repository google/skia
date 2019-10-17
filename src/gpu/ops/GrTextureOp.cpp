/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <new>

#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "include/gpu/GrTexture.h"
#include "include/private/GrRecordingContext.h"
#include "include/private/SkFloatingPoint.h"
#include "include/private/SkTo.h"
#include "src/core/SkMathPriv.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkRectPriv.h"
#include "src/gpu/GrAppliedClip.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDrawOpTest.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrResourceProviderPriv.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrTextureDomain.h"
#include "src/gpu/effects/generated/GrSaturateProcessor.h"
#include "src/gpu/geometry/GrQuad.h"
#include "src/gpu/geometry/GrQuadBuffer.h"
#include "src/gpu/geometry/GrQuadUtils.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/ops/GrFillRectOp.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/ops/GrQuadPerEdgeAA.h"
#include "src/gpu/ops/GrTextureOp.h"

namespace {

using Domain = GrQuadPerEdgeAA::Domain;
using VertexSpec = GrQuadPerEdgeAA::VertexSpec;
using ColorType = GrQuadPerEdgeAA::ColorType;

// Extracts lengths of vertical and horizontal edges of axis-aligned quad. "width" is the edge
// between v0 and v2 (or v1 and v3), "height" is the edge between v0 and v1 (or v2 and v3).
static SkSize axis_aligned_quad_size(const GrQuad& quad) {
    SkASSERT(quad.quadType() == GrQuad::Type::kAxisAligned);
    // Simplification of regular edge length equation, since it's axis aligned and can avoid sqrt
    float dw = sk_float_abs(quad.x(2) - quad.x(0)) + sk_float_abs(quad.y(2) - quad.y(0));
    float dh = sk_float_abs(quad.x(1) - quad.x(0)) + sk_float_abs(quad.y(1) - quad.y(0));
    return {dw, dh};
}

static bool filter_has_effect(const GrQuad& srcQuad, const GrQuad& dstQuad) {
    // If not axis-aligned in src or dst, then always say it has an effect
    if (srcQuad.quadType() != GrQuad::Type::kAxisAligned ||
        dstQuad.quadType() != GrQuad::Type::kAxisAligned) {
        return true;
    }

    SkRect srcRect;
    SkRect dstRect;
    if (srcQuad.asRect(&srcRect) && dstQuad.asRect(&dstRect)) {
        // Disable filtering when there is no scaling (width and height are the same), and the
        // top-left corners have the same fraction (so src and dst snap to the pixel grid
        // identically).
        SkASSERT(srcRect.isSorted());
        return srcRect.width() != dstRect.width() || srcRect.height() != dstRect.height() ||
               SkScalarFraction(srcRect.fLeft) != SkScalarFraction(dstRect.fLeft) ||
               SkScalarFraction(srcRect.fTop) != SkScalarFraction(dstRect.fTop);
    } else {
        // Although the quads are axis-aligned, the local coordinate system is transformed such
        // that fractionally-aligned sample centers will not align with the device coordinate system
        // So disable filtering when edges are the same length and both srcQuad and dstQuad
        // 0th vertex is integer aligned.
        if (SkScalarIsInt(srcQuad.x(0)) && SkScalarIsInt(srcQuad.y(0)) &&
            SkScalarIsInt(dstQuad.x(0)) && SkScalarIsInt(dstQuad.y(0))) {
            // Extract edge lengths
            SkSize srcSize = axis_aligned_quad_size(srcQuad);
            SkSize dstSize = axis_aligned_quad_size(dstQuad);
            return srcSize.fWidth != dstSize.fWidth || srcSize.fHeight != dstSize.fHeight;
        } else {
            return true;
        }
    }
}

// if normalizing the domain then pass 1/width, 1/height, 1 for iw, ih, h. Otherwise pass
// 1, 1, and height.
static void compute_domain(Domain domain, GrSamplerState::Filter filter, GrSurfaceOrigin origin,
                           const SkRect& domainRect, float iw, float ih, float h, SkRect* out) {
    static constexpr SkRect kLargeRect = {-100000, -100000, 1000000, 1000000};
    if (domain == Domain::kNo) {
        // Either the quad has no domain constraint and is batched with a domain constrained op
        // (in which case we want a domain that doesn't restrict normalized tex coords), or the
        // entire op doesn't use the domain, in which case the returned value is ignored.
        *out = kLargeRect;
        return;
    }

    auto ltrb = Sk4f::Load(&domainRect);
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
        const Sk4f kAdd = {0.f, h, 0.f, h};
        ltrb = SkNx_shuffle<0, 3, 2, 1>(kMul * ltrb + kAdd);
    }

    ltrb.store(out);
}

// Normalizes logical src coords and corrects for origin
static void compute_src_quad(GrSurfaceOrigin origin, const GrQuad& srcQuad,
                               float iw, float ih, float h, GrQuad* out) {
    // The src quad should not have any perspective
    SkASSERT(!srcQuad.hasPerspective() && !out->hasPerspective());
    skvx::Vec<4, float> xs = srcQuad.x4f() * iw;
    skvx::Vec<4, float> ys = srcQuad.y4f() * ih;
    if (origin == kBottomLeft_GrSurfaceOrigin) {
        ys = h - ys;
    }
    xs.store(out->xs());
    ys.store(out->ys());
    out->setQuadType(srcQuad.quadType());
}

/**
 * Op that implements GrTextureOp::Make. It draws textured quads. Each quad can modulate against a
 * the texture by color. The blend with the destination is always src-over. The edges are non-AA.
 */
class TextureOp final : public GrMeshDrawOp {
public:
    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                          sk_sp<GrTextureProxy> proxy,
                                          sk_sp<GrColorSpaceXform> textureXform,
                                          GrSamplerState::Filter filter,
                                          const SkPMColor4f& color,
                                          GrTextureOp::Saturate saturate,
                                          GrAAType aaType,
                                          GrQuadAAFlags aaFlags,
                                          const GrQuad& deviceQuad,
                                          const GrQuad& localQuad,
                                          const SkRect* domain) {
        GrOpMemoryPool* pool = context->priv().opMemoryPool();
        return pool->allocate<TextureOp>(std::move(proxy), std::move(textureXform), filter, color,
                                         saturate, aaType, aaFlags, deviceQuad, localQuad, domain);
    }
    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                          const GrRenderTargetContext::TextureSetEntry set[],
                                          int cnt,
                                          GrSamplerState::Filter filter,
                                          GrTextureOp::Saturate saturate,
                                          GrAAType aaType,
                                          SkCanvas::SrcRectConstraint constraint,
                                          const SkMatrix& viewMatrix,
                                          sk_sp<GrColorSpaceXform> textureColorSpaceXform) {
        size_t size = sizeof(TextureOp) + sizeof(Proxy) * (cnt - 1);
        GrOpMemoryPool* pool = context->priv().opMemoryPool();
        void* mem = pool->allocate(size);
        return std::unique_ptr<GrDrawOp>(new (mem) TextureOp(set, cnt, filter, saturate, aaType,
                                                             constraint, viewMatrix,
                                                             std::move(textureColorSpaceXform)));
    }

    ~TextureOp() override {
        for (unsigned p = 0; p < fProxyCnt; ++p) {
            fProxies[p].fProxy->unref();
        }
    }

    const char* name() const override { return "TextureOp"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        for (unsigned p = 0; p < fProxyCnt; ++p) {
            bool mipped = (GrSamplerState::Filter::kMipMap == this->filter());
            func(fProxies[p].fProxy, GrMipMapped(mipped));
        }
    }

#ifdef SK_DEBUG
    SkString dumpInfo() const override {
        SkString str;
        str.appendf("# draws: %d\n", fQuads.count());
        auto iter = fQuads.iterator();
        for (unsigned p = 0; p < fProxyCnt; ++p) {
            str.appendf("Proxy ID: %d, Filter: %d\n", fProxies[p].fProxy->uniqueID().asUInt(),
                        static_cast<int>(fFilter));
            int i = 0;
            while(i < fProxies[p].fQuadCnt && iter.next()) {
                const GrQuad& quad = iter.deviceQuad();
                const GrQuad& uv = iter.localQuad();
                const ColorDomainAndAA& info = iter.metadata();
                str.appendf(
                        "%d: Color: 0x%08x, Domain(%d): [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n"
                        "  UVs  [(%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f)]\n"
                        "  Quad [(%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f)]\n",
                        i, info.fColor.toBytes_RGBA(), info.fHasDomain, info.fDomainRect.fLeft,
                        info.fDomainRect.fTop, info.fDomainRect.fRight, info.fDomainRect.fBottom,
                        quad.point(0).fX, quad.point(0).fY, quad.point(1).fX, quad.point(1).fY,
                        quad.point(2).fX, quad.point(2).fY, quad.point(3).fX, quad.point(3).fY,
                        uv.point(0).fX, uv.point(0).fY, uv.point(1).fX, uv.point(1).fY,
                        uv.point(2).fX, uv.point(2).fY, uv.point(3).fX, uv.point(3).fY);

                i++;
            }
        }
        str += INHERITED::dumpInfo();
        return str;
    }
#endif

    GrProcessorSet::Analysis finalize(
            const GrCaps& caps, const GrAppliedClip*, bool hasMixedSampledCoverage,
            GrClampType clampType) override {
        fColorType = static_cast<unsigned>(ColorType::kNone);
        auto iter = fQuads.metadata();
        while(iter.next()) {
            auto colorType = GrQuadPerEdgeAA::MinColorType(iter->fColor, clampType, caps);
            fColorType = SkTMax(fColorType, static_cast<unsigned>(colorType));
        }
        return GrProcessorSet::EmptySetAnalysis();
    }

    FixedFunctionFlags fixedFunctionFlags() const override {
        return this->aaType() == GrAAType::kMSAA ? FixedFunctionFlags::kUsesHWAA
                                                 : FixedFunctionFlags::kNone;
    }

    DEFINE_OP_CLASS_ID

private:
    friend class ::GrOpMemoryPool;

    struct ColorDomainAndAA {
        ColorDomainAndAA(const SkPMColor4f& color, const SkRect* domainRect, GrQuadAAFlags aaFlags)
                : fColor(color)
                , fDomainRect(domainRect ? *domainRect : SkRect::MakeEmpty())
                , fHasDomain(static_cast<unsigned>(domainRect ? Domain::kYes : Domain::kNo))
                , fAAFlags(static_cast<unsigned>(aaFlags)) {
            SkASSERT(fAAFlags == static_cast<unsigned>(aaFlags));
        }

        SkPMColor4f fColor;
        SkRect fDomainRect;
        unsigned fHasDomain : 1;
        unsigned fAAFlags : 4;

        Domain domain() const { return Domain(fHasDomain); }
        GrQuadAAFlags aaFlags() const { return static_cast<GrQuadAAFlags>(fAAFlags); }
    };
    struct Proxy {
        GrTextureProxy* fProxy;
        int fQuadCnt;
    };

    // dstQuad should be the geometry transformed by the view matrix. If domainRect
    // is not null it will be used to apply the strict src rect constraint.
    TextureOp(sk_sp<GrTextureProxy> proxy,
              sk_sp<GrColorSpaceXform> textureColorSpaceXform,
              GrSamplerState::Filter filter,
              const SkPMColor4f& color,
              GrTextureOp::Saturate saturate,
              GrAAType aaType,
              GrQuadAAFlags aaFlags,
              const GrQuad& dstQuad,
              const GrQuad& srcQuad,
              const SkRect* domainRect)
            : INHERITED(ClassID())
            , fQuads(1, true /* includes locals */)
            , fTextureColorSpaceXform(std::move(textureColorSpaceXform))
            , fSaturate(static_cast<unsigned>(saturate))
            , fFilter(static_cast<unsigned>(filter))
            , fPrePrepared(false) {
        // Clean up disparities between the overall aa type and edge configuration and apply
        // optimizations based on the rect and matrix when appropriate
        GrQuadUtils::ResolveAAType(aaType, aaFlags, dstQuad, &aaType, &aaFlags);
        fAAType = static_cast<unsigned>(aaType);

        // We expect our caller to have already caught this optimization.
        SkASSERT(!domainRect || !domainRect->contains(proxy->getWorstCaseBoundsRect()));

        // We may have had a strict constraint with nearest filter solely due to possible AA bloat.
        // If we don't have (or determined we don't need) coverage AA then we can skip using a
        // domain.
        if (domainRect && this->filter() == GrSamplerState::Filter::kNearest &&
            aaType != GrAAType::kCoverage) {
            domainRect = nullptr;
        }

        fQuads.append(dstQuad, {color, domainRect, aaFlags}, &srcQuad);

        fProxyCnt = 1;
        fProxies[0] = {proxy.release(), 1};
        this->setBounds(dstQuad.bounds(), HasAABloat(aaType == GrAAType::kCoverage),
                        IsHairline::kNo);
        fDomain = static_cast<unsigned>(domainRect != nullptr);
    }
    TextureOp(const GrRenderTargetContext::TextureSetEntry set[],
              int cnt,
              GrSamplerState::Filter filter,
              GrTextureOp::Saturate saturate,
              GrAAType aaType,
              SkCanvas::SrcRectConstraint constraint,
              const SkMatrix& viewMatrix,
              sk_sp<GrColorSpaceXform> textureColorSpaceXform)
            : INHERITED(ClassID())
            , fQuads(cnt, true /* includes locals */)
            , fTextureColorSpaceXform(std::move(textureColorSpaceXform))
            , fSaturate(static_cast<unsigned>(saturate))
            , fFilter(static_cast<unsigned>(filter))
            , fPrePrepared(false) {
        fProxyCnt = SkToUInt(cnt);
        SkRect bounds = SkRectPriv::MakeLargestInverted();
        GrAAType overallAAType = GrAAType::kNone; // aa type maximally compatible with all dst rects
        bool mustFilter = false;
        bool allOpaque = true;
        Domain netDomain = Domain::kNo;
        for (unsigned p = 0; p < fProxyCnt; ++p) {
            fProxies[p].fProxy = SkRef(set[p].fProxy.get());
            fProxies[p].fQuadCnt = 1;
            SkASSERT(fProxies[p].fProxy->textureType() == fProxies[0].fProxy->textureType());
            SkASSERT(fProxies[p].fProxy->config() == fProxies[0].fProxy->config());

            SkMatrix ctm = viewMatrix;
            if (set[p].fPreViewMatrix) {
                ctm.preConcat(*set[p].fPreViewMatrix);
            }

            // Use dstRect/srcRect unless dstClip is provided, in which case derive new source
            // coordinates by mapping dstClipQuad by the dstRect to srcRect transform.
            GrQuad quad, srcQuad;
            if (set[p].fDstClipQuad) {
                quad = GrQuad::MakeFromSkQuad(set[p].fDstClipQuad, ctm);

                SkPoint srcPts[4];
                GrMapRectPoints(set[p].fDstRect, set[p].fSrcRect, set[p].fDstClipQuad, srcPts, 4);
                srcQuad = GrQuad::MakeFromSkQuad(srcPts, SkMatrix::I());
            } else {
                quad = GrQuad::MakeFromRect(set[p].fDstRect, ctm);
                srcQuad = GrQuad(set[p].fSrcRect);
            }

            if (!mustFilter && this->filter() != GrSamplerState::Filter::kNearest) {
                mustFilter = filter_has_effect(srcQuad, quad);
            }

            bounds.joinPossiblyEmptyRect(quad.bounds());
            GrQuadAAFlags aaFlags;
            // Don't update the overall aaType, might be inappropriate for some of the quads
            GrAAType aaForQuad;
            GrQuadUtils::ResolveAAType(aaType, set[p].fAAFlags, quad, &aaForQuad, &aaFlags);
            // Resolve sets aaForQuad to aaType or None, there is never a change between aa methods
            SkASSERT(aaForQuad == GrAAType::kNone || aaForQuad == aaType);
            if (overallAAType == GrAAType::kNone && aaForQuad != GrAAType::kNone) {
                overallAAType = aaType;
            }

            // Calculate metadata for the entry
            const SkRect* domainForQuad = nullptr;
            if (constraint == SkCanvas::kStrict_SrcRectConstraint) {
                // Check (briefly) if the strict constraint is needed for this set entry
                if (!set[p].fSrcRect.contains(fProxies[p].fProxy->getWorstCaseBoundsRect()) &&
                    (mustFilter || aaForQuad == GrAAType::kCoverage)) {
                    // Can't rely on hardware clamping and the draw will access outer texels
                    // for AA and/or bilerp
                    netDomain = Domain::kYes;
                    domainForQuad = &set[p].fSrcRect;
                }
            }
            float alpha = SkTPin(set[p].fAlpha, 0.f, 1.f);
            allOpaque &= (1.f == alpha);
            SkPMColor4f color{alpha, alpha, alpha, alpha};
            fQuads.append(quad, {color, domainForQuad, aaFlags}, &srcQuad);
        }
        fAAType = static_cast<unsigned>(overallAAType);
        if (!mustFilter) {
            fFilter = static_cast<unsigned>(GrSamplerState::Filter::kNearest);
        }
        this->setBounds(bounds, HasAABloat(this->aaType() == GrAAType::kCoverage),
                        IsHairline::kNo);
        fDomain = static_cast<unsigned>(netDomain);
    }

    void tess(void* v, const VertexSpec& spec, const GrTextureProxy* proxy,
              GrQuadBuffer<ColorDomainAndAA>::Iter* iter, int cnt) const {
        TRACE_EVENT0("skia.gpu", TRACE_FUNC);
        auto origin = proxy->origin();
        const auto* texture = proxy->peekTexture();
        float iw, ih, h;
        if (proxy->textureType() == GrTextureType::kRectangle) {
            iw = ih = 1.f;
            h = texture->height();
        } else {
            iw = 1.f / texture->width();
            ih = 1.f / texture->height();
            h = 1.f;
        }

        int i = 0;
        // Explicit ctor ensures ws are 1s, which compute_src_quad requires
        GrQuad srcQuad(SkRect::MakeEmpty());
        SkRect domain;
        while(i < cnt && iter->next()) {
            SkASSERT(iter->isLocalValid());
            const ColorDomainAndAA& info = iter->metadata();
            // Must correct the texture coordinates and domain now that the real texture size
            // is known
            compute_src_quad(origin, iter->localQuad(), iw, ih, h, &srcQuad);
            compute_domain(info.domain(), this->filter(), origin, info.fDomainRect, iw, ih, h,
                           &domain);
            v = GrQuadPerEdgeAA::Tessellate(v, spec, iter->deviceQuad(), info.fColor, srcQuad,
                                            domain, info.aaFlags());
            i++;
        }
    }

    void onPrePrepareDraws(GrRecordingContext* context) override {
        SkASSERT(!fPrePrepared);
        // Pull forward the tessellation of the quads to here

        //GrOpMemoryPool* pool = context->priv().opMemoryPool();

        fPrePrepared = true;
    }

#ifdef SK_DEBUG
    void validate() const override {
        auto textureType = fProxies[0].fProxy->textureType();
        const GrSwizzle& swizzle = fProxies[0].fProxy->textureSwizzle();
        GrAAType aaType = this->aaType();

        for (const auto& op : ChainRange<TextureOp>(this)) {
            for (unsigned p = 0; p < op.fProxyCnt; ++p) {
                auto* proxy = op.fProxies[p].fProxy;
                SkASSERT(proxy);
                SkASSERT(proxy->textureType() == textureType);
                SkASSERT(proxy->textureSwizzle() == swizzle);
            }

            // Each individual op must be a single aaType. kCoverage and kNone ops can chain
            // together but kMSAA ones do not.
            if (aaType == GrAAType::kCoverage || aaType == GrAAType::kNone) {
                SkASSERT(op.aaType() == GrAAType::kCoverage || op.aaType() == GrAAType::kNone);
            } else {
                SkASSERT(aaType == GrAAType::kMSAA && op.aaType() == GrAAType::kMSAA);
            }
        }
    }
#endif

    VertexSpec characterize(int* numProxies, int* numTotalQuads) const {
        GrQuad::Type quadType = GrQuad::Type::kAxisAligned;
        ColorType colorType = ColorType::kNone;
        GrQuad::Type srcQuadType = GrQuad::Type::kAxisAligned;
        Domain domain = Domain::kNo;
        GrAAType overallAAType = this->aaType();

        *numProxies = 0;
        *numTotalQuads = 0;

        for (const auto& op : ChainRange<TextureOp>(this)) {
            if (op.fQuads.deviceQuadType() > quadType) {
                quadType = op.fQuads.deviceQuadType();
            }
            if (op.fQuads.localQuadType() > srcQuadType) {
                srcQuadType = op.fQuads.localQuadType();
            }
            if (op.fDomain) {
                domain = Domain::kYes;
            }
            colorType = SkTMax(colorType, static_cast<ColorType>(op.fColorType));
            *numProxies += op.fProxyCnt;
            for (unsigned p = 0; p < op.fProxyCnt; ++p) {
                *numTotalQuads += op.fProxies[p].fQuadCnt;
            }
            if (op.aaType() == GrAAType::kCoverage) {
                overallAAType = GrAAType::kCoverage;
            }
        }

        return VertexSpec(quadType, colorType, srcQuadType, /* hasLocal */ true, domain,
                          overallAAType, /* alpha as coverage */ true);
    }

    // onPrePrepareDraws may or may not have been called at this point
    void onPrepareDraws(Target* target) override {
        TRACE_EVENT0("skia.gpu", TRACE_FUNC);

        SkDEBUGCODE(this->validate();)

        int numProxies, numTotalQuads;

        const VertexSpec vertexSpec = this->characterize(&numProxies, &numTotalQuads);

        // We'll use a dynamic state array for the GP textures when there are multiple ops.
        // Otherwise, we use fixed dynamic state to specify the single op's proxy.
        GrPipeline::DynamicStateArrays* dynamicStateArrays = nullptr;
        GrPipeline::FixedDynamicState* fixedDynamicState;
        if (numProxies > 1) {
            dynamicStateArrays = target->allocDynamicStateArrays(numProxies, 1, false);
            fixedDynamicState = target->makeFixedDynamicState(0);
        } else {
            fixedDynamicState = target->makeFixedDynamicState(1);
            fixedDynamicState->fPrimitiveProcessorTextures[0] = fProxies[0].fProxy;
        }

        size_t vertexSize = vertexSpec.vertexSize();

        GrMesh* meshes = target->allocMeshes(numProxies);
        sk_sp<const GrBuffer> vbuffer;
        int vertexOffsetInBuffer = 0;
        int numQuadVerticesLeft = numTotalQuads * vertexSpec.verticesPerQuad();
        int numAllocatedVertices = 0;
        void* vdata = nullptr;

        int m = 0;
        for (const auto& op : ChainRange<TextureOp>(this)) {
            auto iter = op.fQuads.iterator();
            for (unsigned p = 0; p < op.fProxyCnt; ++p) {
                int quadCnt = op.fProxies[p].fQuadCnt;
                auto* proxy = op.fProxies[p].fProxy;
                int meshVertexCnt = quadCnt * vertexSpec.verticesPerQuad();
                if (numAllocatedVertices < meshVertexCnt) {
                    vdata = target->makeVertexSpaceAtLeast(
                            vertexSize, meshVertexCnt, numQuadVerticesLeft, &vbuffer,
                            &vertexOffsetInBuffer, &numAllocatedVertices);
                    SkASSERT(numAllocatedVertices <= numQuadVerticesLeft);
                    if (!vdata) {
                        SkDebugf("Could not allocate vertices\n");
                        return;
                    }
                }
                SkASSERT(numAllocatedVertices >= meshVertexCnt);

                op.tess(vdata, vertexSpec, proxy, &iter, quadCnt);

                if (!GrQuadPerEdgeAA::ConfigureMeshIndices(target, &(meshes[m]), vertexSpec,
                                                           quadCnt)) {
                    SkDebugf("Could not allocate indices");
                    return;
                }
                meshes[m].setVertexData(vbuffer, vertexOffsetInBuffer);
                if (dynamicStateArrays) {
                    dynamicStateArrays->fPrimitiveProcessorTextures[m] = proxy;
                }
                ++m;
                numAllocatedVertices -= meshVertexCnt;
                numQuadVerticesLeft -= meshVertexCnt;
                vertexOffsetInBuffer += meshVertexCnt;
                vdata = reinterpret_cast<char*>(vdata) + vertexSize * meshVertexCnt;
            }
            // If quad counts per proxy were calculated correctly, the entire iterator should have
            // been consumed.
            SkASSERT(!iter.next());
        }
        SkASSERT(!numQuadVerticesLeft);
        SkASSERT(!numAllocatedVertices);

        sk_sp<GrGeometryProcessor> gp;

        {
            const GrBackendFormat& backendFormat = fProxies[0].fProxy->backendFormat();
            const GrSwizzle& swizzle = fProxies[0].fProxy->textureSwizzle();

            GrSamplerState samplerState = GrSamplerState(GrSamplerState::WrapMode::kClamp,
                                                         this->filter());

            auto saturate = static_cast<GrTextureOp::Saturate>(fSaturate);

            GrGpu* gpu = target->resourceProvider()->priv().gpu();
            uint32_t extraSamplerKey = gpu->getExtraSamplerKeyForProgram(samplerState,
                                                                         backendFormat);

            gp = GrQuadPerEdgeAA::MakeTexturedProcessor(
                vertexSpec, *target->caps().shaderCaps(), backendFormat, samplerState, swizzle,
                extraSamplerKey, std::move(fTextureColorSpaceXform), saturate);

            SkASSERT(vertexSize == gp->vertexStride());
        }

        target->recordDraw(
                std::move(gp), meshes, numProxies, fixedDynamicState, dynamicStateArrays);
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        auto pipelineFlags = (GrAAType::kMSAA == this->aaType())
                ? GrPipeline::InputFlags::kHWAntialias
                : GrPipeline::InputFlags::kNone;
        flushState->executeDrawsAndUploadsForMeshDrawOp(
                this, chainBounds, GrProcessorSet::MakeEmptySet(), pipelineFlags);
    }

    CombineResult onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        TRACE_EVENT0("skia.gpu", TRACE_FUNC);
        const auto* that = t->cast<TextureOp>();

        if (fPrePrepared || that->fPrePrepared) {
            // This should never happen (since only DDL recorded ops should be prePrepared)
            // but, in any case, we should never combine ops that that been prePrepared
            return CombineResult::kCannotCombine;
        }

        if (fDomain != that->fDomain) {
            // It is technically possible to combine operations across domain modes, but performance
            // testing suggests it's better to make more draw calls where some take advantage of
            // the more optimal shader path without coordinate clamping.
            return CombineResult::kCannotCombine;
        }
        if (!GrColorSpaceXform::Equals(fTextureColorSpaceXform.get(),
                                       that->fTextureColorSpaceXform.get())) {
            return CombineResult::kCannotCombine;
        }
        bool upgradeToCoverageAAOnMerge = false;
        if (this->aaType() != that->aaType()) {
            if (!((this->aaType() == GrAAType::kCoverage && that->aaType() == GrAAType::kNone) ||
                  (that->aaType() == GrAAType::kCoverage && this->aaType() == GrAAType::kNone))) {
                return CombineResult::kCannotCombine;
            }
            upgradeToCoverageAAOnMerge = true;
        }
        if (fSaturate != that->fSaturate) {
            return CombineResult::kCannotCombine;
        }
        if (fFilter != that->fFilter) {
            return CombineResult::kCannotCombine;
        }
        auto thisProxy = fProxies[0].fProxy;
        auto thatProxy = that->fProxies[0].fProxy;
        if (fProxyCnt > 1 || that->fProxyCnt > 1 ||
            thisProxy->uniqueID() != thatProxy->uniqueID()) {
            // We can't merge across different proxies. Check if 'this' can be chained with 'that'.
            if (GrTextureProxy::ProxiesAreCompatibleAsDynamicState(thisProxy, thatProxy) &&
                caps.dynamicStateArrayGeometryProcessorTextureSupport()) {
                return CombineResult::kMayChain;
            }
            return CombineResult::kCannotCombine;
        }

        fDomain |= that->fDomain;
        fColorType = SkTMax(fColorType, that->fColorType);
        if (upgradeToCoverageAAOnMerge) {
            fAAType = static_cast<unsigned>(GrAAType::kCoverage);
        }

        // Concatenate quad lists together
        fQuads.concat(that->fQuads);
        fProxies[0].fQuadCnt += that->fQuads.count();

        return CombineResult::kMerged;
    }

    GrAAType aaType() const { return static_cast<GrAAType>(fAAType); }
    GrSamplerState::Filter filter() const { return static_cast<GrSamplerState::Filter>(fFilter); }

    GrQuadBuffer<ColorDomainAndAA> fQuads;
    sk_sp<GrColorSpaceXform> fTextureColorSpaceXform;
    unsigned fSaturate : 1;
    unsigned fFilter : 2;
    unsigned fAAType : 2;
    unsigned fDomain : 1;
    unsigned fColorType : 2;
    GR_STATIC_ASSERT(GrQuadPerEdgeAA::kColorTypeCount <= 4);
    unsigned fPrePrepared : 1;
    unsigned fProxyCnt : 32 - 7;
    Proxy fProxies[1];

    static_assert(GrQuad::kTypeCount <= 4, "GrQuad::Type does not fit in 2 bits");

    typedef GrMeshDrawOp INHERITED;
};

}  // anonymous namespace

namespace GrTextureOp {

std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                               sk_sp<GrTextureProxy> proxy,
                               GrColorType srcColorType,
                               sk_sp<GrColorSpaceXform> textureXform,
                               GrSamplerState::Filter filter,
                               const SkPMColor4f& color,
                               Saturate saturate,
                               SkBlendMode blendMode,
                               GrAAType aaType,
                               GrQuadAAFlags aaFlags,
                               const GrQuad& deviceQuad,
                               const GrQuad& localQuad,
                               const SkRect* domain) {
    // Apply optimizations that are valid whether or not using GrTextureOp or GrFillRectOp
    if (domain && domain->contains(proxy->getWorstCaseBoundsRect())) {
        // No need for a shader-based domain if hardware clamping achieves the same effect
        domain = nullptr;
    }

    if (filter != GrSamplerState::Filter::kNearest && !filter_has_effect(localQuad, deviceQuad)) {
        filter = GrSamplerState::Filter::kNearest;
    }

    if (blendMode == SkBlendMode::kSrcOver) {
        return TextureOp::Make(context, std::move(proxy), std::move(textureXform), filter, color,
                               saturate, aaType, aaFlags, deviceQuad, localQuad, domain);
    } else {
        // Emulate complex blending using GrFillRectOp
        GrPaint paint;
        paint.setColor4f(color);
        paint.setXPFactory(SkBlendMode_AsXPFactory(blendMode));

        std::unique_ptr<GrFragmentProcessor> fp;
        if (domain) {
            // Update domain to match what GrTextureOp computes during tessellation, using top-left
            // as the origin so that it doesn't depend on final texture size (which the FP handles
            // later, as well as accounting for the true origin).
            SkRect correctedDomain;
            compute_domain(Domain::kYes, filter, kTopLeft_GrSurfaceOrigin, *domain,
                           1.f, 1.f, proxy->height(), &correctedDomain);
            fp = GrTextureDomainEffect::Make(std::move(proxy), srcColorType, SkMatrix::I(),
                                             correctedDomain, GrTextureDomain::kClamp_Mode, filter);
        } else {
            fp = GrSimpleTextureEffect::Make(std::move(proxy), srcColorType, SkMatrix::I(), filter);
        }
        fp = GrColorSpaceXformEffect::Make(std::move(fp), std::move(textureXform));
        paint.addColorFragmentProcessor(std::move(fp));
        if (saturate == GrTextureOp::Saturate::kYes) {
            paint.addColorFragmentProcessor(GrSaturateProcessor::Make());
        }

        return GrFillRectOp::Make(context, std::move(paint), aaType, aaFlags,
                                  deviceQuad, localQuad);
    }
}

std::unique_ptr<GrDrawOp> MakeSet(GrRecordingContext* context,
                                  const GrRenderTargetContext::TextureSetEntry set[],
                                  int cnt,
                                  GrSamplerState::Filter filter,
                                  Saturate saturate,
                                  GrAAType aaType,
                                  SkCanvas::SrcRectConstraint constraint,
                                  const SkMatrix& viewMatrix,
                                  sk_sp<GrColorSpaceXform> textureColorSpaceXform) {
    return TextureOp::Make(context, set, cnt, filter, saturate, aaType, constraint, viewMatrix,
                           std::move(textureColorSpaceXform));
}

}  // namespace GrTextureOp

#if GR_TEST_UTILS
#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"

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
    const GrBackendFormat format =
            context->priv().caps()->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                            GrRenderable::kNo);

    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(
            format, desc, GrRenderable::kNo, 1, origin, mipMapped, fit, SkBudgeted::kNo,
            GrProtected::kNo, GrInternalSurfaceFlags::kNone);

    SkRect rect = GrTest::TestRect(random);
    SkRect srcRect;
    srcRect.fLeft = random->nextRangeScalar(0.f, proxy->width() / 2.f);
    srcRect.fRight = random->nextRangeScalar(0.f, proxy->width()) + proxy->width() / 2.f;
    srcRect.fTop = random->nextRangeScalar(0.f, proxy->height() / 2.f);
    srcRect.fBottom = random->nextRangeScalar(0.f, proxy->height()) + proxy->height() / 2.f;
    SkMatrix viewMatrix = GrTest::TestMatrixPreservesRightAngles(random);
    SkPMColor4f color = SkPMColor4f::FromBytes_RGBA(SkColorToPremulGrColor(random->nextU()));
    GrSamplerState::Filter filter = (GrSamplerState::Filter)random->nextULessThan(
            static_cast<uint32_t>(GrSamplerState::Filter::kMipMap) + 1);
    while (mipMapped == GrMipMapped::kNo && filter == GrSamplerState::Filter::kMipMap) {
        filter = (GrSamplerState::Filter)random->nextULessThan(
                static_cast<uint32_t>(GrSamplerState::Filter::kMipMap) + 1);
    }
    auto texXform = GrTest::TestColorXform(random);
    GrAAType aaType = GrAAType::kNone;
    if (random->nextBool()) {
        aaType = (numSamples > 1) ? GrAAType::kMSAA : GrAAType::kCoverage;
    }
    GrQuadAAFlags aaFlags = GrQuadAAFlags::kNone;
    aaFlags |= random->nextBool() ? GrQuadAAFlags::kLeft : GrQuadAAFlags::kNone;
    aaFlags |= random->nextBool() ? GrQuadAAFlags::kTop : GrQuadAAFlags::kNone;
    aaFlags |= random->nextBool() ? GrQuadAAFlags::kRight : GrQuadAAFlags::kNone;
    aaFlags |= random->nextBool() ? GrQuadAAFlags::kBottom : GrQuadAAFlags::kNone;
    bool useDomain = random->nextBool();
    auto saturate = random->nextBool() ? GrTextureOp::Saturate::kYes : GrTextureOp::Saturate::kNo;
    return GrTextureOp::Make(context, std::move(proxy), GrColorType::kRGBA_8888,
                             std::move(texXform), filter, color, saturate, SkBlendMode::kSrcOver,
                             aaType, aaFlags, GrQuad::MakeFromRect(rect, viewMatrix),
                             GrQuad(srcRect), useDomain ? &srcRect : nullptr);
}

#endif
