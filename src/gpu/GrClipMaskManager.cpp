
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrClipMaskManager.h"
#include "GrGpu.h"
#include "GrRenderTarget.h"
#include "GrStencilBuffer.h"
#include "GrPathRenderer.h"
#include "GrPaint.h"
#include "SkRasterClip.h"

//#define GR_AA_CLIP 1
//#define GR_SW_CLIP 1

////////////////////////////////////////////////////////////////////////////////
void ScissoringSettings::setupScissoring(GrGpu* gpu) {
    if (!fEnableScissoring) {
        gpu->disableScissor();
        return;
    }

    gpu->enableScissoring(fScissorRect);
}

namespace {
// set up the draw state to enable the aa clipping mask. Besides setting up the 
// sampler matrix this also alters the vertex layout
void setup_drawstate_aaclip(GrGpu* gpu, 
                            GrTexture* result, 
                            const GrIRect &bound) {
    GrDrawState* drawState = gpu->drawState();
    GrAssert(drawState);

    static const int maskStage = GrPaint::kTotalStages+1;

    GrMatrix mat;
    mat.setIDiv(result->width(), result->height());
    mat.preTranslate(SkIntToScalar(-bound.fLeft), SkIntToScalar(-bound.fTop));
    mat.preConcat(drawState->getViewMatrix());

    drawState->sampler(maskStage)->reset(GrSamplerState::kClamp_WrapMode,
                                         GrSamplerState::kNearest_Filter,
                                         mat);

    drawState->setTexture(maskStage, result);

    // The AA clipping determination happens long after the geometry has
    // been set up to draw. Here we directly enable the AA clip mask stage
    gpu->addToVertexLayout(
                GrDrawTarget::StagePosAsTexCoordVertexLayoutBit(maskStage));
}

bool create_mask_in_sw() {
    return false;
}

}

////////////////////////////////////////////////////////////////////////////////
// sort out what kind of clip mask needs to be created: alpha, stencil,
// scissor, or entirely software
bool GrClipMaskManager::createClipMask(GrGpu* gpu, 
                                       const GrClip& clipIn,
                                       ScissoringSettings* scissorSettings) {

    GrAssert(scissorSettings);

    scissorSettings->fEnableScissoring = false;
    fClipMaskInStencil = false;
    fClipMaskInAlpha = false;

    GrDrawState* drawState = gpu->drawState();
    if (!drawState->isClipState()) {
        return true;
    }

    GrRenderTarget* rt = drawState->getRenderTarget();

    // GrDrawTarget should have filtered this for us
    GrAssert(NULL != rt);

#if GR_SW_CLIP
    if (create_mask_in_sw()) {
        // The clip geometry is complex enough that it will be more
        // efficient to create it entirely in software
        GrTexture* result = NULL;
        GrIRect bound;
        if (this->createSoftwareClipMask(gpu, clipIn, &result, &bound)) {
            fClipMaskInAlpha = true;

            setup_drawstate_aaclip(gpu, result, bound);
            return true;
        }
    }
#endif

#if GR_AA_CLIP
    // If MSAA is enabled use the (faster) stencil path for AA clipping
    // otherwise the alpha clip mask is our only option
    if (clipIn.requiresAA() && 0 == rt->numSamples()) {
        // Since we are going to create a destination texture of the correct
        // size for the mask (rather than being bound by the size of the
        // render target) we aren't going to use scissoring like the stencil
        // path does (see scissorSettings below)
        GrTexture* result = NULL;
        GrIRect bound;
        if (this->createAlphaClipMask(gpu, clipIn, &result, &bound)) {
            fClipMaskInAlpha = true;

            setup_drawstate_aaclip(gpu, result, bound);
            return true;
        }

        // if alpha clip mask creation fails fall through to the stencil
        // buffer method
    }
#endif // GR_AA_CLIP

    // Either a hard (stencil buffer) clip was explicitly requested or 
    // an antialiased clip couldn't be created. In either case, free up
    // the texture in the antialiased mask cache.
    // TODO: this may require more investigation. Ganesh performs a lot of
    // utility draws (e.g., clears, InOderDrawBuffer playbacks) that hit
    // the stencil buffer path. These may be incorrectly messing up the 
    // AA cache.
    fAACache.reset();

    GrRect bounds;
    GrRect rtRect;
    rtRect.setLTRB(0, 0,
                    GrIntToScalar(rt->width()), GrIntToScalar(rt->height()));
    if (clipIn.hasConservativeBounds()) {
        bounds = clipIn.getConservativeBounds();
        if (!bounds.intersect(rtRect)) {
            bounds.setEmpty();
        }
    } else {
        bounds = rtRect;
    }

    bounds.roundOut(&scissorSettings->fScissorRect);
    if  (scissorSettings->fScissorRect.isEmpty()) {
        scissorSettings->fScissorRect.setLTRB(0,0,0,0);
        // TODO: I think we can do an early exit here - after refactoring try:
        //  set fEnableScissoring to true but leave fClipMaskInStencil false
        //  and return - everything is going to be scissored away anyway!
    }
    scissorSettings->fEnableScissoring = true;

    // use the stencil clip if we can't represent the clip as a rectangle.
    fClipMaskInStencil = !clipIn.isRect() && !clipIn.isEmpty() &&
                         !bounds.isEmpty();

    if (fClipMaskInStencil) {
        return this->createStencilClipMask(gpu, clipIn, bounds, scissorSettings);
    }

    return true;
}

