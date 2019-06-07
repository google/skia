/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrTextureOp.h"
#include <new>
#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "include/gpu/GrTexture.h"
#include "include/private/GrRecordingContext.h"
#include "include/private/GrTextureProxy.h"
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
#include "src/gpu/SkGr.h"
#include "src/gpu/geometry/GrQuad.h"
#include "src/gpu/geometry/GrQuadList.h"
#include "src/gpu/geometry/GrQuadUtils.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/ops/GrQuadPerEdgeAA.h"

namespace {

using Domain = GrQuadPerEdgeAA::Domain;
using VertexSpec = GrQuadPerEdgeAA::VertexSpec;
using ColorType = GrQuadPerEdgeAA::ColorType;

// if normalizing the domain then pass 1/width, 1/height, 1 for iw, ih, h. Otherwise pass
// 1, 1, and height.
static SkRect compute_domain(Domain domain, GrSamplerState::Filter filter, GrSurfaceOrigin origin,
                             const SkRect& srcRect, float iw, float ih, float h) {
    static constexpr SkRect kLargeRect = {-100000, -100000, 1000000, 1000000};
    if (domain == Domain::kNo) {
        // Either the quad has no domain constraint and is batched with a domain constrained op
        // (in which case we want a domain that doesn't restrict normalized tex coords), or the
        // entire op doesn't use the domain, in which case the returned value is ignored.
        return kLargeRect;
    }

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
        const Sk4f kAdd = {0.f, h, 0.f, h};
        ltrb = SkNx_shuffle<0, 3, 2, 1>(kMul * ltrb + kAdd);
    }

    SkRect domainRect;
    ltrb.store(&domainRect);
    return domainRect;
}

// If normalizing the src quad then pass 1/width, 1/height, 1 for iw, ih, h. Otherwise pass
// 1, 1, and height.
static GrQuad compute_src_quad_from_rect(GrSurfaceOrigin origin, const SkRect& srcRect,
                                         float iw, float ih, float h) {
    // Convert the pixel-space src rectangle into normalized texture coordinates
    SkRect texRect = {
        iw * srcRect.fLeft,
        ih * srcRect.fTop,
        iw * srcRect.fRight,
        ih * srcRect.fBottom
    };
    if (origin == kBottomLeft_GrSurfaceOrigin) {
        texRect.fTop = h - texRect.fTop;
        texRect.fBottom = h - texRect.fBottom;
    }
    return GrQuad(texRect);
}
// Normalizes logical src coords and corrects for origin
static GrQuad compute_src_quad(GrSurfaceOrigin origin, const GrQuad& srcQuad,
                               float iw, float ih, float h) {
    // The src quad should not have any perspective
    SkASSERT(!srcQuad.hasPerspective());
    skvx::Vec<4, float> xs = srcQuad.x4f() * iw;
    skvx::Vec<4, float> ys = srcQuad.y4f() * ih;
    if (origin == kBottomLeft_GrSurfaceOrigin) {
        ys = h - ys;
    }
    return GrQuad(xs, ys, srcQuad.quadType());
}

/**
 * Op that implements GrTextureOp::Make. It draws textured quads. Each quad can modulate against a
 * the texture by color. The blend with the destination is always src-over. The edges are non-AA.
 */
