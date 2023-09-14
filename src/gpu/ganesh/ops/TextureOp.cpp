/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <new>

#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkMathPriv.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkRectPriv.h"
#include "src/gpu/ganesh/GrAppliedClip.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDrawOpTest.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrMemoryPool.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrOpsTypes.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrResourceProviderPriv.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/GrXferProcessor.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/effects/GrBlendFragmentProcessor.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/gpu/ganesh/geometry/GrQuad.h"
#include "src/gpu/ganesh/geometry/GrQuadBuffer.h"
#include "src/gpu/ganesh/geometry/GrQuadUtils.h"
#include "src/gpu/ganesh/geometry/GrRect.h"
#include "src/gpu/ganesh/glsl/GrGLSLVarying.h"
#include "src/gpu/ganesh/ops/FillRectOp.h"
#include "src/gpu/ganesh/ops/GrMeshDrawOp.h"
#include "src/gpu/ganesh/ops/GrSimpleMeshDrawOpHelper.h"
#include "src/gpu/ganesh/ops/QuadPerEdgeAA.h"
#include "src/gpu/ganesh/ops/TextureOp.h"

#if defined(GR_TEST_UTILS)
#include "src/gpu/ganesh/GrProxyProvider.h"
#endif

using namespace skgpu::ganesh;

