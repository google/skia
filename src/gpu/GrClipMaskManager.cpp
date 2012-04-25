
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

void ScissoringSettings::setupScissoring(GrGpu* gpu) {
    if (!fEnableScissoring) {
        gpu->disableScissor();
        return;
    }

    gpu->enableScissoring(fScissorRect);
}

// sort out what kind of clip mask needs to be created: A8/R8, stencil or scissor
bool GrClipMaskManager::createClipMask(GrGpu* gpu, 
                                       const GrClip& clipIn,
                                       ScissoringSettings* scissorSettings) {

    GrAssert(scissorSettings);

    scissorSettings->fEnableScissoring = false;
    fClipMaskInStencil = false;

    GrDrawState* drawState = gpu->drawState();
    if (!drawState->isClipState()) {
        return true;
    }

    GrRenderTarget* rt = drawState->getRenderTarget();

    // GrDrawTarget should have filtered this for us
    GrAssert(NULL != rt);

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
// determines how many elements at the head of the clip can be skipped and
// whether the initial clear should be to the inside- or outside-the-clip value,
// and what op should be used to draw the first element that isn't skipped.
int process_initial_clip_elements(const GrClip& clip,
                                  const GrRect& bounds,
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
                    && clip.getRect(curr).contains(bounds)) {
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

        GrRect rtRect;
        rtRect.setLTRB(0, 0,
                       GrIntToScalar(rt->width()), GrIntToScalar(rt->height()));

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

            GrPathRenderer* pr = NULL;
            const GrPath* clipPath = NULL;
            if (kRect_ClipType == clipCopy.getElementType(c)) {
                canRenderDirectToStencil = true;
                fill = kEvenOdd_PathFill;
                fillInverted = false;
                // there is no point in intersecting a screen filling
                // rectangle.
                if (SkRegion::kIntersect_Op == clipCopy.getOp(c) &&
                    clipCopy.getRect(c).contains(rtRect)) {
                    continue;
                }
            } else {
                fill = clipCopy.getPathFill(c);
                fillInverted = GrIsFillInverted(fill);
                fill = GrNonInvertedFill(fill);
                clipPath = &clipCopy.getPath(c);
                pr = this->getClipPathRenderer(gpu, *clipPath, fill);
                if (NULL == pr) {
                    fClipMaskInStencil = false;
                    gpu->setClip(clipCopy);     // restore to the original
                    return false;
                }
                canRenderDirectToStencil =
                    !pr->requiresStencilPass(*clipPath, fill, gpu);
            }

            SkRegion::Op op = (c == start) ? startOp : clipCopy.getOp(c);
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

GrPathRenderer* GrClipMaskManager::getClipPathRenderer(GrGpu* gpu,
                                                       const GrPath& path,
                                                       GrPathFill fill) {
    if (NULL == fPathRendererChain) {
        fPathRendererChain = 
            new GrPathRendererChain(gpu->getContext(),
                                    GrPathRendererChain::kNonAAOnly_UsageFlag);
    }
    return fPathRendererChain->getPathRenderer(path, fill, gpu, false);
}

void GrClipMaskManager::freeResources() {
    // in case path renderer has any GrResources, start from scratch
    GrSafeSetNull(fPathRendererChain);
}