class TextureOp final : public GrMeshDrawOp {
public:
    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                          sk_sp<GrTextureProxy> proxy,
                                          GrSamplerState::Filter filter,
                                          const SkPMColor4f& color,
                                          const SkRect& srcRect,
                                          const SkRect& dstRect,
                                          GrAAType aaType,
                                          GrQuadAAFlags aaFlags,
                                          SkCanvas::SrcRectConstraint constraint,
                                          const SkMatrix& viewMatrix,
                                          sk_sp<GrColorSpaceXform> textureColorSpaceXform) {
        GrQuad dstQuad = GrQuad::MakeFromRect(dstRect, viewMatrix);

        if (dstQuad.quadType() == GrQuad::Type::kAxisAligned) {
            // Disable filtering if possible (note AA optimizations for rects are automatically
            // handled above in GrResolveAATypeForQuad).
            if (filter != GrSamplerState::Filter::kNearest &&
                !GrTextureOp::GetFilterHasEffect(viewMatrix, srcRect, dstRect)) {
                filter = GrSamplerState::Filter::kNearest;
            }
        }

        GrOpMemoryPool* pool = context->priv().opMemoryPool();
        // srcRect provides both local coords and domain (if needed), so use nullptr for srcQuad
        return pool->allocate<TextureOp>(
                std::move(proxy), filter, color, dstQuad, srcRect, constraint,
                nullptr, aaType, aaFlags, std::move(textureColorSpaceXform));
    }
    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                          sk_sp<GrTextureProxy> proxy,
                                          GrSamplerState::Filter filter,
                                          const SkPMColor4f& color,
                                          const SkPoint srcQuad[4],
                                          const SkPoint dstQuad[4],
                                          GrAAType aaType,
                                          GrQuadAAFlags aaFlags,
                                          const SkRect* domain,
                                          const SkMatrix& viewMatrix,
                                          sk_sp<GrColorSpaceXform> textureColorSpaceXform) {
        GrQuad grDstQuad = GrQuad::MakeFromSkQuad(dstQuad, viewMatrix);
        GrQuad grSrcQuad = GrQuad::MakeFromSkQuad(srcQuad, SkMatrix::I());

        // If constraint remains fast, the value in srcRect will be ignored since srcQuads provides
        // the local coordinates and a domain won't be used.
        SkRect srcRect = SkRect::MakeEmpty();
        SkCanvas::SrcRectConstraint constraint = SkCanvas::kFast_SrcRectConstraint;
        if (domain) {
            srcRect = *domain;
            constraint = SkCanvas::kStrict_SrcRectConstraint;
        }

        GrOpMemoryPool* pool = context->priv().opMemoryPool();
        // Pass domain as srcRect if provided, but send srcQuad as a GrQuad for local coords
        return pool->allocate<TextureOp>(
                std::move(proxy), filter, color, grDstQuad, srcRect, constraint, &grSrcQuad,
                aaType, aaFlags, std::move(textureColorSpaceXform));
    }
    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                          const GrRenderTargetContext::TextureSetEntry set[],
                                          int cnt, GrSamplerState::Filter filter, GrAAType aaType,
                                          SkCanvas::SrcRectConstraint constraint,
                                          const SkMatrix& viewMatrix,
                                          sk_sp<GrColorSpaceXform> textureColorSpaceXform) {
        size_t size = sizeof(TextureOp) + sizeof(Proxy) * (cnt - 1);
        GrOpMemoryPool* pool = context->priv().opMemoryPool();
        void* mem = pool->allocate(size);
        return std::unique_ptr<GrDrawOp>(new (mem) TextureOp(
                set, cnt, filter, aaType, constraint, viewMatrix,
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
        int q = 0;
        for (unsigned p = 0; p < fProxyCnt; ++p) {
            str.appendf("Proxy ID: %d, Filter: %d\n", fProxies[p].fProxy->uniqueID().asUInt(),
                        static_cast<int>(fFilter));
            for (int i = 0; i < fProxies[p].fQuadCnt; ++i, ++q) {
                GrQuad quad = fQuads[q];
                const ColorDomainAndAA& info = fQuads.metadata(i);
                str.appendf(
                        "%d: Color: 0x%08x, TexRect [L: %.2f, T: %.2f, R: %.2f, B: %.2f] "
                        "Quad [(%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f)]\n",
                        i, info.fColor.toBytes_RGBA(), info.fSrcRect.fLeft, info.fSrcRect.fTop,
                        info.fSrcRect.fRight, info.fSrcRect.fBottom, quad.point(0).fX,
                        quad.point(0).fY, quad.point(1).fX, quad.point(1).fY,
                        quad.point(2).fX, quad.point(2).fY, quad.point(3).fX,
                        quad.point(3).fY);
            }
        }
        str += INHERITED::dumpInfo();
        return str;
    }
#endif

    GrProcessorSet::Analysis finalize(
            const GrCaps& caps, const GrAppliedClip*, GrFSAAType, GrClampType clampType) override {
        fColorType = static_cast<unsigned>(ColorType::kNone);
        for (int q = 0; q < fQuads.count(); ++q) {
            const ColorDomainAndAA& info = fQuads.metadata(q);
            auto colorType = GrQuadPerEdgeAA::MinColorType(info.fColor, clampType, caps);
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

    // dstQuad and dstQuadType should be the geometry transformed by the view matrix.
    // srcRect represents original src rect and will be used as the domain when constraint is strict
    // If srcQuad is provided, it will be used for the local coords instead of srcRect, although
    // srcRect will still specify the domain constraint if needed.
    TextureOp(sk_sp<GrTextureProxy> proxy, GrSamplerState::Filter filter, const SkPMColor4f& color,
              const GrQuad& dstQuad, const SkRect& srcRect,
              SkCanvas::SrcRectConstraint constraint, const GrQuad* srcQuad, GrAAType aaType,
              GrQuadAAFlags aaFlags, sk_sp<GrColorSpaceXform> textureColorSpaceXform)
            : INHERITED(ClassID())
            , fTextureColorSpaceXform(std::move(textureColorSpaceXform))
            , fFilter(static_cast<unsigned>(filter)) {
        // Clean up disparities between the overall aa type and edge configuration and apply
        // optimizations based on the rect and matrix when appropriate
        GrQuadUtils::ResolveAAType(aaType, aaFlags, dstQuad, &aaType, &aaFlags);
        fAAType = static_cast<unsigned>(aaType);

        // We expect our caller to have already caught this optimization.
        SkASSERT(!srcRect.contains(proxy->getWorstCaseBoundsRect()) ||
                 constraint == SkCanvas::kFast_SrcRectConstraint);

        // We may have had a strict constraint with nearest filter solely due to possible AA bloat.
        // If we don't have (or determined we don't need) coverage AA then we can skip using a
        // domain.
        if (constraint == SkCanvas::kStrict_SrcRectConstraint &&
            this->filter() == GrSamplerState::Filter::kNearest &&
            aaType != GrAAType::kCoverage) {
            constraint = SkCanvas::kFast_SrcRectConstraint;
        }

        Domain domain = constraint == SkCanvas::kStrict_SrcRectConstraint ? Domain::kYes
                                                                          : Domain::kNo;
        // Initially, if srcQuad is provided it will always be at index 0 of fSrcQuads
        fQuads.push_back(dstQuad, {color, srcRect, srcQuad ? 0 : -1, domain, aaFlags});
        if (srcQuad) {
            fSrcQuads.push_back(*srcQuad);
        }
        fProxyCnt = 1;
        fProxies[0] = {proxy.release(), 1};
        this->setBounds(dstQuad.bounds(), HasAABloat(aaType == GrAAType::kCoverage),
                        IsZeroArea::kNo);
        fDomain = static_cast<unsigned>(domain);
    }
    TextureOp(const GrRenderTargetContext::TextureSetEntry set[], int cnt,
              GrSamplerState::Filter filter, GrAAType aaType,
              SkCanvas::SrcRectConstraint constraint, const SkMatrix& viewMatrix,
              sk_sp<GrColorSpaceXform> textureColorSpaceXform)
            : INHERITED(ClassID())
            , fTextureColorSpaceXform(std::move(textureColorSpaceXform))
            , fFilter(static_cast<unsigned>(filter)) {
        fProxyCnt = SkToUInt(cnt);
        SkRect bounds = SkRectPriv::MakeLargestInverted();
        GrAAType overallAAType = GrAAType::kNone; // aa type maximally compatible with all dst rects
        bool mustFilter = false;
        // Most dst rects are transformed by the same view matrix, so their quad types start
        // identical, unless an entry provides a dstClip or additional transform that changes it.
        // The quad list will automatically adapt to that.
        fQuads.reserve(cnt, viewMatrix.hasPerspective());
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

            // Use dstRect unless dstClip is provided, which is assumed to be a quad
            auto quad = set[p].fDstClipQuad == nullptr ?
                    GrQuad::MakeFromRect(set[p].fDstRect, ctm) :
                    GrQuad::MakeFromSkQuad(set[p].fDstClipQuad, ctm);

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
            if (!mustFilter && this->filter() != GrSamplerState::Filter::kNearest) {
                mustFilter = quad.quadType() != GrQuad::Type::kAxisAligned ||
                             GrTextureOp::GetFilterHasEffect(ctm, set[p].fSrcRect,
                                                             set[p].fDstRect);
            }
            Domain domainForQuad = Domain::kNo;
            if (constraint == SkCanvas::kStrict_SrcRectConstraint) {
                // Check (briefly) if the strict constraint is needed for this set entry
                if (!set[p].fSrcRect.contains(fProxies[p].fProxy->getWorstCaseBoundsRect()) &&
                    (mustFilter || aaForQuad == GrAAType::kCoverage)) {
                    // Can't rely on hardware clamping and the draw will access outer texels
                    // for AA and/or bilerp
                    netDomain = Domain::kYes;
                    domainForQuad = Domain::kYes;
                }
            }
            float alpha = SkTPin(set[p].fAlpha, 0.f, 1.f);
            allOpaque &= (1.f == alpha);
            SkPMColor4f color{alpha, alpha, alpha, alpha};
            int srcQuadIndex = -1;
            if (set[p].fDstClipQuad) {
                // Derive new source coordinates that match dstClip's relative locations in dstRect,
                // but with respect to srcRect
                SkPoint srcQuad[4];
                GrMapRectPoints(set[p].fDstRect, set[p].fSrcRect, set[p].fDstClipQuad, srcQuad, 4);
                fSrcQuads.push_back(GrQuad::MakeFromSkQuad(srcQuad, SkMatrix::I()));
                srcQuadIndex = fSrcQuads.count() - 1;
            }
            fQuads.push_back(quad, {color, set[p].fSrcRect, srcQuadIndex, domainForQuad, aaFlags});
        }
        fAAType = static_cast<unsigned>(overallAAType);
        if (!mustFilter) {
            fFilter = static_cast<unsigned>(GrSamplerState::Filter::kNearest);
        }
        this->setBounds(bounds, HasAABloat(this->aaType() == GrAAType::kCoverage), IsZeroArea::kNo);
        fDomain = static_cast<unsigned>(netDomain);
    }

    void tess(void* v, const VertexSpec& spec, const GrTextureProxy* proxy, int start,
              int cnt) const {
        TRACE_EVENT0("skia", TRACE_FUNC);
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

        for (int i = start; i < start + cnt; ++i) {
            const GrQuad& device = fQuads[i];
            const ColorDomainAndAA& info = fQuads.metadata(i);

            GrQuad srcQuad = info.fSrcQuadIndex >= 0 ?
                    compute_src_quad(origin, fSrcQuads[info.fSrcQuadIndex], iw, ih, h) :
                    compute_src_quad_from_rect(origin, info.fSrcRect, iw, ih, h);
            SkRect domain =
                    compute_domain(info.domain(), this->filter(), origin, info.fSrcRect, iw, ih, h);
            v = GrQuadPerEdgeAA::Tessellate(v, spec, device, info.fColor, srcQuad, domain,
                                            info.aaFlags());
        }
    }

    void onPrepareDraws(Target* target) override {
        TRACE_EVENT0("skia", TRACE_FUNC);
        GrQuad::Type quadType = GrQuad::Type::kAxisAligned;
        GrQuad::Type srcQuadType = GrQuad::Type::kAxisAligned;
        Domain domain = Domain::kNo;
        ColorType colorType = ColorType::kNone;
        int numProxies = 0;
        int numTotalQuads = 0;
        auto textureType = fProxies[0].fProxy->textureType();
        auto config = fProxies[0].fProxy->config();
        GrAAType aaType = this->aaType();
        for (const auto& op : ChainRange<TextureOp>(this)) {
            if (op.fQuads.quadType() > quadType) {
                quadType = op.fQuads.quadType();
            }
            if (op.fSrcQuads.quadType() > srcQuadType) {
                // Should only become more general if there are quads to use instead of fSrcRect
                SkASSERT(op.fSrcQuads.count() > 0);
                srcQuadType = op.fSrcQuads.quadType();
            }
            if (op.fDomain) {
                domain = Domain::kYes;
            }
            colorType = SkTMax(colorType, static_cast<ColorType>(op.fColorType));
            numProxies += op.fProxyCnt;
            for (unsigned p = 0; p < op.fProxyCnt; ++p) {
                numTotalQuads += op.fProxies[p].fQuadCnt;
                auto* proxy = op.fProxies[p].fProxy;
                if (!proxy->isInstantiated()) {
                    return;
                }
                SkASSERT(proxy->config() == config);
                SkASSERT(proxy->textureType() == textureType);
            }
            if (op.aaType() == GrAAType::kCoverage) {
                SkASSERT(aaType == GrAAType::kCoverage || aaType == GrAAType::kNone);
                aaType = GrAAType::kCoverage;
            }
        }

        VertexSpec vertexSpec(quadType, colorType, srcQuadType, /* hasLocal */ true, domain, aaType,
                              /* alpha as coverage */ true);

        GrSamplerState samplerState = GrSamplerState(GrSamplerState::WrapMode::kClamp,
                                                     this->filter());
        GrGpu* gpu = target->resourceProvider()->priv().gpu();
        uint32_t extraSamplerKey = gpu->getExtraSamplerKeyForProgram(
                samplerState, fProxies[0].fProxy->backendFormat());

        sk_sp<GrGeometryProcessor> gp = GrQuadPerEdgeAA::MakeTexturedProcessor(
                vertexSpec, *target->caps().shaderCaps(),
                textureType, config, samplerState, extraSamplerKey,
                std::move(fTextureColorSpaceXform));

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

        size_t vertexSize = gp->vertexStride();

        GrMesh* meshes = target->allocMeshes(numProxies);
        sk_sp<const GrBuffer> vbuffer;
        int vertexOffsetInBuffer = 0;
        int numQuadVerticesLeft = numTotalQuads * vertexSpec.verticesPerQuad();
        int numAllocatedVertices = 0;
        void* vdata = nullptr;

        int m = 0;
        for (const auto& op : ChainRange<TextureOp>(this)) {
            int q = 0;
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

                op.tess(vdata, vertexSpec, proxy, q, quadCnt);

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
                q += quadCnt;
            }
        }
        SkASSERT(!numQuadVerticesLeft);
        SkASSERT(!numAllocatedVertices);
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
        TRACE_EVENT0("skia", TRACE_FUNC);
        const auto* that = t->cast<TextureOp>();
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

        // Concatenate quad lists together, updating the fSrcQuadIndex in the appended quads
        // to account for the new starting index in fSrcQuads
        int srcQuadOffset = fSrcQuads.count();
        int oldQuadCount = fQuads.count();

        fSrcQuads.concat(that->fSrcQuads);
        fQuads.concat(that->fQuads);
        fProxies[0].fQuadCnt += that->fQuads.count();

        if (that->fSrcQuads.count() > 0) {
            // Some of the concatenated quads pointed to fSrcQuads, so adjust the indices for the
            // newly appended quads
            for (int i = oldQuadCount; i < fQuads.count(); ++i) {
                if (fQuads.metadata(i).fSrcQuadIndex >= 0) {
                    fQuads.metadata(i).fSrcQuadIndex += srcQuadOffset;
                }
            }
        }

        // Confirm all tracked state makes sense when in debug builds
#ifdef SK_DEBUG
        SkASSERT(fSrcQuads.count() <= fQuads.count());
        for (int i = 0; i < fQuads.count(); ++i) {
            int srcIndex = fQuads.metadata(i).fSrcQuadIndex;
            if (srcIndex >= 0) {
                // Make sure it points to a valid index, in the right region of the list
                SkASSERT(srcIndex < fSrcQuads.count());
                SkASSERT((i < oldQuadCount && srcIndex < srcQuadOffset) ||
                         (i >= oldQuadCount && srcIndex >= srcQuadOffset));
            }
        }
#endif
        return CombineResult::kMerged;
    }

    GrAAType aaType() const { return static_cast<GrAAType>(fAAType); }
    GrSamplerState::Filter filter() const { return static_cast<GrSamplerState::Filter>(fFilter); }

    struct ColorDomainAndAA {
        // Special constructor to convert enums into the packed bits, which should not delete
        // the implicit move constructor (but it does require us to declare an empty ctor for
        // use with the GrTQuadList).
        ColorDomainAndAA(const SkPMColor4f& color, const SkRect& srcRect, int srcQuadIndex,
                         Domain hasDomain, GrQuadAAFlags aaFlags)
                : fColor(color)
                , fSrcRect(srcRect)
                , fSrcQuadIndex(srcQuadIndex)
                , fHasDomain(static_cast<unsigned>(hasDomain))
                , fAAFlags(static_cast<unsigned>(aaFlags)) {
            SkASSERT(fHasDomain == static_cast<unsigned>(hasDomain));
            SkASSERT(fAAFlags == static_cast<unsigned>(aaFlags));
        }
        ColorDomainAndAA() = default;

        SkPMColor4f fColor;
        // Even if fSrcQuadIndex provides source coords, use fSrcRect for domain constraint
        SkRect fSrcRect;
        // If >= 0, use to access fSrcQuads instead of fSrcRect for the source coordinates
        int fSrcQuadIndex;
        unsigned fHasDomain : 1;
        unsigned fAAFlags : 4;

        Domain domain() const { return Domain(fHasDomain); }
        GrQuadAAFlags aaFlags() const { return static_cast<GrQuadAAFlags>(fAAFlags); }
    };
    struct Proxy {
        GrTextureProxy* fProxy;
        int fQuadCnt;
    };
    GrTQuadList<ColorDomainAndAA> fQuads;
    // The majority of texture ops will not track a complete src quad so this is indexed separately
    // and may be of different size to fQuads.
    GrQuadList fSrcQuads;
    sk_sp<GrColorSpaceXform> fTextureColorSpaceXform;
    unsigned fFilter : 2;
    unsigned fAAType : 2;
    unsigned fDomain : 1;
    unsigned fColorType : 2;
    GR_STATIC_ASSERT(GrQuadPerEdgeAA::kColorTypeCount <= 4);
    unsigned fProxyCnt : 32 - 7;
    Proxy fProxies[1];

    static_assert(GrQuad::kTypeCount <= 4, "GrQuad::Type does not fit in 2 bits");

    typedef GrMeshDrawOp INHERITED;
};

}  // anonymous namespace

