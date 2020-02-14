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
#include "src/gpu/effects/generated/GrClampFragmentProcessor.h"
#include "src/gpu/geometry/GrQuad.h"
#include "src/gpu/geometry/GrQuadBuffer.h"
#include "src/gpu/geometry/GrQuadUtils.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/ops/GrFillRectOp.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/ops/GrQuadPerEdgeAA.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"
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

// Describes function for normalizing src coords: [x * iw, y * ih + yOffset] can represent
// regular and rectangular textures, w/ or w/o origin correction.
struct NormalizationParams {
    float fIW; // 1 / width of texture, or 1.0 for texture rectangles
    float fIH; // 1 / height of texture, or 1.0 for tex rects, X -1 if bottom-left origin
    float fYOffset; // 0 for top-left origin, height of [normalized] tex if bottom-left
};
static NormalizationParams proxy_normalization_params(const GrSurfaceProxy* proxy,
                                                      GrSurfaceOrigin origin) {
    // Whether or not the proxy is instantiated, this is the size its texture will be, so we can
    // normalize the src coordinates up front.
    SkISize dimensions = proxy->backingStoreDimensions();
    float iw, ih, h;
    if (proxy->backendFormat().textureType() == GrTextureType::kRectangle) {
        iw = ih = 1.f;
        h = dimensions.height();
    } else {
        iw = 1.f / dimensions.width();
        ih = 1.f / dimensions.height();
        h = 1.f;
    }

    if (origin == kBottomLeft_GrSurfaceOrigin) {
        return {iw, -ih, h};
    } else {
        return {iw, ih, 0.0f};
    }
}

static SkRect inset_domain_for_bilerp(const NormalizationParams& params, const SkRect& domainRect) {
    // Normalized pixel size is also equal to iw and ih, so the insets for bilerp are just
    // in those units and can be applied safely after normalization. However, if the domain is
    // smaller than a texel, it should clamp to the center of that axis.
    float dw = domainRect.width() < params.fIW ? domainRect.width() : params.fIW;
    float dh = domainRect.height() < params.fIH ? domainRect.height() : params.fIH;
    return domainRect.makeInset(0.5f * dw, 0.5f * dh);
}

// Normalize the domain. If 'domainRect' is null, it is assumed no domain constraint is desired,
// so a sufficiently large rect is returned even if the quad ends up batched with an op that uses
// domains overall.
static SkRect normalize_domain(GrSamplerState::Filter filter,
                               const NormalizationParams& params,
                               const SkRect* domainRect) {
    static constexpr SkRect kLargeRect = {-100000, -100000, 1000000, 1000000};
    if (!domainRect) {
        // Either the quad has no domain constraint and is batched with a domain constrained op
        // (in which case we want a domain that doesn't restrict normalized tex coords), or the
        // entire op doesn't use the domain, in which case the returned value is ignored.
        return kLargeRect;
    }

    auto ltrb = skvx::Vec<4, float>::Load(domainRect);
    // Normalize and offset
    ltrb = mad(ltrb, {params.fIW, params.fIH, params.fIW, params.fIH},
               {0.f, params.fYOffset, 0.f, params.fYOffset});
    if (params.fIH < 0.f) {
        // Flip top and bottom to keep the rect sorted when loaded back to SkRect.
        ltrb = skvx::shuffle<0, 3, 2, 1>(ltrb);
    }

    SkRect out;
    ltrb.store(&out);
    return out;
}

// Normalizes logical src coords and corrects for origin
static void normalize_src_quad(const NormalizationParams& params,
                               GrQuad* srcQuad) {
    // The src quad should not have any perspective
    SkASSERT(!srcQuad->hasPerspective());
    skvx::Vec<4, float> xs = srcQuad->x4f() * params.fIW;
    skvx::Vec<4, float> ys = mad(srcQuad->y4f(), params.fIH, params.fYOffset);
    xs.store(srcQuad->xs());
    ys.store(srcQuad->ys());
}

// Count the number of proxy runs in the entry set. This usually is already computed by
// SkGpuDevice, but when the BatchLengthLimiter chops the set up it must determine a new proxy count
// for each split.
static int proxy_run_count(const GrRenderTargetContext::TextureSetEntry set[], int count) {
    int actualProxyRunCount = 0;
    const GrSurfaceProxy* lastProxy = nullptr;
    for (int i = 0; i < count; ++i) {
        if (set[i].fProxyView.proxy() != lastProxy) {
            actualProxyRunCount++;
            lastProxy = set[i].fProxyView.proxy();
        }
    }
    return actualProxyRunCount;
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
                                          DrawQuad* quad,
                                          const SkRect* domain) {
        GrOpMemoryPool* pool = context->priv().opMemoryPool();
        return pool->allocate<TextureOp>(std::move(proxyView), std::move(textureXform), filter,
                                         color, saturate, aaType, quad, domain);
    }

    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                          GrRenderTargetContext::TextureSetEntry set[],
                                          int cnt,
                                          int proxyRunCnt,
                                          GrSamplerState::Filter filter,
                                          GrTextureOp::Saturate saturate,
                                          GrAAType aaType,
                                          SkCanvas::SrcRectConstraint constraint,
                                          const SkMatrix& viewMatrix,
                                          sk_sp<GrColorSpaceXform> textureColorSpaceXform) {
        // Allocate size based on proxyRunCnt, since that determines number of ViewCountPairs.
        SkASSERT(proxyRunCnt <= cnt);

        size_t size = sizeof(TextureOp) + sizeof(ViewCountPair) * (proxyRunCnt - 1);
        GrOpMemoryPool* pool = context->priv().opMemoryPool();
        void* mem = pool->allocate(size);
        return std::unique_ptr<GrDrawOp>(
                new (mem) TextureOp(set, cnt, proxyRunCnt, filter, saturate, aaType, constraint,
                                    viewMatrix, std::move(textureColorSpaceXform)));
    }

    ~TextureOp() override {
        for (unsigned p = 1; p < fMetadata.fProxyCount; ++p) {
            fViewCountPairs[p].~ViewCountPair();
        }
    }

    const char* name() const override { return "TextureOp"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        bool mipped = (GrSamplerState::Filter::kMipMap == fMetadata.filter());
        for (unsigned p = 0; p <  fMetadata.fProxyCount; ++p) {
            func(fViewCountPairs[p].fProxy.get(), GrMipMapped(mipped));
        }
    }