namespace {

using Subset = skgpu::ganesh::QuadPerEdgeAA::Subset;
using VertexSpec = skgpu::ganesh::QuadPerEdgeAA::VertexSpec;
using ColorType = skgpu::ganesh::QuadPerEdgeAA::ColorType;

// Extracts lengths of vertical and horizontal edges of axis-aligned quad. "width" is the edge
// between v0 and v2 (or v1 and v3), "height" is the edge between v0 and v1 (or v2 and v3).
SkSize axis_aligned_quad_size(const GrQuad& quad) {
    SkASSERT(quad.quadType() == GrQuad::Type::kAxisAligned);
    // Simplification of regular edge length equation, since it's axis aligned and can avoid sqrt
    float dw = sk_float_abs(quad.x(2) - quad.x(0)) + sk_float_abs(quad.y(2) - quad.y(0));
    float dh = sk_float_abs(quad.x(1) - quad.x(0)) + sk_float_abs(quad.y(1) - quad.y(0));
    return {dw, dh};
}

std::tuple<bool /* filter */,
           bool /* mipmap */>
filter_and_mm_have_effect(const GrQuad& srcQuad, const GrQuad& dstQuad) {
    // If not axis-aligned in src or dst, then always say it has an effect
    if (srcQuad.quadType() != GrQuad::Type::kAxisAligned ||
        dstQuad.quadType() != GrQuad::Type::kAxisAligned) {
        return {true, true};
    }

    SkRect srcRect;
    SkRect dstRect;
    if (srcQuad.asRect(&srcRect) && dstQuad.asRect(&dstRect)) {
        // Disable filtering when there is no scaling (width and height are the same), and the
        // top-left corners have the same fraction (so src and dst snap to the pixel grid
        // identically).
        SkASSERT(srcRect.isSorted());
        bool filter = srcRect.width() != dstRect.width() || srcRect.height() != dstRect.height() ||
                      SkScalarFraction(srcRect.fLeft) != SkScalarFraction(dstRect.fLeft) ||
                      SkScalarFraction(srcRect.fTop)  != SkScalarFraction(dstRect.fTop);
        bool mm = srcRect.width() > dstRect.width() || srcRect.height() > dstRect.height();
        return {filter, mm};
    }
    // Extract edge lengths
    SkSize srcSize = axis_aligned_quad_size(srcQuad);
    SkSize dstSize = axis_aligned_quad_size(dstQuad);
    // Although the quads are axis-aligned, the local coordinate system is transformed such
    // that fractionally-aligned sample centers will not align with the device coordinate system
    // So disable filtering when edges are the same length and both srcQuad and dstQuad
    // 0th vertex is integer aligned.
    bool filter = srcSize != dstSize ||
                  !SkScalarIsInt(srcQuad.x(0)) ||
                  !SkScalarIsInt(srcQuad.y(0)) ||
                  !SkScalarIsInt(dstQuad.x(0)) ||
                  !SkScalarIsInt(dstQuad.y(0));
    bool mm = srcSize.fWidth > dstSize.fWidth || srcSize.fHeight > dstSize.fHeight;
    return {filter, mm};
}

// Describes function for normalizing src coords: [x * iw, y * ih + yOffset] can represent
// regular and rectangular textures, w/ or w/o origin correction.
struct NormalizationParams {
    float fIW; // 1 / width of texture, or 1.0 for texture rectangles
    float fInvH; // 1 / height of texture, or 1.0 for tex rects, X -1 if bottom-left origin
    float fYOffset; // 0 for top-left origin, height of [normalized] tex if bottom-left
};
NormalizationParams proxy_normalization_params(const GrSurfaceProxy* proxy,
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

// Normalize the subset. If 'subsetRect' is null, it is assumed no subset constraint is desired,
// so a sufficiently large rect is returned even if the quad ends up batched with an op that uses
// subsets overall. When there is a subset it will be inset based on the filter mode. Normalization
// and y-flipping are applied as indicated by NormalizationParams.
SkRect normalize_and_inset_subset(GrSamplerState::Filter filter,
                                  const NormalizationParams& params,
                                  const SkRect* subsetRect) {
    static constexpr SkRect kLargeRect = {-100000, -100000, 1000000, 1000000};
    if (!subsetRect) {
        // Either the quad has no subset constraint and is batched with a subset constrained op
        // (in which case we want a subset that doesn't restrict normalized tex coords), or the
        // entire op doesn't use the subset, in which case the returned value is ignored.
        return kLargeRect;
    }

    auto ltrb = skvx::Vec<4, float>::Load(subsetRect);
    auto flipHi = skvx::Vec<4, float>({1.f, 1.f, -1.f, -1.f});
    if (filter == GrSamplerState::Filter::kNearest) {
        // Make sure our insetting puts us at pixel centers.
        ltrb = skvx::floor(ltrb*flipHi)*flipHi;
    }
    // Inset with pin to the rect center.
    ltrb += skvx::Vec<4, float>({ GrTextureEffect::kLinearInset,  GrTextureEffect::kLinearInset,
                                 -GrTextureEffect::kLinearInset, -GrTextureEffect::kLinearInset});
    auto mid = (skvx::shuffle<2, 3, 0, 1>(ltrb) + ltrb)*0.5f;
    ltrb = skvx::min(ltrb*flipHi, mid*flipHi)*flipHi;

    // Normalize and offset
    ltrb = ltrb * skvx::Vec<4, float>{params.fIW, params.fInvH, params.fIW, params.fInvH} +
               skvx::Vec<4, float>{0.f, params.fYOffset, 0.f, params.fYOffset};
    if (params.fInvH < 0.f) {
        // Flip top and bottom to keep the rect sorted when loaded back to SkRect.
        ltrb = skvx::shuffle<0, 3, 2, 1>(ltrb);
    }

    SkRect out;
    ltrb.store(&out);
    return out;
}

// Normalizes logical src coords and corrects for origin
void normalize_src_quad(const NormalizationParams& params,
                        GrQuad* srcQuad) {
    // The src quad should not have any perspective
    SkASSERT(!srcQuad->hasPerspective());
    skvx::Vec<4, float> xs = srcQuad->x4f() * params.fIW;
    skvx::Vec<4, float> ys = srcQuad->y4f() * params.fInvH + params.fYOffset;
    xs.store(srcQuad->xs());
    ys.store(srcQuad->ys());
}

// Count the number of proxy runs in the entry set. This usually is already computed by
// SkGpuDevice, but when the BatchLengthLimiter chops the set up it must determine a new proxy count
// for each split.
int proxy_run_count(const GrTextureSetEntry set[], int count) {
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

bool safe_to_ignore_subset_rect(GrAAType aaType, GrSamplerState::Filter filter,
                                const DrawQuad& quad, const SkRect& subsetRect) {
    // If both the device and local quad are both axis-aligned, and filtering is off, the local quad
    // can push all the way up to the edges of the the subset rect and the sampler shouldn't
    // overshoot. Unfortunately, antialiasing adds enough jitter that we can only rely on this in
    // the non-antialiased case.
    SkRect localBounds = quad.fLocal.bounds();
    if (aaType == GrAAType::kNone &&
        filter == GrSamplerState::Filter::kNearest &&
        quad.fDevice.quadType() == GrQuad::Type::kAxisAligned &&
        quad.fLocal.quadType() == GrQuad::Type::kAxisAligned &&
        subsetRect.contains(localBounds)) {

        return true;
    }

    // If the local quad is inset by at least 0.5 pixels into the subset rect's bounds, the
    // sampler shouldn't overshoot, even when antialiasing and filtering is taken into account.
    if (subsetRect.makeInset(GrTextureEffect::kLinearInset,
                             GrTextureEffect::kLinearInset)
                  .contains(localBounds)) {
        return true;
    }

    // The subset rect cannot be ignored safely.
    return false;
}

/**
 * Op that implements TextureOp::Make. It draws textured quads. Each quad can modulate against a
 * the texture by color. The blend with the destination is always src-over. The edges are non-AA.
 */
class TextureOpImpl final : public GrMeshDrawOp {
public:
    using Saturate = TextureOp::Saturate;

    static GrOp::Owner Make(GrRecordingContext* context,
                            GrSurfaceProxyView proxyView,
                            sk_sp<GrColorSpaceXform> textureXform,
                            GrSamplerState::Filter filter,
                            GrSamplerState::MipmapMode mm,
                            const SkPMColor4f& color,
                            Saturate saturate,
                            GrAAType aaType,
                            DrawQuad* quad,
                            const SkRect* subset) {

        return GrOp::Make<TextureOpImpl>(context, std::move(proxyView), std::move(textureXform),
                                         filter, mm, color, saturate, aaType, quad, subset);
    }

    static GrOp::Owner Make(GrRecordingContext* context,
                            GrTextureSetEntry set[],
                            int cnt,
                            int proxyRunCnt,
                            GrSamplerState::Filter filter,
                            GrSamplerState::MipmapMode mm,
                            Saturate saturate,
                            GrAAType aaType,
                            SkCanvas::SrcRectConstraint constraint,
                            const SkMatrix& viewMatrix,
                            sk_sp<GrColorSpaceXform> textureColorSpaceXform) {
        // Allocate size based on proxyRunCnt, since that determines number of ViewCountPairs.
        SkASSERT(proxyRunCnt <= cnt);
        return GrOp::MakeWithExtraMemory<TextureOpImpl>(
                context, sizeof(ViewCountPair) * (proxyRunCnt - 1),
                set, cnt, proxyRunCnt, filter, mm, saturate, aaType, constraint,
                viewMatrix, std::move(textureColorSpaceXform));
    }

    ~TextureOpImpl() override {
        for (unsigned p = 1; p < fMetadata.fProxyCount; ++p) {
            fViewCountPairs[p].~ViewCountPair();
        }
    }

    const char* name() const override { return "TextureOp"; }

    void visitProxies(const GrVisitProxyFunc& func) const override {
        bool mipped = (fMetadata.mipmapMode() != GrSamplerState::MipmapMode::kNone);
        for (unsigned p = 0; p <  fMetadata.fProxyCount; ++p) {
            func(fViewCountPairs[p].fProxy.get(), skgpu::Mipmapped(mipped));
        }
        if (fDesc && fDesc->fProgramInfo) {
            fDesc->fProgramInfo->visitFPProxies(func);
        }
    }

#ifdef SK_DEBUG
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

    GrProcessorSet::Analysis finalize(const GrCaps& caps, const GrAppliedClip*,
                                      GrClampType clampType) override {
        SkASSERT(fMetadata.colorType() == ColorType::kNone);
        auto iter = fQuads.metadata();
        while(iter.next()) {
            auto colorType = skgpu::ganesh::QuadPerEdgeAA::MinColorType(iter->fColor);
            colorType = std::max(static_cast<ColorType>(fMetadata.fColorType),
                                 colorType);
            if (caps.reducedShaderMode()) {
                colorType = std::max(colorType, ColorType::kByte);
            }
            fMetadata.fColorType = static_cast<uint16_t>(colorType);
        }
        return GrProcessorSet::EmptySetAnalysis();
    }

    FixedFunctionFlags fixedFunctionFlags() const override {
        return fMetadata.aaType() == GrAAType::kMSAA ? FixedFunctionFlags::kUsesHWAA
                                                     : FixedFunctionFlags::kNone;
    }

    DEFINE_OP_CLASS_ID

private:
    friend class ::GrOp;

    struct ColorSubsetAndAA {
        ColorSubsetAndAA(const SkPMColor4f& color, const SkRect& subsetRect, GrQuadAAFlags aaFlags)
                : fColor(color)
                , fSubsetRect(subsetRect)
                , fAAFlags(static_cast<uint16_t>(aaFlags)) {
            SkASSERT(fAAFlags == static_cast<uint16_t>(aaFlags));
        }

        SkPMColor4f fColor;
        // If the op doesn't use subsets, this is ignored. If the op uses subsets and the specific
        // entry does not, this rect will equal kLargeRect, so it automatically has no effect.
        SkRect fSubsetRect;
        unsigned fAAFlags : 4;

        GrQuadAAFlags aaFlags() const { return static_cast<GrQuadAAFlags>(fAAFlags); }
    };

    struct ViewCountPair {
        // Normally this would be a GrSurfaceProxyView, but TextureOp applies the GrOrigin right
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
        Metadata(const skgpu::Swizzle& swizzle,
                 GrSamplerState::Filter filter,
                 GrSamplerState::MipmapMode mm,
                 Subset subset,
                 Saturate saturate)
            : fSwizzle(swizzle)
            , fProxyCount(1)
            , fTotalQuadCount(1)
            , fFilter(static_cast<uint16_t>(filter))
            , fMipmapMode(static_cast<uint16_t>(mm))
            , fAAType(static_cast<uint16_t>(GrAAType::kNone))
            , fColorType(static_cast<uint16_t>(ColorType::kNone))
            , fSubset(static_cast<uint16_t>(subset))
            , fSaturate(static_cast<uint16_t>(saturate)) {}

        skgpu::Swizzle fSwizzle; // sizeof(skgpu::Swizzle) == uint16_t
        uint16_t  fProxyCount;
        // This will be >= fProxyCount, since a proxy may be drawn multiple times
        uint16_t  fTotalQuadCount;

        // These must be based on uint16_t to help MSVC's pack bitfields optimally
        uint16_t  fFilter     : 2; // GrSamplerState::Filter
        uint16_t  fMipmapMode : 2; // GrSamplerState::MipmapMode
        uint16_t  fAAType     : 2; // GrAAType
        uint16_t  fColorType  : 2; // GrQuadPerEdgeAA::ColorType
        uint16_t  fSubset     : 1; // bool
        uint16_t  fSaturate   : 1; // bool
        uint16_t  fUnused     : 6; // # of bits left before Metadata exceeds 8 bytes

        GrSamplerState::Filter filter() const {
            return static_cast<GrSamplerState::Filter>(fFilter);
        }
        GrSamplerState::MipmapMode mipmapMode() const {
            return static_cast<GrSamplerState::MipmapMode>(fMipmapMode);
        }
        GrAAType aaType() const { return static_cast<GrAAType>(fAAType); }
        ColorType colorType() const { return static_cast<ColorType>(fColorType); }
        Subset subset() const { return static_cast<Subset>(fSubset); }
        Saturate saturate() const { return static_cast<Saturate>(fSaturate); }

        static_assert(GrSamplerState::kFilterCount <= 4);
        static_assert(kGrAATypeCount <= 4);
        static_assert(skgpu::ganesh::QuadPerEdgeAA::kColorTypeCount <= 4);
    };
    static_assert(sizeof(Metadata) == 8);

    // This descriptor is used to store the draw info we decide on during on(Pre)PrepareDraws. We
    // store the data in a separate struct in order to minimize the size of the TextureOp.
    // Historically, increasing the TextureOp's size has caused surprising perf regressions, but we
    // may want to re-evaluate whether this is still necessary.
    //
    // In the onPrePrepareDraws case it is allocated in the creation-time opData arena, and
    // allocatePrePreparedVertices is also called.
    //
    // In the onPrepareDraws case this descriptor is allocated in the flush-time arena (i.e., as
    // part of the flushState).
    struct Desc {
        VertexSpec fVertexSpec;
        int fNumProxies = 0;
        int fNumTotalQuads = 0;

        // This member variable is only used by 'onPrePrepareDraws'.
        char* fPrePreparedVertices = nullptr;

        GrProgramInfo* fProgramInfo = nullptr;

        sk_sp<const GrBuffer> fIndexBuffer;
        sk_sp<const GrBuffer> fVertexBuffer;
        int fBaseVertex;

        // How big should 'fVertices' be to hold all the vertex data?
        size_t totalSizeInBytes() const {
            return this->totalNumVertices() * fVertexSpec.vertexSize();
        }

        int totalNumVertices() const {
            return fNumTotalQuads * fVertexSpec.verticesPerQuad();
        }

        void allocatePrePreparedVertices(SkArenaAlloc* arena) {
            fPrePreparedVertices = arena->makeArrayDefault<char>(this->totalSizeInBytes());
        }
    };
    // If subsetRect is not null it will be used to apply a strict src rect-style constraint.
    TextureOpImpl(GrSurfaceProxyView proxyView,
                  sk_sp<GrColorSpaceXform> textureColorSpaceXform,
                  GrSamplerState::Filter filter,
                  GrSamplerState::MipmapMode mm,
                  const SkPMColor4f& color,
                  Saturate saturate,
                  GrAAType aaType,
                  DrawQuad* quad,
                  const SkRect* subsetRect)
            : INHERITED(ClassID())
            , fQuads(1, true /* includes locals */)
            , fTextureColorSpaceXform(std::move(textureColorSpaceXform))
            , fDesc(nullptr)
            , fMetadata(proxyView.swizzle(), filter, mm, Subset(!!subsetRect), saturate) {
        // Clean up disparities between the overall aa type and edge configuration and apply
        // optimizations based on the rect and matrix when appropriate
        GrQuadUtils::ResolveAAType(aaType, quad->fEdgeFlags, quad->fDevice,
                                   &aaType, &quad->fEdgeFlags);
        fMetadata.fAAType = static_cast<uint16_t>(aaType);

        // We expect our caller to have already caught this optimization.
        SkASSERT(!subsetRect ||
                 !subsetRect->contains(proxyView.proxy()->backingStoreBoundsRect()));

        // We may have had a strict constraint with nearest filter solely due to possible AA bloat.
        // Try to identify cases where the subsetting isn't actually necessary, and skip it.
        if (subsetRect) {
            if (safe_to_ignore_subset_rect(aaType, filter, *quad, *subsetRect)) {
                subsetRect = nullptr;
                fMetadata.fSubset = static_cast<uint16_t>(Subset::kNo);
            }
        }

        // Normalize src coordinates and the subset (if set)
        NormalizationParams params = proxy_normalization_params(proxyView.proxy(),
                                                                proxyView.origin());
        normalize_src_quad(params, &quad->fLocal);
        SkRect subset = normalize_and_inset_subset(filter, params, subsetRect);

        // Set bounds before clipping so we don't have to worry about unioning the bounds of
        // the two potential quads (GrQuad::bounds() is perspective-safe).
        bool hairline = GrQuadUtils::WillUseHairline(quad->fDevice, aaType, quad->fEdgeFlags);
        this->setBounds(quad->fDevice.bounds(), HasAABloat(aaType == GrAAType::kCoverage),
                        hairline ? IsHairline::kYes : IsHairline::kNo);
        int quadCount = this->appendQuad(quad, color, subset);
        fViewCountPairs[0] = {proxyView.detachProxy(), quadCount};
    }

    TextureOpImpl(GrTextureSetEntry set[],
                  int cnt,
                  int proxyRunCnt,
                  const GrSamplerState::Filter filter,
                  const GrSamplerState::MipmapMode mm,
                  const Saturate saturate,
                  const GrAAType aaType,
                  const SkCanvas::SrcRectConstraint constraint,
                  const SkMatrix& viewMatrix,
                  sk_sp<GrColorSpaceXform> textureColorSpaceXform)
            : INHERITED(ClassID())
            , fQuads(cnt, true /* includes locals */)
            , fTextureColorSpaceXform(std::move(textureColorSpaceXform))
            , fDesc(nullptr)
            , fMetadata(set[0].fProxyView.swizzle(),
                        GrSamplerState::Filter::kNearest,
                        GrSamplerState::MipmapMode::kNone,
                        Subset::kNo,
                        saturate) {
        // Update counts to reflect the batch op
        fMetadata.fProxyCount = SkToUInt(proxyRunCnt);
        fMetadata.fTotalQuadCount = SkToUInt(cnt);

        SkRect bounds = SkRectPriv::MakeLargestInverted();

        GrAAType netAAType = GrAAType::kNone; // aa type maximally compatible with all dst rects
        Subset netSubset = Subset::kNo;
        GrSamplerState::Filter netFilter = GrSamplerState::Filter::kNearest;
        GrSamplerState::MipmapMode netMM = GrSamplerState::MipmapMode::kNone;
        bool hasSubpixel = false;

        const GrSurfaceProxy* curProxy = nullptr;

        // 'q' is the index in 'set' and fQuadBuffer; 'p' is the index in fViewCountPairs and only
        // increases when set[q]'s proxy changes.
        int p = 0;
        for (int q = 0; q < cnt; ++q) {
            SkASSERT(mm == GrSamplerState::MipmapMode::kNone ||
                     (set[0].fProxyView.proxy()->asTextureProxy()->mipmapped() ==
                      skgpu::Mipmapped::kYes));
            if (q == 0) {
                // We do not placement new the first ViewCountPair since that one is allocated and
                // initialized as part of the TextureOp creation.
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

            // This may be reduced per-quad from the requested aggregate filtering level, and used
            // to determine if the subset is needed for the entry as well.
            GrSamplerState::Filter filterForQuad = filter;
            if (netFilter != filter || netMM != mm) {
                // The only way netFilter != filter is if linear is requested and we haven't yet
                // found a quad that requires linear (so net is still nearest). Similar for mip
                // mapping.
                SkASSERT(filter == netFilter ||
                         (netFilter == GrSamplerState::Filter::kNearest && filter > netFilter));
                SkASSERT(mm == netMM ||
                         (netMM == GrSamplerState::MipmapMode::kNone && mm > netMM));
                auto [mustFilter, mustMM] = filter_and_mm_have_effect(quad.fLocal, quad.fDevice);
                if (filter != GrSamplerState::Filter::kNearest) {
                    if (mustFilter) {
                        netFilter = filter; // upgrade batch to higher filter level
                    } else {
                        filterForQuad = GrSamplerState::Filter::kNearest; // downgrade entry
                    }
                }
                if (mustMM && mm != GrSamplerState::MipmapMode::kNone) {
                    netMM = mm;
                }
            }

            // Determine the AA type for the quad, then merge with net AA type
            GrAAType aaForQuad;
            GrQuadUtils::ResolveAAType(aaType, set[q].fAAFlags, quad.fDevice,
                                       &aaForQuad, &quad.fEdgeFlags);
            // Update overall bounds of the op as the union of all quads
            bounds.joinPossiblyEmptyRect(quad.fDevice.bounds());
            hasSubpixel |= GrQuadUtils::WillUseHairline(quad.fDevice, aaForQuad, quad.fEdgeFlags);

            // Resolve sets aaForQuad to aaType or None, there is never a change between aa methods
            SkASSERT(aaForQuad == GrAAType::kNone || aaForQuad == aaType);
            if (netAAType == GrAAType::kNone && aaForQuad != GrAAType::kNone) {
                netAAType = aaType;
            }

            // Calculate metadata for the entry
            const SkRect* subsetForQuad = nullptr;
            if (constraint == SkCanvas::kStrict_SrcRectConstraint) {
                // Check (briefly) if the subset rect is actually needed for this set entry.
                SkRect* subsetRect = &set[q].fSrcRect;
                if (!subsetRect->contains(curProxy->backingStoreBoundsRect())) {
                    if (!safe_to_ignore_subset_rect(aaForQuad, filterForQuad, quad, *subsetRect)) {
                        netSubset = Subset::kYes;
                        subsetForQuad = subsetRect;
                    }
                }
            }

            // Normalize the src quads and apply origin
            NormalizationParams proxyParams = proxy_normalization_params(
                    curProxy, set[q].fProxyView.origin());
            normalize_src_quad(proxyParams, &quad.fLocal);

            // This subset may represent a no-op, otherwise it will have the origin and dimensions
            // of the texture applied to it.
            SkRect subset = normalize_and_inset_subset(filter, proxyParams, subsetForQuad);

            // Always append a quad (or 2 if perspective clipped), it just may refer back to a prior
            // ViewCountPair (this frequently happens when Chrome draws 9-patches).
            fViewCountPairs[p].fQuadCnt += this->appendQuad(&quad, set[q].fColor, subset);
        }
        // The # of proxy switches should match what was provided (+1 because we incremented p
        // when a new proxy was encountered).
        SkASSERT((p + 1) == fMetadata.fProxyCount);
        SkASSERT(fQuads.count() == fMetadata.fTotalQuadCount);

        fMetadata.fAAType = static_cast<uint16_t>(netAAType);
        fMetadata.fFilter = static_cast<uint16_t>(netFilter);
        fMetadata.fSubset = static_cast<uint16_t>(netSubset);

        this->setBounds(bounds, HasAABloat(netAAType == GrAAType::kCoverage),
                        hasSubpixel ? IsHairline::kYes : IsHairline::kNo);
    }

    int appendQuad(DrawQuad* quad, const SkPMColor4f& color, const SkRect& subset) {
        DrawQuad extra;
        // Always clip to W0 to stay consistent with GrQuad::bounds
        int quadCount = GrQuadUtils::ClipToW0(quad, &extra);
        if (quadCount == 0) {
            // We can't discard the op at this point, but disable AA flags so it won't go through
            // inset/outset processing
            quad->fEdgeFlags = GrQuadAAFlags::kNone;
            quadCount = 1;
        }
        fQuads.append(quad->fDevice, {color, subset, quad->fEdgeFlags},  &quad->fLocal);
        if (quadCount > 1) {
            fQuads.append(extra.fDevice, {color, subset, extra.fEdgeFlags}, &extra.fLocal);
            fMetadata.fTotalQuadCount++;
        }
        return quadCount;
    }

    GrProgramInfo* programInfo() override {
        // Although this Op implements its own onPrePrepareDraws it calls GrMeshDrawOps' version so
        // this entry point will be called.
        return (fDesc) ? fDesc->fProgramInfo : nullptr;
    }

    void onCreateProgramInfo(const GrCaps* caps,
                             SkArenaAlloc* arena,
                             const GrSurfaceProxyView& writeView,
                             bool usesMSAASurface,
                             GrAppliedClip&& appliedClip,
                             const GrDstProxyView& dstProxyView,
                             GrXferBarrierFlags renderPassXferBarriers,
                             GrLoadOp colorLoadOp) override {
        SkASSERT(fDesc);

        GrGeometryProcessor* gp;

        {
            const GrBackendFormat& backendFormat =
                    fViewCountPairs[0].fProxy->backendFormat();

            GrSamplerState samplerState = GrSamplerState(GrSamplerState::WrapMode::kClamp,
                                                         fMetadata.filter());

            gp = skgpu::ganesh::QuadPerEdgeAA::MakeTexturedProcessor(
                    arena,
                    fDesc->fVertexSpec,
                    *caps->shaderCaps(),
                    backendFormat,
                    samplerState,
                    fMetadata.fSwizzle,
                    std::move(fTextureColorSpaceXform),
                    fMetadata.saturate());

            SkASSERT(fDesc->fVertexSpec.vertexSize() == gp->vertexStride());
        }

        fDesc->fProgramInfo = GrSimpleMeshDrawOpHelper::CreateProgramInfo(
                caps, arena, writeView, usesMSAASurface, std::move(appliedClip), dstProxyView, gp,
                GrProcessorSet::MakeEmptySet(), fDesc->fVertexSpec.primitiveType(),
                renderPassXferBarriers, colorLoadOp, GrPipeline::InputFlags::kNone);
    }

    void onPrePrepareDraws(GrRecordingContext* context,
                           const GrSurfaceProxyView& writeView,
                           GrAppliedClip* clip,
                           const GrDstProxyView& dstProxyView,
                           GrXferBarrierFlags renderPassXferBarriers,
                           GrLoadOp colorLoadOp) override {
        TRACE_EVENT0("skia.gpu", TRACE_FUNC);

        SkDEBUGCODE(this->validate();)
        SkASSERT(!fDesc);

        SkArenaAlloc* arena = context->priv().recordTimeAllocator();

        fDesc = arena->make<Desc>();
        this->characterize(fDesc);
        fDesc->allocatePrePreparedVertices(arena);
        FillInVertices(*context->priv().caps(), this, fDesc, fDesc->fPrePreparedVertices);

        // This will call onCreateProgramInfo and register the created program with the DDL.
        this->INHERITED::onPrePrepareDraws(context, writeView, clip, dstProxyView,
                                           renderPassXferBarriers, colorLoadOp);
    }

    static void FillInVertices(const GrCaps& caps,
                               TextureOpImpl* texOp,
                               Desc* desc,
                               char* vertexData) {
        SkASSERT(vertexData);

        SkDEBUGCODE(int totQuadsSeen = 0;) SkDEBUGCODE(int totVerticesSeen = 0;)
                SkDEBUGCODE(const size_t vertexSize = desc->fVertexSpec.vertexSize();)
                        SkDEBUGCODE(auto startMark{vertexData};)

                                skgpu::ganesh::QuadPerEdgeAA::Tessellator tessellator(
                                        desc->fVertexSpec, vertexData);
        for (const auto& op : ChainRange<TextureOpImpl>(texOp)) {
            auto iter = op.fQuads.iterator();
            for (unsigned p = 0; p < op.fMetadata.fProxyCount; ++p) {
                const int quadCnt = op.fViewCountPairs[p].fQuadCnt;
                SkDEBUGCODE(int meshVertexCnt = quadCnt * desc->fVertexSpec.verticesPerQuad());

                for (int i = 0; i < quadCnt && iter.next(); ++i) {
                    SkASSERT(iter.isLocalValid());
                    const ColorSubsetAndAA& info = iter.metadata();

                    tessellator.append(iter.deviceQuad(), iter.localQuad(), info.fColor,
                                       info.fSubsetRect, info.aaFlags());
                }

                SkASSERT((totVerticesSeen + meshVertexCnt) * vertexSize
                         == (size_t)(tessellator.vertexMark() - startMark));

                SkDEBUGCODE(totQuadsSeen += quadCnt;)
                SkDEBUGCODE(totVerticesSeen += meshVertexCnt);
                SkASSERT(totQuadsSeen * desc->fVertexSpec.verticesPerQuad() == totVerticesSeen);
            }

            // If quad counts per proxy were calculated correctly, the entire iterator
            // should have been consumed.
            SkASSERT(!iter.next());
        }

        SkASSERT(desc->totalSizeInBytes() == (size_t)(tessellator.vertexMark() - startMark));
        SkASSERT(totQuadsSeen == desc->fNumTotalQuads);
        SkASSERT(totVerticesSeen == desc->totalNumVertices());
    }

#ifdef SK_DEBUG
    static int validate_op(GrTextureType textureType,
                           GrAAType aaType,
                           skgpu::Swizzle swizzle,
                           const TextureOpImpl* op) {
        SkASSERT(op->fMetadata.fSwizzle == swizzle);

        int quadCount = 0;
        for (unsigned p = 0; p < op->fMetadata.fProxyCount; ++p) {
            auto* proxy = op->fViewCountPairs[p].fProxy->asTextureProxy();
            quadCount += op->fViewCountPairs[p].fQuadCnt;
            SkASSERT(proxy);
            SkASSERT(proxy->textureType() == textureType);
        }

        SkASSERT(aaType == op->fMetadata.aaType());
        return quadCount;
    }

    void validate() const override {
        // NOTE: Since this is debug-only code, we use the virtual asTextureProxy()
        auto textureType = fViewCountPairs[0].fProxy->asTextureProxy()->textureType();
        GrAAType aaType = fMetadata.aaType();
        skgpu::Swizzle swizzle = fMetadata.fSwizzle;

        int quadCount = validate_op(textureType, aaType, swizzle, this);

        for (const GrOp* tmp = this->prevInChain(); tmp; tmp = tmp->prevInChain()) {
            quadCount += validate_op(textureType, aaType, swizzle,
                                     static_cast<const TextureOpImpl*>(tmp));
        }

        for (const GrOp* tmp = this->nextInChain(); tmp; tmp = tmp->nextInChain()) {
            quadCount += validate_op(textureType, aaType, swizzle,
                                     static_cast<const TextureOpImpl*>(tmp));
        }

        SkASSERT(quadCount == this->numChainedQuads());
    }

#endif

#if defined(GR_TEST_UTILS)
    int numQuads() const final { return this->totNumQuads(); }
#endif

    void characterize(Desc* desc) const {
        SkDEBUGCODE(this->validate();)

        GrQuad::Type quadType = GrQuad::Type::kAxisAligned;
        ColorType colorType = ColorType::kNone;
        GrQuad::Type srcQuadType = GrQuad::Type::kAxisAligned;
        Subset subset = Subset::kNo;
        GrAAType overallAAType = fMetadata.aaType();

        desc->fNumProxies = 0;
        desc->fNumTotalQuads = 0;
        int maxQuadsPerMesh = 0;

        for (const auto& op : ChainRange<TextureOpImpl>(this)) {
            if (op.fQuads.deviceQuadType() > quadType) {
                quadType = op.fQuads.deviceQuadType();
            }
            if (op.fQuads.localQuadType() > srcQuadType) {
                srcQuadType = op.fQuads.localQuadType();
            }
            if (op.fMetadata.subset() == Subset::kYes) {
                subset = Subset::kYes;
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

        auto indexBufferOption =
                skgpu::ganesh::QuadPerEdgeAA::CalcIndexBufferOption(overallAAType, maxQuadsPerMesh);

        desc->fVertexSpec = VertexSpec(quadType, colorType, srcQuadType, /* hasLocal */ true,
                                       subset, overallAAType, /* alpha as coverage */ true,
                                       indexBufferOption);

        SkASSERT(desc->fNumTotalQuads <=
                 skgpu::ganesh::QuadPerEdgeAA::QuadLimit(indexBufferOption));
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
            numChainedQuads += ((const TextureOpImpl*)tmp)->totNumQuads();
        }

        for (const GrOp* tmp = this->nextInChain(); tmp; tmp = tmp->nextInChain()) {
            numChainedQuads += ((const TextureOpImpl*)tmp)->totNumQuads();
        }

        return numChainedQuads;
    }

    // onPrePrepareDraws may or may not have been called at this point
    void onPrepareDraws(GrMeshDrawTarget* target) override {
        TRACE_EVENT0("skia.gpu", TRACE_FUNC);

        SkDEBUGCODE(this->validate();)

        SkASSERT(!fDesc || fDesc->fPrePreparedVertices);

        if (!fDesc) {
            SkArenaAlloc* arena = target->allocator();
            fDesc = arena->make<Desc>();
            this->characterize(fDesc);
            SkASSERT(!fDesc->fPrePreparedVertices);
        }

        size_t vertexSize = fDesc->fVertexSpec.vertexSize();

        void* vdata = target->makeVertexSpace(vertexSize, fDesc->totalNumVertices(),
                                              &fDesc->fVertexBuffer, &fDesc->fBaseVertex);
        if (!vdata) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        if (fDesc->fVertexSpec.needsIndexBuffer()) {
            fDesc->fIndexBuffer = skgpu::ganesh::QuadPerEdgeAA::GetIndexBuffer(
                    target, fDesc->fVertexSpec.indexBufferOption());
            if (!fDesc->fIndexBuffer) {
                SkDebugf("Could not allocate indices\n");
                return;
            }
        }

        if (fDesc->fPrePreparedVertices) {
            memcpy(vdata, fDesc->fPrePreparedVertices, fDesc->totalSizeInBytes());
        } else {
            FillInVertices(target->caps(), this, fDesc, (char*) vdata);
        }
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        if (!fDesc->fVertexBuffer) {
            return;
        }

        if (fDesc->fVertexSpec.needsIndexBuffer() && !fDesc->fIndexBuffer) {
            return;
        }

        if (!fDesc->fProgramInfo) {
            this->createProgramInfo(flushState);
            SkASSERT(fDesc->fProgramInfo);
        }

        flushState->bindPipelineAndScissorClip(*fDesc->fProgramInfo, chainBounds);
        flushState->bindBuffers(std::move(fDesc->fIndexBuffer), nullptr,
                                std::move(fDesc->fVertexBuffer));

        int totQuadsSeen = 0;
        SkDEBUGCODE(int numDraws = 0;)
        for (const auto& op : ChainRange<TextureOpImpl>(this)) {
            for (unsigned p = 0; p < op.fMetadata.fProxyCount; ++p) {
                const int quadCnt = op.fViewCountPairs[p].fQuadCnt;
                SkASSERT(numDraws < fDesc->fNumProxies);
                flushState->bindTextures(fDesc->fProgramInfo->geomProc(),
                                         *op.fViewCountPairs[p].fProxy,
                                         fDesc->fProgramInfo->pipeline());
                skgpu::ganesh::QuadPerEdgeAA::IssueDraw(flushState->caps(),
                                                        flushState->opsRenderPass(),
                                                        fDesc->fVertexSpec,
                                                        totQuadsSeen,
                                                        quadCnt,
                                                        fDesc->totalNumVertices(),
                                                        fDesc->fBaseVertex);
                totQuadsSeen += quadCnt;
                SkDEBUGCODE(++numDraws;)
            }
        }

        SkASSERT(totQuadsSeen == fDesc->fNumTotalQuads);
        SkASSERT(numDraws == fDesc->fNumProxies);
    }

    void propagateCoverageAAThroughoutChain() {
        fMetadata.fAAType = static_cast<uint16_t>(GrAAType::kCoverage);

        for (GrOp* tmp = this->prevInChain(); tmp; tmp = tmp->prevInChain()) {
            auto tex = static_cast<TextureOpImpl*>(tmp);
            SkASSERT(tex->fMetadata.aaType() == GrAAType::kCoverage ||
                     tex->fMetadata.aaType() == GrAAType::kNone);
            tex->fMetadata.fAAType = static_cast<uint16_t>(GrAAType::kCoverage);
        }

        for (GrOp* tmp = this->nextInChain(); tmp; tmp = tmp->nextInChain()) {
            auto tex = static_cast<TextureOpImpl*>(tmp);
            SkASSERT(tex->fMetadata.aaType() == GrAAType::kCoverage ||
                     tex->fMetadata.aaType() == GrAAType::kNone);
            tex->fMetadata.fAAType = static_cast<uint16_t>(GrAAType::kCoverage);
        }
    }

    CombineResult onCombineIfPossible(GrOp* t, SkArenaAlloc*, const GrCaps& caps) override {
        TRACE_EVENT0("skia.gpu", TRACE_FUNC);
        auto that = t->cast<TextureOpImpl>();

        SkDEBUGCODE(this->validate();)
        SkDEBUGCODE(that->validate();)

        if (fDesc || that->fDesc) {
            // This should never happen (since only DDL recorded ops should be prePrepared)
            // but, in any case, we should never combine ops that that been prePrepared
            return CombineResult::kCannotCombine;
        }

        if (fMetadata.subset() != that->fMetadata.subset()) {
            // It is technically possible to combine operations across subset modes, but performance
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
        if (fMetadata.mipmapMode() != that->fMetadata.mipmapMode()) {
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
                caps.dynamicStateArrayGeometryProcessorTextureSupport() &&
                fMetadata.aaType() == that->fMetadata.aaType()) {
                // We only allow chaining when the aaTypes match bc otherwise the AA type
                // reported by the chain can be inconsistent. That is, since chaining doesn't
                // propagate revised AA information throughout the chain, the head of the chain
                // could have an AA setting of kNone while the chain as a whole could have a
                // setting of kCoverage. This inconsistency would then interfere with the validity
                // of the CombinedQuadCountWillOverflow calls.
                // This problem doesn't occur w/ merging bc we do propagate the AA information
                // (in propagateCoverageAAThroughoutChain) below.
                return CombineResult::kMayChain;
            }
            return CombineResult::kCannotCombine;
        }

        fMetadata.fSubset |= that->fMetadata.fSubset;
        fMetadata.fColorType = std::max(fMetadata.fColorType, that->fMetadata.fColorType);

        // Concatenate quad lists together
        fQuads.concat(that->fQuads);
        fViewCountPairs[0].fQuadCnt += that->fQuads.count();
        fMetadata.fTotalQuadCount += that->fQuads.count();

        if (upgradeToCoverageAAOnMerge) {
            // This merger may be the start of a concatenation of two chains. When one
            // of the chains mutates its AA the other must follow suit or else the above AA
            // check may prevent later ops from chaining together. A specific example of this is
            // when chain2 is prepended onto chain1:
            //  chain1 (that): opA (non-AA/mergeable) opB (non-AA/non-mergeable)
            //  chain2 (this): opC (cov-AA/non-mergeable) opD (cov-AA/mergeable)
            // W/o this propagation, after opD & opA merge, opB and opC would say they couldn't
            // chain - which would stop the concatenation process.
            this->propagateCoverageAAThroughoutChain();
            that->propagateCoverageAAThroughoutChain();
        }

        SkDEBUGCODE(this->validate();)

        return CombineResult::kMerged;
    }

#if defined(GR_TEST_UTILS)
    SkString onDumpInfo() const override {
        SkString str = SkStringPrintf("# draws: %d\n", fQuads.count());
        auto iter = fQuads.iterator();
        for (unsigned p = 0; p < fMetadata.fProxyCount; ++p) {
            SkString proxyStr = fViewCountPairs[p].fProxy->dump();
            str.append(proxyStr);
            str.appendf(", Filter: %d, MM: %d\n",
                        static_cast<int>(fMetadata.fFilter),
                        static_cast<int>(fMetadata.fMipmapMode));
            for (int i = 0; i < fViewCountPairs[p].fQuadCnt && iter.next(); ++i) {
                const GrQuad* quad = iter.deviceQuad();
                GrQuad uv = iter.isLocalValid() ? *(iter.localQuad()) : GrQuad();
                const ColorSubsetAndAA& info = iter.metadata();
                str.appendf(
                        "%d: Color: 0x%08x, Subset(%d): [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n"
                        "  UVs  [(%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f)]\n"
                        "  Quad [(%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f)]\n",
                        i, info.fColor.toBytes_RGBA(), fMetadata.fSubset, info.fSubsetRect.fLeft,
                        info.fSubsetRect.fTop, info.fSubsetRect.fRight, info.fSubsetRect.fBottom,
                        quad->point(0).fX, quad->point(0).fY, quad->point(1).fX, quad->point(1).fY,
                        quad->point(2).fX, quad->point(2).fY, quad->point(3).fX, quad->point(3).fY,
                        uv.point(0).fX, uv.point(0).fY, uv.point(1).fX, uv.point(1).fY,
                        uv.point(2).fX, uv.point(2).fY, uv.point(3).fX, uv.point(3).fY);
            }
        }
        return str;
    }
#endif

    GrQuadBuffer<ColorSubsetAndAA> fQuads;
    sk_sp<GrColorSpaceXform> fTextureColorSpaceXform;
    // Most state of TextureOp is packed into these two field to minimize the op's size.
    // Historically, increasing the size of TextureOp has caused surprising perf regressions, so
    // consider/measure changes with care.
    Desc* fDesc;
    Metadata fMetadata;

    // This field must go last. When allocating this op, we will allocate extra space to hold
    // additional ViewCountPairs immediately after the op's allocation so we can treat this
    // as an fProxyCnt-length array.
    ViewCountPair fViewCountPairs[1];

    using INHERITED = GrMeshDrawOp;
};

}  // anonymous namespace

namespace skgpu::ganesh {

#if defined(GR_TEST_UTILS)
uint32_t TextureOp::ClassID() {
    return TextureOpImpl::ClassID();
}
#endif

GrOp::Owner TextureOp::Make(GrRecordingContext* context,
                            GrSurfaceProxyView proxyView,
                            SkAlphaType alphaType,
                            sk_sp<GrColorSpaceXform> textureXform,
                            GrSamplerState::Filter filter,
                            GrSamplerState::MipmapMode mm,
                            const SkPMColor4f& color,
                            Saturate saturate,
                            SkBlendMode blendMode,
                            GrAAType aaType,
                            DrawQuad* quad,
                            const SkRect* subset) {
    // Apply optimizations that are valid whether or not using TextureOp or FillRectOp
    if (subset && subset->contains(proxyView.proxy()->backingStoreBoundsRect())) {
        // No need for a shader-based subset if hardware clamping achieves the same effect
        subset = nullptr;
    }

    if (filter != GrSamplerState::Filter::kNearest || mm != GrSamplerState::MipmapMode::kNone) {
        auto [mustFilter, mustMM] = filter_and_mm_have_effect(quad->fLocal, quad->fDevice);
        if (!mustFilter) {
            filter = GrSamplerState::Filter::kNearest;
        }
        if (!mustMM) {
            mm = GrSamplerState::MipmapMode::kNone;
        }
    }

    if (blendMode == SkBlendMode::kSrcOver) {
        return TextureOpImpl::Make(context, std::move(proxyView), std::move(textureXform), filter,
                                   mm, color, saturate, aaType, std::move(quad), subset);
    } else {
        // Emulate complex blending using FillRectOp
        GrSamplerState samplerState(GrSamplerState::WrapMode::kClamp, filter, mm);
        GrPaint paint;
        paint.setColor4f(color);
        paint.setXPFactory(GrXPFactory::FromBlendMode(blendMode));

        std::unique_ptr<GrFragmentProcessor> fp;
        const auto& caps = *context->priv().caps();
        if (subset) {
            SkRect localRect;
            if (quad->fLocal.asRect(&localRect)) {
                fp = GrTextureEffect::MakeSubset(std::move(proxyView), alphaType, SkMatrix::I(),
                                                 samplerState, *subset, localRect, caps);
            } else {
                fp = GrTextureEffect::MakeSubset(std::move(proxyView), alphaType, SkMatrix::I(),
                                                 samplerState, *subset, caps);
            }
        } else {
            fp = GrTextureEffect::Make(std::move(proxyView), alphaType, SkMatrix::I(), samplerState,
                                       caps);
        }
        fp = GrColorSpaceXformEffect::Make(std::move(fp), std::move(textureXform));
        fp = GrBlendFragmentProcessor::Make<SkBlendMode::kModulate>(std::move(fp), nullptr);
        if (saturate == Saturate::kYes) {
            fp = GrFragmentProcessor::ClampOutput(std::move(fp));
        }
        paint.setColorFragmentProcessor(std::move(fp));
        return ganesh::FillRectOp::Make(context, std::move(paint), aaType, quad);
    }
}

// A helper class that assists in breaking up bulk API quad draws into manageable chunks.
class TextureOp::BatchSizeLimiter {
public:
    BatchSizeLimiter(ganesh::SurfaceDrawContext* sdc,
                     const GrClip* clip,
                     GrRecordingContext* rContext,
                     int numEntries,
                     GrSamplerState::Filter filter,
                     GrSamplerState::MipmapMode mm,
                     Saturate saturate,
                     SkCanvas::SrcRectConstraint constraint,
                     const SkMatrix& viewMatrix,
                     sk_sp<GrColorSpaceXform> textureColorSpaceXform)
            : fSDC(sdc)
            , fClip(clip)
            , fContext(rContext)
            , fFilter(filter)
            , fMipmapMode(mm)
            , fSaturate(saturate)
            , fConstraint(constraint)
            , fViewMatrix(viewMatrix)
            , fTextureColorSpaceXform(textureColorSpaceXform)
            , fNumLeft(numEntries) {}

    void createOp(GrTextureSetEntry set[], int clumpSize, GrAAType aaType) {

        int clumpProxyCount = proxy_run_count(&set[fNumClumped], clumpSize);
        GrOp::Owner op = TextureOpImpl::Make(fContext,
                                             &set[fNumClumped],
                                             clumpSize,
                                             clumpProxyCount,
                                             fFilter,
                                             fMipmapMode,
                                             fSaturate,
                                             aaType,
                                             fConstraint,
                                             fViewMatrix,
                                             fTextureColorSpaceXform);
        fSDC->addDrawOp(fClip, std::move(op));

        fNumLeft -= clumpSize;
        fNumClumped += clumpSize;
    }

    int numLeft() const { return fNumLeft;  }
    int baseIndex() const { return fNumClumped; }

private:
    ganesh::SurfaceDrawContext* fSDC;
    const GrClip*               fClip;
    GrRecordingContext*         fContext;
    GrSamplerState::Filter      fFilter;
    GrSamplerState::MipmapMode  fMipmapMode;
    Saturate                    fSaturate;
    SkCanvas::SrcRectConstraint fConstraint;
    const SkMatrix&             fViewMatrix;
    sk_sp<GrColorSpaceXform>    fTextureColorSpaceXform;

    int                         fNumLeft;
    int                         fNumClumped = 0; // also the offset for the start of the next clump
};

// Greedily clump quad draws together until the index buffer limit is exceeded.
void TextureOp::AddTextureSetOps(ganesh::SurfaceDrawContext* sdc,
                                 const GrClip* clip,
                                 GrRecordingContext* context,
                                 GrTextureSetEntry set[],
                                 int cnt,
                                 int proxyRunCnt,
                                 GrSamplerState::Filter filter,
                                 GrSamplerState::MipmapMode mm,
                                 Saturate saturate,
                                 SkBlendMode blendMode,
                                 GrAAType aaType,
                                 SkCanvas::SrcRectConstraint constraint,
                                 const SkMatrix& viewMatrix,
                                 sk_sp<GrColorSpaceXform> textureColorSpaceXform) {
    // Ensure that the index buffer limits are lower than the proxy and quad count limits of
    // the op's metadata so we don't need to worry about overflow.
    SkDEBUGCODE(TextureOpImpl::ValidateResourceLimits();)
    SkASSERT(proxy_run_count(set, cnt) == proxyRunCnt);

    // First check if we can support batches as a single op
    if (blendMode != SkBlendMode::kSrcOver ||
        !context->priv().caps()->dynamicStateArrayGeometryProcessorTextureSupport()) {
        // Append each entry as its own op; these may still be GrTextureOps if the blend mode is
        // src-over but the backend doesn't support dynamic state changes. Otherwise Make()
        // automatically creates the appropriate FillRectOp to emulate TextureOp.
        SkMatrix ctm;
        for (int i = 0; i < cnt; ++i) {
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

            const SkRect* subset = constraint == SkCanvas::kStrict_SrcRectConstraint
                    ? &set[i].fSrcRect : nullptr;

            auto op = Make(context, set[i].fProxyView, set[i].fSrcAlphaType, textureColorSpaceXform,
                           filter, mm, set[i].fColor, saturate, blendMode, aaType, &quad, subset);
            sdc->addDrawOp(clip, std::move(op));
        }
        return;
    }

    // Second check if we can always just make a single op and avoid the extra iteration
    // needed to clump things together.
    if (cnt <= std::min(GrResourceProvider::MaxNumNonAAQuads(),
                      GrResourceProvider::MaxNumAAQuads())) {
        auto op = TextureOpImpl::Make(context, set, cnt, proxyRunCnt, filter, mm, saturate, aaType,
                                      constraint, viewMatrix, std::move(textureColorSpaceXform));
        sdc->addDrawOp(clip, std::move(op));
        return;
    }

    BatchSizeLimiter state(sdc, clip, context, cnt, filter, mm, saturate, constraint, viewMatrix,
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

                if (set[absIndex].fAAFlags != GrQuadAAFlags::kNone ||
                    runningAA == GrAAType::kCoverage) {

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
                        // large. Calve it off as its own TextureOp.
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

} // namespace skgpu::ganesh

#if defined(GR_TEST_UTILS)
GR_DRAW_OP_TEST_DEFINE(TextureOpImpl) {
    SkISize dims;
    dims.fHeight = random->nextULessThan(90) + 10;
    dims.fWidth = random->nextULessThan(90) + 10;
    auto origin = random->nextBool() ? kTopLeft_GrSurfaceOrigin : kBottomLeft_GrSurfaceOrigin;
    skgpu::Mipmapped mipmapped =
            random->nextBool() ? skgpu::Mipmapped::kYes : skgpu::Mipmapped::kNo;
    SkBackingFit fit = SkBackingFit::kExact;
    if (mipmapped == skgpu::Mipmapped::kNo) {
        fit = random->nextBool() ? SkBackingFit::kApprox : SkBackingFit::kExact;
    }
    const GrBackendFormat format =
            context->priv().caps()->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                            GrRenderable::kNo);
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(format,
                                                             dims,
                                                             GrRenderable::kNo,
                                                             1,
                                                             mipmapped,
                                                             fit,
                                                             skgpu::Budgeted::kNo,
                                                             GrProtected::kNo,
                                                             /*label=*/"TextureOp",
                                                             GrInternalSurfaceFlags::kNone);

    SkRect rect = GrTest::TestRect(random);
    SkRect srcRect;
    srcRect.fLeft = random->nextRangeScalar(0.f, proxy->width() / 2.f);
    srcRect.fRight = random->nextRangeScalar(0.f, proxy->width()) + proxy->width() / 2.f;
    srcRect.fTop = random->nextRangeScalar(0.f, proxy->height() / 2.f);
    srcRect.fBottom = random->nextRangeScalar(0.f, proxy->height()) + proxy->height() / 2.f;
    SkMatrix viewMatrix = GrTest::TestMatrixPreservesRightAngles(random);
    SkPMColor4f color = SkPMColor4f::FromBytes_RGBA(SkColorToPremulGrColor(random->nextU()));
    GrSamplerState::Filter filter = (GrSamplerState::Filter)random->nextULessThan(
            static_cast<uint32_t>(GrSamplerState::Filter::kLast) + 1);
    GrSamplerState::MipmapMode mm = GrSamplerState::MipmapMode::kNone;
    if (mipmapped == skgpu::Mipmapped::kYes) {
        mm = (GrSamplerState::MipmapMode)random->nextULessThan(
                static_cast<uint32_t>(GrSamplerState::MipmapMode::kLast) + 1);
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
    bool useSubset = random->nextBool();
    auto saturate = random->nextBool() ? TextureOp::Saturate::kYes
                                       : TextureOp::Saturate::kNo;
    GrSurfaceProxyView proxyView(
            std::move(proxy), origin,
            context->priv().caps()->getReadSwizzle(format, GrColorType::kRGBA_8888));
    auto alphaType = static_cast<SkAlphaType>(
            random->nextRangeU(kUnknown_SkAlphaType + 1, kLastEnum_SkAlphaType));

    DrawQuad quad = {GrQuad::MakeFromRect(rect, viewMatrix), GrQuad(srcRect), aaFlags};
    return TextureOp::Make(context, std::move(proxyView), alphaType,
                           std::move(texXform), filter, mm, color, saturate,
                           SkBlendMode::kSrcOver, aaType, &quad,
                           useSubset ? &srcRect : nullptr);
}

#endif // defined(GR_TEST_UTILS)
