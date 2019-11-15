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
                                          GrSurfaceProxyView proxyView,
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
        return pool->allocate<TextureOp>(std::move(proxyView), std::move(textureXform), filter,
                                         color, saturate, aaType, aaFlags, deviceQuad, localQuad,
                                         domain);
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
        size_t size = sizeof(TextureOp) + sizeof(ViewCountPair) * (cnt - 1);
        GrOpMemoryPool* pool = context->priv().opMemoryPool();
        void* mem = pool->allocate(size);
        return std::unique_ptr<GrDrawOp>(new (mem) TextureOp(set, cnt, filter, saturate, aaType,
                                                             constraint, viewMatrix,
                                                             std::move(textureColorSpaceXform)));
    }

    ~TextureOp() override {
        for (unsigned p = 1; p < fProxyCnt; ++p) {
            fViewCountPairs[p].~ViewCountPair();
        }
    }

    const char* name() const override { return "TextureOp"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        for (unsigned p = 0; p < fProxyCnt; ++p) {
            bool mipped = (GrSamplerState::Filter::kMipMap == this->filter());
            func(fViewCountPairs[p].fProxyView.asTextureProxy(), GrMipMapped(mipped));
        }
    }

#ifdef SK_DEBUG
    SkString dumpInfo() const override {
        SkString str;
        str.appendf("# draws: %d\n", fQuads.count());
        auto iter = fQuads.iterator();
        for (unsigned p = 0; p < fProxyCnt; ++p) {
            str.appendf("Proxy ID: %d, Filter: %d\n",
                        fViewCountPairs[p].fProxyView.proxy()->uniqueID().asUInt(),
                        static_cast<int>(fFilter));
            int i = 0;
            while(i < fViewCountPairs[p].fQuadCnt && iter.next()) {
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
    struct ViewCountPair {
        GrSurfaceProxyView fProxyView;
        int fQuadCnt;
    };

    // This descriptor is used in both onPrePrepareDraws and onPrepareDraws.
    //
    // In the onPrePrepareDraws case it is allocated in the creation-time opData
    // arena. Both allocateCommon and allocatePrePrepareOnly are called and they also allocate
    // their memory in the creation-time opData arena.
    //
    // In the onPrepareDraws case this descriptor is created on the stack and only
    // allocateCommon is called. In this case the common memory fields are allocated
    // in the flush-time arena (i.e., as part of the flushState).
    struct PrePreparedDesc {
        VertexSpec                      fVertexSpec;
        int                             fNumProxies = 0;
        int                             fNumTotalQuads = 0;
        GrPipeline::DynamicStateArrays* fDynamicStateArrays = nullptr;
        GrPipeline::FixedDynamicState*  fFixedDynamicState = nullptr;

        // This member variable is only used by 'onPrePrepareDraws'. The prior five are also
        // used by 'onPrepareDraws'
        char*                           fVertices = nullptr;

        // How big should 'fVertices' be to hold all the vertex data?
        size_t totalSizeInBytes() const {
            return fNumTotalQuads * fVertexSpec.verticesPerQuad() * fVertexSpec.vertexSize();
        }

        int totalNumVertices() const {
            return fNumTotalQuads * fVertexSpec.verticesPerQuad();
        }

        // Helper to fill in the fFixedDynamicState and fDynamicStateArrays. If there is more
        // than one mesh/proxy they are stored in fDynamicStateArrays but if there is only one
        // it is stored in fFixedDynamicState.
        void setMeshProxy(int index, GrTextureProxy* proxy) {
            SkASSERT(index < fNumProxies);

            if (fDynamicStateArrays) {
                SkASSERT(fDynamicStateArrays->fPrimitiveProcessorTextures);
                SkASSERT(fNumProxies > 1);

                fDynamicStateArrays->fPrimitiveProcessorTextures[index] = proxy;
            } else {
                SkASSERT(fFixedDynamicState);
                SkASSERT(fNumProxies == 1);

                fFixedDynamicState->fPrimitiveProcessorTextures[index] = proxy;
            }
        }

        // Allocate the fields required in both onPrePrepareDraws and onPrepareDraws
        void allocateCommon(SkArenaAlloc* arena, const GrAppliedClip* clip) {
            // We'll use a dynamic state array for the GP textures when there are multiple ops.
            // Otherwise, we use fixed dynamic state to specify the single op's proxy.
            if (fNumProxies > 1) {
                fDynamicStateArrays = Target::AllocDynamicStateArrays(arena, fNumProxies, 1, false);
                fFixedDynamicState = Target::MakeFixedDynamicState(arena, clip, 0);
            } else {
                fFixedDynamicState = Target::MakeFixedDynamicState(arena, clip, 1);
            }
        }

        // Allocate the fields only needed by onPrePrepareDraws
        void allocatePrePrepareOnly(SkArenaAlloc* arena) {
            fVertices = arena->makeArrayDefault<char>(this->totalSizeInBytes());
        }

    };

    // dstQuad should be the geometry transformed by the view matrix. If domainRect
    // is not null it will be used to apply the strict src rect constraint.
    TextureOp(GrSurfaceProxyView proxyView,
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
            , fPrePreparedDesc(nullptr)
            , fSaturate(static_cast<unsigned>(saturate))
            , fFilter(static_cast<unsigned>(filter)) {
        // Clean up disparities between the overall aa type and edge configuration and apply
        // optimizations based on the rect and matrix when appropriate
        GrQuadUtils::ResolveAAType(aaType, aaFlags, dstQuad, &aaType, &aaFlags);
        fAAType = static_cast<unsigned>(aaType);

        // We expect our caller to have already caught this optimization.
        SkASSERT(!domainRect ||
                 !domainRect->contains(proxyView.proxy()->backingStoreBoundsRect()));

        // We may have had a strict constraint with nearest filter solely due to possible AA bloat.
        // If we don't have (or determined we don't need) coverage AA then we can skip using a
        // domain.
        if (domainRect && this->filter() == GrSamplerState::Filter::kNearest &&
            aaType != GrAAType::kCoverage) {
            domainRect = nullptr;
        }

        fQuads.append(dstQuad, {color, domainRect, aaFlags}, &srcQuad);

        fProxyCnt = 1;
        fViewCountPairs[0] = {std::move(proxyView), 1};
        fTotNumQuads = 1;
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
            , fPrePreparedDesc(nullptr)
            , fSaturate(static_cast<unsigned>(saturate))
            , fFilter(static_cast<unsigned>(filter)) {
        fProxyCnt = SkToUInt(cnt);
        SkRect bounds = SkRectPriv::MakeLargestInverted();
        GrAAType overallAAType = GrAAType::kNone; // aa type maximally compatible with all dst rects
        bool mustFilter = false;
        bool allOpaque = true;
        Domain netDomain = Domain::kNo;
        GrTextureProxy* curProxy = nullptr;

        for (unsigned p = 0; p < fProxyCnt; ++p) {
            if (p == 0) {
                // We do not placement new the first ViewCountPair since that one is allocated and
                // initialized as part of the GrTextureOp creation.
                fViewCountPairs[p].fProxyView = std::move(set[p].fProxyView);
                fViewCountPairs[p].fQuadCnt = 1;
            } else {
                // We must placement new the ViewCountPairs here so that the sk_sps in the
                // GrSurfaceProxyView get initialized properly.
                new(&fViewCountPairs[p])ViewCountPair({std::move(set[p].fProxyView), 1});
            }
            fTotNumQuads += 1;
            curProxy = fViewCountPairs[p].fProxyView.asTextureProxy();
            SkASSERT(curProxy->textureType() ==
                     fViewCountPairs[0].fProxyView.asTextureProxy()->textureType());
            SkASSERT(curProxy->config() == fViewCountPairs[0].fProxyView.proxy()->config());

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
                if (!set[p].fSrcRect.contains(curProxy->backingStoreBoundsRect()) &&
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

    static void Tess(void* v, const VertexSpec& spec, const GrTextureProxy* proxy,
                     GrQuadBuffer<ColorDomainAndAA>::Iter* iter, int cnt,
                     GrSamplerState::Filter filter) {
        TRACE_EVENT0("skia.gpu", TRACE_FUNC);
        auto origin = proxy->origin();
        SkISize dimensions = proxy->backingStoreDimensions();

        float iw, ih, h;
        if (proxy->textureType() == GrTextureType::kRectangle) {
            iw = ih = 1.f;
            h = dimensions.height();
        } else {
            iw = 1.f / dimensions.width();
            ih = 1.f / dimensions.height();
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
            compute_domain(info.domain(), filter, origin, info.fDomainRect, iw, ih, h,
                           &domain);
            v = GrQuadPerEdgeAA::Tessellate(v, spec, iter->deviceQuad(), info.fColor, srcQuad,
                                            domain, info.aaFlags());
            i++;
        }
    }

    void onPrePrepareDraws(GrRecordingContext* context,
                           const GrSurfaceProxyView* dstView,
                           const GrAppliedClip* clip) override {
        TRACE_EVENT0("skia.gpu", TRACE_FUNC);

        SkDEBUGCODE(this->validate();)
        SkASSERT(!fPrePreparedDesc);

        SkArenaAlloc* arena = context->priv().recordTimeAllocator();

        fPrePreparedDesc = arena->make<PrePreparedDesc>();

        this->characterize(fPrePreparedDesc);

        fPrePreparedDesc->allocateCommon(arena, clip);

        fPrePreparedDesc->allocatePrePrepareOnly(arena);

        // At this juncture we only fill in the vertex data and state arrays. Filling in of
        // the meshes is left until onPrepareDraws.
        SkAssertResult(FillInData(this, fPrePreparedDesc, fPrePreparedDesc->fVertices,
                                  nullptr, 0, nullptr, nullptr));
    }

    static bool FillInData(TextureOp* texOp, PrePreparedDesc* desc,
                           char* pVertexData, GrMesh* meshes, int absBufferOffset,
                           sk_sp<const GrBuffer> vertexBuffer,
                           sk_sp<const GrBuffer> indexBuffer) {
        int totQuadsSeen = 0;
        SkDEBUGCODE(int totVerticesSeen = 0;)
        char* dst = pVertexData;
        const size_t vertexSize = desc->fVertexSpec.vertexSize();

        int meshIndex = 0;
        for (const auto& op : ChainRange<TextureOp>(texOp)) {
            auto iter = op.fQuads.iterator();
            for (unsigned p = 0; p < op.fProxyCnt; ++p) {
                GrTextureProxy* proxy = op.fViewCountPairs[p].fProxyView.asTextureProxy();

                int quadCnt = op.fViewCountPairs[p].fQuadCnt;

                const int meshVertexCnt = quadCnt * desc->fVertexSpec.verticesPerQuad();

                SkASSERT(meshIndex < desc->fNumProxies);

                if (dst) {
                    Tess(dst, desc->fVertexSpec, proxy, &iter, quadCnt, op.filter());
                    desc->setMeshProxy(meshIndex, proxy);

                    SkASSERT(totVerticesSeen * vertexSize == (size_t)(dst - pVertexData));
                    dst += vertexSize * meshVertexCnt;
                }

                if (meshes) {
                    GrQuadPerEdgeAA::ConfigureMesh(&(meshes[meshIndex]), desc->fVertexSpec,
                                                   totQuadsSeen, quadCnt, desc->totalNumVertices(),
                                                   vertexBuffer, indexBuffer, absBufferOffset);
                }

                ++meshIndex;

                totQuadsSeen += quadCnt;
                SkDEBUGCODE(totVerticesSeen += meshVertexCnt);
                SkASSERT(totQuadsSeen * desc->fVertexSpec.verticesPerQuad() == totVerticesSeen);
            }

            // If quad counts per proxy were calculated correctly, the entire iterator
            // should have been consumed.
            SkASSERT(!dst || !iter.next());
        }

        SkASSERT(!dst || (desc->totalSizeInBytes() == (size_t)(dst - pVertexData)));
        SkASSERT(meshIndex == desc->fNumProxies);
        SkASSERT(totQuadsSeen == desc->fNumTotalQuads);
        SkASSERT(totVerticesSeen == desc->totalNumVertices());
        return true;
    }

#ifdef SK_DEBUG
    void validate() const override {
        auto textureType = fViewCountPairs[0].fProxyView.asTextureProxy()->textureType();
        GrAAType aaType = this->aaType();

        int quadCount = 0;
        for (const auto& op : ChainRange<TextureOp>(this)) {
            for (unsigned p = 0; p < op.fProxyCnt; ++p) {
                auto* proxy = op.fViewCountPairs[p].fProxyView.asTextureProxy();
                quadCount += op.fViewCountPairs[p].fQuadCnt;
                SkASSERT(proxy);
                SkASSERT(proxy->textureType() == textureType);
                SkASSERT(op.fViewCountPairs[p].fProxyView.swizzle() ==
                         fViewCountPairs[0].fProxyView.swizzle());
            }

            // Each individual op must be a single aaType. kCoverage and kNone ops can chain
            // together but kMSAA ones do not.
            if (aaType == GrAAType::kCoverage || aaType == GrAAType::kNone) {
                SkASSERT(op.aaType() == GrAAType::kCoverage || op.aaType() == GrAAType::kNone);
            } else {
                SkASSERT(aaType == GrAAType::kMSAA && op.aaType() == GrAAType::kMSAA);
            }
        }

        SkASSERT(quadCount == this->numChainedQuads());
    }
#endif

#if GR_TEST_UTILS
    int numQuads() const final { return this->totNumQuads(); }
#endif

    void characterize(PrePreparedDesc* desc) const {
        GrQuad::Type quadType = GrQuad::Type::kAxisAligned;
        ColorType colorType = ColorType::kNone;
        GrQuad::Type srcQuadType = GrQuad::Type::kAxisAligned;
        Domain domain = Domain::kNo;
        GrAAType overallAAType = this->aaType();

        desc->fNumProxies = 0;
        desc->fNumTotalQuads = 0;
        int maxQuadsPerMesh = 0;

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
            desc->fNumProxies += op.fProxyCnt;

            for (unsigned p = 0; p < op.fProxyCnt; ++p) {
                maxQuadsPerMesh = SkTMax(op.fViewCountPairs[p].fQuadCnt, maxQuadsPerMesh);
            }
            desc->fNumTotalQuads += op.totNumQuads();

            if (op.aaType() == GrAAType::kCoverage) {
                overallAAType = GrAAType::kCoverage;
            }
        }

        SkASSERT(desc->fNumTotalQuads == this->numChainedQuads());

        SkASSERT(!CombinedQuadCountWillOverflow(overallAAType, false, desc->fNumTotalQuads));

        auto indexBufferOption = GrQuadPerEdgeAA::CalcIndexBufferOption(overallAAType,
                                                                        maxQuadsPerMesh);

        desc->fVertexSpec = VertexSpec(quadType, colorType, srcQuadType, /* hasLocal */ true,
                                       domain, overallAAType, /* alpha as coverage */ true,
                                       indexBufferOption);

        SkASSERT(desc->fNumTotalQuads <= GrQuadPerEdgeAA::QuadLimit(indexBufferOption));
    }

    int totNumQuads() const {
#ifdef SK_DEBUG
        int tmp = 0;
        for (unsigned p = 0; p < fProxyCnt; ++p) {
            tmp += fViewCountPairs[p].fQuadCnt;
        }
        SkASSERT(tmp == fTotNumQuads);
#endif

        return fTotNumQuads;
    }

    int numChainedQuads() const {
        int numChainedQuads = this->totNumQuads();

        for (const GrOp* tmp = this->prevInChain(); tmp; tmp = tmp->prevInChain()) {
            numChainedQuads += ((const TextureOp*)tmp)->totNumQuads();
        }

        for (const GrOp* tmp = this->nextInChain(); tmp; tmp = tmp->nextInChain()) {
            numChainedQuads += ((const TextureOp*)tmp)->totNumQuads();
        }

        return numChainedQuads;
    }

    // onPrePrepareDraws may or may not have been called at this point
    void onPrepareDraws(Target* target) override {
        TRACE_EVENT0("skia.gpu", TRACE_FUNC);

        SkDEBUGCODE(this->validate();)

        PrePreparedDesc desc;

        if (fPrePreparedDesc) {
            desc = *fPrePreparedDesc;
        } else {
            SkArenaAlloc* arena = target->allocator();

            this->characterize(&desc);
            desc.allocateCommon(arena, target->appliedClip());

            SkASSERT(!desc.fVertices);
        }

        size_t vertexSize = desc.fVertexSpec.vertexSize();

        sk_sp<const GrBuffer> vbuffer;
        int vertexOffsetInBuffer = 0;

        void* vdata = target->makeVertexSpace(vertexSize, desc.totalNumVertices(),
                                              &vbuffer, &vertexOffsetInBuffer);
        if (!vdata) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        sk_sp<const GrBuffer> indexBuffer;
        if (desc.fVertexSpec.needsIndexBuffer()) {
            indexBuffer = GrQuadPerEdgeAA::GetIndexBuffer(target,
                                                          desc.fVertexSpec.indexBufferOption());
            if (!indexBuffer) {
                SkDebugf("Could not allocate indices\n");
                return;
            }
        }

        // Note: this allocation is always in the flush-time arena (i.e., the flushState)
        GrMesh* meshes = target->allocMeshes(desc.fNumProxies);

        bool result;
        if (fPrePreparedDesc) {
            memcpy(vdata, desc.fVertices, desc.totalSizeInBytes());
            // The above memcpy filled in the vertex data - just call FillInData to fill in the
            // mesh data
            result = FillInData(this, &desc, nullptr, meshes, vertexOffsetInBuffer,
                                std::move(vbuffer), std::move(indexBuffer));
        } else {
            // Fills in both vertex data and mesh data
            result = FillInData(this, &desc, (char*) vdata, meshes, vertexOffsetInBuffer,
                                std::move(vbuffer), std::move(indexBuffer));
        }

        if (!result) {
            return;
        }

        sk_sp<GrGeometryProcessor> gp;

        {
            const GrBackendFormat& backendFormat =
                    fViewCountPairs[0].fProxyView.proxy()->backendFormat();
            const GrSwizzle& swizzle = fViewCountPairs[0].fProxyView.swizzle();

            GrSamplerState samplerState = GrSamplerState(GrSamplerState::WrapMode::kClamp,
                                                         this->filter());

            auto saturate = static_cast<GrTextureOp::Saturate>(fSaturate);

            gp = GrQuadPerEdgeAA::MakeTexturedProcessor(
                desc.fVertexSpec, *target->caps().shaderCaps(), backendFormat,
                samplerState, swizzle, std::move(fTextureColorSpaceXform), saturate);

            SkASSERT(vertexSize == gp->vertexStride());
        }

        target->recordDraw(std::move(gp), meshes, desc.fNumProxies,
                           desc.fFixedDynamicState, desc.fDynamicStateArrays,
                           desc.fVertexSpec.primitiveType());
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

        if (fPrePreparedDesc || that->fPrePreparedDesc) {
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
            if (!CanUpgradeAAOnMerge(this->aaType(), that->aaType())) {
                return CombineResult::kCannotCombine;
            }
            upgradeToCoverageAAOnMerge = true;
        }

        if (CombinedQuadCountWillOverflow(this->aaType(), upgradeToCoverageAAOnMerge,
                                          this->numChainedQuads() + that->numChainedQuads())) {
            return CombineResult::kCannotCombine;
        }

        if (fSaturate != that->fSaturate) {
            return CombineResult::kCannotCombine;
        }
        if (fFilter != that->fFilter) {
            return CombineResult::kCannotCombine;
        }
        const auto& thisView = fViewCountPairs[0].fProxyView;
        const auto& thatView = that->fViewCountPairs[0].fProxyView;
        auto thisProxy = thisView.asTextureProxy();
        auto thatProxy = thatView.asTextureProxy();
        if (fProxyCnt > 1 || that->fProxyCnt > 1 || thisView != thatView) {
            // We can't merge across different proxies. Check if 'this' can be chained with 'that'.
            if (GrTextureProxy::ProxiesAreCompatibleAsDynamicState(thisProxy, thatProxy) &&
                caps.dynamicStateArrayGeometryProcessorTextureSupport() &&
                thisView.swizzle() == thatView.swizzle() &&
                thisView.origin() == thatView.origin()) {
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
        fViewCountPairs[0].fQuadCnt += that->fQuads.count();
        fTotNumQuads += that->fQuads.count();

        return CombineResult::kMerged;
    }

    GrAAType aaType() const { return static_cast<GrAAType>(fAAType); }
    GrSamplerState::Filter filter() const { return static_cast<GrSamplerState::Filter>(fFilter); }

    GrQuadBuffer<ColorDomainAndAA> fQuads;
    sk_sp<GrColorSpaceXform> fTextureColorSpaceXform;
    // 'fPrePreparedDesc' is only filled in when this op has been prePrepared. In that case,
    // it - and the matching dynamic and fixed state - have been allocated in the opPOD arena
    // not in the FlushState arena.
    PrePreparedDesc* fPrePreparedDesc;
    int fTotNumQuads = 0;   // the total number of quads in this op (but not in the whole chain)
    unsigned fSaturate : 1;
    unsigned fFilter : 2;
    unsigned fAAType : 2;
    unsigned fDomain : 1;
    unsigned fColorType : 2;
    GR_STATIC_ASSERT(GrQuadPerEdgeAA::kColorTypeCount <= 4);
    unsigned fProxyCnt : 32 - 8;

    // This field must go last. When allocating this op, we will allocate extra space to hold
    // additional ViewCountPairs immediately after the op's allocation so we can treat this
    // as an fProxyCnt-length array.
    ViewCountPair fViewCountPairs[1];

    static_assert(GrQuad::kTypeCount <= 4, "GrQuad::Type does not fit in 2 bits");

    typedef GrMeshDrawOp INHERITED;
};

}  // anonymous namespace

#if GR_TEST_UTILS
uint32_t GrTextureOp::ClassID() {
    return TextureOp::ClassID();
}
#endif

std::unique_ptr<GrDrawOp> GrTextureOp::Make(GrRecordingContext* context,
                                            GrSurfaceProxyView proxyView,
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
    GrTextureProxy* proxy = proxyView.asTextureProxy();
    // Apply optimizations that are valid whether or not using GrTextureOp or GrFillRectOp
    if (domain && domain->contains(proxy->backingStoreBoundsRect())) {
        // No need for a shader-based domain if hardware clamping achieves the same effect
        domain = nullptr;
    }

    if (filter != GrSamplerState::Filter::kNearest && !filter_has_effect(localQuad, deviceQuad)) {
        filter = GrSamplerState::Filter::kNearest;
    }

    if (blendMode == SkBlendMode::kSrcOver) {
        return TextureOp::Make(context, std::move(proxyView), std::move(textureXform), filter,
                               color, saturate, aaType, aaFlags, deviceQuad, localQuad, domain);
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
            fp = GrTextureDomainEffect::Make(sk_ref_sp(proxy), srcColorType, SkMatrix::I(),
                                             correctedDomain, GrTextureDomain::kClamp_Mode, filter);
        } else {
            fp = GrSimpleTextureEffect::Make(sk_ref_sp(proxy), srcColorType, SkMatrix::I(), filter);
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

// A helper class that assists in breaking up bulk API quad draws into manageable chunks.
class GrTextureOp::BatchSizeLimiter {
public:
    BatchSizeLimiter(GrRenderTargetContext* rtc,
                     const GrClip& clip,
                     GrRecordingContext* context,
                     int numEntries,
                     GrSamplerState::Filter filter,
                     GrTextureOp::Saturate saturate,
                     SkCanvas::SrcRectConstraint constraint,
                     const SkMatrix& viewMatrix,
                     sk_sp<GrColorSpaceXform> textureColorSpaceXform)
            : fRTC(rtc)
            , fClip(clip)
            , fContext(context)
            , fFilter(filter)
            , fSaturate(saturate)
            , fConstraint(constraint)
            , fViewMatrix(viewMatrix)
            , fTextureColorSpaceXform(textureColorSpaceXform)
            , fNumLeft(numEntries) {
    }

    void createOp(const GrRenderTargetContext::TextureSetEntry set[],
                  int clumpSize,
                  GrAAType aaType) {
        std::unique_ptr<GrDrawOp> op = TextureOp::Make(fContext, &set[fNumClumped], clumpSize,
                                                       fFilter, fSaturate, aaType,
                                                       fConstraint, fViewMatrix,
                                                       fTextureColorSpaceXform);
        fRTC->addDrawOp(fClip, std::move(op));

        fNumLeft -= clumpSize;
        fNumClumped += clumpSize;
    }

    int numLeft() const { return fNumLeft;  }
    int baseIndex() const { return fNumClumped; }

private:
    GrRenderTargetContext*      fRTC;
    const GrClip&               fClip;
    GrRecordingContext*         fContext;
    GrSamplerState::Filter      fFilter;
    GrTextureOp::Saturate       fSaturate;
    SkCanvas::SrcRectConstraint fConstraint;
    const SkMatrix&             fViewMatrix;
    sk_sp<GrColorSpaceXform>    fTextureColorSpaceXform;

    int                         fNumLeft;
    int                         fNumClumped = 0; // also the offset for the start of the next clump
};

// Greedily clump quad draws together until the index buffer limit is exceeded.
void GrTextureOp::CreateTextureSetOps(GrRenderTargetContext* rtc,
                                      const GrClip& clip,
                                      GrRecordingContext* context,
                                      const GrRenderTargetContext::TextureSetEntry set[],
                                      int cnt,
                                      GrSamplerState::Filter filter,
                                      Saturate saturate,
                                      GrAAType aaType,
                                      SkCanvas::SrcRectConstraint constraint,
                                      const SkMatrix& viewMatrix,
                                      sk_sp<GrColorSpaceXform> textureColorSpaceXform) {

    // First check if we can always just make a single op and avoid the extra iteration
    // needed to clump things together.
    if (cnt <= SkTMin(GrResourceProvider::MaxNumNonAAQuads(),
                      GrResourceProvider::MaxNumAAQuads())) {
        auto op = TextureOp::Make(context, set, cnt, filter, saturate, aaType,
                                  constraint, viewMatrix, std::move(textureColorSpaceXform));
        rtc->addDrawOp(clip, std::move(op));
        return;
    }

    BatchSizeLimiter state(rtc, clip, context, cnt, filter, saturate, constraint, viewMatrix,
                           std::move(textureColorSpaceXform));

    // kNone and kMSAA never get altered
    if (aaType == GrAAType::kNone || aaType == GrAAType::kMSAA) {
        // Clump these into series of MaxNumNonAAQuads-sized GrTextureOps
        while (state.numLeft() > 0) {
            int clumpSize = SkTMin(state.numLeft(), GrResourceProvider::MaxNumNonAAQuads());

            state.createOp(set, clumpSize, aaType);
        }
    } else {
        // kCoverage can be downgraded to kNone. Note that the following is conservative. kCoverage
        // can also get downgraded to kNone if all the quads are on integer coordinates and
        // axis-aligned.
        SkASSERT(aaType == GrAAType::kCoverage);

        while (state.numLeft() > 0) {
            GrAAType runningAA = GrAAType::kNone;
            bool clumped = false;

            for (int i = 0; i < state.numLeft(); ++i) {
                int absIndex = state.baseIndex() + i;

                if (set[absIndex].fAAFlags != GrQuadAAFlags::kNone) {

                    if (i >= GrResourceProvider::MaxNumAAQuads()) {
                        // Here we either need to boost the AA type to kCoverage, but doing so with
                        // all the accumulated quads would overflow, or we have a set of AA quads
                        // that has just gotten too large. In either case, calve off the existing
                        // quads as their own TextureOp.
                        state.createOp(
                            set,
                            runningAA == GrAAType::kNone ? i : GrResourceProvider::MaxNumAAQuads(),
                            runningAA); // maybe downgrading AA here
                        clumped = true;
                        break;
                    }

                    runningAA = GrAAType::kCoverage;
                } else if (runningAA == GrAAType::kNone) {

                    if (i >= GrResourceProvider::MaxNumNonAAQuads()) {
                        // Here we've found a consistent batch of non-AA quads that has gotten too
                        // large. Calve it off as its own GrTextureOp.
                        state.createOp(set, GrResourceProvider::MaxNumNonAAQuads(),
                                       GrAAType::kNone); // definitely downgrading AA here
                        clumped = true;
                        break;
                    }
                }
            }

            if (!clumped) {
                // We ran through the above loop w/o hitting a limit. Spit out this last clump of
                // quads and call it a day.
                state.createOp(set, state.numLeft(), runningAA); // maybe downgrading AA here
            }
        }
    }
}

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
    GrSurfaceProxyView proxyView(
            std::move(proxy), origin,
            context->priv().caps()->getTextureSwizzle(format, GrColorType::kRGBA_8888));

    return GrTextureOp::Make(context, std::move(proxyView), GrColorType::kRGBA_8888,
                             std::move(texXform), filter, color, saturate, SkBlendMode::kSrcOver,
                             aaType, aaFlags, GrQuad::MakeFromRect(rect, viewMatrix),
                             GrQuad(srcRect), useDomain ? &srcRect : nullptr);
}

#endif