#define VISUALIZE_COMPLEX_CLIP 0

#if VISUALIZE_COMPLEX_CLIP
    #include "GrRandom.h"
    GrRandom gRandom;
    #define SET_RANDOM_COLOR drawState->setColor(0xff000000 | gRandom.nextU());
#else
    #define SET_RANDOM_COLOR
#endif

namespace {
/**
 * Does "container" contain "containee"? If either is empty then
 * no containment is possible.
 */
bool contains(const SkRect& container, const SkIRect& containee) {
    return  !containee.isEmpty() && !container.isEmpty() &&
            container.fLeft <= SkIntToScalar(containee.fLeft) && 
            container.fTop <= SkIntToScalar(containee.fTop) &&
            container.fRight >= SkIntToScalar(containee.fRight) && 
            container.fBottom >= SkIntToScalar(containee.fBottom);
}


////////////////////////////////////////////////////////////////////////////////
// determines how many elements at the head of the clip can be skipped and
// whether the initial clear should be to the inside- or outside-the-clip value,
// and what op should be used to draw the first element that isn't skipped.
int process_initial_clip_elements(const GrClip& clip,
                                  const GrIRect& bounds,
                                  bool* clearToInside,
                                  SkRegion::Op* startOp) {

    // logically before the first element of the clip stack is 
    // processed the clip is entirely open. However, depending on the
    // first set op we may prefer to clear to 0 for performance. We may
    // also be able to skip the initial clip paths/rects. We loop until
    // we cannot skip an element.
    int curr;
    bool done = false;
    *clearToInside = true;
    int count = clip.getElementCount();

    for (curr = 0; curr < count && !done; ++curr) {
        switch (clip.getOp(curr)) {
            case SkRegion::kReplace_Op:
                // replace ignores everything previous
                *startOp = SkRegion::kReplace_Op;
                *clearToInside = false;
                done = true;
                break;
            case SkRegion::kIntersect_Op:
                // if this element contains the entire bounds then we
                // can skip it.
                if (kRect_ClipType == clip.getElementType(curr)
                    && contains(clip.getRect(curr), bounds)) {
                    break;
                }
                // if everything is initially clearToInside then intersect is
                // same as clear to 0 and treat as a replace. Otherwise,
                // set stays empty.
                if (*clearToInside) {
                    *startOp = SkRegion::kReplace_Op;
                    *clearToInside = false;
                    done = true;
                }
                break;
                // we can skip a leading union.
            case SkRegion::kUnion_Op:
                // if everything is initially outside then union is
                // same as replace. Otherwise, every pixel is still 
                // clearToInside
                if (!*clearToInside) {
                    *startOp = SkRegion::kReplace_Op;
                    done = true;
                }
                break;
            case SkRegion::kXOR_Op:
                // xor is same as difference or replace both of which
                // can be 1-pass instead of 2 for xor.
                if (*clearToInside) {
                    *startOp = SkRegion::kDifference_Op;
                } else {
                    *startOp = SkRegion::kReplace_Op;
                }
                done = true;
                break;
            case SkRegion::kDifference_Op:
                // if all pixels are clearToInside then we have to process the
                // difference, otherwise it has no effect and all pixels
                // remain outside.
                if (*clearToInside) {
                    *startOp = SkRegion::kDifference_Op;
                    done = true;
                }
                break;
            case SkRegion::kReverseDifference_Op:
                // if all pixels are clearToInside then reverse difference
                // produces empty set. Otherise it is same as replace
                if (*clearToInside) {
                    *clearToInside = false;
                } else {
                    *startOp = SkRegion::kReplace_Op;
                    done = true;
                }
                break;
            default:
                GrCrash("Unknown set op.");
        }
    }
    return done ? curr-1 : count;
}

}


