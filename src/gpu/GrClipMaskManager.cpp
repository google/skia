/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrClipMaskManager.h"
#include "GrCaps.h"
#include "GrDrawingManager.h"
#include "GrDrawContext.h"
#include "GrDrawTarget.h"
#include "GrGpuResourcePriv.h"
#include "GrPaint.h"
#include "GrPathRenderer.h"
#include "GrRenderTarget.h"
#include "GrRenderTargetPriv.h"
#include "GrResourceProvider.h"
#include "GrStencilAttachment.h"
#include "GrSWMaskHelper.h"
#include "SkRasterClip.h"
#include "SkTLazy.h"
#include "effects/GrConvexPolyEffect.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "effects/GrRRectEffect.h"
#include "effects/GrTextureDomain.h"

typedef SkClipStack::Element Element;

////////////////////////////////////////////////////////////////////////////////
// set up the draw state to enable the aa clipping mask. Besides setting up the
// stage matrix this also alters the vertex layout
static const GrFragmentProcessor* create_fp_for_mask(GrTexture* result, const SkIRect &devBound) {
    SkMatrix mat;
    // We use device coords to compute the texture coordinates. We set our matrix to be a
    // translation to the devBound, and then a scaling matrix to normalized coords.
    mat.setIDiv(result->width(), result->height());
    mat.preTranslate(SkIntToScalar(-devBound.fLeft),
                     SkIntToScalar(-devBound.fTop));

    SkIRect domainTexels = SkIRect::MakeWH(devBound.width(), devBound.height());
    return GrTextureDomainEffect::Create(result,
                                         mat,
                                         GrTextureDomain::MakeTexelDomain(result, domainTexels),
                                         GrTextureDomain::kDecal_Mode,
                                         GrTextureParams::kNone_FilterMode,
                                         kDevice_GrCoordSet);
}

// Does the path in 'element' require SW rendering? If so, return true (and,
// optionally, set 'prOut' to NULL. If not, return false (and, optionally, set
// 'prOut' to the non-SW path renderer that will do the job).
bool GrClipMaskManager::PathNeedsSWRenderer(GrContext* context,
                                            bool isStencilDisabled,
                                            const GrRenderTarget* rt,
                                            const SkMatrix& viewMatrix,
                                            const Element* element,
                                            GrPathRenderer** prOut,
                                            bool needsStencil) {
    if (Element::kRect_Type == element->getType()) {
        // rects can always be drawn directly w/o using the software path
        // TODO: skip rrects once we're drawing them directly.
        if (prOut) {
            *prOut = nullptr;
        }
        return false;
    } else {
        // We shouldn't get here with an empty clip element.
        SkASSERT(Element::kEmpty_Type != element->getType());

        // the gpu alpha mask will draw the inverse paths as non-inverse to a temp buffer
        SkPath path;
        element->asPath(&path);
        if (path.isInverseFillType()) {
            path.toggleInverseFillType();
        }
        GrStrokeInfo stroke(SkStrokeRec::kFill_InitStyle);
    
        GrPathRendererChain::DrawType type;
        
        if (needsStencil) {
            type = element->isAA()
                            ? GrPathRendererChain::kStencilAndColorAntiAlias_DrawType
                            : GrPathRendererChain::kStencilAndColor_DrawType;
        } else {
            type = element->isAA()
                            ? GrPathRendererChain::kColorAntiAlias_DrawType
                            : GrPathRendererChain::kColor_DrawType;    
        }
    
        GrPathRenderer::CanDrawPathArgs canDrawArgs;
        canDrawArgs.fShaderCaps = context->caps()->shaderCaps();
        canDrawArgs.fViewMatrix = &viewMatrix;
        canDrawArgs.fPath = &path;
        canDrawArgs.fStroke = &stroke;
        canDrawArgs.fAntiAlias = element->isAA();
        canDrawArgs.fIsStencilDisabled = isStencilDisabled;
        canDrawArgs.fIsStencilBufferMSAA = rt->isStencilBufferMultisampled();

        // the 'false' parameter disallows use of the SW path renderer
        GrPathRenderer* pr = context->drawingManager()->getPathRenderer(canDrawArgs, false, type);
        if (prOut) {
            *prOut = pr;
        }
        return SkToBool(!pr);
    }
}

// Determines whether it is possible to draw the element to both the stencil buffer and the
// alpha mask simultaneously. If so and the element is a path a compatible path renderer is
// also returned.
GrPathRenderer* GrClipMaskManager::GetPathRenderer(GrContext* context,
                                                   GrTexture* texture,
                                                   const SkMatrix& viewMatrix,
                                                   const SkClipStack::Element* element) {
    GrPathRenderer* pr;
    static const bool kNeedsStencil = true;
    static const bool kStencilIsDisabled = true;
    PathNeedsSWRenderer(context,
                        kStencilIsDisabled,
                        texture->asRenderTarget(),
                        viewMatrix,
                        element,
                        &pr,
                        kNeedsStencil);
    return pr;
}

GrClipMaskManager::GrClipMaskManager(GrDrawTarget* drawTarget, bool debugClipBatchToBounds)
    : fDrawTarget(drawTarget)
    , fClipMode(kIgnoreClip_StencilClipMode)
    , fDebugClipBatchToBounds(debugClipBatchToBounds) {
}

GrContext* GrClipMaskManager::getContext() {
    return fDrawTarget->cmmAccess().context();
}

const GrCaps* GrClipMaskManager::caps() const {
    return fDrawTarget->caps();
}

GrResourceProvider* GrClipMaskManager::resourceProvider() {
    return fDrawTarget->cmmAccess().resourceProvider();
}
/*
 * This method traverses the clip stack to see if the GrSoftwarePathRenderer
 * will be used on any element. If so, it returns true to indicate that the
 * entire clip should be rendered in SW and then uploaded en masse to the gpu.
 */
