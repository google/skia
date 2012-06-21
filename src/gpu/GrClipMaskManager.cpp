
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
#include "GrAAConvexPathRenderer.h"
#include "GrAAHairLinePathRenderer.h"

// TODO: move GrSWMaskHelper out of GrSoftwarePathRender.h & remove this include
#include "GrSoftwarePathRenderer.h"

//#define GR_AA_CLIP 1
//#define GR_SW_CLIP 1

////////////////////////////////////////////////////////////////////////////////
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

bool path_needs_SW_renderer(GrContext* context,
                            GrGpu* gpu,
                            const SkPath& path,
                            GrPathFill fill,
                            bool doAA) {
    // last (false) parameter disallows use of the SW path renderer
    return NULL == context->getPathRenderer(path, fill, gpu, doAA, false);
}

}

/*
 * This method traverses the clip stack to see if the GrSoftwarePathRenderer
 * will be used on any element. If so, it returns true to indicate that the
 * entire clip should be rendered in SW and then uploaded en masse to the gpu.
 */
bool GrClipMaskManager::useSWOnlyPath(const GrClip& clipIn) {

    if (!clipIn.requiresAA()) {
        // The stencil buffer can handle this case
        return false;
    }

    // TODO: generalize this function so that when
    // a clip gets complex enough it can just be done in SW regardless
    // of whether it would invoke the GrSoftwarePathRenderer.
    bool useSW = false;

    for (int i = 0; i < clipIn.getElementCount(); ++i) {

        if (SkRegion::kReplace_Op == clipIn.getOp(i)) {
            // Everything before a replace op can be ignored so start
            // afresh w.r.t. determining if any element uses the SW path
            useSW = false;
        }

        // rects can always be drawn directly w/o using the software path
        // so only paths need to be checked
        if (kPath_ClipType == clipIn.getElementType(i) &&
            path_needs_SW_renderer(this->getContext(), fGpu, 
                                    clipIn.getPath(i), 
                                    clipIn.getPathFill(i), 
                                    clipIn.getDoAA(i))) {
            useSW = true;
        }
    }

    return useSW;
}

