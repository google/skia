/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureOp.h"
#include <new>
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
#include "GrQuadPerEdgeAA.h"
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
#include "SkRectPriv.h"
#include "SkTo.h"
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLVarying.h"

namespace {

using Domain = GrQuadPerEdgeAA::Domain;
using VertexSpec = GrQuadPerEdgeAA::VertexSpec;
using ColorType = GrQuadPerEdgeAA::ColorType;

/**
 * Geometry Processor that draws a texture modulated by a vertex color (though, this is meant to be
 * the same value across all vertices of a quad and uses flat interpolation when available). This is
 * used by TextureOp below.
 */
class TextureGeometryProcessor : public GrGeometryProcessor {
public:

    static sk_sp<GrGeometryProcessor> Make(GrTextureType textureType, GrPixelConfig textureConfig,
                                           const GrSamplerState::Filter filter,
                                           sk_sp<GrColorSpaceXform> textureColorSpaceXform,
                                           const VertexSpec& vertexSpec, const GrShaderCaps& caps) {
        return sk_sp<TextureGeometryProcessor>(new TextureGeometryProcessor(
                textureType, textureConfig, filter, std::move(textureColorSpaceXform), vertexSpec,
                caps));
    }

    const char* name() const override { return "TextureGeometryProcessor"; }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        b->add32(GrColorSpaceXform::XformKey(fTextureColorSpaceXform.get()));
        b->add32(fAttrs.getKey());
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
            }

        private:
            void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
                const auto& textureGP = args.fGP.cast<TextureGeometryProcessor>();
                fTextureColorSpaceXformHelper.emitCode(
                        args.fUniformHandler, textureGP.fTextureColorSpaceXform.get());
                if (!textureGP.fAttrs.needsPerspectiveInterpolation()) {
                    args.fVaryingHandler->setNoPerspective();
                }
                args.fVaryingHandler->emitAttributes(textureGP);
                gpArgs->fPositionVar = textureGP.fAttrs.positions().asShaderVar();

                this->emitTransforms(args.fVertBuilder,
                                     args.fVaryingHandler,
                                     args.fUniformHandler,
                                     textureGP.fAttrs.localCoords().asShaderVar(),
                                     args.fFPCoordTransformHandler);
                textureGP.fAttrs.emitColor(args, "paintColor");
                textureGP.fAttrs.emitExplicitLocalCoords(args, "texCoord", "domain");

                args.fFragBuilder->codeAppendf("%s = ", args.fOutputColor);
                args.fFragBuilder->appendTextureLookupAndModulate(
                        args.fOutputColor, args.fTexSamplers[0], "texCoord", kFloat2_GrSLType,
                        &fTextureColorSpaceXformHelper);
                args.fFragBuilder->codeAppend(";");

                textureGP.fAttrs.emitCoverage(args, "aaDist");
            }
            GrGLSLColorSpaceXformHelper fTextureColorSpaceXformHelper;
        };
        return new GLSLProcessor;
    }

private:
    TextureGeometryProcessor(GrTextureType textureType, GrPixelConfig textureConfig,
                             GrSamplerState::Filter filter,
                             sk_sp<GrColorSpaceXform> textureColorSpaceXform,
                             const VertexSpec& vertexSpec, const GrShaderCaps& caps)
            : INHERITED(kTextureGeometryProcessor_ClassID)
            , fAttrs(vertexSpec)
            , fTextureColorSpaceXform(std::move(textureColorSpaceXform))
            , fSampler(textureType, textureConfig, filter) {
        SkASSERT(vertexSpec.hasVertexColors() && vertexSpec.localDimensionality() == 2);
        this->setTextureSamplerCnt(1);
        this->setVertexAttributes(fAttrs.attributes(), fAttrs.attributeCount());
    }

    const TextureSampler& onTextureSampler(int) const override { return fSampler; }

    GrQuadPerEdgeAA::GPAttributes fAttrs;
    sk_sp<GrColorSpaceXform> fTextureColorSpaceXform;
    TextureSampler fSampler;

    typedef GrGeometryProcessor INHERITED;
};