bool GrClipMaskManager::useSWOnlyPath(const GrPipelineBuilder& pipelineBuilder,
                                      const GrRenderTarget* rt,
                                      const SkVector& clipToMaskOffset,
                                      const GrReducedClip::ElementList& elements) {
    // TODO: generalize this function so that when
    // a clip gets complex enough it can just be done in SW regardless
    // of whether it would invoke the GrSoftwarePathRenderer.

    // Set the matrix so that rendered clip elements are transformed to mask space from clip
    // space.
    const SkMatrix translate = SkMatrix::MakeTrans(clipToMaskOffset.fX, clipToMaskOffset.fY);

    for (GrReducedClip::ElementList::Iter iter(elements.headIter()); iter.get(); iter.next()) {
        const Element* element = iter.get();

        SkRegion::Op op = element->getOp();
        bool invert = element->isInverseFilled();
        bool needsStencil = invert || 
                            SkRegion::kIntersect_Op == op || SkRegion::kReverseDifference_Op == op;

        if (PathNeedsSWRenderer(this->getContext(), pipelineBuilder.getStencil().isDisabled(),
                                rt, translate, element, nullptr, needsStencil)) {
            return true;
        }
    }
    return false;
}

bool GrClipMaskManager::getAnalyticClipProcessor(const GrReducedClip::ElementList& elements,
                                                 bool abortIfAA,
                                                 SkVector& clipToRTOffset,
                                                 const SkRect* drawBounds,
                                                 const GrFragmentProcessor** resultFP) {
    SkRect boundsInClipSpace;
    if (drawBounds) {
        boundsInClipSpace = *drawBounds;
        boundsInClipSpace.offset(-clipToRTOffset.fX, -clipToRTOffset.fY);
    }
    SkASSERT(elements.count() <= kMaxAnalyticElements);
    const GrFragmentProcessor* fps[kMaxAnalyticElements];
    for (int i = 0; i < kMaxAnalyticElements; ++i) {
        fps[i] = nullptr;
    }
    int fpCnt = 0;
    GrReducedClip::ElementList::Iter iter(elements);
    bool failed = false;
    while (iter.get()) {
        SkRegion::Op op = iter.get()->getOp();
        bool invert;
        bool skip = false;
        switch (op) {
            case SkRegion::kReplace_Op:
                SkASSERT(iter.get() == elements.head());
                // Fallthrough, handled same as intersect.
            case SkRegion::kIntersect_Op:
                invert = false;
                if (drawBounds && iter.get()->contains(boundsInClipSpace)) {
                    skip = true;
                }
                break;
            case SkRegion::kDifference_Op:
                invert = true;
                // We don't currently have a cheap test for whether a rect is fully outside an
                // element's primitive, so don't attempt to set skip.
                break;
            default:
                failed = true;
                break;
        }
        if (failed) {
            break;
        }
        if (!skip) {
            GrPrimitiveEdgeType edgeType;
            if (iter.get()->isAA()) {
                if (abortIfAA) {
                    failed = true;
                    break;
                }
                edgeType =
                    invert ? kInverseFillAA_GrProcessorEdgeType : kFillAA_GrProcessorEdgeType;
            } else {
                edgeType =
                    invert ? kInverseFillBW_GrProcessorEdgeType : kFillBW_GrProcessorEdgeType;
            }

            switch (iter.get()->getType()) {
                case SkClipStack::Element::kPath_Type:
                    fps[fpCnt] = GrConvexPolyEffect::Create(edgeType, iter.get()->getPath(),
                                                            &clipToRTOffset);
                    break;
                case SkClipStack::Element::kRRect_Type: {
                    SkRRect rrect = iter.get()->getRRect();
                    rrect.offset(clipToRTOffset.fX, clipToRTOffset.fY);
                    fps[fpCnt] = GrRRectEffect::Create(edgeType, rrect);
                    break;
                }
                case SkClipStack::Element::kRect_Type: {
                    SkRect rect = iter.get()->getRect();
                    rect.offset(clipToRTOffset.fX, clipToRTOffset.fY);
                    fps[fpCnt] = GrConvexPolyEffect::Create(edgeType, rect);
                    break;
                }
                default:
                    break;
            }
            if (!fps[fpCnt]) {
                failed = true;
                break;
            }
            fpCnt++;
        }
        iter.next();
    }

    *resultFP = nullptr;
    if (!failed && fpCnt) {
        *resultFP = GrFragmentProcessor::RunInSeries(fps, fpCnt);
    }
    for (int i = 0; i < fpCnt; ++i) {
        fps[i]->unref();
    }
    return !failed;
}