////////////////////////////////////////////////////////////////////////////////
// sort out what kind of clip mask needs to be created: alpha, stencil,
// scissor, or entirely software
bool GrClipMaskManager::setupClipping(const GrClip& clipIn) {
    fCurrClipMaskType = kNone_ClipMaskType;

    GrDrawState* drawState = fGpu->drawState();
    if (!drawState->isClipState() || clipIn.isEmpty()) {
        fGpu->disableScissor();
        this->setGpuStencil();
        return true;
    }

    GrRenderTarget* rt = drawState->getRenderTarget();
    // GrDrawTarget should have filtered this for us
    GrAssert(NULL != rt);

    GrIRect bounds;
    GrIRect rtRect;
    rtRect.setLTRB(0, 0, rt->width(), rt->height());
    if (clipIn.hasConservativeBounds()) {
        GrRect softBounds = clipIn.getConservativeBounds();
        softBounds.roundOut(&bounds);
        if (!bounds.intersect(rtRect)) {
            bounds.setEmpty();
        }
        if (bounds.isEmpty()) {
            return false;
        }
    } else {
        bounds = rtRect;
    }

#if GR_SW_CLIP
    // If MSAA is enabled we can do everything in the stencil buffer.
    // Otherwise check if we should just create the entire clip mask 
    // in software (this will only happen if the clip mask is anti-aliased
    // and too complex for the gpu to handle in its entirety)
    if (0 == rt->numSamples() && this->useSWOnlyPath(clipIn)) {
        // The clip geometry is complex enough that it will be more
        // efficient to create it entirely in software
        GrTexture* result = NULL;
        GrIRect bound;
        if (this->createSoftwareClipMask(clipIn, &result, &bound)) {
            setup_drawstate_aaclip(fGpu, result, bound);
            fGpu->disableScissor();
            this->setGpuStencil();
            return true;
        }

        // if SW clip mask creation fails fall through to the other
        // two possible methods (bottoming out at stencil clipping)
    }
#endif // GR_SW_CLIP

#if GR_AA_CLIP
    // If MSAA is enabled use the (faster) stencil path for AA clipping
    // otherwise the alpha clip mask is our only option
    if (0 == rt->numSamples() && clipIn.requiresAA()) {
        // Since we are going to create a destination texture of the correct
        // size for the mask (rather than being bound by the size of the
        // render target) we aren't going to use scissoring like the stencil
        // path does (see scissorSettings below)
        GrTexture* result = NULL;
        GrIRect bound;
        if (this->createAlphaClipMask(clipIn, &result, &bound)) {
            setup_drawstate_aaclip(fGpu, result, bound);
            fGpu->disableScissor();
            this->setGpuStencil();
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
    // utility draws (e.g., clears, InOrderDrawBuffer playbacks) that hit
    // the stencil buffer path. These may be "incorrectly" clearing the 
    // AA cache.
    fAACache.reset();

    // If the clip is a rectangle then just set the scissor. Otherwise, create
    // a stencil mask.
    if (clipIn.isRect()) {
        fGpu->enableScissor(bounds);
        this->setGpuStencil();
        return true;
    }

    // use the stencil clip if we can't represent the clip as a rectangle.
    bool useStencil = !clipIn.isRect() && !clipIn.isEmpty() &&
                      !bounds.isEmpty();

    if (useStencil) {
        this->createStencilClipMask(clipIn, bounds);
    }
    // This must occur after createStencilClipMask. That function may change
    // the scissor. Also, it only guarantees that the stencil mask is correct
    // within the bounds it was passed, so we must use both stencil and scissor
    // test to the bounds for the final draw.
    fGpu->enableScissor(bounds);
    this->setGpuStencil();
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
            drawState->setBlendFunc(kOne_GrBlendCoeff, kZero_GrBlendCoeff);
            break;
        case SkRegion::kIntersect_Op:
            drawState->setBlendFunc(kDC_GrBlendCoeff, kZero_GrBlendCoeff);
            break;
        case SkRegion::kUnion_Op:
            drawState->setBlendFunc(kOne_GrBlendCoeff, kISC_GrBlendCoeff);
            break;
        case SkRegion::kXOR_Op:
            drawState->setBlendFunc(kIDC_GrBlendCoeff, kISC_GrBlendCoeff);
            break;
        case SkRegion::kDifference_Op:
            drawState->setBlendFunc(kZero_GrBlendCoeff, kISC_GrBlendCoeff);
            break;
        case SkRegion::kReverseDifference_Op:
            drawState->setBlendFunc(kIDC_GrBlendCoeff, kZero_GrBlendCoeff);
            break;
        default:
            GrAssert(false);
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////
bool draw_path(GrContext* context,
               GrGpu* gpu,
               const SkPath& path,
               GrPathFill fill,
               bool doAA) {

    GrPathRenderer* pr = context->getPathRenderer(path, fill, gpu, doAA, true);
    if (NULL == pr) {
        return false;
    }

    pr->drawPath(path, fill, NULL, gpu, 0, doAA);
    return true;
}

}

////////////////////////////////////////////////////////////////////////////////
bool GrClipMaskManager::drawClipShape(GrTexture* target,
                                      const GrClip& clipIn,
                                      int index) {
    GrDrawState* drawState = fGpu->drawState();
    GrAssert(NULL != drawState);

    drawState->setRenderTarget(target->asRenderTarget());

    if (kRect_ClipType == clipIn.getElementType(index)) {
        if (clipIn.getDoAA(index)) {
            getContext()->getAARectRenderer()->fillAARect(fGpu, fGpu,
                                                          clipIn.getRect(index), 
                                                          true);
        } else {
            fGpu->drawSimpleRect(clipIn.getRect(index), NULL, 0);
        }
    } else {
        return draw_path(this->getContext(), fGpu,
                         clipIn.getPath(index),
                         clipIn.getPathFill(index),
                         clipIn.getDoAA(index));
    }
    return true;
}

void GrClipMaskManager::drawTexture(GrTexture* target,
                                    GrTexture* texture) {
    GrDrawState* drawState = fGpu->drawState();
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

    fGpu->drawSimpleRect(rect, NULL, 1 << 0);

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

    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit|kNoStencil_GrTextureFlagBit;
    desc.fWidth = bounds.width();
    desc.fHeight = bounds.height();
    desc.fConfig = kAlpha_8_GrPixelConfig;

    temp->set(this->getContext(), desc);
}


void GrClipMaskManager::setupCache(const GrClip& clipIn,
                                   const GrIRect& bounds) {
    // Since we are setting up the cache we know the last lookup was a miss
    // Free up the currently cached mask so it can be reused
    fAACache.reset();

    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit|kNoStencil_GrTextureFlagBit;
    desc.fWidth = bounds.width();
    desc.fHeight = bounds.height();
    desc.fConfig = kAlpha_8_GrPixelConfig;

    fAACache.acquireMask(clipIn, desc, bounds);
}

////////////////////////////////////////////////////////////////////////////////
// Shared preamble between gpu and SW-only AA clip mask creation paths.
// Handles caching, determination of clip mask bound & allocation (if needed)
// of the result texture
// Returns true if there is no more work to be done (i.e., we got a cache hit)
bool GrClipMaskManager::clipMaskPreamble(const GrClip& clipIn,
                                         GrTexture** result,
                                         GrIRect *resultBounds) {
    GrDrawState* origDrawState = fGpu->drawState();
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
bool GrClipMaskManager::createAlphaClipMask(const GrClip& clipIn,
                                            GrTexture** result,
                                            GrIRect *resultBounds) {

    GrAssert(kNone_ClipMaskType == fCurrClipMaskType);

    if (this->clipMaskPreamble(clipIn, result, resultBounds)) {
        fCurrClipMaskType = kAlpha_ClipMaskType;
        return true;
    }

    GrTexture* accum = fAACache.getLastMask();
    if (NULL == accum) {
        fAACache.reset();
        return false;
    }

    GrDrawTarget::AutoStateRestore asr(fGpu, GrDrawTarget::kReset_ASRInit);
    GrDrawState* drawState = fGpu->drawState();

    GrDrawTarget::AutoGeometryPush agp(fGpu);

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

    clear(fGpu, accum, clearToInside ? 0xffffffff : 0x00000000);

    GrAutoScratchTexture temp;

    // walk through each clip element and perform its set op
    for (int c = start; c < count; ++c) {

        SkRegion::Op op = (c == start) ? startOp : clipIn.getOp(c);

        if (SkRegion::kReplace_Op == op) {
            // TODO: replace is actually a lot faster then intersection
            // for this path - refactor the stencil path so it can handle
            // replace ops and alter GrClip to allow them through

            // clear the accumulator and draw the new object directly into it
            clear(fGpu, accum, 0x00000000);

            setup_boolean_blendcoeffs(drawState, op);
            this->drawClipShape(accum, clipIn, c);

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
                fAACache.reset();
                return false;
            }

            // clear the temp target & draw into it
            clear(fGpu, temp.texture(), 0x00000000);

            setup_boolean_blendcoeffs(drawState, SkRegion::kReplace_Op);
            this->drawClipShape(temp.texture(), clipIn, c);

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
            this->drawTexture(accum, temp.texture());

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
            this->drawClipShape(accum, clipIn, c);
        }
    }

    *result = accum;
    fCurrClipMaskType = kAlpha_ClipMaskType;
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Create a 1-bit clip mask in the stencil buffer
bool GrClipMaskManager::createStencilClipMask(const GrClip& clipIn,
                                              const GrIRect& bounds) {

    GrAssert(kNone_ClipMaskType == fCurrClipMaskType);

    GrDrawState* drawState = fGpu->drawState();
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
        fGpu->setClip(GrClip(bounds));

        GrDrawTarget::AutoStateRestore asr(fGpu, GrDrawTarget::kReset_ASRInit);
        drawState = fGpu->drawState();
        drawState->setRenderTarget(rt);
        GrDrawTarget::AutoGeometryPush agp(fGpu);

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

        fGpu->clearStencilClip(bounds, clearToInside);

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
                fill = kEvenOdd_GrPathFill;
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
                pr = this->getContext()->getPathRenderer(*clipPath,
                                                         fill, fGpu, false,
                                                         true);
                if (NULL == pr) {
                    fGpu->setClip(clipCopy);     // restore to the original
                    return false;
                }
                canRenderDirectToStencil =
                    !pr->requiresStencilPass(*clipPath, fill, fGpu);
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
                    fGpu->drawSimpleRect(clipCopy.getRect(c), NULL, 0);
                } else {
                    if (canRenderDirectToStencil) {
                        *drawState->stencil() = gDrawToStencil;
                        pr->drawPath(*clipPath, fill, NULL, fGpu, 0, false);
                    } else {
                        pr->drawPathToStencil(*clipPath, fill, fGpu);
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
                        fGpu->drawSimpleRect(clipCopy.getRect(c), NULL, 0);
                    } else {
                        SET_RANDOM_COLOR
                        pr->drawPath(*clipPath, fill, NULL, fGpu, 0, false);
                    }
                } else {
                    SET_RANDOM_COLOR
                    GrRect rect = GrRect::MakeLTRB(
                            SkIntToScalar(bounds.fLeft),
                            SkIntToScalar(bounds.fTop),
                            SkIntToScalar(bounds.fRight),
                            SkIntToScalar(bounds.fBottom));
                    fGpu->drawSimpleRect(rect, NULL, 0);
                }
            }
        }
        // restore clip
        fGpu->setClip(clipCopy);
    }
    // set this last because recursive draws may overwrite it back to kNone.
    GrAssert(kNone_ClipMaskType == fCurrClipMaskType);
    fCurrClipMaskType = kStencil_ClipMaskType;
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

