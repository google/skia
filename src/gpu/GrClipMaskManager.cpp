/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrClipMaskManager.h"
#include "GrCaps.h"
#include "GrDrawingManager.h"
#include "GrDrawContextPriv.h"
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
#include "batches/GrRectBatchFactory.h"
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

static void draw_non_aa_rect(GrDrawTarget* drawTarget,
                             const GrPipelineBuilder& pipelineBuilder,
                             const GrClip& clip,
                             GrColor color,
                             const SkMatrix& viewMatrix,
                             const SkRect& rect) {
    SkAutoTUnref<GrDrawBatch> batch(GrRectBatchFactory::CreateNonAAFill(color, viewMatrix, rect,
                                                                        nullptr, nullptr));
    drawTarget->drawBatch(pipelineBuilder, clip, batch);
}

// Does the path in 'element' require SW rendering? If so, return true (and,
// optionally, set 'prOut' to NULL. If not, return false (and, optionally, set
// 'prOut' to the non-SW path renderer that will do the job).
bool GrClipMaskManager::PathNeedsSWRenderer(GrContext* context,
                                            bool hasUserStencilSettings,
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
        canDrawArgs.fStyle = &GrStyle::SimpleFill();
        canDrawArgs.fAntiAlias = element->isAA();
        canDrawArgs.fHasUserStencilSettings = hasUserStencilSettings;
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
    constexpr bool kNeedsStencil = true;
    constexpr bool kHasUserStencilSettings = false;
    PathNeedsSWRenderer(context,
                        kHasUserStencilSettings,
                        texture->asRenderTarget(),
                        viewMatrix,
                        element,
                        &pr,
                        kNeedsStencil);
    return pr;
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
bool GrClipMaskManager::UseSWOnlyPath(GrContext* context,
                                      const GrPipelineBuilder& pipelineBuilder,
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

        if (PathNeedsSWRenderer(context, pipelineBuilder.hasUserStencilSettings(),
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

////////////////////////////////////////////////////////////////////////////////
// sort out what kind of clip mask needs to be created: alpha, stencil,
// scissor, or entirely software
bool GrClipMaskManager::setupClipping(const GrPipelineBuilder& pipelineBuilder,
                                      const GrClipStackClip& clip,
                                      const SkRect* devBounds,
                                      GrAppliedClip* out) {
    if (!clip.clipStack() || clip.clipStack()->isWideOpen()) {
        return true;
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
    clipSpaceRTIBounds.offset(clip.origin());

    SkIRect clipSpaceReduceQueryBounds;
#define DISABLE_DEV_BOUNDS_FOR_CLIP_REDUCTION 0
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
                return true;
            }
        } else {
            return false;
        }
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
        bool disallowAnalyticAA = rt->isStencilBufferMultisampled();
        if (disallowAnalyticAA && !rt->numColorSamples()) {
            // With a single color sample, any coverage info is lost from color once it hits the
            // color buffer anyway, so we may as well use coverage AA if nothing else in the pipe
            // is multisampled.
            disallowAnalyticAA = pipelineBuilder.isHWAntialias() ||
                                 pipelineBuilder.hasUserStencilSettings();
        }
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
            out->fClipCoverageFP.reset(clipFP);
            return true;
        }
    }

    // If the stencil buffer is multisampled we can use it to do everything.
    if (!rt->isStencilBufferMultisampled() && requiresAA) {
        sk_sp<GrTexture> result;

        // The top-left of the mask corresponds to the top-left corner of the bounds.
        SkVector clipToMaskOffset = {
            SkIntToScalar(-clipSpaceIBounds.fLeft),
            SkIntToScalar(-clipSpaceIBounds.fTop)
        };

        if (UseSWOnlyPath(this->getContext(), pipelineBuilder, rt, clipToMaskOffset, elements)) {
            // The clip geometry is complex enough that it will be more efficient to create it
            // entirely in software
            result = CreateSoftwareClipMask(this->getContext(),
                                            genID,
                                            initialState,
                                            elements,
                                            clipToMaskOffset,
                                            clipSpaceIBounds);
        } else {
            result = CreateAlphaClipMask(this->getContext(),
                                         genID,
                                         initialState,
                                         elements,
                                         clipToMaskOffset,
                                         clipSpaceIBounds);
            // If createAlphaClipMask fails it means UseSWOnlyPath has a bug
            SkASSERT(result);
        }

        if (result) {
            // The mask's top left coord should be pinned to the rounded-out top left corner of
            // clipSpace bounds. We determine the mask's position WRT to the render target here.
            SkIRect rtSpaceMaskBounds = clipSpaceIBounds;
            rtSpaceMaskBounds.offset(-clip.origin());
            out->fClipCoverageFP.reset(create_fp_for_mask(result.get(), rtSpaceMaskBounds));
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
    out->fHasStencilClip = true;
    return true;
}

static bool stencil_element(GrDrawContext* dc,
                            const GrFixedClip& clip,
                            const GrUserStencilSettings* ss,
                            const SkMatrix& viewMatrix,
                            const SkClipStack::Element* element) {

    // TODO: Draw rrects directly here.
    switch (element->getType()) {
        case Element::kEmpty_Type:
            SkDEBUGFAIL("Should never get here with an empty element.");
            break;
        case Element::kRect_Type:
            return dc->drawContextPriv().drawAndStencilRect(clip, ss,
                                                            element->getOp(),
                                                            element->isInverseFilled(),
                                                            element->isAA(),
                                                            viewMatrix, element->getRect());
            break;
        default: {
            SkPath path;
            element->asPath(&path);
            if (path.isInverseFillType()) {
                path.toggleInverseFillType();
            }

            return dc->drawContextPriv().drawAndStencilPath(clip, ss,
                                                            element->getOp(),
                                                            element->isInverseFilled(),
                                                            element->isAA(), viewMatrix, path);
            break;
        }
    }

    return false;
}

static void draw_element(GrDrawContext* dc,
                         const GrClip& clip, // TODO: can this just always be WideOpen?
                         const GrPaint &paint,
                         const SkMatrix& viewMatrix,
                         const SkClipStack::Element* element) {

    // TODO: Draw rrects directly here.
    switch (element->getType()) {
        case Element::kEmpty_Type:
            SkDEBUGFAIL("Should never get here with an empty element.");
            break;
        case Element::kRect_Type:
            dc->drawRect(clip, paint, viewMatrix, element->getRect());
            break;
        default: {
            SkPath path;
            element->asPath(&path);
            if (path.isInverseFillType()) {
                path.toggleInverseFillType();
            }

            dc->drawPath(clip, paint, viewMatrix, path, GrStyle::SimpleFill());
            break;
        }
    }
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

sk_sp<GrTexture> GrClipMaskManager::CreateAlphaClipMask(GrContext* context,
                                                        int32_t elementsGenID,
                                                        GrReducedClip::InitialState initialState,
                                                        const GrReducedClip::ElementList& elements,
                                                        const SkVector& clipToMaskOffset,
                                                        const SkIRect& clipSpaceIBounds) {
    GrResourceProvider* resourceProvider = context->resourceProvider();
    GrUniqueKey key;
    GetClipMaskKey(elementsGenID, clipSpaceIBounds, &key);
    if (GrTexture* texture = resourceProvider->findAndRefTextureByUniqueKey(key)) {
        return sk_sp<GrTexture>(texture);
    }

    // There's no texture in the cache. Let's try to allocate it then.
    GrPixelConfig config = kRGBA_8888_GrPixelConfig;
    if (context->caps()->isConfigRenderable(kAlpha_8_GrPixelConfig, false)) {
        config = kAlpha_8_GrPixelConfig;
    }

    sk_sp<GrDrawContext> dc(context->newDrawContext(SkBackingFit::kApprox,
                                                    clipSpaceIBounds.width(),
                                                    clipSpaceIBounds.height(),
                                                    config));
    if (!dc) {
        return nullptr;
    }
    
    // The texture may be larger than necessary, this rect represents the part of the texture
    // we populate with a rasterization of the clip.
    SkIRect maskSpaceIBounds = SkIRect::MakeWH(clipSpaceIBounds.width(), clipSpaceIBounds.height());

    // The scratch texture that we are drawing into can be substantially larger than the mask. Only
    // clear the part that we care about.
    dc->clear(&maskSpaceIBounds,
              GrReducedClip::kAllIn_InitialState == initialState ? 0xffffffff : 0x00000000,
              true);

    // Set the matrix so that rendered clip elements are transformed to mask space from clip
    // space.
    const SkMatrix translate = SkMatrix::MakeTrans(clipToMaskOffset.fX, clipToMaskOffset.fY);

    // It is important that we use maskSpaceIBounds as the stencil rect in the below loop.
    // The second pass that zeros the stencil buffer renders the rect maskSpaceIBounds so the first
    // pass must not set values outside of this bounds or stencil values outside the rect won't be
    // cleared.

    // walk through each clip element and perform its set op
    for (GrReducedClip::ElementList::Iter iter = elements.headIter(); iter.get(); iter.next()) {
        const Element* element = iter.get();
        SkRegion::Op op = element->getOp();
        bool invert = element->isInverseFilled();
        if (invert || SkRegion::kIntersect_Op == op || SkRegion::kReverseDifference_Op == op) {
            GrFixedClip clip(maskSpaceIBounds);

            // draw directly into the result with the stencil set to make the pixels affected
            // by the clip shape be non-zero.
            static constexpr GrUserStencilSettings kStencilInElement(
                 GrUserStencilSettings::StaticInit<
                     0xffff,
                     GrUserStencilTest::kAlways,
                     0xffff,
                     GrUserStencilOp::kReplace,
                     GrUserStencilOp::kReplace,
                     0xffff>()
            );
            if (!stencil_element(dc.get(), clip, &kStencilInElement,
                                 translate, element)) {
                return nullptr;
            }

            // Draw to the exterior pixels (those with a zero stencil value).
            static constexpr GrUserStencilSettings kDrawOutsideElement(
                 GrUserStencilSettings::StaticInit<
                     0x0000,
                     GrUserStencilTest::kEqual,
                     0xffff,
                     GrUserStencilOp::kZero,
                     GrUserStencilOp::kZero,
                     0xffff>()
            );
            if (!dc->drawContextPriv().drawAndStencilRect(clip, &kDrawOutsideElement,
                                                          op, !invert, false,
                                                          translate,
                                                          SkRect::Make(clipSpaceIBounds))) {
                return nullptr;
            }
        } else {
            // all the remaining ops can just be directly draw into the accumulation buffer
            GrPaint paint;
            paint.setAntiAlias(element->isAA());
            paint.setCoverageSetOpXPFactory(op, false);

            draw_element(dc.get(), GrNoClip(), paint, translate, element);
        }
    }

    sk_sp<GrTexture> texture(dc->asTexture());
    SkASSERT(texture);
    texture->resourcePriv().setUniqueKey(key);
    return texture;
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
        GrFixedClip clip(stencilSpaceIBounds);

        fDrawTarget->cmmAccess().clearStencilClip(stencilSpaceIBounds,
            GrReducedClip::kAllIn_InitialState == initialState, rt);

        // walk through each clip element and perform its set op
        // with the existing clip.
        for (GrReducedClip::ElementList::Iter iter(elements.headIter()); iter.get(); iter.next()) {
            const Element* element = iter.get();

            GrPipelineBuilder pipelineBuilder;
            pipelineBuilder.setRenderTarget(rt);

            pipelineBuilder.setDisableColorXPFactory();

            // if the target is MSAA then we want MSAA enabled when the clip is soft
            if (rt->isStencilBufferMultisampled()) {
                pipelineBuilder.setState(GrPipelineBuilder::kHWAntialias_Flag, element->isAA());
            }

            bool fillInverted = false;
            // enabled at bottom of loop
            clip.enableStencilClip(false);

            // This will be used to determine whether the clip shape can be rendered into the
            // stencil with arbitrary stencil settings.
            GrPathRenderer::StencilSupport stencilSupport;

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

                SkASSERT(!pipelineBuilder.hasUserStencilSettings());

                GrPathRenderer::CanDrawPathArgs canDrawArgs;
                canDrawArgs.fShaderCaps = this->getContext()->caps()->shaderCaps();
                canDrawArgs.fViewMatrix = &viewMatrix;
                canDrawArgs.fPath = &clipPath;
                canDrawArgs.fStyle = &GrStyle::SimpleFill();
                canDrawArgs.fAntiAlias = false;
                canDrawArgs.fHasUserStencilSettings = pipelineBuilder.hasUserStencilSettings();
                canDrawArgs.fIsStencilBufferMSAA = rt->isStencilBufferMultisampled();

                pr = this->getContext()->drawingManager()->getPathRenderer(canDrawArgs, false,
                                                                           GrPathRendererChain::kStencilOnly_DrawType,
                                                                           &stencilSupport);
                if (nullptr == pr) {
                    return false;
                }
            }

            bool canRenderDirectToStencil =
                GrPathRenderer::kNoRestriction_StencilSupport == stencilSupport;
            bool drawDirectToClip; // Given the renderer, the element,
                                   // fill rule, and set operation should
                                   // we render the element directly to
                                   // stencil bit used for clipping.
            GrUserStencilSettings const* const* stencilPasses =
                GrStencilSettings::GetClipPasses(op, canRenderDirectToStencil, fillInverted,
                                                 &drawDirectToClip);

            // draw the element to the client stencil bits if necessary
            if (!drawDirectToClip) {
                static constexpr GrUserStencilSettings kDrawToStencil(
                     GrUserStencilSettings::StaticInit<
                         0x0000,
                         GrUserStencilTest::kAlways,
                         0xffff,
                         GrUserStencilOp::kIncMaybeClamp,
                         GrUserStencilOp::kIncMaybeClamp,
                         0xffff>()
                );
                if (Element::kRect_Type == element->getType()) {
                    pipelineBuilder.setUserStencil(&kDrawToStencil);

                    draw_non_aa_rect(fDrawTarget, pipelineBuilder, clip, GrColor_WHITE, viewMatrix,
                                     element->getRect());
                } else {
                    if (!clipPath.isEmpty()) {
                        if (canRenderDirectToStencil) {
                            pipelineBuilder.setUserStencil(&kDrawToStencil);

                            GrPathRenderer::DrawPathArgs args;
                            args.fTarget = fDrawTarget;
                            args.fResourceProvider = this->getContext()->resourceProvider();
                            args.fPipelineBuilder = &pipelineBuilder;
                            args.fClip = &clip;
                            args.fColor = GrColor_WHITE;
                            args.fViewMatrix = &viewMatrix;
                            args.fPath = &clipPath;
                            args.fStyle = &GrStyle::SimpleFill();
                            args.fAntiAlias = false;
                            args.fGammaCorrect = false;
                            pr->drawPath(args);
                        } else {
                            GrPathRenderer::StencilPathArgs args;
                            args.fTarget = fDrawTarget;
                            args.fResourceProvider = this->getContext()->resourceProvider();
                            args.fPipelineBuilder = &pipelineBuilder;
                            args.fClip = &clip;
                            args.fViewMatrix = &viewMatrix;
                            args.fPath = &clipPath;
                            pr->stencilPath(args);
                        }
                    }
                }
            }

            // now we modify the clip bit by rendering either the clip
            // element directly or a bounding rect of the entire clip.
            clip.enableStencilClip(true);
            for (GrUserStencilSettings const* const* pass = stencilPasses; *pass; ++pass) {
                pipelineBuilder.setUserStencil(*pass);

                if (drawDirectToClip) {
                    if (Element::kRect_Type == element->getType()) {
                        draw_non_aa_rect(fDrawTarget, pipelineBuilder, clip, GrColor_WHITE,
                                         viewMatrix, element->getRect());
                    } else {
                        GrPathRenderer::DrawPathArgs args;
                        args.fTarget = fDrawTarget;
                        args.fResourceProvider = this->getContext()->resourceProvider();
                        args.fPipelineBuilder = &pipelineBuilder;
                        args.fClip = &clip;
                        args.fColor = GrColor_WHITE;
                        args.fViewMatrix = &viewMatrix;
                        args.fPath = &clipPath;
                        args.fStyle = &GrStyle::SimpleFill();
                        args.fAntiAlias = false;
                        args.fGammaCorrect = false;
                        pr->drawPath(args);
                    }
                } else {
                    // The view matrix is setup to do clip space -> stencil space translation, so
                    // draw rect in clip space.
                    draw_non_aa_rect(fDrawTarget, pipelineBuilder, clip, GrColor_WHITE, viewMatrix,
                                     SkRect::Make(clipSpaceIBounds));
                }
            }
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////
sk_sp<GrTexture> GrClipMaskManager::CreateSoftwareClipMask(
                                                    GrContext* context,
                                                    int32_t elementsGenID,
                                                    GrReducedClip::InitialState initialState,
                                                    const GrReducedClip::ElementList& elements,
                                                    const SkVector& clipToMaskOffset,
                                                    const SkIRect& clipSpaceIBounds) {
    GrUniqueKey key;
    GetClipMaskKey(elementsGenID, clipSpaceIBounds, &key);
    GrResourceProvider* resourceProvider = context->resourceProvider();
    if (GrTexture* texture = resourceProvider->findAndRefTextureByUniqueKey(key)) {
        return sk_sp<GrTexture>(texture);
    }

    // The mask texture may be larger than necessary. We round out the clip space bounds and pin
    // the top left corner of the resulting rect to the top left of the texture.
    SkIRect maskSpaceIBounds = SkIRect::MakeWH(clipSpaceIBounds.width(), clipSpaceIBounds.height());

    GrSWMaskHelper helper(context);

    // Set the matrix so that rendered clip elements are transformed to mask space from clip
    // space.
    SkMatrix translate;
    translate.setTranslate(clipToMaskOffset);

    helper.init(maskSpaceIBounds, &translate);
    helper.clear(GrReducedClip::kAllIn_InitialState == initialState ? 0xFF : 0x00);

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
                helper.drawRect(temp, SkRegion::kXOR_Op, false, 0xFF);
            }
            SkPath clipPath;
            element->asPath(&clipPath);
            clipPath.toggleInverseFillType();
            helper.drawPath(clipPath, GrStyle::SimpleFill(), SkRegion::kReplace_Op,
                            element->isAA(), 0x00);
            continue;
        }

        // The other ops (union, xor, diff) only affect pixels inside
        // the geometry so they can just be drawn normally
        if (Element::kRect_Type == element->getType()) {
            helper.drawRect(element->getRect(), op, element->isAA(), 0xFF);
        } else {
            SkPath path;
            element->asPath(&path);
            helper.drawPath(path, GrStyle::SimpleFill(), op, element->isAA(), 0xFF);
        }
    }

    // Allocate clip mask texture
    GrSurfaceDesc desc;
    desc.fWidth = clipSpaceIBounds.width();
    desc.fHeight = clipSpaceIBounds.height();
    desc.fConfig = kAlpha_8_GrPixelConfig;

    sk_sp<GrTexture> result(context->resourceProvider()->createApproxTexture(desc, 0));
    if (!result) {
        return nullptr;
    }
    result->resourcePriv().setUniqueKey(key);

    helper.toTexture(result.get());

    return result;
}