static void add_rect_to_clip(const GrClip& clip, const SkRect& devRect, GrClip* out) {
    switch (clip.clipType()) {
        case GrClip::kClipStack_ClipType: {
            SkClipStack* stack = new SkClipStack;
            *stack = *clip.clipStack();
            // The stack is actually in clip space not device space.
            SkRect clipRect = devRect;
            SkPoint origin = { SkIntToScalar(clip.origin().fX), SkIntToScalar(clip.origin().fY) };
            clipRect.offset(origin);
            SkIRect iclipRect;
            clipRect.roundOut(&iclipRect);
            clipRect = SkRect::Make(iclipRect);
            stack->clipDevRect(clipRect, SkRegion::kIntersect_Op, false);
            out->setClipStack(stack, &clip.origin());
            break;
        }
        case GrClip::kWideOpen_ClipType:
            *out = GrClip(devRect);
            break;
        case GrClip::kIRect_ClipType: {
            SkIRect intersect;
            devRect.roundOut(&intersect);
            if (intersect.intersect(clip.irect())) {
                *out = GrClip(intersect);
            } else {
                *out = clip;
            }
            break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// sort out what kind of clip mask needs to be created: alpha, stencil,
// scissor, or entirely software
bool GrClipMaskManager::setupClipping(const GrPipelineBuilder& pipelineBuilder,
                                      GrPipelineBuilder::AutoRestoreStencil* ars,
                                      const SkRect* devBounds,
                                      GrAppliedClip* out) {
    if (kRespectClip_StencilClipMode == fClipMode) {
        fClipMode = kIgnoreClip_StencilClipMode;
    }

    GrReducedClip::ElementList elements;
    int32_t genID = 0;
    GrReducedClip::InitialState initialState = GrReducedClip::kAllIn_InitialState;
    SkIRect clipSpaceIBounds;
    bool requiresAA = false;
    GrRenderTarget* rt = pipelineBuilder.getRenderTarget();

    // GrDrawTarget should have filtered this for us
    SkASSERT(rt);

    SkIRect clipSpaceRTIBounds = SkIRect::MakeWH(rt->width(), rt->height());
    GrClip devBoundsClip;
    bool doDevBoundsClip = fDebugClipBatchToBounds && devBounds;
    if (doDevBoundsClip) {
        add_rect_to_clip(pipelineBuilder.clip(), *devBounds, &devBoundsClip);
    }
    const GrClip& clip = doDevBoundsClip ? devBoundsClip : pipelineBuilder.clip();

    if (clip.isWideOpen(clipSpaceRTIBounds)) {
        this->setPipelineBuilderStencil(pipelineBuilder, ars);
        return true;
    }

    // The clip mask manager always draws with a single IRect so we special case that logic here
    // Image filters just use a rect, so we also special case that logic
    switch (clip.clipType()) {
        case GrClip::kWideOpen_ClipType:
            SkFAIL("Should have caught this with clip.isWideOpen()");
            return true;
        case GrClip::kIRect_ClipType: {
            SkIRect scissor = clip.irect();
            if (scissor.intersect(clipSpaceRTIBounds)) {
                out->fScissorState.set(scissor);
                this->setPipelineBuilderStencil(pipelineBuilder, ars);
                return true;
            }
            return false;
        }
        case GrClip::kClipStack_ClipType: {
            clipSpaceRTIBounds.offset(clip.origin());
            SkIRect clipSpaceReduceQueryBounds;
#define DISABLE_DEV_BOUNDS_FOR_CLIP_REDUCTION 1
            if (devBounds && !DISABLE_DEV_BOUNDS_FOR_CLIP_REDUCTION) {
                SkIRect devIBounds = devBounds->roundOut();
                devIBounds.offset(clip.origin());
                if (!clipSpaceReduceQueryBounds.intersect(clipSpaceRTIBounds, devIBounds)) {
                    return false;
                }
            } else {
                clipSpaceReduceQueryBounds = clipSpaceRTIBounds;
            }
            GrReducedClip::ReduceClipStack(*clip.clipStack(),
                                            clipSpaceReduceQueryBounds,
                                            &elements,
                                            &genID,
                                            &initialState,
                                            &clipSpaceIBounds,
                                            &requiresAA);
            if (elements.isEmpty()) {
                if (GrReducedClip::kAllIn_InitialState == initialState) {
                    if (clipSpaceIBounds == clipSpaceRTIBounds) {
                        this->setPipelineBuilderStencil(pipelineBuilder, ars);
                        return true;
                    }
                } else {
                    return false;
                }
            }
        } break;
    }

    // An element count of 4 was chosen because of the common pattern in Blink of:
    //   isect RR
    //   diff  RR
    //   isect convex_poly
    //   isect convex_poly
    // when drawing rounded div borders. This could probably be tuned based on a
    // configuration's relative costs of switching RTs to generate a mask vs
    // longer shaders.
    if (elements.count() <= kMaxAnalyticElements) {
        SkVector clipToRTOffset = { SkIntToScalar(-clip.origin().fX),
                                    SkIntToScalar(-clip.origin().fY) };
        // When there are multiple samples we want to do per-sample clipping, not compute a
        // fractional pixel coverage.
        bool disallowAnalyticAA = rt->isUnifiedMultisampled() || pipelineBuilder.hasMixedSamples();
        const GrFragmentProcessor* clipFP = nullptr;
        if (elements.isEmpty() ||
            (requiresAA &&
             this->getAnalyticClipProcessor(elements, disallowAnalyticAA, clipToRTOffset, devBounds,
                                            &clipFP))) {
            SkIRect scissorSpaceIBounds(clipSpaceIBounds);
            scissorSpaceIBounds.offset(-clip.origin());
            if (nullptr == devBounds ||
                !SkRect::Make(scissorSpaceIBounds).contains(*devBounds)) {
                out->fScissorState.set(scissorSpaceIBounds);
            }
            this->setPipelineBuilderStencil(pipelineBuilder, ars);
            out->fClipCoverageFP.reset(clipFP);
            return true;
        }
    }

    // If the stencil buffer is multisampled we can use it to do everything.
    if (!rt->isStencilBufferMultisampled() && requiresAA) {
        SkAutoTUnref<GrTexture> result;

        // The top-left of the mask corresponds to the top-left corner of the bounds.
        SkVector clipToMaskOffset = {
            SkIntToScalar(-clipSpaceIBounds.fLeft),
            SkIntToScalar(-clipSpaceIBounds.fTop)
        };

        if (this->useSWOnlyPath(pipelineBuilder, rt, clipToMaskOffset, elements)) {
            // The clip geometry is complex enough that it will be more efficient to create it
            // entirely in software
            result.reset(this->createSoftwareClipMask(genID,
                                                      initialState,
                                                      elements,
                                                      clipToMaskOffset,
                                                      clipSpaceIBounds));
        } else {
            result.reset(this->createAlphaClipMask(genID,
                                                   initialState,
                                                   elements,
                                                   clipToMaskOffset,
                                                   clipSpaceIBounds));
            // If createAlphaClipMask fails it means useSWOnlyPath has a bug
            SkASSERT(result);
        }

        if (result) {
            // The mask's top left coord should be pinned to the rounded-out top left corner of
            // clipSpace bounds. We determine the mask's position WRT to the render target here.
            SkIRect rtSpaceMaskBounds = clipSpaceIBounds;
            rtSpaceMaskBounds.offset(-clip.origin());
            out->fClipCoverageFP.reset(create_fp_for_mask(result, rtSpaceMaskBounds));
            this->setPipelineBuilderStencil(pipelineBuilder, ars);
            return true;
        }
        // if alpha clip mask creation fails fall through to the non-AA code paths
    }

    // use the stencil clip if we can't represent the clip as a rectangle.
    SkIPoint clipSpaceToStencilSpaceOffset = -clip.origin();
    this->createStencilClipMask(rt,
                                genID,
                                initialState,
                                elements,
                                clipSpaceIBounds,
                                clipSpaceToStencilSpaceOffset);

    // This must occur after createStencilClipMask. That function may change the scissor. Also, it
    // only guarantees that the stencil mask is correct within the bounds it was passed, so we must
    // use both stencil and scissor test to the bounds for the final draw.
    SkIRect scissorSpaceIBounds(clipSpaceIBounds);
    scissorSpaceIBounds.offset(clipSpaceToStencilSpaceOffset);
    out->fScissorState.set(scissorSpaceIBounds);
    this->setPipelineBuilderStencil(pipelineBuilder, ars);
    return true;
}

namespace {
////////////////////////////////////////////////////////////////////////////////
// Set a coverage drawing XPF on the pipelineBuilder for the given op and invertCoverage mode
void set_coverage_drawing_xpf(SkRegion::Op op, bool invertCoverage,
                              GrPipelineBuilder* pipelineBuilder) {
    SkASSERT(op <= SkRegion::kLastOp);
    pipelineBuilder->setCoverageSetOpXPFactory(op, invertCoverage);
}
}

////////////////////////////////////////////////////////////////////////////////
bool GrClipMaskManager::drawElement(GrPipelineBuilder* pipelineBuilder,
                                    const SkMatrix& viewMatrix,
                                    GrTexture* target,
                                    const SkClipStack::Element* element,
                                    GrPathRenderer* pr) {

    GrRenderTarget* rt = target->asRenderTarget();
    pipelineBuilder->setRenderTarget(rt);

    // The color we use to draw does not matter since we will always be using a GrCoverageSetOpXP
    // which ignores color.
    GrColor color = GrColor_WHITE;

    // TODO: Draw rrects directly here.
    switch (element->getType()) {
        case Element::kEmpty_Type:
            SkDEBUGFAIL("Should never get here with an empty element.");
            break;
        case Element::kRect_Type:
            // TODO: Do rects directly to the accumulator using a aa-rect GrProcessor that covers
            // the entire mask bounds and writes 0 outside the rect.
            if (element->isAA()) {
                SkRect devRect = element->getRect();
                viewMatrix.mapRect(&devRect);

                fDrawTarget->drawAARect(*pipelineBuilder, color, viewMatrix,
                                        element->getRect(), devRect);
            } else {
                fDrawTarget->drawNonAARect(*pipelineBuilder, color, viewMatrix,
                                           element->getRect());
            }
            return true;
        default: {
            SkPath path;
            element->asPath(&path);
            if (path.isInverseFillType()) {
                path.toggleInverseFillType();
            }
            GrStrokeInfo stroke(SkStrokeRec::kFill_InitStyle);
            if (nullptr == pr) {
                GrPathRendererChain::DrawType type;
                type = element->isAA() ? GrPathRendererChain::kColorAntiAlias_DrawType :
                                         GrPathRendererChain::kColor_DrawType;

                GrPathRenderer::CanDrawPathArgs canDrawArgs;
                canDrawArgs.fShaderCaps = this->getContext()->caps()->shaderCaps();
                canDrawArgs.fViewMatrix = &viewMatrix;
                canDrawArgs.fPath = &path;
                canDrawArgs.fStroke = &stroke;
                canDrawArgs.fAntiAlias = element->isAA();;
                canDrawArgs.fIsStencilDisabled = pipelineBuilder->getStencil().isDisabled();
                canDrawArgs.fIsStencilBufferMSAA = rt->isStencilBufferMultisampled();

                pr = this->getContext()->drawingManager()->getPathRenderer(canDrawArgs, false, type);
            }
            if (nullptr == pr) {
                return false;
            }
            GrPathRenderer::DrawPathArgs args;
            args.fTarget = fDrawTarget;
            args.fResourceProvider = this->getContext()->resourceProvider();
            args.fPipelineBuilder = pipelineBuilder;
            args.fColor = color;
            args.fViewMatrix = &viewMatrix;
            args.fPath = &path;
            args.fStroke = &stroke;
            args.fAntiAlias = element->isAA();
            pr->drawPath(args);
            break;
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Create a 8-bit clip mask in alpha

static void GetClipMaskKey(int32_t clipGenID, const SkIRect& bounds, GrUniqueKey* key) {
    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey::Builder builder(key, kDomain, 3);
    builder[0] = clipGenID;
    builder[1] = SkToU16(bounds.fLeft) | (SkToU16(bounds.fRight) << 16);
    builder[2] = SkToU16(bounds.fTop) | (SkToU16(bounds.fBottom) << 16);
}

GrTexture* GrClipMaskManager::createCachedMask(int width, int height, const GrUniqueKey& key,
                                               bool renderTarget) {
    GrSurfaceDesc desc;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fFlags = renderTarget ? kRenderTarget_GrSurfaceFlag : kNone_GrSurfaceFlags;
    if (!renderTarget || this->caps()->isConfigRenderable(kAlpha_8_GrPixelConfig, false)) {
        desc.fConfig = kAlpha_8_GrPixelConfig;
    } else {
        desc.fConfig = kRGBA_8888_GrPixelConfig;
    }

    GrTexture* texture = this->resourceProvider()->createApproxTexture(desc, 0);
    if (!texture) {
        return nullptr;
    }
    texture->resourcePriv().setUniqueKey(key);
    return texture;
}

GrTexture* GrClipMaskManager::createAlphaClipMask(int32_t elementsGenID,
                                                  GrReducedClip::InitialState initialState,
                                                  const GrReducedClip::ElementList& elements,
                                                  const SkVector& clipToMaskOffset,
                                                  const SkIRect& clipSpaceIBounds) {
    GrResourceProvider* resourceProvider = this->resourceProvider();
    GrUniqueKey key;
    GetClipMaskKey(elementsGenID, clipSpaceIBounds, &key);
    if (GrTexture* texture = resourceProvider->findAndRefTextureByUniqueKey(key)) {
        return texture;
    }

    // There's no texture in the cache. Let's try to allocate it then.
    SkAutoTUnref<GrTexture> texture(this->createCachedMask(
        clipSpaceIBounds.width(), clipSpaceIBounds.height(), key, true));
    if (!texture) {
        return nullptr;
    }

    // Set the matrix so that rendered clip elements are transformed to mask space from clip
    // space.
    const SkMatrix translate = SkMatrix::MakeTrans(clipToMaskOffset.fX, clipToMaskOffset.fY);

    // The texture may be larger than necessary, this rect represents the part of the texture
    // we populate with a rasterization of the clip.
    SkIRect maskSpaceIBounds = SkIRect::MakeWH(clipSpaceIBounds.width(), clipSpaceIBounds.height());

    // The scratch texture that we are drawing into can be substantially larger than the mask. Only
    // clear the part that we care about.
    fDrawTarget->clear(&maskSpaceIBounds,
                       GrReducedClip::kAllIn_InitialState == initialState ? 0xffffffff : 0x00000000,
                       true,
                       texture->asRenderTarget());

    // When we use the stencil in the below loop it is important to have this clip installed.
    // The second pass that zeros the stencil buffer renders the rect maskSpaceIBounds so the first
    // pass must not set values outside of this bounds or stencil values outside the rect won't be
    // cleared.
    const GrClip clip(maskSpaceIBounds);

    // walk through each clip element and perform its set op
    for (GrReducedClip::ElementList::Iter iter = elements.headIter(); iter.get(); iter.next()) {
        const Element* element = iter.get();
        SkRegion::Op op = element->getOp();
        bool invert = element->isInverseFilled();
        if (invert || SkRegion::kIntersect_Op == op || SkRegion::kReverseDifference_Op == op) {

            GrPathRenderer* pr = GetPathRenderer(this->getContext(),
                                                 texture, translate, element);
            if (Element::kRect_Type != element->getType() && !pr) {
                // useSWOnlyPath should now filter out all cases where gpu-side mask merging would
                // be performed (i.e., pr would be NULL for a non-rect path). See https://bug.skia.org/4519
                // for rationale and details.
                SkASSERT(0);
                continue;
            }

            {
                GrPipelineBuilder pipelineBuilder;

                pipelineBuilder.setClip(clip);
                pipelineBuilder.setRenderTarget(texture->asRenderTarget());
                SkASSERT(pipelineBuilder.getStencil().isDisabled());

                // draw directly into the result with the stencil set to make the pixels affected
                // by the clip shape be non-zero.
                GR_STATIC_CONST_SAME_STENCIL(kStencilInElement,
                                             kReplace_StencilOp,
                                             kReplace_StencilOp,
                                             kAlways_StencilFunc,
                                             0xffff,
                                             0xffff,
                                             0xffff);
                pipelineBuilder.setStencil(kStencilInElement);
                set_coverage_drawing_xpf(op, invert, &pipelineBuilder);

                if (!this->drawElement(&pipelineBuilder, translate, texture, element, pr)) {
                    texture->resourcePriv().removeUniqueKey();
                    return nullptr;
                }
            }

            {
                GrPipelineBuilder backgroundPipelineBuilder;
                backgroundPipelineBuilder.setRenderTarget(texture->asRenderTarget());

                set_coverage_drawing_xpf(op, !invert, &backgroundPipelineBuilder);
                // Draw to the exterior pixels (those with a zero stencil value).
                GR_STATIC_CONST_SAME_STENCIL(kDrawOutsideElement,
                                             kZero_StencilOp,
                                             kZero_StencilOp,
                                             kEqual_StencilFunc,
                                             0xffff,
                                             0x0000,
                                             0xffff);
                backgroundPipelineBuilder.setStencil(kDrawOutsideElement);

                // The color passed in here does not matter since the coverageSetOpXP won't read it.
                fDrawTarget->drawNonAARect(backgroundPipelineBuilder, GrColor_WHITE, translate,
                                           clipSpaceIBounds);
            }
        } else {
            GrPipelineBuilder pipelineBuilder;

            // all the remaining ops can just be directly draw into the accumulation buffer
            set_coverage_drawing_xpf(op, false, &pipelineBuilder);
            // The color passed in here does not matter since the coverageSetOpXP won't read it.
            this->drawElement(&pipelineBuilder, translate, texture, element);
        }
    }

    return texture.detach();
}

////////////////////////////////////////////////////////////////////////////////
// Create a 1-bit clip mask in the stencil buffer. 'devClipBounds' are in device
// (as opposed to canvas) coordinates
bool GrClipMaskManager::createStencilClipMask(GrRenderTarget* rt,
                                              int32_t elementsGenID,
                                              GrReducedClip::InitialState initialState,
                                              const GrReducedClip::ElementList& elements,
                                              const SkIRect& clipSpaceIBounds,
                                              const SkIPoint& clipSpaceToStencilOffset) {
    SkASSERT(rt);

    GrStencilAttachment* stencilAttachment = this->resourceProvider()->attachStencilAttachment(rt);
    if (nullptr == stencilAttachment) {
        return false;
    }

    if (stencilAttachment->mustRenderClip(elementsGenID, clipSpaceIBounds, clipSpaceToStencilOffset)) {
        stencilAttachment->setLastClip(elementsGenID, clipSpaceIBounds, clipSpaceToStencilOffset);
        // Set the matrix so that rendered clip elements are transformed from clip to stencil space.
        SkVector translate = {
            SkIntToScalar(clipSpaceToStencilOffset.fX),
            SkIntToScalar(clipSpaceToStencilOffset.fY)
        };
        SkMatrix viewMatrix;
        viewMatrix.setTranslate(translate);

        // We set the current clip to the bounds so that our recursive draws are scissored to them.
        SkIRect stencilSpaceIBounds(clipSpaceIBounds);
        stencilSpaceIBounds.offset(clipSpaceToStencilOffset);
        GrClip clip(stencilSpaceIBounds);

        int clipBit = stencilAttachment->bits();
        SkASSERT((clipBit <= 16) && "Ganesh only handles 16b or smaller stencil buffers");
        clipBit = (1 << (clipBit-1));

        fDrawTarget->cmmAccess().clearStencilClip(stencilSpaceIBounds,
            GrReducedClip::kAllIn_InitialState == initialState, rt);

        // walk through each clip element and perform its set op
        // with the existing clip.
        for (GrReducedClip::ElementList::Iter iter(elements.headIter()); iter.get(); iter.next()) {
            const Element* element = iter.get();

            GrPipelineBuilder pipelineBuilder;
            pipelineBuilder.setClip(clip);
            pipelineBuilder.setRenderTarget(rt);

            pipelineBuilder.setDisableColorXPFactory();

            // if the target is MSAA then we want MSAA enabled when the clip is soft
            if (rt->isStencilBufferMultisampled()) {
                pipelineBuilder.setState(GrPipelineBuilder::kHWAntialias_Flag, element->isAA());
            }

            bool fillInverted = false;
            // enabled at bottom of loop
            fClipMode = kIgnoreClip_StencilClipMode;

            // This will be used to determine whether the clip shape can be rendered into the
            // stencil with arbitrary stencil settings.
            GrPathRenderer::StencilSupport stencilSupport;

            GrStrokeInfo stroke(SkStrokeRec::kFill_InitStyle);
            SkRegion::Op op = element->getOp();

            GrPathRenderer* pr = nullptr;
            SkPath clipPath;
            if (Element::kRect_Type == element->getType()) {
                stencilSupport = GrPathRenderer::kNoRestriction_StencilSupport;
                fillInverted = false;
            } else {
                element->asPath(&clipPath);
                fillInverted = clipPath.isInverseFillType();
                if (fillInverted) {
                    clipPath.toggleInverseFillType();
                }

                SkASSERT(pipelineBuilder.getStencil().isDisabled());

                GrPathRenderer::CanDrawPathArgs canDrawArgs;
                canDrawArgs.fShaderCaps = this->getContext()->caps()->shaderCaps();
                canDrawArgs.fViewMatrix = &viewMatrix;
                canDrawArgs.fPath = &clipPath;
                canDrawArgs.fStroke = &stroke;
                canDrawArgs.fAntiAlias = false;
                canDrawArgs.fIsStencilDisabled = pipelineBuilder.getStencil().isDisabled();
                canDrawArgs.fIsStencilBufferMSAA = rt->isStencilBufferMultisampled();

                pr = this->getContext()->drawingManager()->getPathRenderer(canDrawArgs, false,
                                                                           GrPathRendererChain::kStencilOnly_DrawType,
                                                                           &stencilSupport);
                if (nullptr == pr) {
                    return false;
                }
            }

            int passes;
            GrStencilSettings stencilSettings[GrStencilSettings::kMaxStencilClipPasses];

            bool canRenderDirectToStencil =
                GrPathRenderer::kNoRestriction_StencilSupport == stencilSupport;
            bool canDrawDirectToClip; // Given the renderer, the element,
                                      // fill rule, and set operation can
                                      // we render the element directly to
                                      // stencil bit used for clipping.
            canDrawDirectToClip = GrStencilSettings::GetClipPasses(op,
                                                                   canRenderDirectToStencil,
                                                                   clipBit,
                                                                   fillInverted,
                                                                   &passes,
                                                                   stencilSettings);

            // draw the element to the client stencil bits if necessary
            if (!canDrawDirectToClip) {
                GR_STATIC_CONST_SAME_STENCIL(gDrawToStencil,
                                             kIncClamp_StencilOp,
                                             kIncClamp_StencilOp,
                                             kAlways_StencilFunc,
                                             0xffff,
                                             0x0000,
                                             0xffff);
                if (Element::kRect_Type == element->getType()) {
                    *pipelineBuilder.stencil() = gDrawToStencil;

                    // We need this AGP until everything is in GrBatch
                    fDrawTarget->drawNonAARect(pipelineBuilder,
                                               GrColor_WHITE,
                                               viewMatrix,
                                               element->getRect());
                } else {
                    if (!clipPath.isEmpty()) {
                        if (canRenderDirectToStencil) {
                            *pipelineBuilder.stencil() = gDrawToStencil;

                            GrPathRenderer::DrawPathArgs args;
                            args.fTarget = fDrawTarget;
                            args.fResourceProvider = this->getContext()->resourceProvider();
                            args.fPipelineBuilder = &pipelineBuilder;
                            args.fColor = GrColor_WHITE;
                            args.fViewMatrix = &viewMatrix;
                            args.fPath = &clipPath;
                            args.fStroke = &stroke;
                            args.fAntiAlias = false;
                            pr->drawPath(args);
                        } else {
                            GrPathRenderer::StencilPathArgs args;
                            args.fTarget = fDrawTarget;
                            args.fResourceProvider = this->getContext()->resourceProvider();
                            args.fPipelineBuilder = &pipelineBuilder;
                            args.fViewMatrix = &viewMatrix;
                            args.fPath = &clipPath;
                            args.fStroke = &stroke;
                            pr->stencilPath(args);
                        }
                    }
                }
            }

            // now we modify the clip bit by rendering either the clip
            // element directly or a bounding rect of the entire clip.
            fClipMode = kModifyClip_StencilClipMode;
            for (int p = 0; p < passes; ++p) {
                *pipelineBuilder.stencil() = stencilSettings[p];

                if (canDrawDirectToClip) {
                    if (Element::kRect_Type == element->getType()) {
                        // We need this AGP until everything is in GrBatch
                        fDrawTarget->drawNonAARect(pipelineBuilder,
                                                   GrColor_WHITE,
                                                   viewMatrix,
                                                   element->getRect());
                    } else {
                        GrPathRenderer::DrawPathArgs args;
                        args.fTarget = fDrawTarget;
                        args.fResourceProvider = this->getContext()->resourceProvider();
                        args.fPipelineBuilder = &pipelineBuilder;
                        args.fColor = GrColor_WHITE;
                        args.fViewMatrix = &viewMatrix;
                        args.fPath = &clipPath;
                        args.fStroke = &stroke;
                        args.fAntiAlias = false;
                        pr->drawPath(args);
                    }
                } else {
                    // The view matrix is setup to do clip space -> stencil space translation, so
                    // draw rect in clip space.
                    fDrawTarget->drawNonAARect(pipelineBuilder,
                                               GrColor_WHITE,
                                               viewMatrix,
                                               SkRect::Make(clipSpaceIBounds));
                }
            }
        }
    }
    fClipMode = kRespectClip_StencilClipMode;
    return true;
}

// mapping of clip-respecting stencil funcs to normal stencil funcs
// mapping depends on whether stencil-clipping is in effect.
static const GrStencilFunc
    gSpecialToBasicStencilFunc[2][kClipStencilFuncCount] = {
    {// Stencil-Clipping is DISABLED,  we are effectively always inside the clip
        // In the Clip Funcs
        kAlways_StencilFunc,          // kAlwaysIfInClip_StencilFunc
        kEqual_StencilFunc,           // kEqualIfInClip_StencilFunc
        kLess_StencilFunc,            // kLessIfInClip_StencilFunc
        kLEqual_StencilFunc,          // kLEqualIfInClip_StencilFunc
        // Special in the clip func that forces user's ref to be 0.
        kNotEqual_StencilFunc,        // kNonZeroIfInClip_StencilFunc
                                      // make ref 0 and do normal nequal.
    },
    {// Stencil-Clipping is ENABLED
        // In the Clip Funcs
        kEqual_StencilFunc,           // kAlwaysIfInClip_StencilFunc
                                      // eq stencil clip bit, mask
                                      // out user bits.

        kEqual_StencilFunc,           // kEqualIfInClip_StencilFunc
                                      // add stencil bit to mask and ref

        kLess_StencilFunc,            // kLessIfInClip_StencilFunc
        kLEqual_StencilFunc,          // kLEqualIfInClip_StencilFunc
                                      // for both of these we can add
                                      // the clip bit to the mask and
                                      // ref and compare as normal
        // Special in the clip func that forces user's ref to be 0.
        kLess_StencilFunc,            // kNonZeroIfInClip_StencilFunc
                                      // make ref have only the clip bit set
                                      // and make comparison be less
                                      // 10..0 < 1..user_bits..
    }
};

namespace {
// Sets the settings to clip against the stencil buffer clip while ignoring the
// client bits.
const GrStencilSettings& basic_apply_stencil_clip_settings() {
    // stencil settings to use when clip is in stencil
    GR_STATIC_CONST_SAME_STENCIL_STRUCT(gSettings,
        kKeep_StencilOp,
        kKeep_StencilOp,
        kAlwaysIfInClip_StencilFunc,
        0x0000,
        0x0000,
        0x0000);
    return *GR_CONST_STENCIL_SETTINGS_PTR_FROM_STRUCT_PTR(&gSettings);
}
}

void GrClipMaskManager::setPipelineBuilderStencil(const GrPipelineBuilder& pipelineBuilder,
                                                  GrPipelineBuilder::AutoRestoreStencil* ars) {
    // We make two copies of the StencilSettings here (except in the early
    // exit scenario. One copy from draw state to the stack var. Then another
    // from the stack var to the gpu. We could make this class hold a ptr to
    // GrGpu's fStencilSettings and eliminate the stack copy here.

    // use stencil for clipping if clipping is enabled and the clip
    // has been written into the stencil.
    GrStencilSettings settings;

    // The GrGpu client may not be using the stencil buffer but we may need to
    // enable it in order to respect a stencil clip.
    if (pipelineBuilder.getStencil().isDisabled()) {
        if (GrClipMaskManager::kRespectClip_StencilClipMode == fClipMode) {
            settings = basic_apply_stencil_clip_settings();
        } else {
            return;
        }
    } else {
        settings = pipelineBuilder.getStencil();
    }

    int stencilBits = 0;
    GrRenderTarget* rt = pipelineBuilder.getRenderTarget();
    GrStencilAttachment* stencilAttachment = this->resourceProvider()->attachStencilAttachment(rt);
    if (stencilAttachment) {
        stencilBits = stencilAttachment->bits();
    }

    SkASSERT(this->caps()->stencilWrapOpsSupport() || !settings.usesWrapOp());
    SkASSERT(this->caps()->twoSidedStencilSupport() || !settings.isTwoSided());
    this->adjustStencilParams(&settings, fClipMode, stencilBits);
    ars->set(&pipelineBuilder);
    ars->setStencil(settings);
}

void GrClipMaskManager::adjustStencilParams(GrStencilSettings* settings,
                                            StencilClipMode mode,
                                            int stencilBitCnt) {
    SkASSERT(stencilBitCnt > 0);

    if (kModifyClip_StencilClipMode == mode) {
        // We assume that this clip manager itself is drawing to the GrGpu and
        // has already setup the correct values.
        return;
    }

    unsigned int clipBit = (1 << (stencilBitCnt - 1));
    unsigned int userBits = clipBit - 1;

    GrStencilSettings::Face face = GrStencilSettings::kFront_Face;
    bool twoSided = this->caps()->twoSidedStencilSupport();

    bool finished = false;
    while (!finished) {
        GrStencilFunc func = settings->func(face);
        uint16_t writeMask = settings->writeMask(face);
        uint16_t funcMask = settings->funcMask(face);
        uint16_t funcRef = settings->funcRef(face);

        SkASSERT((unsigned) func < kStencilFuncCount);

        writeMask &= userBits;

        if (func >= kBasicStencilFuncCount) {
            int respectClip = kRespectClip_StencilClipMode == mode;
            if (respectClip) {
                switch (func) {
                    case kAlwaysIfInClip_StencilFunc:
                        funcMask = clipBit;
                        funcRef = clipBit;
                        break;
                    case kEqualIfInClip_StencilFunc:
                    case kLessIfInClip_StencilFunc:
                    case kLEqualIfInClip_StencilFunc:
                        funcMask = (funcMask & userBits) | clipBit;
                        funcRef  = (funcRef  & userBits) | clipBit;
                        break;
                    case kNonZeroIfInClip_StencilFunc:
                        funcMask = (funcMask & userBits) | clipBit;
                        funcRef = clipBit;
                        break;
                    default:
                        SkFAIL("Unknown stencil func");
                }
            } else {
                funcMask &= userBits;
                funcRef &= userBits;
            }
            const GrStencilFunc* table =
                gSpecialToBasicStencilFunc[respectClip];
            func = table[func - kBasicStencilFuncCount];
            SkASSERT(func >= 0 && func < kBasicStencilFuncCount);
        } else {
            funcMask &= userBits;
            funcRef &= userBits;
        }

        settings->setFunc(face, func);
        settings->setWriteMask(face, writeMask);
        settings->setFuncMask(face, funcMask);
        settings->setFuncRef(face, funcRef);

        if (GrStencilSettings::kFront_Face == face) {
            face = GrStencilSettings::kBack_Face;
            finished = !twoSided;
        } else {
            finished = true;
        }
    }
    if (!twoSided) {
        settings->copyFrontSettingsToBack();
    }
}

////////////////////////////////////////////////////////////////////////////////
GrTexture* GrClipMaskManager::createSoftwareClipMask(int32_t elementsGenID,
                                                     GrReducedClip::InitialState initialState,
                                                     const GrReducedClip::ElementList& elements,
                                                     const SkVector& clipToMaskOffset,
                                                     const SkIRect& clipSpaceIBounds) {
    GrUniqueKey key;
    GetClipMaskKey(elementsGenID, clipSpaceIBounds, &key);
    GrResourceProvider* resourceProvider = this->resourceProvider();
    if (GrTexture* texture = resourceProvider->findAndRefTextureByUniqueKey(key)) {
        return texture;
    }

    // The mask texture may be larger than necessary. We round out the clip space bounds and pin
    // the top left corner of the resulting rect to the top left of the texture.
    SkIRect maskSpaceIBounds = SkIRect::MakeWH(clipSpaceIBounds.width(), clipSpaceIBounds.height());

    GrSWMaskHelper helper(this->getContext());

    // Set the matrix so that rendered clip elements are transformed to mask space from clip
    // space.
    SkMatrix translate;
    translate.setTranslate(clipToMaskOffset);

    helper.init(maskSpaceIBounds, &translate, false);
    helper.clear(GrReducedClip::kAllIn_InitialState == initialState ? 0xFF : 0x00);
    SkStrokeRec stroke(SkStrokeRec::kFill_InitStyle);

    for (GrReducedClip::ElementList::Iter iter(elements.headIter()) ; iter.get(); iter.next()) {
        const Element* element = iter.get();
        SkRegion::Op op = element->getOp();

        if (SkRegion::kIntersect_Op == op || SkRegion::kReverseDifference_Op == op) {
            // Intersect and reverse difference require modifying pixels outside of the geometry
            // that is being "drawn". In both cases we erase all the pixels outside of the geometry
            // but leave the pixels inside the geometry alone. For reverse difference we invert all
            // the pixels before clearing the ones outside the geometry.
            if (SkRegion::kReverseDifference_Op == op) {
                SkRect temp = SkRect::Make(clipSpaceIBounds);
                // invert the entire scene
                helper.draw(temp, SkRegion::kXOR_Op, false, 0xFF);
            }
            SkPath clipPath;
            element->asPath(&clipPath);
            clipPath.toggleInverseFillType();
            helper.draw(clipPath, stroke, SkRegion::kReplace_Op, element->isAA(), 0x00);
            continue;
        }

        // The other ops (union, xor, diff) only affect pixels inside
        // the geometry so they can just be drawn normally
        if (Element::kRect_Type == element->getType()) {
            helper.draw(element->getRect(), op, element->isAA(), 0xFF);
        } else {
            SkPath path;
            element->asPath(&path);
            helper.draw(path, stroke, op, element->isAA(), 0xFF);
        }
    }

    // Allocate clip mask texture
    GrTexture* result = this->createCachedMask(clipSpaceIBounds.width(), clipSpaceIBounds.height(),
                                               key, false);
    if (nullptr == result) {
        return nullptr;
    }
    helper.toTexture(result);

    return result;
}

////////////////////////////////////////////////////////////////////////////////

void GrClipMaskManager::adjustPathStencilParams(const GrStencilAttachment* stencilAttachment,
                                                GrStencilSettings* settings) {
    if (stencilAttachment) {
        int stencilBits = stencilAttachment->bits();
        this->adjustStencilParams(settings, fClipMode, stencilBits);
    }
}