static bool filter_has_effect_for_rect_stays_rect(const GrPerspQuad& quad, const SkRect& srcRect) {
    SkASSERT(quad.quadType() == GrQuadType::kRect);
    float ql = quad.x(0);
    float qt = quad.y(0);
    float qr = quad.x(3);
    float qb = quad.y(3);
    // Disable filtering when there is no scaling of the src rect and the src rect and dst rect
    // align fractionally. If we allow inverted src rects this logic needs to consider that.
    SkASSERT(srcRect.isSorted());
    return (qr - ql) != srcRect.width() || (qb - qt) != srcRect.height() ||
           SkScalarFraction(ql) != SkScalarFraction(srcRect.fLeft) ||
           SkScalarFraction(qt) != SkScalarFraction(srcRect.fTop);
}

static SkRect compute_domain(Domain domain, GrSamplerState::Filter filter,
                             GrSurfaceOrigin origin, const SkRect& srcRect, float iw, float ih) {
    static constexpr SkRect kLargeRect = {-2, -2, 2, 2};
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
        static const Sk4f kAdd = {0.f, 1.f, 0.f, 1.f};
        ltrb = SkNx_shuffle<0, 3, 2, 1>(kMul * ltrb + kAdd);
    }

    SkRect domainRect;
    ltrb.store(&domainRect);
    return domainRect;
}

static GrPerspQuad compute_src_quad(GrSurfaceOrigin origin, const SkRect& srcRect,
                                    float iw, float ih) {
    // Convert the pixel-space src rectangle into normalized texture coordinates
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
    return GrPerspQuad(texRect, SkMatrix::I());
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
                                          const SkPMColor4f& color,
                                          const SkRect& srcRect,
                                          const SkRect& dstRect,
                                          GrAAType aaType,
                                          GrQuadAAFlags aaFlags,
                                          SkCanvas::SrcRectConstraint constraint,
                                          const SkMatrix& viewMatrix,
                                          sk_sp<GrColorSpaceXform> textureColorSpaceXform) {
        GrOpMemoryPool* pool = context->contextPriv().opMemoryPool();

        return pool->allocate<TextureOp>(
                std::move(proxy), filter, color, srcRect, dstRect, aaType, aaFlags, constraint,
                viewMatrix, std::move(textureColorSpaceXform));
    }
    static std::unique_ptr<GrDrawOp> Make(GrContext* context,
                                          const GrRenderTargetContext::TextureSetEntry set[],
                                          int cnt, GrSamplerState::Filter filter, GrAAType aaType,
                                          const SkMatrix& viewMatrix,
                                          sk_sp<GrColorSpaceXform> textureColorSpaceXform) {
        size_t size = sizeof(TextureOp) + sizeof(Proxy) * (cnt - 1);
        GrOpMemoryPool* pool = context->contextPriv().opMemoryPool();
        void* mem = pool->allocate(size);
        return std::unique_ptr<GrDrawOp>(new (mem) TextureOp(set, cnt, filter, aaType, viewMatrix,
                                                             std::move(textureColorSpaceXform)));
    }

    ~TextureOp() override {
        for (unsigned p = 0; p < fProxyCnt; ++p) {
            if (fFinalized) {
                fProxies[p].fProxy->completedRead();
            } else {
                fProxies[p].fProxy->unref();
            }
        }
    }

    const char* name() const override { return "TextureOp"; }

    void visitProxies(const VisitProxyFunc& func, VisitorType visitor) const override {
        if (visitor == VisitorType::kAllocatorGather && fCanSkipAllocatorGather) {
            return;
        }
        for (unsigned p = 0; p < fProxyCnt; ++p) {
            func(fProxies[p].fProxy);
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
                const Quad& quad = fQuads[q];
                str.appendf(
                        "%d: Color: 0x%08x, TexRect [L: %.2f, T: %.2f, R: %.2f, B: %.2f] "
                        "Quad [(%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f)]\n",
                        i, quad.color().toBytes_RGBA(), quad.srcRect().fLeft, quad.srcRect().fTop,
                        quad.srcRect().fRight, quad.srcRect().fBottom, quad.quad().point(0).fX,
                        quad.quad().point(0).fY, quad.quad().point(1).fX, quad.quad().point(1).fY,
                        quad.quad().point(2).fX, quad.quad().point(2).fY, quad.quad().point(3).fX,
                        quad.quad().point(3).fY);
            }
        }
        str += INHERITED::dumpInfo();
        return str;
    }
