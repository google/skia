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
#include "src/gpu/GrClip.h"
#include "src/gpu/GrDrawOpTest.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrResourceProviderPriv.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrSurfaceContextPriv.h"
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
static NormalizationParams proxy_normalization_params(const GrSurfaceProxyView& view) {
    return proxy_normalization_params(view.proxy(), view.origin());
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
static SkRect normalize_domain(const NormalizationParams& params,
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
static void normalize_src_quad(const NormalizationParams& params, GrQuad* srcQuad) {
    // The src quad should not have any perspective
    SkASSERT(!srcQuad->hasPerspective());
    skvx::Vec<4, float> xs = srcQuad->x4f() * params.fIW;
    skvx::Vec<4, float> ys = mad(srcQuad->y4f(), params.fIH, params.fYOffset);
    xs.store(srcQuad->xs());
    ys.store(srcQuad->ys());
}

// TextureOp and ViewCountPair are 8 byte aligned. This is packed into 8 bytes to minimally
// increase the size of the op; increasing the op size can have a surprising impact on
// performance (since texture ops are one of the most commonly used in an app).
struct Metadata {
    // ColorType is determined in finalize()
    Metadata(sk_sp<GrColorSpaceXform> colorXform, const GrSwizzle& swizzle,
             GrSamplerState::Filter filter, GrQuadPerEdgeAA::Domain domain,
             GrDeferredTextureOp::Saturate saturate, GrAAType aaType)
            : fTextureColorSpaceXform(std::move(colorXform))
            , fSwizzle(swizzle)
            , fProxyCount(0)
            , fTotalQuadCount(0)
            , fFilter(static_cast<uint16_t>(filter))
            , fAAType(static_cast<uint16_t>(aaType))
            , fColorType(static_cast<uint16_t>(ColorType::kNone))
            , fDomain(static_cast<uint16_t>(domain))
            , fSaturate(static_cast<uint16_t>(saturate)) {}

    sk_sp<GrColorSpaceXform> fTextureColorSpaceXform;

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
    GrDeferredTextureOp::Saturate saturate() const {
        return static_cast<GrDeferredTextureOp::Saturate>(fSaturate);
    }

    bool isCompatible(const Metadata& other, bool opFinalized) const {
        return this->isCompatible(other, opFinalized, fTotalQuadCount + other.fTotalQuadCount);
    }

    bool isCompatible(const Metadata& other, bool opFinalized, int netQuadCount) const {
        SkASSERT(netQuadCount >= fTotalQuadCount + other.fTotalQuadCount);

        if (this->saturate() != other.saturate()) {
            return false;
        }

        bool upgradeToCoverageAAOnMerge = false;
        if (this->aaType() != other.aaType()) {
            if (GrMeshDrawOp::CanUpgradeAAOnMerge(this->aaType(), other.aaType())) {
                upgradeToCoverageAAOnMerge = true;
            } else {
                return false;
            }
        }
        if (GrMeshDrawOp::CombinedQuadCountWillOverflow(this->aaType(), upgradeToCoverageAAOnMerge,
                                          netQuadCount)) {
            return false;
        }

        if (this->domain() != other.domain()) {
            return false;
        }

        // FIXME okay, so it definitely matters for not lifting up domains, which is kind of annoying
        // since if chrome was just better, we'd be good to go.
        // if (opFinalized) {
            // Once ops have been finalized, we restrict promoting an existing op that had no
            // domain constraint or used kNearest to a slower version. Allowing this promotion
            // while appending to a deferred op strikes a reasonable balance between using
            // fast paths and reducing the overall number of ops.
            // if (this->domain() != other.domain() || this->filter() != other.filter()) {
            if (this->filter() != other.filter()) {
                return false;
            }
        // }
        if (!GrColorSpaceXform::Equals(fTextureColorSpaceXform.get(),
                                       other.fTextureColorSpaceXform.get())) {
            return false;
        }

        return true;
    }

    void combine(const Metadata& other, bool opFinalized) {
        SkASSERT(this->isCompatible(other, opFinalized));

        if (!opFinalized) {
            // When not finalized, the domain and filter are flexible
            // fDomain |= other.fDomain;
            // fFilter = std::max(fFilter, other.fFilter);
        }
        // Color type always generalizes to the max required
        fColorType = std::max(fColorType, other.fColorType);
        // Same with AA type (although isCompatible() enforces that this only moves from
        // kNone to kCoverage).
        if (fAAType != other.fAAType) {
            SkASSERT(GrMeshDrawOp::CanUpgradeAAOnMerge(this->aaType(), other.aaType()));
            fAAType = static_cast<uint16_t>(GrAAType::kCoverage);
        }

        fTotalQuadCount += other.fTotalQuadCount;
    }

    static_assert(GrSamplerState::kFilterCount <= 4);
    static_assert(kGrAATypeCount <= 4);
    static_assert(GrQuadPerEdgeAA::kColorTypeCount <= 4);
};
static_assert(sizeof(Metadata) == 8 + sizeof(sk_sp<GrColorSpaceXform>));

/**
 * Op that implements GrTextureOp::Make. It draws textured quads. Each quad can modulate against a
 * the texture by color. The blend with the destination is always src-over. The edges are non-AA.
 */
class TextureOp final : public GrMeshDrawOp {
public:
    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext::Arenas* arenas,
                                          Metadata&& metadata,
                                          int expectedQuadCount,
                                          bool hasPerspective) {
        // TextureOp includes space for one ViewCountPair already
        size_t size = sizeof(TextureOp) + sizeof(ViewCountPair) * (expectedQuadCount - 1);
        void* mem = arenas->opMemoryPool()->allocate(size);
        return std::unique_ptr<GrDrawOp>(new (mem) TextureOp(arenas->quadAllocator(),
                                                             std::move(metadata),
                                                             expectedQuadCount, hasPerspective));
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
    friend class ::GrDeferredTextureOp;

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

    TextureOp(GrQuadAllocator* arena, Metadata&& metadata,
              int expectedQuadCount, bool hasPerspective)
            : INHERITED(ClassID())
            , fPrePreparedDesc(nullptr)
            , fMetadata(std::move(metadata))
            , fReservedProxyCount(expectedQuadCount) {
        fQuads.reserve(arena, expectedQuadCount, hasPerspective ? GrQuad::Type::kPerspective
                                                                : GrQuad::Type::kAxisAligned,
                       GrQuad::Type::kAxisAligned, /* hasLocals */ true);
    }

    bool grow(GrRecordingContext::Arenas* arenas, int numExtraProxies) {
        // Attempt to expand the reserved space at the end of this op to account for numExtraProxies
        int remaining = fReservedProxyCount - fMetadata.fProxyCount;
        if (remaining >= numExtraProxies) {
            return true;
        } else {
            numExtraProxies -= remaining;
        }

        // SkDebugf("Attempting to add %d proxies ", numExtraProxies);
        if (arenas->opMemoryPool()->resize(this, numExtraProxies * sizeof(ViewCountPair))) {
            fReservedProxyCount += numExtraProxies;
            // SkDebugf("success? 1\n");
            return true;
        } else {
            // SkDebugf("success? 0\n");
            return false;
        }
    }

    void trim(GrRecordingContext::Arenas* arenas) {
        int extraProxies = fReservedProxyCount - fMetadata.fProxyCount;
        if (extraProxies > 0) {
            bool result = arenas->opMemoryPool()->resize(this, -extraProxies * sizeof(ViewCountPair));
            if (result) {
                fReservedProxyCount = fMetadata.fProxyCount;
            }
            // SkDebugf("Attempted resize to free %d proxies, success? %d\n", extraProxies, result);
        } else {
            // SkDebugf("no extra proxies to resize\n");
        }
        // fQuads.trim(arenas->quadAllocator());
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
                samplerState, fMetadata.fSwizzle, std::move(fMetadata.fTextureColorSpaceXform),
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

    CombineResult onCombineIfPossible(GrOp* t, GrRecordingContext::Arenas* arenas,
                                      const GrCaps& caps) override {
        TRACE_EVENT0("skia.gpu", TRACE_FUNC);
        auto* that = t->cast<TextureOp>();

        if (fPrePreparedDesc || that->fPrePreparedDesc) {
            // This should never happen (since only DDL recorded ops should be prePrepared)
            // but, in any case, we should never combine ops that that been prePrepared
            return CombineResult::kCannotCombine;
        }

        if (!fMetadata.isCompatible(that->fMetadata, /* finalized */ true,
                                    this->numChainedQuads() + that->numChainedQuads())) {
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

        // Concatenate quads (which due to to proxy rules above, are associated with the
        // first ViewCountPair).
        fMetadata.combine(that->fMetadata, /* finalized */ true);
        fQuads.concat(arenas->quadAllocator(), &that->fQuads);
        fViewCountPairs[0].fQuadCnt += that->fQuads.count();

        return CombineResult::kMerged;
    }

    GrQuadBuffer<ColorDomainAndAA> fQuads;
    // 'fPrePreparedDesc' is only filled in when this op has been prePrepared. In that case,
    // it - and the matching dynamic and fixed state - have been allocated in the opPOD arena
    // not in the FlushState arena.
    PrePreparedDesc* fPrePreparedDesc;
    // All configurable state of TextureOp is packed into one field to minimize the op's size.
    // Historically, increasing the size of TextureOp has caused surprising perf regressions, so
    // consider/measure changes with care.
    Metadata fMetadata;

    // FIXME not the ideal place for this field... maybe it lives on the deferred OP? or packed into metadata...?
    int fReservedProxyCount;

    // This field must go last. When allocating this op, we will allocate extra space to hold
    // additional ViewCountPairs immediately after the op's allocation so we can treat this
    // as an fProxyCnt-length array.
    ViewCountPair fViewCountPairs[1];

    typedef GrMeshDrawOp INHERITED;
};

}  // anonymous namespace

#if GR_TEST_UTILS
uint32_t GrDeferredTextureOp::ClassID() {
    return TextureOp::ClassID();
}
#endif

bool GrDeferredTextureOp::open(GrRenderTargetContext* rtc, const GrClip& clip,
                               GrAAType aaType, Saturate saturate, int expectedCount) {
    // Since we don't yet know how many texture ops will be used by the clip, just use the
    // render target bounds so that the bounds after applying represent the clips' bounds.
    //  - This means that some draws may keep a clip even if they would have been entirely inside it
    //  - But it also means we may combine ops together under a shared clip that would have been
    //    finalized into different clips specific to their geometries
    //  - Since quad optimizations already gets the common case of dropping the clip, or combining
    //    the clip with the geometry explicitly, we probably won't see a hit due to the first case.
    // SkDebugf("Open deferred op, expected: %d\n", expectedCount);
    SkRect rtBounds = SkRect::MakeIWH(rtc->width(), rtc->height());
    GrRecordingContext* ctx = rtc->surfPriv().getContext();
    bool usesHWAA = aaType == GrAAType::kMSAA;
    GrAppliedClip appliedClip;
    if (!clip.apply(ctx, rtc, usesHWAA, false, &appliedClip, &rtBounds)) {
        // The draw wouldn't happen, so do not open a deferred op
        return false;
    }

    if (fOp) {
        // Have an existing deferred op, so if the clips aren't the same, it must be finalized.
        // If the fixed state isn't compatible, it must also be finalized.
        bool stateCompatible = saturate == fSaturate &&
                               (aaType == fAAType || GrMeshDrawOp::CanUpgradeAAOnMerge(aaType, fAAType));
        if (stateCompatible && appliedClip == fClip) {
            // No need to re-open, but if we got here, the clip bounds ought to be the same
            SkASSERT(rtBounds == fClipBounds);
            fExpected += expectedCount;
            // The grow here is not mandatory, since append() will be one proxy at a time, the
            // existing op may be able to contain some of the expected proxies even if it couldn't
            // expand for the entire amount.
            GrRecordingContext::Arenas arenas = ctx->priv().arenas();
            static_cast<TextureOp*>(fOp.get())->grow(&arenas, expectedCount);
            return true;
        }

        this->finalizeAndSubmit(rtc);
    }

    fExpected = expectedCount;
    fClip = std::move(appliedClip);
    fClipBounds = rtBounds;
    fSaturate = saturate;
    fAAType = aaType;
    return true;
}

void GrDeferredTextureOp::append(GrRenderTargetContext* rtc,
                                 GrSurfaceProxyView proxyView,
                                 SkAlphaType alphaType,
                                 sk_sp<GrColorSpaceXform> textureXform,
                                 GrSamplerState::Filter filter,
                                 const SkPMColor4f& color,
                                 DrawQuad* quad,
                                 const SkRect* domain) {
    SkASSERT(fExpected > 0);

    SkRect deviceBounds = quad->fDevice.bounds();
    if (!deviceBounds.intersects(fClipBounds)) {
        // Would not be visible
        return;
    }

    GrRecordingContext* ctx = rtc->surfPriv().getContext();

    // Attempt to reduce the filter quality, AA, and clamping if they would have no visual effect.
    // Since we're appending to a non-finalized op, these downgrades will not impact compatibility
    // and may end up being re-lifted in order to be merged into the op.
    if (filter != GrSamplerState::Filter::kNearest &&
        !filter_has_effect(quad->fLocal, quad->fDevice)) {
        filter = GrSamplerState::Filter::kNearest;
    }
    GrAAType aaType;
    GrQuadUtils::ResolveAAType(fAAType, quad->fEdgeFlags, quad->fDevice, &aaType, &quad->fEdgeFlags);

    // Calculate the normalized domain and local coords before we clip to the W plane
    // (since it can produce 2 quads we'd need to then normalize).
    if (domain && domain->contains(proxyView.proxy()->backingStoreBoundsRect())) {
        // No need for a shader-based domain if hardware clamping achieves the same effect
        // Since TextureOp only clamps, and clamp is always available, this needs no shader fallback
        domain = nullptr;
    }

    NormalizationParams proxyParams = proxy_normalization_params(proxyView);
    normalize_src_quad(proxyParams, &quad->fLocal);
    SkRect normalizedDomain = normalize_domain(proxyParams, domain);

    // Only clip when there's anti-aliasing. When non-aa, the GPU clips just fine and there's
    // no inset/outset math that requires w > 0.
    DrawQuad extra;
    int quadCount = quad->fEdgeFlags != GrQuadAAFlags::kNone ? GrQuadUtils::ClipToW0(quad, &extra)
                                                             : 1;
    if (quadCount == 0) {
        // No need to actually append anything
        return;
    }
    SkASSERT(quadCount == 1 || quadCount == 2);

    Metadata metadata{std::move(textureXform), proxyView.swizzle(), filter, Domain(domain != nullptr), fSaturate, aaType};
    // By storing the incoming quad count here, it is automatically compared against resource
    // limits and updated properly if we reuse fOp or make a new one based solely on this metadata.
    metadata.fTotalQuadCount = quadCount;
    // However, when we determine which view count pair to associate with the quads, we may need
    // to initialize the proxy
    int viewIndex = -1;
    bool initializeProxy = true;

    TextureOp* op = nullptr;
    GrRecordingContext::Arenas arenas = ctx->priv().arenas();
    if (fOp) {
        // Attempt to append to the existing op
        op = static_cast<TextureOp*>(fOp.get());
        SkASSERT(op->fMetadata.fProxyCount > 0);
        bool combined = false;
        if (op->fMetadata.isCompatible(metadata, /* finalized */ false)) {
            // All non-modifiable state is compatible, so we can append if there's room to grow
            // the op in the memory pool. If the incoming proxy is the same as the last proxy,
            // we can skip the op growth entirely.
            GrSurfaceProxy* lastProxy = op->fViewCountPairs[op->fMetadata.fProxyCount - 1].fProxy.get();

            if (lastProxy == proxyView.proxy()) {
                // Will share the existing view count pair, just add more geometric data
                viewIndex = op->fMetadata.fProxyCount - 1;
                initializeProxy = false;
                combined = true;
                // SkDebugf("same as last proxy, index = %d, proxy count = %d, ptr null? %d\n",
                    // viewIndex, op->fMetadata.fProxyCount, lastProxy == nullptr);
            } else if (GrTextureProxy::ProxiesAreCompatibleAsDynamicState(lastProxy, proxyView.proxy()) &&
                       ctx->priv().caps()->dynamicStateArrayGeometryProcessorTextureSupport() &&
                       op->grow(&arenas, 1)) {
                // Will append a new view count pair to the existing op
                viewIndex = op->fMetadata.fProxyCount;
                combined = true;
                // SkDebugf("growing existing op, index = %d, proxy count = %d, ptr null? %d\n",
                    // viewIndex, op->fMetadata.fProxyCount, lastProxy == nullptr);
            }

        }

        if (!combined) {
            // Incompatible state or unable to make room for appending, so finalize the old one and
            // start anew (don't "close" the op however, since we want to keep the clip from when
            // this was opened).
            this->submit(rtc, /* close */ false);
            // SkDebugf("submitted old op, now considering op as null? %d\n", fOp == nullptr);
        } else {
            // Include the original ops' bounds in deviceBounds and update metadata of the op
            deviceBounds.joinPossiblyEmptyRect(op->bounds());
            op->fMetadata.combine(metadata, /* finalized */ false);
            // SkDebugf("combining into existing op, index: %d, current proxy count: %d\n",
                // viewIndex, op->fMetadata.fProxyCount);
        }
    }

    if (!fOp) {
        // Must allocate a new op, in which case we know the quads will be assigned to the first
        // view count pair.
        fOp = TextureOp::Make(&arenas, std::move(metadata), fExpected,
                              quad->fDevice.hasPerspective());
        op = static_cast<TextureOp*>(fOp.get());
        viewIndex = 0;
        // SkDebugf("allocating new op, base proxy count: %d\n", op->fMetadata.fProxyCount);
    }
    SkASSERT(op);
    // SkDebugf("viewIndex: %d, reserved count: %d, expected: %d\n",
        // viewIndex, op->fReservedProxyCount, fExpected);
    SkASSERT(viewIndex >= 0 && viewIndex < op->fReservedProxyCount);

    // Store the proxy
    if (initializeProxy) {
        SkASSERT(op->fMetadata.fProxyCount < op->fReservedProxyCount);
        if (viewIndex == 0) {
            // Do not placement new since this was initialized when the op was created
            op->fViewCountPairs[0].fProxy = proxyView.detachProxy();
            op->fViewCountPairs[0].fQuadCnt = 0;
        } else {
            // We must placement new the view count pairs here so that the sk_sps in the
            // GrSurfaceProxyView get initialized properly
            new (&op->fViewCountPairs[viewIndex]) TextureOp::ViewCountPair({proxyView.detachProxy(), 0});
        }
        op->fMetadata.fProxyCount++;
    }

    // Store the geometry
    op->fViewCountPairs[viewIndex].fQuadCnt += quadCount;
    op->fQuads.append(arenas.quadAllocator(), quad->fDevice,
                       {color, normalizedDomain, quad->fEdgeFlags}, &quad->fLocal);
    if (quadCount > 1) {
        op->fQuads.append(arenas.quadAllocator(), extra.fDevice,
                           {color, normalizedDomain, extra.fEdgeFlags}, &extra.fLocal);
    }
    op->setBounds(deviceBounds, TextureOp::HasAABloat(op->fMetadata.aaType() == GrAAType::kCoverage), TextureOp::IsHairline::kNo);

    // SkDebugf("Append finished, total quad count: %d, total proxy count: %d\n",
    //     op->fMetadata.fTotalQuadCount, op->fMetadata.fProxyCount);
}

void GrDeferredTextureOp::submit(GrRenderTargetContext* rtc, bool close) {
    if (!fOp) {
        // SkDebugf("submit is no-op\n");
        // Nothing was ever appended after opening (e.g. they were clipped by the w plane)
        return;
    }
    // SkDebugf("submitting op, closing? %d\n", close);

    TextureOp* op = static_cast<TextureOp*>(fOp.get());
    SkASSERT(!GrMeshDrawOp::CombinedQuadCountWillOverflow(op->fMetadata.aaType(), false,
                                            op->fMetadata.fTotalQuadCount));
    SkASSERT(op->fMetadata.fProxyCount > 0 && op->fMetadata.fTotalQuadCount >= op->fMetadata.fProxyCount);

    // Calculate final bounds of the op
    // SkDebugf("submitting op, so calculating op bounds\n");
    SkRect clippedBounds = fOp->bounds();
    // SkDebugf("got the bounds\n");
    if (fOp->hasAABloat()) {
        clippedBounds.outset(0.5f, 0.5f);
    }
    SkAssertResult(clippedBounds.intersect(fClipBounds));

    // Return any excess space reserved at the end of the op
    GrRecordingContext::Arenas arenas = rtc->surfPriv().getContext()->priv().arenas();
    op->trim(&arenas);

    if (close) {
        fExpected = 0;
        rtc->addClippedDrawOp(std::move(fClip), std::move(fOp), clippedBounds);
    } else {
        // Adjust the estimate by the number of view count pairs the submitted op handled
        fExpected = std::max(1, fExpected - op->fMetadata.fProxyCount);

        // Don't move the applied clip, so that it remains valid for subsequent appended draws
        GrAppliedClip copyOfClip = fClip.clone();
        rtc->addClippedDrawOp(std::move(copyOfClip), std::move(fOp), clippedBounds);
    }
    SkASSERT(!fOp);
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
    auto saturate = random->nextBool() ? GrDeferredTextureOp::Saturate::kYes : GrDeferredTextureOp::Saturate::kNo;
    GrSurfaceProxyView proxyView(
            std::move(proxy), origin,
            context->priv().caps()->getReadSwizzle(format, GrColorType::kRGBA_8888));
    auto alphaType = static_cast<SkAlphaType>(
            random->nextRangeU(kUnknown_SkAlphaType + 1, kLastEnum_SkAlphaType));

    DrawQuad quad = {GrQuad::MakeFromRect(rect, viewMatrix), GrQuad(srcRect), aaFlags};
    // return GrTextureOp::Make(context, std::move(proxyView), alphaType, std::move(texXform), filter,
    //                          color, saturate, SkBlendMode::kSrcOver, aaType,
    //                          &quad, useDomain ? &srcRect : nullptr);
    return nullptr;
}

#endif