namespace {

////////////////////////////////////////////////////////////////////////////////
// set up the OpenGL blend function to perform the specified 
// boolean operation for alpha clip mask creation 
void setup_boolean_blendcoeffs(GrDrawState* drawState, SkRegion::Op op) {

    switch (op) {
        case SkRegion::kReplace_Op:
            drawState->setBlendFunc(kOne_BlendCoeff, kZero_BlendCoeff);
            break;
        case SkRegion::kIntersect_Op:
            drawState->setBlendFunc(kDC_BlendCoeff, kZero_BlendCoeff);
            break;
        case SkRegion::kUnion_Op:
            drawState->setBlendFunc(kOne_BlendCoeff, kISC_BlendCoeff);
            break;
        case SkRegion::kXOR_Op:
            drawState->setBlendFunc(kIDC_BlendCoeff, kISC_BlendCoeff);
            break;
        case SkRegion::kDifference_Op:
            drawState->setBlendFunc(kZero_BlendCoeff, kISC_BlendCoeff);
            break;
        case SkRegion::kReverseDifference_Op:
            drawState->setBlendFunc(kIDC_BlendCoeff, kZero_BlendCoeff);
            break;
        default:
            GrAssert(false);
            break;
    }
}

}

////////////////////////////////////////////////////////////////////////////////
bool GrClipMaskManager::drawPath(GrGpu* gpu,
                                 const SkPath& path,
                                 GrPathFill fill,
                                 bool doAA) {

    GrPathRenderer* pr = this->getClipPathRenderer(gpu, path, fill, doAA);
    if (NULL == pr) {
        return false;
    }

    pr->drawPath(path, fill, NULL, gpu, 0, doAA);
    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool GrClipMaskManager::drawClipShape(GrGpu* gpu,
                                      GrTexture* target,
                                      const GrClip& clipIn,
                                      int index) {
    GrDrawState* drawState = gpu->drawState();
    GrAssert(NULL != drawState);

    drawState->setRenderTarget(target->asRenderTarget());

    if (kRect_ClipType == clipIn.getElementType(index)) {
        if (clipIn.getDoAA(index)) {
            // convert the rect to a path for AA
            SkPath temp;
            temp.addRect(clipIn.getRect(index));

            return this->drawPath(gpu, temp,
                                  kEvenOdd_PathFill, clipIn.getDoAA(index));
        } else {
            gpu->drawSimpleRect(clipIn.getRect(index), NULL, 0);
        }
    } else {
        return this->drawPath(gpu,
                              clipIn.getPath(index),
                              clipIn.getPathFill(index),
                              clipIn.getDoAA(index));
    }
    return true;
}

void GrClipMaskManager::drawTexture(GrGpu* gpu,
                                    GrTexture* target,
                                    GrTexture* texture) {
    GrDrawState* drawState = gpu->drawState();
    GrAssert(NULL != drawState);

    // no AA here since it is encoded in the texture
    drawState->setRenderTarget(target->asRenderTarget());

    GrMatrix sampleM;
    sampleM.setIDiv(texture->width(), texture->height());
    drawState->setTexture(0, texture);

    drawState->sampler(0)->reset(GrSamplerState::kClamp_WrapMode,
                                 GrSamplerState::kNearest_Filter,
                                 sampleM);

    GrRect rect = GrRect::MakeWH(SkIntToScalar(target->width()), 
                                 SkIntToScalar(target->height()));

    gpu->drawSimpleRect(rect, NULL, 1 << 0);

    drawState->setTexture(0, NULL);
}

namespace {

void clear(GrGpu* gpu,
           GrTexture* target,
           GrColor color) {
    GrDrawState* drawState = gpu->drawState();
    GrAssert(NULL != drawState);

    // zap entire target to specified color
    drawState->setRenderTarget(target->asRenderTarget());
    gpu->clear(NULL, color);
}

}

// get a texture to act as a temporary buffer for AA clip boolean operations
// TODO: given the expense of createTexture we may want to just cache this too
void GrClipMaskManager::getTemp(const GrIRect& bounds, 
                                GrAutoScratchTexture* temp) {
    if (NULL != temp->texture()) {
        // we've already allocated the temp texture
        return;
    }

    const GrTextureDesc desc = {
        kRenderTarget_GrTextureFlagBit|kNoStencil_GrTextureFlagBit,
        bounds.width(),
        bounds.height(),
        kAlpha_8_GrPixelConfig,
        0           // samples
    };

    temp->set(fAACache.getContext(), desc);
}


void GrClipMaskManager::setupCache(const GrClip& clipIn,
                                   const GrIRect& bounds) {
    // Since we are setting up the cache we know the last lookup was a miss
    // Free up the currently cached mask so it can be reused
    fAACache.reset();

    const GrTextureDesc desc = {
        kRenderTarget_GrTextureFlagBit|kNoStencil_GrTextureFlagBit,
        bounds.width(),
        bounds.height(),
        kAlpha_8_GrPixelConfig,
        0           // samples
    };

    fAACache.acquireMask(clipIn, desc, bounds);
}

////////////////////////////////////////////////////////////////////////////////
// Shared preamble between gpu and SW-only AA clip mask creation paths.
// Handles caching, determination of clip mask bound & allocation (if needed)
// of the result texture
// Returns true if there is no more work to be done (i.e., we got a cache hit)
bool GrClipMaskManager::clipMaskPreamble(GrGpu* gpu,
                                         const GrClip& clipIn,
                                         GrTexture** result,
                                         GrIRect *resultBounds) {
    GrDrawState* origDrawState = gpu->drawState();
    GrAssert(origDrawState->isClipState());

    GrRenderTarget* rt = origDrawState->getRenderTarget();
    GrAssert(NULL != rt);

    GrRect rtRect;
    rtRect.setLTRB(0, 0,
                    GrIntToScalar(rt->width()), GrIntToScalar(rt->height()));

    // unlike the stencil path the alpha path is not bound to the size of the
    // render target - determine the minimum size required for the mask
    GrRect bounds;

    if (clipIn.hasConservativeBounds()) {
        bounds = clipIn.getConservativeBounds();
        if (!bounds.intersect(rtRect)) {
            // the mask will be empty in this case
            GrAssert(false);
            bounds.setEmpty();
        }
    } else {
        // still locked to the size of the render target
        bounds = rtRect;
    }

    GrIRect intBounds;
    bounds.roundOut(&intBounds);

    // need to outset a pixel since the standard bounding box computation
    // path doesn't leave any room for antialiasing (esp. w.r.t. rects)
    intBounds.outset(1, 1);

    // TODO: make sure we don't outset if bounds are still 0,0 @ min

    if (fAACache.canReuse(clipIn, 
                          intBounds.width(),
                          intBounds.height())) {
        *result = fAACache.getLastMask();
        fAACache.getLastBound(resultBounds);
        return true;
    }

    this->setupCache(clipIn, intBounds);

    *resultBounds = intBounds;
    return false;
}

////////////////////////////////////////////////////////////////////////////////
// Create a 8-bit clip mask in alpha
bool GrClipMaskManager::createAlphaClipMask(GrGpu* gpu,
                                            const GrClip& clipIn,
                                            GrTexture** result,
                                            GrIRect *resultBounds) {

    if (this->clipMaskPreamble(gpu, clipIn, result, resultBounds)) {
        return true;
    }

    GrTexture* accum = fAACache.getLastMask();
    if (NULL == accum) {
        fClipMaskInAlpha = false;
        fAACache.reset();
        return false;
    }

    GrDrawTarget::AutoStateRestore asr(gpu, GrDrawTarget::kReset_ASRInit);
    GrDrawState* drawState = gpu->drawState();

    GrDrawTarget::AutoGeometryPush agp(gpu);

    int count = clipIn.getElementCount();

    if (0 != resultBounds->fTop || 0 != resultBounds->fLeft) {
        // if we were able to trim down the size of the mask we need to 
        // offset the paths & rects that will be used to compute it
        GrMatrix m;

        m.setTranslate(SkIntToScalar(-resultBounds->fLeft), 
                       SkIntToScalar(-resultBounds->fTop));

        drawState->setViewMatrix(m);
    }

    bool clearToInside;
    SkRegion::Op startOp = SkRegion::kReplace_Op; // suppress warning
    int start = process_initial_clip_elements(clipIn,
                                              *resultBounds,
                                              &clearToInside,
                                              &startOp);

    clear(gpu, accum, clearToInside ? 0xffffffff : 0x00000000);

    GrAutoScratchTexture temp;

    // walk through each clip element and perform its set op
    for (int c = start; c < count; ++c) {

        SkRegion::Op op = (c == start) ? startOp : clipIn.getOp(c);

        if (SkRegion::kReplace_Op == op) {
            // TODO: replace is actually a lot faster then intersection
            // for this path - refactor the stencil path so it can handle
            // replace ops and alter GrClip to allow them through

            // clear the accumulator and draw the new object directly into it
            clear(gpu, accum, 0x00000000);

            setup_boolean_blendcoeffs(drawState, op);
            this->drawClipShape(gpu, accum, clipIn, c);

        } else if (SkRegion::kReverseDifference_Op == op ||
                   SkRegion::kIntersect_Op == op) {
            // there is no point in intersecting a screen filling rectangle.
            if (SkRegion::kIntersect_Op == op &&
                kRect_ClipType == clipIn.getElementType(c) &&
                contains(clipIn.getRect(c), *resultBounds)) {
                continue;
            }

            getTemp(*resultBounds, &temp);
            if (NULL == temp.texture()) {
                fClipMaskInAlpha = false;
                fAACache.reset();
                return false;
            }

            // clear the temp target & draw into it
            clear(gpu, temp.texture(), 0x00000000);

            setup_boolean_blendcoeffs(drawState, SkRegion::kReplace_Op);
            this->drawClipShape(gpu, temp.texture(), clipIn, c);

            // TODO: rather than adding these two translations here
            // compute the bounding box needed to render the texture
            // into temp
            if (0 != resultBounds->fTop || 0 != resultBounds->fLeft) {
                GrMatrix m;

                m.setTranslate(SkIntToScalar(resultBounds->fLeft), 
                               SkIntToScalar(resultBounds->fTop));

                drawState->preConcatViewMatrix(m);
            }

            // Now draw into the accumulator using the real operation
            // and the temp buffer as a texture
            setup_boolean_blendcoeffs(drawState, op);
            this->drawTexture(gpu, accum, temp.texture());

            if (0 != resultBounds->fTop || 0 != resultBounds->fLeft) {
                GrMatrix m;

                m.setTranslate(SkIntToScalar(-resultBounds->fLeft), 
                               SkIntToScalar(-resultBounds->fTop));

                drawState->preConcatViewMatrix(m);
            }

        } else {
            // all the remaining ops can just be directly draw into 
            // the accumulation buffer
            setup_boolean_blendcoeffs(drawState, op);
            this->drawClipShape(gpu, accum, clipIn, c);
        }
    }

    *result = accum;

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Create a 1-bit clip mask in the stencil buffer
bool GrClipMaskManager::createStencilClipMask(GrGpu* gpu, 
                                              const GrClip& clipIn,
                                              const GrRect& bounds,
                                              ScissoringSettings* scissorSettings) {

    GrAssert(fClipMaskInStencil);

    GrDrawState* drawState = gpu->drawState();
    GrAssert(drawState->isClipState());

    GrRenderTarget* rt = drawState->getRenderTarget();
    GrAssert(NULL != rt);

    // TODO: dynamically attach a SB when needed.
    GrStencilBuffer* stencilBuffer = rt->getStencilBuffer();
    if (NULL == stencilBuffer) {
        return false;
    }

    if (stencilBuffer->mustRenderClip(clipIn, rt->width(), rt->height())) {

        stencilBuffer->setLastClip(clipIn, rt->width(), rt->height());

        // we set the current clip to the bounds so that our recursive
        // draws are scissored to them. We use the copy of the complex clip
        // we just stashed on the SB to render from. We set it back after
        // we finish drawing it into the stencil.
        const GrClip& clipCopy = stencilBuffer->getLastClip();
        gpu->setClip(GrClip(bounds));

        GrDrawTarget::AutoStateRestore asr(gpu, GrDrawTarget::kReset_ASRInit);
        drawState = gpu->drawState();
        drawState->setRenderTarget(rt);
        GrDrawTarget::AutoGeometryPush agp(gpu);

        gpu->disableScissor();
#if !VISUALIZE_COMPLEX_CLIP
        drawState->enableState(GrDrawState::kNoColorWrites_StateBit);
#endif

        int count = clipCopy.getElementCount();
        int clipBit = stencilBuffer->bits();
        SkASSERT((clipBit <= 16) &&
                    "Ganesh only handles 16b or smaller stencil buffers");
        clipBit = (1 << (clipBit-1));

        GrIRect rtRect = GrIRect::MakeWH(rt->width(), rt->height());

        bool clearToInside;
        SkRegion::Op startOp = SkRegion::kReplace_Op; // suppress warning
        int start = process_initial_clip_elements(clipCopy,
                                                    rtRect,
                                                    &clearToInside,
                                                    &startOp);

        gpu->clearStencilClip(scissorSettings->fScissorRect, clearToInside);

        // walk through each clip element and perform its set op
        // with the existing clip.
        for (int c = start; c < count; ++c) {
            GrPathFill fill;
            bool fillInverted;
            // enabled at bottom of loop
            drawState->disableState(GrGpu::kModifyStencilClip_StateBit);

            bool canRenderDirectToStencil; // can the clip element be drawn
                                           // directly to the stencil buffer
                                           // with a non-inverted fill rule
                                           // without extra passes to
                                           // resolve in/out status.

            SkRegion::Op op = (c == start) ? startOp : clipCopy.getOp(c);

            GrPathRenderer* pr = NULL;
            const SkPath* clipPath = NULL;
            if (kRect_ClipType == clipCopy.getElementType(c)) {
                canRenderDirectToStencil = true;
                fill = kEvenOdd_PathFill;
                fillInverted = false;
                // there is no point in intersecting a screen filling
                // rectangle.
                if (SkRegion::kIntersect_Op == op &&
                    contains(clipCopy.getRect(c), rtRect)) {
                    continue;
                }
            } else {
                fill = clipCopy.getPathFill(c);
                fillInverted = GrIsFillInverted(fill);
                fill = GrNonInvertedFill(fill);
                clipPath = &clipCopy.getPath(c);
                pr = this->getClipPathRenderer(gpu, *clipPath, fill, false);
                if (NULL == pr) {
                    fClipMaskInStencil = false;
                    gpu->setClip(clipCopy);     // restore to the original
                    return false;
                }
                canRenderDirectToStencil =
                    !pr->requiresStencilPass(*clipPath, fill, gpu);
            }

            int passes;
            GrStencilSettings stencilSettings[GrStencilSettings::kMaxStencilClipPasses];

            bool canDrawDirectToClip; // Given the renderer, the element,
                                        // fill rule, and set operation can
                                        // we render the element directly to
                                        // stencil bit used for clipping.
            canDrawDirectToClip =
                GrStencilSettings::GetClipPasses(op,
                                                    canRenderDirectToStencil,
                                                    clipBit,
                                                    fillInverted,
                                                    &passes, stencilSettings);

            // draw the element to the client stencil bits if necessary
            if (!canDrawDirectToClip) {
                GR_STATIC_CONST_SAME_STENCIL(gDrawToStencil,
                    kIncClamp_StencilOp,
                    kIncClamp_StencilOp,
                    kAlways_StencilFunc,
                    0xffff,
                    0x0000,
                    0xffff);
                SET_RANDOM_COLOR
                if (kRect_ClipType == clipCopy.getElementType(c)) {
                    *drawState->stencil() = gDrawToStencil;
                    gpu->drawSimpleRect(clipCopy.getRect(c), NULL, 0);
                } else {
                    if (canRenderDirectToStencil) {
                        *drawState->stencil() = gDrawToStencil;
                        pr->drawPath(*clipPath, fill, NULL, gpu, 0, false);
                    } else {
                        pr->drawPathToStencil(*clipPath, fill, gpu);
                    }
                }
            }

            // now we modify the clip bit by rendering either the clip
            // element directly or a bounding rect of the entire clip.
            drawState->enableState(GrGpu::kModifyStencilClip_StateBit);
            for (int p = 0; p < passes; ++p) {
                *drawState->stencil() = stencilSettings[p];
                if (canDrawDirectToClip) {
                    if (kRect_ClipType == clipCopy.getElementType(c)) {
                        SET_RANDOM_COLOR
                        gpu->drawSimpleRect(clipCopy.getRect(c), NULL, 0);
                    } else {
                        SET_RANDOM_COLOR
                        pr->drawPath(*clipPath, fill, NULL, gpu, 0, false);
                    }
                } else {
                    SET_RANDOM_COLOR
                    gpu->drawSimpleRect(bounds, NULL, 0);
                }
            }
        }
        // restore clip
        gpu->setClip(clipCopy);
        // recusive draws would have disabled this since they drew with
        // the clip bounds as clip.
        fClipMaskInStencil = true;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool GrClipMaskManager::createSoftwareClipMask(GrGpu* gpu,
                                               const GrClip& clipIn,
                                               GrTexture** result,
                                               GrIRect *resultBounds) {

    if (this->clipMaskPreamble(gpu, clipIn, result, resultBounds)) {
        return true;
    }

    GrTexture* accum = fAACache.getLastMask();
    if (NULL == accum) {
        fClipMaskInAlpha = false;
        fAACache.reset();
        return false;
    }

#if 0
    SkRasterClip rasterClip;

    // TODO: refactor GrClip out of existance and use SkCanvas's ClipVisitor
    //      - may have to move it to SkClipStack
    for (int i = 0; i < clipIn.getElementCount(); ++i) {
        if (kRect_ClipType == clipIn.getElementType(i)) {
            rasterClip.op(clipIn.getRect(i), clipIn.getOp(i), clipIn.getDoAA(i));
        } else {
            GrAssert(kPath_ClipType == clipIn.getElementType(i));

            SkIPoint deviceSize = SkIPoint::Make(resultBounds->width(), 
                                                 resultBounds->height());

            SkRasterClip::clipPathHelper(&rasterClip, 
                                         clipIn.getPath(i),
                                         clipIn.getOp(i),
                                         clipIn.getDoAA(i),
                                         deviceSize);
        }
    }

    // TODO: need to get pixels out of SkRasterClip & into the texture!
#endif

    *result = accum;

    return true;
}


////////////////////////////////////////////////////////////////////////////////
GrPathRenderer* GrClipMaskManager::getClipPathRenderer(GrGpu* gpu,
                                                       const SkPath& path,
                                                       GrPathFill fill,
                                                       bool antiAlias) {
    if (NULL == fPathRendererChain) {
        fPathRendererChain = 
            new GrPathRendererChain(gpu->getContext(),
                                    GrPathRendererChain::kNone_UsageFlag);
    }
    return fPathRendererChain->getPathRenderer(path, fill, gpu, antiAlias);
}

////////////////////////////////////////////////////////////////////////////////
void GrClipMaskManager::releaseResources() {
    // in case path renderer has any GrResources, start from scratch
    GrSafeSetNull(fPathRendererChain);
    fAACache.releaseResources();
}