namespace GrTextureOp {

std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                               sk_sp<GrTextureProxy> proxy,
                               GrSamplerState::Filter filter,
                               const SkPMColor4f& color,
                               const SkRect& srcRect,
                               const SkRect& dstRect,
                               GrAAType aaType,
                               GrQuadAAFlags aaFlags,
                               SkCanvas::SrcRectConstraint constraint,
                               const SkMatrix& viewMatrix,
                               sk_sp<GrColorSpaceXform> textureColorSpaceXform) {
    return TextureOp::Make(context, std::move(proxy), filter, color, srcRect, dstRect, aaType,
                           aaFlags, constraint, viewMatrix, std::move(textureColorSpaceXform));
}

std::unique_ptr<GrDrawOp> MakeQuad(GrRecordingContext* context,
                                  sk_sp<GrTextureProxy> proxy,
                                  GrSamplerState::Filter filter,
                                  const SkPMColor4f& color,
                                  const SkPoint srcQuad[4],
                                  const SkPoint dstQuad[4],
                                  GrAAType aaType,
                                  GrQuadAAFlags aaFlags,
                                  const SkRect* domain,
                                  const SkMatrix& viewMatrix,
                                  sk_sp<GrColorSpaceXform> textureXform) {
    return TextureOp::Make(context, std::move(proxy), filter, color, srcQuad, dstQuad, aaType,
                           aaFlags, domain, viewMatrix, std::move(textureXform));
}