#endif

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override {
        SkASSERT(!fFinalized);
        fFinalized = true;
        for (unsigned p = 0; p < fProxyCnt; ++p) {
            fProxies[p].fProxy->addPendingRead();
            fProxies[p].fProxy->unref();
        }
        return RequiresDstTexture::kNo;
    }

    FixedFunctionFlags fixedFunctionFlags() const override {
        return this->aaType() == GrAAType::kMSAA ? FixedFunctionFlags::kUsesHWAA
                                                 : FixedFunctionFlags::kNone;
    }

    DEFINE_OP_CLASS_ID

private:
    friend class ::GrOpMemoryPool;

    TextureOp(sk_sp<GrTextureProxy> proxy, GrSamplerState::Filter filter, const SkPMColor4f& color,
              const SkRect& srcRect, const SkRect& dstRect, GrAAType aaType, GrQuadAAFlags aaFlags,
              SkCanvas::SrcRectConstraint constraint, const SkMatrix& viewMatrix,
              sk_sp<GrColorSpaceXform> textureColorSpaceXform)
            : INHERITED(ClassID())
            , fTextureColorSpaceXform(std::move(textureColorSpaceXform))
            , fFilter(static_cast<unsigned>(filter))
            , fFinalized(0) {
        GrQuadType quadType = GrQuadTypeForTransformedRect(viewMatrix);
        auto quad = GrPerspQuad(dstRect, viewMatrix);

        // Clean up disparities between the overall aa type and edge configuration and apply
        // optimizations based on the rect and matrix when appropriate
        GrResolveAATypeForQuad(aaType, aaFlags, quad, quadType, &aaType, &aaFlags);
        fAAType = static_cast<unsigned>(aaType);

        fQuadType = static_cast<unsigned>(quadType);
        // We expect our caller to have already caught this optimization.
        SkASSERT(!srcRect.contains(proxy->getWorstCaseBoundsRect()) ||
                 constraint == SkCanvas::kFast_SrcRectConstraint);
        if (quadType == GrQuadType::kRect) {
            // Disable filtering if possible (note AA optimizations for rects are automatically
            // handled above in GrResolveAATypeForQuad).
            if (this->filter() != GrSamplerState::Filter::kNearest &&
                !filter_has_effect_for_rect_stays_rect(quad, srcRect)) {
                fFilter = static_cast<unsigned>(GrSamplerState::Filter::kNearest);
            }
        }
        // We may have had a strict constraint with nearest filter solely due to possible AA bloat.
        // If we don't have (or determined we don't need) coverage AA then we can skip using a
        // domain.
        if (constraint == SkCanvas::kStrict_SrcRectConstraint &&
            this->filter() == GrSamplerState::Filter::kNearest &&
            aaType != GrAAType::kCoverage) {
            constraint = SkCanvas::kFast_SrcRectConstraint;
        }
        const auto& draw = fQuads.emplace_back(srcRect, quad, aaFlags, constraint, color);
        fProxyCnt = 1;
        fProxies[0] = {proxy.release(), 1};
        auto bounds = quad.bounds();
        this->setBounds(bounds, HasAABloat(aaType == GrAAType::kCoverage), IsZeroArea::kNo);
        fDomain = static_cast<unsigned>(draw.domain());
        fWideColor = !SkPMColor4fFitsInBytes(color);
        fCanSkipAllocatorGather =
                static_cast<unsigned>(fProxies[0].fProxy->canSkipResourceAllocator());
    }
    TextureOp(const GrRenderTargetContext::TextureSetEntry set[], int cnt,
              GrSamplerState::Filter filter, GrAAType aaType, const SkMatrix& viewMatrix,
              sk_sp<GrColorSpaceXform> textureColorSpaceXform)
            : INHERITED(ClassID())
            , fTextureColorSpaceXform(std::move(textureColorSpaceXform))
            , fFilter(static_cast<unsigned>(filter))
            , fFinalized(0) {
        fQuads.reserve(cnt);
        fProxyCnt = SkToUInt(cnt);
        SkRect bounds = SkRectPriv::MakeLargestInverted();
        GrAAType overallAAType = GrAAType::kNone; // aa type maximally compatible with all dst rects
        bool mustFilter = false;
        fCanSkipAllocatorGather = static_cast<unsigned>(true);
        // All dst rects are transformed by the same view matrix, so their quad types are identical
        GrQuadType quadType = GrQuadTypeForTransformedRect(viewMatrix);
        for (unsigned p = 0; p < fProxyCnt; ++p) {
            fProxies[p].fProxy = SkRef(set[p].fProxy.get());
            fProxies[p].fQuadCnt = 1;
            SkASSERT(fProxies[p].fProxy->textureType() == fProxies[0].fProxy->textureType());
            SkASSERT(fProxies[p].fProxy->config() == fProxies[0].fProxy->config());
            if (!fProxies[p].fProxy->canSkipResourceAllocator()) {
                fCanSkipAllocatorGather = static_cast<unsigned>(false);
            }
            auto quad = GrPerspQuad(set[p].fDstRect, viewMatrix);
            bounds.joinPossiblyEmptyRect(quad.bounds());
            GrQuadAAFlags aaFlags;
            // Don't update the overall aaType, might be inappropriate for some of the quads
            GrAAType aaForQuad;
            GrResolveAATypeForQuad(aaType, set[p].fAAFlags, quad, quadType, &aaForQuad, &aaFlags);
            // Resolve sets aaForQuad to aaType or None, there is never a change between aa methods
            SkASSERT(aaForQuad == GrAAType::kNone || aaForQuad == aaType);
            if (overallAAType == GrAAType::kNone && aaForQuad != GrAAType::kNone) {
                overallAAType = aaType;
            }
            if (!mustFilter && this->filter() != GrSamplerState::Filter::kNearest) {
                mustFilter = quadType != GrQuadType::kRect ||
                             filter_has_effect_for_rect_stays_rect(quad, set[p].fSrcRect);
            }
            float alpha = SkTPin(set[p].fAlpha, 0.f, 1.f);
            SkPMColor4f color{alpha, alpha, alpha, alpha};
            fQuads.emplace_back(set[p].fSrcRect, quad, aaFlags, SkCanvas::kFast_SrcRectConstraint,
                                color);
        }
        fAAType = static_cast<unsigned>(overallAAType);
        if (!mustFilter) {
            fFilter = static_cast<unsigned>(GrSamplerState::Filter::kNearest);
        }
        this->setBounds(bounds, HasAABloat(this->aaType() == GrAAType::kCoverage), IsZeroArea::kNo);
        fQuadType = static_cast<unsigned>(quadType);
        fDomain = static_cast<unsigned>(false);
        fWideColor = static_cast<unsigned>(false);
    }

    void tess(void* v, const VertexSpec& spec, const GrTextureProxy* proxy, int start,
              int cnt) const {
        TRACE_EVENT0("skia", TRACE_FUNC);
        auto origin = proxy->origin();
        const auto* texture = proxy->peekTexture();
        float iw = 1.f / texture->width();
        float ih = 1.f / texture->height();

        for (int i = start; i < start + cnt; ++i) {
            const auto q = fQuads[i];
            GrPerspQuad srcQuad = compute_src_quad(origin, q.srcRect(), iw, ih);
            SkRect domain = compute_domain(q.domain(), this->filter(), origin, q.srcRect(), iw, ih);
            v = GrQuadPerEdgeAA::Tessellate(v, spec, q.quad(), q.color(), srcQuad, domain,
                                            q.aaFlags());
        }
    }

    void onPrepareDraws(Target* target) override {
        TRACE_EVENT0("skia", TRACE_FUNC);
        GrQuadType quadType = GrQuadType::kRect;
        Domain domain = Domain::kNo;
        bool wideColor = false;
        int numProxies = 0;
        int numTotalQuads = 0;
        auto textureType = fProxies[0].fProxy->textureType();
        auto config = fProxies[0].fProxy->config();
        GrAAType aaType = this->aaType();
        for (const auto& op : ChainRange<TextureOp>(this)) {
            if (op.quadType() > quadType) {
                quadType = op.quadType();
            }
            if (op.fDomain) {
                domain = Domain::kYes;
            }
            wideColor |= op.fWideColor;
            numProxies += op.fProxyCnt;
            for (unsigned p = 0; p < op.fProxyCnt; ++p) {
                numTotalQuads += op.fProxies[p].fQuadCnt;
                auto* proxy = op.fProxies[p].fProxy;
                if (!proxy->instantiate(target->resourceProvider())) {
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

        VertexSpec vertexSpec(quadType, wideColor ? ColorType::kHalf : ColorType::kByte,
                              GrQuadType::kRect, /* hasLocal */ true, domain, aaType);

        sk_sp<GrGeometryProcessor> gp = TextureGeometryProcessor::Make(
                textureType, config, this->filter(), std::move(fTextureColorSpaceXform),
                vertexSpec, *target->caps().shaderCaps());
        GrPipeline::InitArgs args;
        args.fProxy = target->proxy();
        args.fCaps = &target->caps();
        args.fResourceProvider = target->resourceProvider();
        args.fFlags = 0;
        if (aaType == GrAAType::kMSAA) {
            args.fFlags |= GrPipeline::kHWAntialias_Flag;
        }

        auto clip = target->detachAppliedClip();
        // We'll use a dynamic state array for the GP textures when there are multiple ops.
        // Otherwise, we use fixed dynamic state to specify the single op's proxy.
        GrPipeline::DynamicStateArrays* dynamicStateArrays = nullptr;
        GrPipeline::FixedDynamicState* fixedDynamicState;
        if (numProxies > 1) {
            dynamicStateArrays = target->allocDynamicStateArrays(numProxies, 1, false);
            fixedDynamicState = target->allocFixedDynamicState(clip.scissorState().rect(), 0);
        } else {
            fixedDynamicState = target->allocFixedDynamicState(clip.scissorState().rect(), 1);
            fixedDynamicState->fPrimitiveProcessorTextures[0] = fProxies[0].fProxy;
        }
        const auto* pipeline =
                target->allocPipeline(args, GrProcessorSet::MakeEmptySet(), std::move(clip));

        size_t vertexSize = gp->vertexStride();

        GrMesh* meshes = target->allocMeshes(numProxies);
        const GrBuffer* vbuffer;
        int vertexOffsetInBuffer = 0;
        int numQuadVerticesLeft = numTotalQuads * 4;
        int numAllocatedVertices = 0;
        void* vdata = nullptr;

        int m = 0;
        for (const auto& op : ChainRange<TextureOp>(this)) {
            int q = 0;
            for (unsigned p = 0; p < op.fProxyCnt; ++p) {
                int quadCnt = op.fProxies[p].fQuadCnt;
                auto* proxy = op.fProxies[p].fProxy;
                int meshVertexCnt = quadCnt * 4;
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

                if (quadCnt > 1) {
                    meshes[m].setPrimitiveType(GrPrimitiveType::kTriangles);
                    sk_sp<const GrBuffer> ibuffer =
                            target->resourceProvider()->refQuadIndexBuffer();
                    if (!ibuffer) {
                        SkDebugf("Could not allocate quad indices\n");
                        return;
                    }
                    meshes[m].setIndexedPatterned(ibuffer.get(), 6, 4, quadCnt,
                                                  GrResourceProvider::QuadCountOfQuadBuffer());
                } else {
                    meshes[m].setPrimitiveType(GrPrimitiveType::kTriangleStrip);
                    meshes[m].setNonIndexedNonInstanced(4);
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
        target->draw(std::move(gp), pipeline, fixedDynamicState, dynamicStateArrays, meshes,
                     numProxies);
    }

    CombineResult onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        TRACE_EVENT0("skia", TRACE_FUNC);
        const auto* that = t->cast<TextureOp>();
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
            if (thisProxy->config() == thatProxy->config() &&
                thisProxy->textureType() == thatProxy->textureType() &&
                caps.dynamicStateArrayGeometryProcessorTextureSupport()) {
                return CombineResult::kMayChain;
            }
            return CombineResult::kCannotCombine;
        }
        fProxies[0].fQuadCnt += that->fQuads.count();
        fQuads.push_back_n(that->fQuads.count(), that->fQuads.begin());
        if (that->fQuadType > fQuadType) {
            fQuadType = that->fQuadType;
        }
        fDomain |= that->fDomain;
        fWideColor |= that->fWideColor;
        if (upgradeToCoverageAAOnMerge) {
            fAAType = static_cast<unsigned>(GrAAType::kCoverage);
        }
        return CombineResult::kMerged;
    }

    GrAAType aaType() const { return static_cast<GrAAType>(fAAType); }
    GrSamplerState::Filter filter() const { return static_cast<GrSamplerState::Filter>(fFilter); }
    GrQuadType quadType() const { return static_cast<GrQuadType>(fQuadType); }

    class Quad {
    public:
        Quad(const SkRect& srcRect, const GrPerspQuad& quad, GrQuadAAFlags aaFlags,
             SkCanvas::SrcRectConstraint constraint, const SkPMColor4f& color)
                : fSrcRect(srcRect)
                , fQuad(quad)
                , fColor(color)
                , fHasDomain(constraint == SkCanvas::kStrict_SrcRectConstraint)
                , fAAFlags(static_cast<unsigned>(aaFlags)) {
            SkASSERT(fAAFlags == static_cast<unsigned>(aaFlags));
        }
        const GrPerspQuad& quad() const { return fQuad; }
        const SkRect& srcRect() const { return fSrcRect; }
        SkPMColor4f color() const { return fColor; }
        Domain domain() const { return Domain(fHasDomain); }
        GrQuadAAFlags aaFlags() const { return static_cast<GrQuadAAFlags>(fAAFlags); }

    private:
        SkRect fSrcRect;
        GrPerspQuad fQuad;
        SkPMColor4f fColor;
        unsigned fHasDomain : 1;
        unsigned fAAFlags : 4;
    };
    struct Proxy {
        GrTextureProxy* fProxy;
        int fQuadCnt;
    };
    SkSTArray<1, Quad, true> fQuads;
    sk_sp<GrColorSpaceXform> fTextureColorSpaceXform;
    unsigned fFilter : 2;
    unsigned fAAType : 2;
    unsigned fQuadType : 2; // Device quad, src quad is always trivial
    unsigned fDomain : 1;
    unsigned fWideColor : 1;
    // Used to track whether fProxy is ref'ed or has a pending IO after finalize() is called.
    unsigned fFinalized : 1;
    unsigned fCanSkipAllocatorGather : 1;
    unsigned fProxyCnt : 32 - 10;
    Proxy fProxies[1];

    static_assert(kGrQuadTypeCount <= 4, "GrQuadType does not fit in 2 bits");

    typedef GrMeshDrawOp INHERITED;
};

}  // anonymous namespace

namespace GrTextureOp {

std::unique_ptr<GrDrawOp> Make(GrContext* context,
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

std::unique_ptr<GrDrawOp> Make(GrContext* context,
                               const GrRenderTargetContext::TextureSetEntry set[],
                               int cnt,
                               GrSamplerState::Filter filter,
                               GrAAType aaType,
                               const SkMatrix& viewMatrix,
                               sk_sp<GrColorSpaceXform> textureColorSpaceXform) {
    return TextureOp::Make(context, set, cnt, filter, aaType, viewMatrix,
                           std::move(textureColorSpaceXform));
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

    const GrBackendFormat format =
            context->contextPriv().caps()->getBackendFormatFromColorType(kRGBA_8888_SkColorType);

    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
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
