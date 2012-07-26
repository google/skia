
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
#include "GrSWMaskHelper.h"

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

    drawState->sampler(maskStage)->reset(mat);

    drawState->createTextureEffect(maskStage, result);
}

bool path_needs_SW_renderer(GrContext* context,
                            GrGpu* gpu,
                            const SkPath& path,
                            GrPathFill fill,
                            bool doAA) {
    // last (false) parameter disallows use of the SW path renderer
    return NULL == context->getPathRenderer(path, fill, gpu, doAA, false);
}

GrPathFill get_path_fill(const SkPath& path) {
    switch (path.getFillType()) {
        case SkPath::kWinding_FillType:
            return kWinding_GrPathFill;
        case SkPath::kEvenOdd_FillType:
            return  kEvenOdd_GrPathFill;
        case SkPath::kInverseWinding_FillType:
            return kInverseWinding_GrPathFill;
        case SkPath::kInverseEvenOdd_FillType:
            return kInverseEvenOdd_GrPathFill;
        default:
            GrCrash("Unsupported path fill in clip.");
            return kWinding_GrPathFill; // suppress warning
    }
}

/**
 * Does any individual clip in 'clipIn' use anti-aliasing?
 */
bool requires_AA(const GrClip& clipIn) {

    GrClip::Iter iter;
    iter.reset(clipIn, GrClip::Iter::kBottom_IterStart);

    const GrClip::Iter::Clip* clip = NULL;
    for (clip = iter.skipToTopmost(SkRegion::kReplace_Op);
         NULL != clip;
         clip = iter.next()) {

        if (clip->fDoAA) {
            return true;
        }
    }

    return false;
}

}

/*
 * This method traverses the clip stack to see if the GrSoftwarePathRenderer
 * will be used on any element. If so, it returns true to indicate that the
 * entire clip should be rendered in SW and then uploaded en masse to the gpu.
 */
bool GrClipMaskManager::useSWOnlyPath(const GrClip& clipIn) {

    // TODO: generalize this function so that when
    // a clip gets complex enough it can just be done in SW regardless
    // of whether it would invoke the GrSoftwarePathRenderer.
    bool useSW = false;

    GrClip::Iter iter(clipIn, GrClip::Iter::kBottom_IterStart);
    const GrClip::Iter::Clip* clip = NULL;

    for (clip = iter.skipToTopmost(SkRegion::kReplace_Op);
         NULL != clip;
         clip = iter.next()) {

        if (SkRegion::kReplace_Op == clip->fOp) {
            // Everything before a replace op can be ignored so start
            // afresh w.r.t. determining if any element uses the SW path
            useSW = false;
        }

        // rects can always be drawn directly w/o using the software path
        // so only paths need to be checked
        if (NULL != clip->fPath &&
            path_needs_SW_renderer(this->getContext(), fGpu, 
                                   *clip->fPath, 
                                   get_path_fill(*clip->fPath),
                                   clip->fDoAA)) {
            useSW = true;
        }
    }

    return useSW;
}