void GrClipMaskManager::setGpuStencil() {
    // We make two copies of the StencilSettings here (except in the early
    // exit scenario. One copy from draw state to the stack var. Then another
    // from the stack var to the gpu. We could make this class hold a ptr to
    // GrGpu's fStencilSettings and eliminate the stack copy here.

    const GrDrawState& drawState = fGpu->getDrawState();

    // use stencil for clipping if clipping is enabled and the clip
    // has been written into the stencil.
    GrClipMaskManager::StencilClipMode clipMode;
    if (this->isClipInStencil() && drawState.isClipState()) {
        clipMode = GrClipMaskManager::kRespectClip_StencilClipMode;
        // We can't be modifying the clip and respecting it at the same time.
        GrAssert(!drawState.isStateFlagEnabled(
                    GrGpu::kModifyStencilClip_StateBit));
    } else if (drawState.isStateFlagEnabled(
                    GrGpu::kModifyStencilClip_StateBit)) {
        clipMode = GrClipMaskManager::kModifyClip_StencilClipMode;
    } else {
        clipMode = GrClipMaskManager::kIgnoreClip_StencilClipMode;
    }

    GrStencilSettings settings;
    // The GrGpu client may not be using the stencil buffer but we may need to
    // enable it in order to respect a stencil clip.
    if (drawState.getStencil().isDisabled()) {
        if (GrClipMaskManager::kRespectClip_StencilClipMode == clipMode) {
            settings = basic_apply_stencil_clip_settings();
        } else {
            fGpu->disableStencil();
            return;
        }
    } else {
        settings = drawState.getStencil();
    }

    // TODO: dynamically attach a stencil buffer
    int stencilBits = 0;
    GrStencilBuffer* stencilBuffer = 
        drawState.getRenderTarget()->getStencilBuffer();
    if (NULL != stencilBuffer) {
        stencilBits = stencilBuffer->bits();
    }

#if GR_DEBUG
    if (!fGpu->getCaps().fStencilWrapOpsSupport) {
        GrAssert(settings.frontPassOp() != kIncWrap_StencilOp);
        GrAssert(settings.frontPassOp() != kDecWrap_StencilOp);
        GrAssert(settings.frontFailOp() != kIncWrap_StencilOp);
        GrAssert(settings.backFailOp() != kDecWrap_StencilOp);
        GrAssert(settings.backPassOp() != kIncWrap_StencilOp);
        GrAssert(settings.backPassOp() != kDecWrap_StencilOp);
        GrAssert(settings.backFailOp() != kIncWrap_StencilOp);
        GrAssert(settings.frontFailOp() != kDecWrap_StencilOp);
    }
#endif
    this->adjustStencilParams(&settings, clipMode, stencilBits);
    fGpu->setStencilSettings(settings);
}