#ifdef SK_DEBUG
    SkString dumpInfo() const override {
        SkString str;
        str.appendf("# draws: %d\n", fQuads.count());
        auto iter = fQuads.iterator();
        for (unsigned p = 0; p < fMetadata.fProxyCount; ++p) {
            str.appendf("Proxy ID: %d, Filter: %d\n",
                        fViewCountPairs[p].fProxy->uniqueID().asUInt(),
                        static_cast<int>(fMetadata.fFilter));
            int i = 0;
            while(i < fViewCountPairs[p].fQuadCnt && iter.next()) {
                const GrQuad* quad = iter.deviceQuad();
                GrQuad uv = iter.isLocalValid() ? *(iter.localQuad()) : GrQuad();
                const ColorDomainAndAA& info = iter.metadata();
                str.appendf(
                        "%d: Color: 0x%08x, Domain(%d): [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n"
                        "  UVs  [(%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f)]\n"
                        "  Quad [(%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f)]\n",
                        i, info.fColor.toBytes_RGBA(), fMetadata.fDomain, info.fDomainRect.fLeft,
                        info.fDomainRect.fTop, info.fDomainRect.fRight, info.fDomainRect.fBottom,
                        quad->point(0).fX, quad->point(0).fY, quad->point(1).fX, quad->point(1).fY,
                        quad->point(2).fX, quad->point(2).fY, quad->point(3).fX, quad->point(3).fY,
                        uv.point(0).fX, uv.point(0).fY, uv.point(1).fX, uv.point(1).fY,
                        uv.point(2).fX, uv.point(2).fY, uv.point(3).fX, uv.point(3).fY);

                i++;
            }
        }
        str += INHERITED::dumpInfo();
        return str;
    }

    static void ValidateResourceLimits() {
        // The op implementation has an upper bound on the number of quads that it can represent.
        // However, the resource manager imposes its own limit on the number of quads, which should
        // always be lower than the numerical limit this op can hold.
        using CountStorage = decltype(Metadata::fTotalQuadCount);
        CountStorage maxQuadCount = std::numeric_limits<CountStorage>::max();
        // GrResourceProvider::Max...() is typed as int, so don't compare across signed/unsigned.
        int resourceLimit = SkTo<int>(maxQuadCount);
        SkASSERT(GrResourceProvider::MaxNumAAQuads() <= resourceLimit &&
                 GrResourceProvider::MaxNumNonAAQuads() <= resourceLimit);
    }
#endif

    GrProcessorSet::Analysis finalize(
            const GrCaps& caps, const GrAppliedClip*, bool hasMixedSampledCoverage,
            GrClampType clampType) override {
        SkASSERT(fMetadata.colorType() == ColorType::kNone);
        auto iter = fQuads.metadata();
        while(iter.next()) {
            auto colorType = GrQuadPerEdgeAA::MinColorType(iter->fColor);
            fMetadata.fColorType = std::max(fMetadata.fColorType, static_cast<uint16_t>(colorType));
        }
        return GrProcessorSet::EmptySetAnalysis();
    }

    FixedFunctionFlags fixedFunctionFlags() const override {
        return fMetadata.aaType() == GrAAType::kMSAA ? FixedFunctionFlags::kUsesHWAA
                                                     : FixedFunctionFlags::kNone;
    }

    DEFINE_OP_CLASS_ID