////////////////////////////////////////////////////////////////////////////////
// sort out what kind of clip mask needs to be created: alpha, stencil,
// scissor, or entirely software
bool GrClipMaskManager::setupClipping(const GrClipData* clipDataIn) {
    fCurrClipMaskType = kNone_ClipMaskType;

    GrDrawState* drawState = fGpu->drawState();
    if (!drawState->isClipState() || clipDataIn->fClipStack->isEmpty()) {
        fGpu->disableScissor();
        this->setGpuStencil();
        return true;
    }

    GrRenderTarget* rt = drawState->getRenderTarget();
    // GrDrawTarget should have filtered this for us
    GrAssert(NULL != rt);

    GrIRect bounds;
    bool isIntersectionOfRects = false;

    clipDataIn->getConservativeBounds(rt, &bounds, &isIntersectionOfRects);
    if (bounds.isEmpty()) {
        return false;
    }

    bool requiresAA = requires_AA(*clipDataIn->fClipStack);
    GrAssert(requiresAA == clipDataIn->fClipStack->requiresAA());

#if GR_SW_CLIP
    // If MSAA is enabled we can do everything in the stencil buffer.
    // Otherwise check if we should just create the entire clip mask 
    // in software (this will only happen if the clip mask is anti-aliased
    // and too complex for the gpu to handle in its entirety)
    if (0 == rt->numSamples() && 
        requiresAA && 
        this->useSWOnlyPath(*clipDataIn->fClipStack)) {
        // The clip geometry is complex enough that it will be more
        // efficient to create it entirely in software
        GrTexture* result = NULL;
        GrIRect bound;
        if (this->createSoftwareClipMask(*clipDataIn, &result, &bound)) {
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
    if (0 == rt->numSamples() && requiresAA) {
        // Since we are going to create a destination texture of the correct
        // size for the mask (rather than being bound by the size of the
        // render target) we aren't going to use scissoring like the stencil
        // path does (see scissorSettings below)
        GrTexture* result = NULL;
        GrIRect bound;
        if (this->createAlphaClipMask(*clipDataIn, &result, &bound)) {
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
    if (clipDataIn->fClipStack->isRect()) {
        fGpu->enableScissor(bounds);
        this->setGpuStencil();
        return true;
    }

    // use the stencil clip if we can't represent the clip as a rectangle.
    bool useStencil = !clipDataIn->fClipStack->isEmpty() && !bounds.isEmpty();

    if (useStencil) {
        this->createStencilClipMask(*clipDataIn, bounds);
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
const GrClip::Iter::Clip* process_initial_clip_elements(
                                  GrClip::Iter* iter,
                                  const GrIRect& bounds,
                                  bool* clearToInside,
                                  SkRegion::Op* firstOp) {

    GrAssert(NULL != iter && NULL != clearToInside && NULL != firstOp);

    // logically before the first element of the clip stack is 
    // processed the clip is entirely open. However, depending on the
    // first set op we may prefer to clear to 0 for performance. We may
    // also be able to skip the initial clip paths/rects. We loop until
    // we cannot skip an element.
    bool done = false;
    *clearToInside = true;

    const GrClip::Iter::Clip* clip = NULL;

    for (clip = iter->skipToTopmost(SkRegion::kReplace_Op);
         NULL != clip && !done;
         clip = iter->next()) {
        switch (clip->fOp) {
            case SkRegion::kReplace_Op:
                // replace ignores everything previous
                *firstOp = SkRegion::kReplace_Op;
                *clearToInside = false;
                done = true;
                break;
            case SkRegion::kIntersect_Op:
                // if this element contains the entire bounds then we
                // can skip it.
                if (NULL != clip->fRect && contains(*clip->fRect, bounds)) {
                    break;
                }
                // if everything is initially clearToInside then intersect is
                // same as clear to 0 and treat as a replace. Otherwise,
                // set stays empty.
                if (*clearToInside) {
                    *firstOp = SkRegion::kReplace_Op;
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
                    *firstOp = SkRegion::kReplace_Op;
                    done = true;
                }
                break;
            case SkRegion::kXOR_Op:
                // xor is same as difference or replace both of which
                // can be 1-pass instead of 2 for xor.
                if (*clearToInside) {
                    *firstOp = SkRegion::kDifference_Op;
                } else {
                    *firstOp = SkRegion::kReplace_Op;
                }
                done = true;
                break;
            case SkRegion::kDifference_Op:
                // if all pixels are clearToInside then we have to process the
                // difference, otherwise it has no effect and all pixels
                // remain outside.
                if (*clearToInside) {
                    *firstOp = SkRegion::kDifference_Op;
                    done = true;
                }
                break;
            case SkRegion::kReverseDifference_Op:
                // if all pixels are clearToInside then reverse difference
                // produces empty set. Otherise it is same as replace
                if (*clearToInside) {
                    *clearToInside = false;
                } else {
                    *firstOp = SkRegion::kReplace_Op;
                    done = true;
                }
                break;
            default:
                GrCrash("Unknown set op.");
        }

        if (done) {
            // we need to break out here (rather than letting the test in
            // the loop do it) since backing up the iterator is very expensive
            break;
        }
    }
    return clip;
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
bool draw_path_in_software(GrContext* context,
                           GrGpu* gpu,
                           const SkPath& path,
                           GrPathFill fill,
                           bool doAA,
                           const GrIRect& resultBounds) {

    SkAutoTUnref<GrTexture> texture(
                GrSWMaskHelper::DrawPathMaskToTexture(context, path, 
                                                      resultBounds, fill, 
                                                      doAA, NULL));
    if (NULL == texture) {
        return false;
    }

    // The ClipMaskManager accumulates the clip mask in the UL corner
    GrIRect rect = GrIRect::MakeWH(resultBounds.width(), resultBounds.height());

    GrSWMaskHelper::DrawToTargetWithPathMask(texture, gpu, rect);

    GrAssert(!GrIsFillInverted(fill));
    return true;
}


////////////////////////////////////////////////////////////////////////////////
bool draw_path(GrContext* context,
               GrGpu* gpu,
               const SkPath& path,
               GrPathFill fill,
               bool doAA,
               const GrIRect& resultBounds) {

    GrPathRenderer* pr = context->getPathRenderer(path, fill, gpu, doAA, false);
    if (NULL == pr) {
        return draw_path_in_software(context, gpu, path, fill, doAA, resultBounds);
    }

    pr->drawPath(path, fill, NULL, gpu, doAA);
    return true;
}

}

////////////////////////////////////////////////////////////////////////////////
bool GrClipMaskManager::drawClipShape(GrTexture* target,
                                      const GrClip::Iter::Clip* clip,
                                      const GrIRect& resultBounds) {
    GrDrawState* drawState = fGpu->drawState();
    GrAssert(NULL != drawState);

    drawState->setRenderTarget(target->asRenderTarget());

    if (NULL != clip->fRect) {
        if (clip->fDoAA) {
            getContext()->getAARectRenderer()->fillAARect(fGpu, fGpu,
                                                          *clip->fRect, 
                                                          true);
        } else {
            fGpu->drawSimpleRect(*clip->fRect, NULL);
        }
    } else if (NULL != clip->fPath) {
        return draw_path(this->getContext(), fGpu,
                         *clip->fPath,
                         get_path_fill(*clip->fPath),
                         clip->fDoAA,
                         resultBounds);
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

    drawState->sampler(0)->reset(sampleM);
    drawState->createTextureEffect(0, texture);

    GrRect rect = GrRect::MakeWH(SkIntToScalar(target->width()), 
                                 SkIntToScalar(target->height()));

    fGpu->drawSimpleRect(rect, NULL);

    drawState->disableStage(0);
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
bool GrClipMaskManager::clipMaskPreamble(const GrClipData& clipDataIn,
                                         GrTexture** result,
                                         GrIRect* resultBounds) {
    GrDrawState* origDrawState = fGpu->drawState();
    GrAssert(origDrawState->isClipState());

    GrRenderTarget* rt = origDrawState->getRenderTarget();
    GrAssert(NULL != rt);

    // unlike the stencil path the alpha path is not bound to the size of the
    // render target - determine the minimum size required for the mask
    GrIRect intBounds;
    clipDataIn.getConservativeBounds(rt, &intBounds);

    // need to outset a pixel since the standard bounding box computation
    // path doesn't leave any room for antialiasing (esp. w.r.t. rects)
    intBounds.outset(1, 1);

    // TODO: make sure we don't outset if bounds are still 0,0 @ min

    if (fAACache.canReuse(*clipDataIn.fClipStack, 
                          intBounds.width(),
                          intBounds.height())) {
        *result = fAACache.getLastMask();
        fAACache.getLastBound(resultBounds);
        return true;
    }

    this->setupCache(*clipDataIn.fClipStack, intBounds);

    *resultBounds = intBounds;
    return false;
}

////////////////////////////////////////////////////////////////////////////////
// Create a 8-bit clip mask in alpha
bool GrClipMaskManager::createAlphaClipMask(const GrClipData& clipDataIn,
                                            GrTexture** result,
                                            GrIRect *resultBounds) {
    GrAssert(NULL != resultBounds);
    GrAssert(kNone_ClipMaskType == fCurrClipMaskType);

    if (this->clipMaskPreamble(clipDataIn, result, resultBounds)) {
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

    if (0 != resultBounds->fTop || 0 != resultBounds->fLeft) {
        // if we were able to trim down the size of the mask we need to 
        // offset the paths & rects that will be used to compute it
        GrMatrix m;

        m.setTranslate(SkIntToScalar(-resultBounds->fLeft), 
                       SkIntToScalar(-resultBounds->fTop));

        drawState->setViewMatrix(m);
    }

    bool clearToInside;
    SkRegion::Op firstOp = SkRegion::kReplace_Op; // suppress warning

    GrClip::Iter iter(*clipDataIn.fClipStack, GrClip::Iter::kBottom_IterStart);
    const GrClip::Iter::Clip* clip = process_initial_clip_elements(&iter,
                                                              *resultBounds,
                                                              &clearToInside,
                                                              &firstOp);

    fGpu->clear(NULL, 
                clearToInside ? 0xffffffff : 0x00000000, 
                accum->asRenderTarget());

    GrAutoScratchTexture temp;
    bool first = true;
    // walk through each clip element and perform its set op
    for ( ; NULL != clip; clip = iter.next()) {

        SkRegion::Op op = clip->fOp;
        if (first) {
            first = false;
            op = firstOp;
        }

        if (SkRegion::kReplace_Op == op) {
            // TODO: replace is actually a lot faster then intersection
            // for this path - refactor the stencil path so it can handle
            // replace ops and alter GrClip to allow them through

            // clear the accumulator and draw the new object directly into it
            fGpu->clear(NULL, 0x00000000, accum->asRenderTarget());

            setup_boolean_blendcoeffs(drawState, op);
            this->drawClipShape(accum, clip, *resultBounds);

        } else if (SkRegion::kReverseDifference_Op == op ||
                   SkRegion::kIntersect_Op == op) {
            // there is no point in intersecting a screen filling rectangle.
            if (SkRegion::kIntersect_Op == op &&
                NULL != clip->fRect &&
                contains(*clip->fRect, *resultBounds)) {
                continue;
            }

            getTemp(*resultBounds, &temp);
            if (NULL == temp.texture()) {
                fAACache.reset();
                return false;
            }

            // clear the temp target & draw into it
            fGpu->clear(NULL, 0x00000000, temp.texture()->asRenderTarget());

            setup_boolean_blendcoeffs(drawState, SkRegion::kReplace_Op);
            this->drawClipShape(temp.texture(), clip, *resultBounds);

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
            this->drawClipShape(accum, clip, *resultBounds);
        }
    }

    *result = accum;
    fCurrClipMaskType = kAlpha_ClipMaskType;
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Create a 1-bit clip mask in the stencil buffer
bool GrClipMaskManager::createStencilClipMask(const GrClipData& clipDataIn,
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

    if (stencilBuffer->mustRenderClip(clipDataIn, rt->width(), rt->height())) {

        stencilBuffer->setLastClip(clipDataIn, rt->width(), rt->height());

        // we set the current clip to the bounds so that our recursive
        // draws are scissored to them. We use the copy of the complex clip
        // we just stashed on the SB to render from. We set it back after
        // we finish drawing it into the stencil.
        const GrClipData* oldClipData = fGpu->getClip();

        GrClip newClipStack(bounds);
        GrClipData newClipData;
        newClipData.fClipStack = &newClipStack;

        fGpu->setClip(&newClipData);

        GrDrawTarget::AutoStateRestore asr(fGpu, GrDrawTarget::kReset_ASRInit);
        drawState = fGpu->drawState();
        drawState->setRenderTarget(rt);
        GrDrawTarget::AutoGeometryPush agp(fGpu);

#if !VISUALIZE_COMPLEX_CLIP
        drawState->enableState(GrDrawState::kNoColorWrites_StateBit);
#endif

        int clipBit = stencilBuffer->bits();
        SkASSERT((clipBit <= 16) &&
                    "Ganesh only handles 16b or smaller stencil buffers");
        clipBit = (1 << (clipBit-1));

        GrIRect rtRect = GrIRect::MakeWH(rt->width(), rt->height());

        bool clearToInside;
        SkRegion::Op firstOp = SkRegion::kReplace_Op; // suppress warning

        GrClip::Iter iter(*oldClipData->fClipStack, 
                          GrClip::Iter::kBottom_IterStart);
        const GrClip::Iter::Clip* clip = process_initial_clip_elements(&iter,
                                                  rtRect,
                                                  &clearToInside,
                                                  &firstOp);

        fGpu->clearStencilClip(bounds, clearToInside);
        bool first = true;

        // walk through each clip element and perform its set op
        // with the existing clip.
        for ( ; NULL != clip; clip = iter.next()) {
            GrPathFill fill;
            bool fillInverted;
            // enabled at bottom of loop
            drawState->disableState(GrGpu::kModifyStencilClip_StateBit);
            // if the target is MSAA then we want MSAA enabled when the clip is soft
            if (rt->isMultisampled()) {
                if (clip->fDoAA) {
                    drawState->enableState(GrDrawState::kHWAntialias_StateBit);
                } else {
                    drawState->disableState(GrDrawState::kHWAntialias_StateBit);
                }
            }

            bool canRenderDirectToStencil; // can the clip element be drawn
                                           // directly to the stencil buffer
                                           // with a non-inverted fill rule
                                           // without extra passes to
                                           // resolve in/out status.

            SkRegion::Op op = clip->fOp;
            if (first) {
                first = false;
                op = firstOp;
            }

            GrPathRenderer* pr = NULL;
            const SkPath* clipPath = NULL;
            if (NULL != clip->fRect) {
                canRenderDirectToStencil = true;
                fill = kEvenOdd_GrPathFill;
                fillInverted = false;
                // there is no point in intersecting a screen filling
                // rectangle.
                if (SkRegion::kIntersect_Op == op &&
                    contains(*clip->fRect, rtRect)) {
                    continue;
                }
            } else if (NULL != clip->fPath) {
                fill = get_path_fill(*clip->fPath);
                fillInverted = GrIsFillInverted(fill);
                fill = GrNonInvertedFill(fill);
                clipPath = clip->fPath;
                pr = this->getContext()->getPathRenderer(*clipPath,
                                                         fill, fGpu, false,
                                                         true);
                if (NULL == pr) {
                    fGpu->setClip(oldClipData);     // restore to the original
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
                if (NULL != clip->fRect) {
                    *drawState->stencil() = gDrawToStencil;
                    fGpu->drawSimpleRect(*clip->fRect, NULL);
                } else {
                    if (canRenderDirectToStencil) {
                        *drawState->stencil() = gDrawToStencil;
                        pr->drawPath(*clipPath, fill, NULL, fGpu, false);
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
                    if (NULL != clip->fRect) {
                        SET_RANDOM_COLOR
                        fGpu->drawSimpleRect(*clip->fRect, NULL);
                    } else {
                        SET_RANDOM_COLOR
                        pr->drawPath(*clipPath, fill, NULL, fGpu, false);
                    }
                } else {
                    SET_RANDOM_COLOR
                    GrRect rect = GrRect::MakeLTRB(
                            SkIntToScalar(bounds.fLeft),
                            SkIntToScalar(bounds.fTop),
                            SkIntToScalar(bounds.fRight),
                            SkIntToScalar(bounds.fBottom));
                    fGpu->drawSimpleRect(rect, NULL);
                }
            }
        }
        // restore clip
        fGpu->setClip(oldClipData);
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

    GrAssert(fGpu->getCaps().fStencilWrapOpsSupport ||
             !settings.usesWrapOp());
    GrAssert(fGpu->getCaps().fTwoSidedStencilSupport || !settings.isTwoSided());
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

bool GrClipMaskManager::createSoftwareClipMask(const GrClipData& clipDataIn,
                                               GrTexture** result,
                                               GrIRect* resultBounds) {
    GrAssert(kNone_ClipMaskType == fCurrClipMaskType);

    if (this->clipMaskPreamble(clipDataIn, result, resultBounds)) {
        return true;
    }

    GrTexture* accum = fAACache.getLastMask();
    if (NULL == accum) {
        fAACache.reset();
        return false;
    }

    GrSWMaskHelper helper(this->getContext());

    helper.init(*resultBounds, NULL);

    bool clearToInside;
    SkRegion::Op firstOp = SkRegion::kReplace_Op; // suppress warning

    GrClip::Iter iter(*clipDataIn.fClipStack, GrClip::Iter::kBottom_IterStart);
    const GrClip::Iter::Clip* clip = process_initial_clip_elements(&iter,
                                              *resultBounds,
                                              &clearToInside,
                                              &firstOp);

    helper.clear(clearToInside ? 0xFF : 0x00);

    bool first = true;
    for ( ; NULL != clip; clip = iter.next()) {

        SkRegion::Op op = clip->fOp;
        if (first) {
            first = false;
            op = firstOp;
        }

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
                helper.draw(temp, SkRegion::kXOR_Op, false, 0xFF);
            }

            if (NULL != clip->fRect) {

                // convert the rect to a path so we can invert the fill
                SkPath temp;
                temp.addRect(*clip->fRect);

                helper.draw(temp, SkRegion::kReplace_Op, 
                            kInverseEvenOdd_GrPathFill, clip->fDoAA,
                            0x00);
            } else if (NULL != clip->fPath) {
                helper.draw(*clip->fPath,
                            SkRegion::kReplace_Op,
                            invert_fill(get_path_fill(*clip->fPath)),
                            clip->fDoAA,
                            0x00);
            }

            continue;
        }

        // The other ops (union, xor, diff) only affect pixels inside
        // the geometry so they can just be drawn normally
        if (NULL != clip->fRect) {

            helper.draw(*clip->fRect,
                        op,
                        clip->fDoAA, 0xFF);

        } else if (NULL != clip->fPath) {
            helper.draw(*clip->fPath, 
                        op,
                        get_path_fill(*clip->fPath),
                        clip->fDoAA, 0xFF);
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
    fGpu->clear(NULL, 0x00000000, accum->asRenderTarget());
    // can't leave the accum bound as a rendertarget
    drawState->setRenderTarget(temp);

    helper.toTexture(accum, clearToInside ? 0xFF : 0x00);

    *result = accum;

    fCurrClipMaskType = kAlpha_ClipMaskType;
    return true;
}

////////////////////////////////////////////////////////////////////////////////
void GrClipMaskManager::releaseResources() {
    fAACache.releaseResources();
}