void GrClipMaskManager::adjustStencilParams(GrStencilSettings* settings,
                                            StencilClipMode mode,
                                            int stencilBitCnt) {
    GrAssert(stencilBitCnt > 0);

    if (kModifyClip_StencilClipMode == mode) {
        // We assume that this clip manager itself is drawing to the GrGpu and
        // has already setup the correct values.
        return;
    }

    unsigned int clipBit = (1 << (stencilBitCnt - 1));
    unsigned int userBits = clipBit - 1;

    GrStencilSettings::Face face = GrStencilSettings::kFront_Face;
    bool twoSided = fGpu->getCaps().fTwoSidedStencilSupport;

    bool finished = false;
    while (!finished) {
        GrStencilFunc func = settings->func(face);
        uint16_t writeMask = settings->writeMask(face);
        uint16_t funcMask = settings->funcMask(face);
        uint16_t funcRef = settings->funcRef(face);

        GrAssert((unsigned) func < kStencilFuncCount);

        writeMask &= userBits;

        if (func >= kBasicStencilFuncCount) {
            int respectClip = kRespectClip_StencilClipMode == mode;
            if (respectClip) {
                // The GrGpu class should have checked this
                GrAssert(this->isClipInStencil());
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
                        GrCrash("Unknown stencil func");
                }
            } else {
                funcMask &= userBits;
                funcRef &= userBits;
            }
            const GrStencilFunc* table = 
                gSpecialToBasicStencilFunc[respectClip];
            func = table[func - kBasicStencilFuncCount];
            GrAssert(func >= 0 && func < kBasicStencilFuncCount);
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

namespace {

GrPathFill invert_fill(GrPathFill fill) {
    static const GrPathFill gInvertedFillTable[] = {
        kInverseWinding_GrPathFill, // kWinding_GrPathFill
        kInverseEvenOdd_GrPathFill, // kEvenOdd_GrPathFill
        kWinding_GrPathFill,        // kInverseWinding_GrPathFill
        kEvenOdd_GrPathFill,        // kInverseEvenOdd_GrPathFill
        kHairLine_GrPathFill,       // kHairLine_GrPathFill
    };
    GR_STATIC_ASSERT(0 == kWinding_GrPathFill);
    GR_STATIC_ASSERT(1 == kEvenOdd_GrPathFill);
    GR_STATIC_ASSERT(2 == kInverseWinding_GrPathFill);
    GR_STATIC_ASSERT(3 == kInverseEvenOdd_GrPathFill);
    GR_STATIC_ASSERT(4 == kHairLine_GrPathFill);
    GR_STATIC_ASSERT(5 == kGrPathFillCount);
    return gInvertedFillTable[fill];
}

}

bool GrClipMaskManager::createSoftwareClipMask(const GrClip& clipIn,
                                               GrTexture** result,
                                               GrIRect *resultBounds) {
    GrAssert(kNone_ClipMaskType == fCurrClipMaskType);

    if (this->clipMaskPreamble(clipIn, result, resultBounds)) {
        return true;
    }

    GrTexture* accum = fAACache.getLastMask();
    if (NULL == accum) {
        fAACache.reset();
        return false;
    }

    GrSWMaskHelper helper(this->getContext());

    helper.init(*resultBounds, NULL, false);

    int count = clipIn.getElementCount();

    bool clearToInside;
    SkRegion::Op startOp = SkRegion::kReplace_Op; // suppress warning
    int start = process_initial_clip_elements(clipIn,
                                              *resultBounds,
                                              &clearToInside,
                                              &startOp);

    helper.clear(clearToInside ? SK_ColorWHITE : 0x00000000);

    for (int i = start; i < count; ++i) {

        SkRegion::Op op = (i == start) ? startOp : clipIn.getOp(i);

        if (SkRegion::kIntersect_Op == op ||
            SkRegion::kReverseDifference_Op == op) {
            // Intersect and reverse difference require modifying pixels
            // outside of the geometry that is being "drawn". In both cases
            // we erase all the pixels outside of the geometry but
            // leave the pixels inside the geometry alone. For reverse
            // difference we invert all the pixels before clearing the ones
            // outside the geometry.
            if (SkRegion::kReverseDifference_Op == op) {
                SkRect temp = SkRect::MakeLTRB(
                                       SkIntToScalar(resultBounds->left()),
                                       SkIntToScalar(resultBounds->top()),
                                       SkIntToScalar(resultBounds->right()),
                                       SkIntToScalar(resultBounds->bottom()));

                // invert the entire scene
                helper.draw(temp, SkRegion::kXOR_Op, false, SK_ColorWHITE);
            }

            if (kRect_ClipType == clipIn.getElementType(i)) {

                // convert the rect to a path so we can invert the fill
                SkPath temp;
                temp.addRect(clipIn.getRect(i));

                helper.draw(temp, SkRegion::kReplace_Op, 
                            kInverseEvenOdd_GrPathFill, clipIn.getDoAA(i),
                            0x00000000);
            } else {
                GrAssert(kPath_ClipType == clipIn.getElementType(i));

                helper.draw(clipIn.getPath(i),
                            SkRegion::kReplace_Op,
                            invert_fill(clipIn.getPathFill(i)),
                            clipIn.getDoAA(i),
                            0x00000000);
            }

            continue;
        }

        // The other ops (union, xor, diff) only affect pixels inside
        // the geometry so they can just be drawn normally
        if (kRect_ClipType == clipIn.getElementType(i)) {

            helper.draw(clipIn.getRect(i),
                        op,
                        clipIn.getDoAA(i), SK_ColorWHITE);

        } else {
            GrAssert(kPath_ClipType == clipIn.getElementType(i));

            helper.draw(clipIn.getPath(i), 
                        op,
                        clipIn.getPathFill(i), 
                        clipIn.getDoAA(i), SK_ColorWHITE);
        }
    }

    // Because we are using the scratch texture cache, "accum" may be
    // larger than expected and have some cruft in the areas we aren't using.
    // Clear it out.

    // TODO: need a simpler way to clear the texture - can we combine
    // the clear and the writePixels (inside toTexture)
    GrDrawState* drawState = fGpu->drawState();
    GrAssert(NULL != drawState);
    GrRenderTarget* temp = drawState->getRenderTarget();
    clear(fGpu, accum, 0x00000000);
    // can't leave the accum bound as a rendertarget
    drawState->setRenderTarget(temp);

    helper.toTexture(accum);

    *result = accum;

    fCurrClipMaskType = kAlpha_ClipMaskType;
    return true;
}

////////////////////////////////////////////////////////////////////////////////
void GrClipMaskManager::releaseResources() {
    fAACache.releaseResources();
}