private:
    friend class ::GrOpMemoryPool;

    struct ColorDomainAndAA {
        ColorDomainAndAA(const SkPMColor4f& color, const SkRect& domainRect, GrQuadAAFlags aaFlags)
                : fColor(color)
                , fDomainRect(domainRect)
                , fAAFlags(static_cast<uint16_t>(aaFlags)) {
            SkASSERT(fAAFlags == static_cast<uint16_t>(aaFlags));
        }

        SkPMColor4f fColor;
        // If the op doesn't use domains, this is ignored. If the op uses domains and the specific
        // entry does not, this rect will equal kLargeRect, so it automatically has no effect.
        SkRect fDomainRect;
        unsigned fAAFlags : 4;

        GrQuadAAFlags aaFlags() const { return static_cast<GrQuadAAFlags>(fAAFlags); }
    };

    struct ViewCountPair {
        // Normally this would be a GrSurfaceProxyView, but GrTextureOp applies the GrOrigin right
        // away so it doesn't need to be stored, and all ViewCountPairs in an op have the same
        // swizzle so that is stored in the op metadata.
        sk_sp<GrSurfaceProxy> fProxy;
        int fQuadCnt;
    };

    // TextureOp and ViewCountPair are 8 byte aligned. This is packed into 8 bytes to minimally
    // increase the size of the op; increasing the op size can have a surprising impact on
    // performance (since texture ops are one of the most commonly used in an app).
    struct Metadata {
        // AAType must be filled after initialization; ColorType is determined in finalize()
        Metadata(const GrSwizzle& swizzle, GrSamplerState::Filter filter,
                 GrQuadPerEdgeAA::Domain domain, GrTextureOp::Saturate saturate)
                : fSwizzle(swizzle)
                , fProxyCount(1)
                , fTotalQuadCount(1)
                , fFilter(static_cast<uint16_t>(filter))
                , fAAType(static_cast<uint16_t>(GrAAType::kNone))
                , fColorType(static_cast<uint16_t>(ColorType::kNone))
                , fDomain(static_cast<uint16_t>(domain))
                , fSaturate(static_cast<uint16_t>(saturate)) {}

        GrSwizzle fSwizzle; // sizeof(GrSwizzle) == uint16_t
        uint16_t  fProxyCount;
        // This will be >= fProxyCount, since a proxy may be drawn multiple times
        uint16_t  fTotalQuadCount;

        // These must be based on uint16_t to help MSVC's pack bitfields optimally
        uint16_t  fFilter     : 2; // GrSamplerState::Filter
        uint16_t  fAAType     : 2; // GrAAType
        uint16_t  fColorType  : 2; // GrQuadPerEdgeAA::ColorType
        uint16_t  fDomain     : 1; // bool
        uint16_t  fSaturate   : 1; // bool
        uint16_t  fUnused     : 8; // # of bits left before Metadata exceeds 8 bytes

        GrSamplerState::Filter filter() const {
            return static_cast<GrSamplerState::Filter>(fFilter);
        }
        GrAAType aaType() const { return static_cast<GrAAType>(fAAType); }
        ColorType colorType() const { return static_cast<ColorType>(fColorType); }
        Domain domain() const { return static_cast<Domain>(fDomain); }
        GrTextureOp::Saturate saturate() const {
            return static_cast<GrTextureOp::Saturate>(fSaturate);
        }

        static_assert(GrSamplerState::kFilterCount <= 4);
        static_assert(kGrAATypeCount <= 4);
        static_assert(GrQuadPerEdgeAA::kColorTypeCount <= 4);
    };
    static_assert(sizeof(Metadata) == 8);

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
        void setMeshProxy(int index, GrSurfaceProxy* proxy) {
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

    // If domainRect is not null it will be used to apply a strict src rect-style constraint.
    TextureOp(GrSurfaceProxyView proxyView,
              sk_sp<GrColorSpaceXform> textureColorSpaceXform,
              GrSamplerState::Filter filter,
              const SkPMColor4f& color,
              GrTextureOp::Saturate saturate,
              GrAAType aaType,
              DrawQuad* quad,
              const SkRect* domainRect)
            : INHERITED(ClassID())
            , fQuads(1, true /* includes locals */)
            , fTextureColorSpaceXform(std::move(textureColorSpaceXform))
            , fPrePreparedDesc(nullptr)
            , fMetadata(proxyView.swizzle(), filter, Domain(!!domainRect), saturate) {

        // Clean up disparities between the overall aa type and edge configuration and apply
        // optimizations based on the rect and matrix when appropriate
        GrQuadUtils::ResolveAAType(aaType, quad->fEdgeFlags, quad->fDevice,
                                   &aaType, &quad->fEdgeFlags);
        fMetadata.fAAType = static_cast<uint16_t>(aaType);

        // We expect our caller to have already caught this optimization.
        SkASSERT(!domainRect ||
                 !domainRect->contains(proxyView.proxy()->backingStoreBoundsRect()));

        // We may have had a strict constraint with nearest filter solely due to possible AA bloat.
        // If we don't have (or determined we don't need) coverage AA then we can skip using a
        // domain.
        if (domainRect && filter == GrSamplerState::Filter::kNearest &&
            aaType != GrAAType::kCoverage) {
            domainRect = nullptr;
            fMetadata.fDomain = static_cast<uint16_t>(Domain::kNo);
        }

        // Normalize src coordinates and the domain (if set)
        NormalizationParams params = proxy_normalization_params(proxyView.proxy(),
                                                                proxyView.origin());
        normalize_src_quad(params, &quad->fLocal);
        SkRect domain = normalize_domain(filter, params, domainRect);

        // Set bounds before clipping so we don't have to worry about unioning the bounds of
        // the two potential quads (GrQuad::bounds() is perspective-safe).
        this->setBounds(quad->fDevice.bounds(), HasAABloat(aaType == GrAAType::kCoverage),
                        IsHairline::kNo);

        int quadCount = this->appendQuad(quad, color, domain);
        fViewCountPairs[0] = {proxyView.detachProxy(), quadCount};
    }

    TextureOp(GrRenderTargetContext::TextureSetEntry set[],
              int cnt,
              int proxyRunCnt,
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
            , fMetadata(set[0].fProxyView.swizzle(), GrSamplerState::Filter::kNearest,
                        Domain::kNo, saturate) {
        // Update counts to reflect the batch op
        fMetadata.fProxyCount = SkToUInt(proxyRunCnt);
        fMetadata.fTotalQuadCount = SkToUInt(cnt);

        SkRect bounds = SkRectPriv::MakeLargestInverted();

        GrAAType netAAType = GrAAType::kNone; // aa type maximally compatible with all dst rects
        Domain netDomain = Domain::kNo;
        GrSamplerState::Filter netFilter = GrSamplerState::Filter::kNearest;

        const GrSurfaceProxy* curProxy = nullptr;

        // 'q' is the index in 'set' and fQuadBuffer; 'p' is the index in fViewCountPairs and only
        // increases when set[q]'s proxy changes.
        int p = 0;
        for (int q = 0; q < cnt; ++q) {
            if (q == 0) {
                // We do not placement new the first ViewCountPair since that one is allocated and
                // initialized as part of the GrTextureOp creation.
                fViewCountPairs[0].fProxy = set[0].fProxyView.detachProxy();
                fViewCountPairs[0].fQuadCnt = 0;
                curProxy = fViewCountPairs[0].fProxy.get();
            } else if (set[q].fProxyView.proxy() != curProxy) {
                // We must placement new the ViewCountPairs here so that the sk_sps in the
                // GrSurfaceProxyView get initialized properly.
                new(&fViewCountPairs[++p])ViewCountPair({set[q].fProxyView.detachProxy(), 0});

                curProxy = fViewCountPairs[p].fProxy.get();
                SkASSERT(GrTextureProxy::ProxiesAreCompatibleAsDynamicState(
                        curProxy, fViewCountPairs[0].fProxy.get()));
                SkASSERT(fMetadata.fSwizzle == set[q].fProxyView.swizzle());
            } // else another quad referencing the same proxy

            SkMatrix ctm = viewMatrix;
            if (set[q].fPreViewMatrix) {
                ctm.preConcat(*set[q].fPreViewMatrix);
            }

            // Use dstRect/srcRect unless dstClip is provided, in which case derive new source
            // coordinates by mapping dstClipQuad by the dstRect to srcRect transform.
            DrawQuad quad;
            if (set[q].fDstClipQuad) {
                quad.fDevice = GrQuad::MakeFromSkQuad(set[q].fDstClipQuad, ctm);

                SkPoint srcPts[4];
                GrMapRectPoints(set[q].fDstRect, set[q].fSrcRect, set[q].fDstClipQuad, srcPts, 4);
                quad.fLocal = GrQuad::MakeFromSkQuad(srcPts, SkMatrix::I());
            } else {
                quad.fDevice = GrQuad::MakeFromRect(set[q].fDstRect, ctm);
                quad.fLocal = GrQuad(set[q].fSrcRect);
            }

            if (netFilter != filter && filter_has_effect(quad.fLocal, quad.fDevice)) {
                // The only way netFilter != filter is if bilerp is requested and we haven't yet
                // found a quad that requires bilerp (so net is still nearest).
                SkASSERT(netFilter == GrSamplerState::Filter::kNearest &&
                         filter == GrSamplerState::Filter::kBilerp);
                netFilter = GrSamplerState::Filter::kBilerp;
            }

            // Normalize the src quads and apply origin
            NormalizationParams proxyParams = proxy_normalization_params(
                    curProxy, set[q].fProxyView.origin());
            normalize_src_quad(proxyParams, &quad.fLocal);

            // Update overall bounds of the op as the union of all quads
            bounds.joinPossiblyEmptyRect(quad.fDevice.bounds());

            // Determine the AA type for the quad, then merge with net AA type
            GrAAType aaForQuad;
            GrQuadUtils::ResolveAAType(aaType, set[q].fAAFlags, quad.fDevice,
                                       &aaForQuad, &quad.fEdgeFlags);
            // Resolve sets aaForQuad to aaType or None, there is never a change between aa methods
            SkASSERT(aaForQuad == GrAAType::kNone || aaForQuad == aaType);
            if (netAAType == GrAAType::kNone && aaForQuad != GrAAType::kNone) {
                netAAType = aaType;
            }

            // Calculate metadata for the entry
            const SkRect* domainForQuad = nullptr;
            if (constraint == SkCanvas::kStrict_SrcRectConstraint) {
                // Check (briefly) if the strict constraint is needed for this set entry
                if (!set[q].fSrcRect.contains(curProxy->backingStoreBoundsRect()) &&
                    (filter == GrSamplerState::Filter::kBilerp ||
                     aaForQuad == GrAAType::kCoverage)) {
                    // Can't rely on hardware clamping and the draw will access outer texels
                    // for AA and/or bilerp. Unlike filter quality, this op still has per-quad
                    // control over AA so that can check aaForQuad, not netAAType.
                    netDomain = Domain::kYes;
                    domainForQuad = &set[q].fSrcRect;
                }
            }
            // This domain may represent a no-op, otherwise it will have the origin and dimensions
            // of the texture applied to it. Insetting for bilinear filtering is deferred until
            // on[Pre]Prepare so that the overall filter can be lazily determined.
            SkRect domain = normalize_domain(filter, proxyParams, domainForQuad);

            // Always append a quad (or 2 if perspective clipped), it just may refer back to a prior
            // ViewCountPair (this frequently happens when Chrome draws 9-patches).
            float alpha = SkTPin(set[q].fAlpha, 0.f, 1.f);
            fViewCountPairs[p].fQuadCnt += this->appendQuad(
                    &quad, {alpha, alpha, alpha, alpha}, domain);
        }
        // The # of proxy switches should match what was provided (+1 because we incremented p
        // when a new proxy was encountered).
        SkASSERT((p + 1) == fMetadata.fProxyCount);
        SkASSERT(fQuads.count() == fMetadata.fTotalQuadCount);

        fMetadata.fAAType = static_cast<uint16_t>(netAAType);
        fMetadata.fFilter = static_cast<uint16_t>(netFilter);
        fMetadata.fDomain = static_cast<uint16_t>(netDomain);

        this->setBounds(bounds, HasAABloat(netAAType == GrAAType::kCoverage), IsHairline::kNo);
    }

    int appendQuad(DrawQuad* quad, const SkPMColor4f& color, const SkRect& domain) {
        DrawQuad extra;
        // Only clip when there's anti-aliasing. When non-aa, the GPU clips just fine and there's
        // no inset/outset math that requires w > 0.
        int quadCount = quad->fEdgeFlags != GrQuadAAFlags::kNone ?
                GrQuadUtils::ClipToW0(quad, &extra) : 1;
        if (quadCount == 0) {
            // We can't discard the op at this point, but disable AA flags so it won't go through
            // inset/outset processing
            quad->fEdgeFlags = GrQuadAAFlags::kNone;
            quadCount = 1;
        }
        fQuads.append(quad->fDevice, {color, domain, quad->fEdgeFlags},  &quad->fLocal);
        if (quadCount > 1) {
            fQuads.append(extra.fDevice, {color, domain, extra.fEdgeFlags}, &extra.fLocal);
            fMetadata.fTotalQuadCount++;
        }
        return quadCount;
    }

    void onPrePrepareDraws(GrRecordingContext* context,
                           const GrSurfaceProxyView* dstView,
                           GrAppliedClip* clip,
                           const GrXferProcessor::DstProxyView& dstProxyView) override {
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
        SkAssertResult(FillInData(*context->priv().caps(), this, fPrePreparedDesc,
                                  fPrePreparedDesc->fVertices, nullptr, 0, nullptr, nullptr));
    }

    static bool FillInData(const GrCaps& caps, TextureOp* texOp, PrePreparedDesc* desc,
                           char* pVertexData, GrMesh* meshes, int absBufferOffset,
                           sk_sp<const GrBuffer> vertexBuffer,
                           sk_sp<const GrBuffer> indexBuffer) {
        int totQuadsSeen = 0;
        SkDEBUGCODE(int totVerticesSeen = 0;)
        SkDEBUGCODE(const size_t vertexSize = desc->fVertexSpec.vertexSize());

        GrQuadPerEdgeAA::Tessellator tessellator(desc->fVertexSpec, pVertexData);
        int meshIndex = 0;
        for (const auto& op : ChainRange<TextureOp>(texOp)) {
            auto iter = op.fQuads.iterator();
            for (unsigned p = 0; p < op.fMetadata.fProxyCount; ++p) {
                const int quadCnt = op.fViewCountPairs[p].fQuadCnt;
                SkDEBUGCODE(int meshVertexCnt = quadCnt * desc->fVertexSpec.verticesPerQuad());
                SkASSERT(meshIndex < desc->fNumProxies);

                if (pVertexData) {
                    // Can just use top-left for origin here since we only need the dimensions to
                    // determine the texel size for insetting.
                    NormalizationParams params = proxy_normalization_params(
                            op.fViewCountPairs[p].fProxy.get(), kTopLeft_GrSurfaceOrigin);

                    bool inset = texOp->fMetadata.filter() != GrSamplerState::Filter::kNearest;

                    for (int i = 0; i < quadCnt && iter.next(); ++i) {
                        SkASSERT(iter.isLocalValid());
                        const ColorDomainAndAA& info = iter.metadata();

                        tessellator.append(iter.deviceQuad(), iter.localQuad(), info.fColor,
                                           inset ? inset_domain_for_bilerp(params, info.fDomainRect)
                                                 : info.fDomainRect,
                                           info.aaFlags());
                    }
                    desc->setMeshProxy(meshIndex, op.fViewCountPairs[p].fProxy.get());

                    SkASSERT((totVerticesSeen + meshVertexCnt) * vertexSize
                             == (size_t)(tessellator.vertices() - pVertexData));
                }

                if (meshes) {
                    GrQuadPerEdgeAA::ConfigureMesh(caps, &(meshes[meshIndex]), desc->fVertexSpec,
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
            SkASSERT(!pVertexData || !iter.next());
        }

        SkASSERT(!pVertexData ||
                 (desc->totalSizeInBytes() == (size_t)(tessellator.vertices() - pVertexData)));
        SkASSERT(meshIndex == desc->fNumProxies);
        SkASSERT(totQuadsSeen == desc->fNumTotalQuads);
        SkASSERT(totVerticesSeen == desc->totalNumVertices());
        return true;
    }

#ifdef SK_DEBUG
    void validate() const override {
        // NOTE: Since this is debug-only code, we use the virtual asTextureProxy()
        auto textureType = fViewCountPairs[0].fProxy->asTextureProxy()->textureType();
        GrAAType aaType = fMetadata.aaType();

        int quadCount = 0;
        for (const auto& op : ChainRange<TextureOp>(this)) {
            SkASSERT(op.fMetadata.fSwizzle == fMetadata.fSwizzle);

            for (unsigned p = 0; p < op.fMetadata.fProxyCount; ++p) {
                auto* proxy = op.fViewCountPairs[p].fProxy->asTextureProxy();
                quadCount += op.fViewCountPairs[p].fQuadCnt;
                SkASSERT(proxy);
                SkASSERT(proxy->textureType() == textureType);
            }

            // Each individual op must be a single aaType. kCoverage and kNone ops can chain
            // together but kMSAA ones do not.
            if (aaType == GrAAType::kCoverage || aaType == GrAAType::kNone) {
                SkASSERT(op.fMetadata.aaType() == GrAAType::kCoverage ||
                         op.fMetadata.aaType() == GrAAType::kNone);
            } else {
                SkASSERT(aaType == GrAAType::kMSAA && op.fMetadata.aaType() == GrAAType::kMSAA);
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
        GrAAType overallAAType = fMetadata.aaType();

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
            if (op.fMetadata.domain() == Domain::kYes) {
                domain = Domain::kYes;
            }
            colorType = std::max(colorType, op.fMetadata.colorType());
            desc->fNumProxies += op.fMetadata.fProxyCount;

            for (unsigned p = 0; p < op.fMetadata.fProxyCount; ++p) {
                maxQuadsPerMesh = std::max(op.fViewCountPairs[p].fQuadCnt, maxQuadsPerMesh);
            }
            desc->fNumTotalQuads += op.totNumQuads();

            if (op.fMetadata.aaType() == GrAAType::kCoverage) {
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
        for (unsigned p = 0; p < fMetadata.fProxyCount; ++p) {
            tmp += fViewCountPairs[p].fQuadCnt;
        }
        SkASSERT(tmp == fMetadata.fTotalQuadCount);
#endif

        return fMetadata.fTotalQuadCount;
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
            result = FillInData(target->caps(), this, &desc, nullptr, meshes, vertexOffsetInBuffer,
                                std::move(vbuffer), std::move(indexBuffer));
        } else {
            // Fills in both vertex data and mesh data
            result = FillInData(target->caps(), this, &desc, (char*) vdata, meshes,
                                vertexOffsetInBuffer, std::move(vbuffer), std::move(indexBuffer));
        }

        if (!result) {
            return;
        }

        GrGeometryProcessor* gp;

        {
            const GrBackendFormat& backendFormat =
                    fViewCountPairs[0].fProxy->backendFormat();

            GrSamplerState samplerState = GrSamplerState(GrSamplerState::WrapMode::kClamp,
                                                         fMetadata.filter());

            gp = GrQuadPerEdgeAA::MakeTexturedProcessor(target->allocator(),
                desc.fVertexSpec, *target->caps().shaderCaps(), backendFormat,
                samplerState, fMetadata.fSwizzle, std::move(fTextureColorSpaceXform),
                fMetadata.saturate());

            SkASSERT(vertexSize == gp->vertexStride());
        }

        target->recordDraw(gp, meshes, desc.fNumProxies,
                           desc.fFixedDynamicState, desc.fDynamicStateArrays,
                           desc.fVertexSpec.primitiveType());
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        auto pipelineFlags = (GrAAType::kMSAA == fMetadata.aaType())
                ? GrPipeline::InputFlags::kHWAntialias
                : GrPipeline::InputFlags::kNone;

        auto pipeline = GrSimpleMeshDrawOpHelper::CreatePipeline(flushState,
                                                                 GrProcessorSet::MakeEmptySet(),
                                                                 pipelineFlags);

        flushState->executeDrawsAndUploadsForMeshDrawOp(this, chainBounds, pipeline);
    }

    CombineResult onCombineIfPossible(GrOp* t, GrRecordingContext::Arenas*,
                                      const GrCaps& caps) override {
        TRACE_EVENT0("skia.gpu", TRACE_FUNC);
        const auto* that = t->cast<TextureOp>();

        if (fPrePreparedDesc || that->fPrePreparedDesc) {
            // This should never happen (since only DDL recorded ops should be prePrepared)
            // but, in any case, we should never combine ops that that been prePrepared
            return CombineResult::kCannotCombine;
        }

        if (fMetadata.domain() != that->fMetadata.domain()) {
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
        if (fMetadata.aaType() != that->fMetadata.aaType()) {
            if (!CanUpgradeAAOnMerge(fMetadata.aaType(), that->fMetadata.aaType())) {
                return CombineResult::kCannotCombine;
            }
            upgradeToCoverageAAOnMerge = true;
        }

        if (CombinedQuadCountWillOverflow(fMetadata.aaType(), upgradeToCoverageAAOnMerge,
                                          this->numChainedQuads() + that->numChainedQuads())) {
            return CombineResult::kCannotCombine;
        }

        if (fMetadata.saturate() != that->fMetadata.saturate()) {
            return CombineResult::kCannotCombine;
        }
        if (fMetadata.filter() != that->fMetadata.filter()) {
            return CombineResult::kCannotCombine;
        }
        if (fMetadata.fSwizzle != that->fMetadata.fSwizzle) {
            return CombineResult::kCannotCombine;
        }
        const auto* thisProxy = fViewCountPairs[0].fProxy.get();
        const auto* thatProxy = that->fViewCountPairs[0].fProxy.get();
        if (fMetadata.fProxyCount > 1 || that->fMetadata.fProxyCount > 1 ||
            thisProxy != thatProxy) {
            // We can't merge across different proxies. Check if 'this' can be chained with 'that'.
            if (GrTextureProxy::ProxiesAreCompatibleAsDynamicState(thisProxy, thatProxy) &&
                caps.dynamicStateArrayGeometryProcessorTextureSupport()) {
                return CombineResult::kMayChain;
            }
            return CombineResult::kCannotCombine;
        }

        fMetadata.fDomain |= that->fMetadata.fDomain;
        fMetadata.fColorType = std::max(fMetadata.fColorType, that->fMetadata.fColorType);
        if (upgradeToCoverageAAOnMerge) {
            fMetadata.fAAType = static_cast<uint16_t>(GrAAType::kCoverage);
        }

        // Concatenate quad lists together
        fQuads.concat(that->fQuads);
        fViewCountPairs[0].fQuadCnt += that->fQuads.count();
        fMetadata.fTotalQuadCount += that->fQuads.count();

        return CombineResult::kMerged;
    }

    GrQuadBuffer<ColorDomainAndAA> fQuads;
    sk_sp<GrColorSpaceXform> fTextureColorSpaceXform;
    // 'fPrePreparedDesc' is only filled in when this op has been prePrepared. In that case,
    // it - and the matching dynamic and fixed state - have been allocated in the opPOD arena
    // not in the FlushState arena.
    PrePreparedDesc* fPrePreparedDesc;
    // All configurable state of TextureOp is packed into one field to minimize the op's size.
    // Historically, increasing the size of TextureOp has caused surprising perf regressions, so
    // consider/measure changes with care.
    Metadata fMetadata;

    // This field must go last. When allocating this op, we will allocate extra space to hold
    // additional ViewCountPairs immediately after the op's allocation so we can treat this
    // as an fProxyCnt-length array.
    ViewCountPair fViewCountPairs[1];

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
                                            SkAlphaType alphaType,
                                            sk_sp<GrColorSpaceXform> textureXform,
                                            GrSamplerState::Filter filter,
                                            const SkPMColor4f& color,
                                            Saturate saturate,
                                            SkBlendMode blendMode,
                                            GrAAType aaType,
                                            DrawQuad* quad,
                                            const SkRect* domain) {
    // Apply optimizations that are valid whether or not using GrTextureOp or GrFillRectOp
    if (domain && domain->contains(proxyView.proxy()->backingStoreBoundsRect())) {
        // No need for a shader-based domain if hardware clamping achieves the same effect
        domain = nullptr;
    }

    if (filter != GrSamplerState::Filter::kNearest &&
        !filter_has_effect(quad->fLocal, quad->fDevice)) {
        filter = GrSamplerState::Filter::kNearest;
    }

    if (blendMode == SkBlendMode::kSrcOver) {
        return TextureOp::Make(context, std::move(proxyView), std::move(textureXform), filter,
                               color, saturate, aaType, std::move(quad), domain);
    } else {
        // Emulate complex blending using GrFillRectOp
        GrPaint paint;
        paint.setColor4f(color);
        paint.setXPFactory(SkBlendMode_AsXPFactory(blendMode));

        std::unique_ptr<GrFragmentProcessor> fp;
        if (domain) {
            const auto& caps = *context->priv().caps();
            SkRect localRect;
            if (quad->fLocal.asRect(&localRect)) {
                fp = GrTextureEffect::MakeSubset(std::move(proxyView), alphaType, SkMatrix::I(), filter,
                                                 *domain, localRect, caps);
            } else {
                fp = GrTextureEffect::MakeSubset(std::move(proxyView), alphaType, SkMatrix::I(), filter,
                                                 *domain, caps);
            }
        } else {
            fp = GrTextureEffect::Make(std::move(proxyView), alphaType, SkMatrix::I(), filter);
        }
        fp = GrColorSpaceXformEffect::Make(std::move(fp), std::move(textureXform));
        paint.addColorFragmentProcessor(std::move(fp));
        if (saturate == GrTextureOp::Saturate::kYes) {
            paint.addColorFragmentProcessor(GrClampFragmentProcessor::Make(false));
        }

        return GrFillRectOp::Make(context, std::move(paint), aaType, quad);
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

    void createOp(GrRenderTargetContext::TextureSetEntry set[],
                  int clumpSize,
                  GrAAType aaType) {
        int clumpProxyCount = proxy_run_count(&set[fNumClumped], clumpSize);
        std::unique_ptr<GrDrawOp> op = TextureOp::Make(fContext, &set[fNumClumped], clumpSize,
                                                       clumpProxyCount, fFilter, fSaturate, aaType,
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
void GrTextureOp::AddTextureSetOps(GrRenderTargetContext* rtc,
                                   const GrClip& clip,
                                   GrRecordingContext* context,
                                   GrRenderTargetContext::TextureSetEntry set[],
                                   int cnt,
                                   int proxyRunCnt,
                                   GrSamplerState::Filter filter,
                                   Saturate saturate,
                                   SkBlendMode blendMode,
                                   GrAAType aaType,
                                   SkCanvas::SrcRectConstraint constraint,
                                   const SkMatrix& viewMatrix,
                                   sk_sp<GrColorSpaceXform> textureColorSpaceXform) {
    // Ensure that the index buffer limits are lower than the proxy and quad count limits of
    // the op's metadata so we don't need to worry about overflow.
    SkDEBUGCODE(TextureOp::ValidateResourceLimits();)
    SkASSERT(proxy_run_count(set, cnt) == proxyRunCnt);

    // First check if we can support batches as a single op
    if (blendMode != SkBlendMode::kSrcOver ||
        !context->priv().caps()->dynamicStateArrayGeometryProcessorTextureSupport()) {
        // Append each entry as its own op; these may still be GrTextureOps if the blend mode is
        // src-over but the backend doesn't support dynamic state changes. Otherwise Make()
        // automatically creates the appropriate GrFillRectOp to emulate GrTextureOp.
        SkMatrix ctm;
        for (int i = 0; i < cnt; ++i) {
            float alpha = set[i].fAlpha;
            ctm = viewMatrix;
            if (set[i].fPreViewMatrix) {
                ctm.preConcat(*set[i].fPreViewMatrix);
            }

            DrawQuad quad;
            quad.fEdgeFlags = set[i].fAAFlags;
            if (set[i].fDstClipQuad) {
                quad.fDevice = GrQuad::MakeFromSkQuad(set[i].fDstClipQuad, ctm);

                SkPoint srcPts[4];
                GrMapRectPoints(set[i].fDstRect, set[i].fSrcRect, set[i].fDstClipQuad, srcPts, 4);
                quad.fLocal = GrQuad::MakeFromSkQuad(srcPts, SkMatrix::I());
            } else {
                quad.fDevice = GrQuad::MakeFromRect(set[i].fDstRect, ctm);
                quad.fLocal = GrQuad(set[i].fSrcRect);
            }

            const SkRect* domain = constraint == SkCanvas::kStrict_SrcRectConstraint
                    ? &set[i].fSrcRect : nullptr;

            auto op = Make(context, set[i].fProxyView, set[i].fSrcAlphaType, textureColorSpaceXform,
                           filter, {alpha, alpha, alpha, alpha}, saturate, blendMode, aaType,
                           &quad, domain);
            rtc->addDrawOp(clip, std::move(op));
        }
        return;
    }

    // Second check if we can always just make a single op and avoid the extra iteration
    // needed to clump things together.
    if (cnt <= std::min(GrResourceProvider::MaxNumNonAAQuads(),
                      GrResourceProvider::MaxNumAAQuads())) {
        auto op = TextureOp::Make(context, set, cnt, proxyRunCnt, filter, saturate, aaType,
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
            int clumpSize = std::min(state.numLeft(), GrResourceProvider::MaxNumNonAAQuads());

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
    SkISize dims;
    dims.fHeight = random->nextULessThan(90) + 10;
    dims.fWidth = random->nextULessThan(90) + 10;
    auto origin = random->nextBool() ? kTopLeft_GrSurfaceOrigin : kBottomLeft_GrSurfaceOrigin;
    GrMipMapped mipMapped = random->nextBool() ? GrMipMapped::kYes : GrMipMapped::kNo;
    SkBackingFit fit = SkBackingFit::kExact;
    if (mipMapped == GrMipMapped::kNo) {
        fit = random->nextBool() ? SkBackingFit::kApprox : SkBackingFit::kExact;
    }
    const GrBackendFormat format =
            context->priv().caps()->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                            GrRenderable::kNo);
    GrSwizzle swizzle = context->priv().caps()->getReadSwizzle(format, GrColorType::kRGBA_8888);

    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(
            format, dims, swizzle, GrRenderable::kNo, 1, mipMapped, fit, SkBudgeted::kNo,
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
            context->priv().caps()->getReadSwizzle(format, GrColorType::kRGBA_8888));
    auto alphaType = static_cast<SkAlphaType>(
            random->nextRangeU(kUnknown_SkAlphaType + 1, kLastEnum_SkAlphaType));

    DrawQuad quad = {GrQuad::MakeFromRect(rect, viewMatrix), GrQuad(srcRect), aaFlags};
    return GrTextureOp::Make(context, std::move(proxyView), alphaType, std::move(texXform), filter,
                             color, saturate, SkBlendMode::kSrcOver, aaType,
                             &quad, useDomain ? &srcRect : nullptr);
}

#endif