std::unique_ptr<GrDrawOp> MakeSet(GrRecordingContext* context,
                                  const GrRenderTargetContext::TextureSetEntry set[],
                                  int cnt,
                                  GrSamplerState::Filter filter,
                                  GrAAType aaType,
                                  SkCanvas::SrcRectConstraint constraint,
                                  const SkMatrix& viewMatrix,
                                  sk_sp<GrColorSpaceXform> textureColorSpaceXform) {
    return TextureOp::Make(context, set, cnt, filter, aaType, constraint, viewMatrix,
                           std::move(textureColorSpaceXform));
}

bool GetFilterHasEffect(const SkMatrix& viewMatrix, const SkRect& srcRect, const SkRect& dstRect) {
    // Hypothetically we could disable bilerp filtering when flipping or rotating 90 degrees, but
    // that makes the math harder and we don't want to increase the overhead of the checks
    if (!viewMatrix.isScaleTranslate() ||
        viewMatrix.getScaleX() < 0.0f || viewMatrix.getScaleY() < 0.0f) {
        return true;
    }

    // Given the matrix conditions ensured above, this computes the device space coordinates for
    // the top left corner of dstRect and its size.
    SkScalar dw = viewMatrix.getScaleX() * dstRect.width();
    SkScalar dh = viewMatrix.getScaleY() * dstRect.height();
    SkScalar dl = viewMatrix.getScaleX() * dstRect.fLeft + viewMatrix.getTranslateX();
    SkScalar dt = viewMatrix.getScaleY() * dstRect.fTop + viewMatrix.getTranslateY();

    // Disable filtering when there is no scaling of the src rect and the src rect and dst rect
    // align fractionally. If we allow inverted src rects this logic needs to consider that.
    SkASSERT(srcRect.isSorted());
    return dw != srcRect.width() || dh != srcRect.height() ||
           SkScalarFraction(dl) != SkScalarFraction(srcRect.fLeft) ||
           SkScalarFraction(dt) != SkScalarFraction(srcRect.fTop);
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
            context->priv().caps()->getBackendFormatFromColorType(kRGBA_8888_SkColorType);

    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(format, desc, origin, mipMapped, fit,
                                                             SkBudgeted::kNo,
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
            static_cast<uint32_t>(GrSamplerState::Filter::kMipMap) + 1);
    while (mipMapped == GrMipMapped::kNo && filter == GrSamplerState::Filter::kMipMap) {
        filter = (GrSamplerState::Filter)random->nextULessThan(
                static_cast<uint32_t>(GrSamplerState::Filter::kMipMap) + 1);
    }
    auto texXform = GrTest::TestColorXform(random);
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
                             aaFlags, constraint, viewMatrix, std::move(texXform));
}

#endif
