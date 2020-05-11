/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrStencilMaskHelper.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/geometry/GrShape.h"
#include "src/gpu/geometry/GrStyledShape.h"

namespace {

// Get the stencil stencil settings per-pass to achieve the given fill+region op effect on the
// stencil buffer. 'drawDirectToClip' will be set to true if the draws can apply to the clip bit,
// otherwise user stencil bits must be used with a follow-up covering pass.
static GrUserStencilSettings const* const* get_stencil_passes(
        SkRegion::Op op, GrPathRenderer::StencilSupport stencilSupport, bool fillInverted,
        bool* drawDirectToClip) {
    bool canRenderDirectToStencil =
            GrPathRenderer::kNoRestriction_StencilSupport == stencilSupport;
    return GrStencilSettings::GetClipPasses(op, canRenderDirectToStencil, fillInverted,
                                            drawDirectToClip);
}

static void draw_stencil_rect(GrRenderTargetContext* rtc, const GrHardClip& clip,
                              const GrUserStencilSettings* ss, const SkMatrix& matrix,
                              const SkRect& rect, GrAA aa) {
    GrPaint paint;
    paint.setXPFactory(GrDisableColorXPFactory::Get());
    rtc->priv().stencilRect(clip, ss, std::move(paint), aa, matrix, rect);
}

static void draw_path(GrRecordingContext* context, GrRenderTargetContext* rtc,
                      GrPathRenderer* pr, const GrHardClip& clip, const SkIRect& bounds,
                      const GrUserStencilSettings* ss,  const SkMatrix& matrix,
                      const GrStyledShape& shape, GrAA aa) {
    GrPaint paint;
    paint.setXPFactory(GrDisableColorXPFactory::Get());

    // Since we are only drawing to the stencil buffer, we can use kMSAA even if the render
    // target is mixed sampled.
    GrAAType pathAAType = aa == GrAA::kYes ? GrAAType::kMSAA : GrAAType::kNone;

    GrPathRenderer::DrawPathArgs args{context,
                                      std::move(paint),
                                      ss,
                                      rtc,
                                      &clip,
                                      &bounds,
                                      &matrix,
                                      &shape,
                                      pathAAType,
                                      false};
    pr->drawPath(args);
}

static void stencil_path(GrRecordingContext* context, GrRenderTargetContext* rtc,
                         GrPathRenderer* pr, const GrFixedClip& clip, const SkMatrix& matrix,
                         const GrStyledShape& shape, GrAA aa) {
    GrPathRenderer::StencilPathArgs args;
    args.fContext = context;
    args.fRenderTargetContext = rtc;
    args.fClip = &clip;
    args.fClipConservativeBounds = &clip.scissorRect();
    args.fViewMatrix = &matrix;
    args.fShape = &shape;
    args.fDoStencilMSAA = aa;

    pr->stencilPath(args);
}

static GrAA supported_aa(GrRenderTargetContext* rtc, GrAA aa) {
    // MIXED SAMPLES TODO: We can use stencil with mixed samples as well.
    return rtc->numSamples() > 1 ? aa : GrAA::kNo;
}

static constexpr GrUserStencilSettings kDrawToStencil(
                 GrUserStencilSettings::StaticInit<
                     0x0000,
                     GrUserStencilTest::kAlways,
                     0xffff,
                     GrUserStencilOp::kIncMaybeClamp,
                     GrUserStencilOp::kIncMaybeClamp,
                     0xffff>()
            );

} // anonymous

bool GrStencilMaskHelper::init(const SkIRect& bounds, uint32_t genID,
                               const GrWindowRectangles& windowRects, int numFPs) {
    if (!fRTC->priv().mustRenderClip(genID, bounds, numFPs)) {
        return false;
    }

    fClip.setStencilClip(genID);
    fClip.fixedClip().setScissor(bounds);
    if (!windowRects.empty()) {
        fClip.fixedClip().setWindowRectangles(
                windowRects, GrWindowRectsState::Mode::kExclusive);
    }
    fNumFPs = numFPs;
    return true;
}

void GrStencilMaskHelper::drawRect(const SkRect& rect,
                                   const SkMatrix& matrix,
                                   SkRegion::Op op,
                                   GrAA aa) {
    if (rect.isEmpty()) {
        return;
    }

    bool drawDirectToClip;
    auto passes = get_stencil_passes(op, GrPathRenderer::kNoRestriction_StencilSupport, false,
                                     &drawDirectToClip);
    aa = supported_aa(fRTC, aa);

    if (!drawDirectToClip) {
        // Draw to client stencil bits first
        draw_stencil_rect(fRTC, fClip.fixedClip(), &kDrawToStencil, matrix, rect, aa);
    }

    // Now modify the clip bit (either by rendering directly), or by covering the bounding box
    // of the clip
    for (GrUserStencilSettings const* const* pass = passes; *pass; ++pass) {
        if (drawDirectToClip) {
            draw_stencil_rect(fRTC, fClip, *pass, matrix, rect, aa);
        } else {
            draw_stencil_rect(fRTC, fClip, *pass, SkMatrix::I(),
                              SkRect::Make(fClip.fixedClip().scissorRect()), aa);
        }
    }
}

bool GrStencilMaskHelper::drawPath(const SkPath& path,
                                   const SkMatrix& matrix,
                                   SkRegion::Op op,
                                   GrAA aa) {
    if (path.isEmpty()) {
        return true;
    }

    // drawPath follows a similar approach to drawRect(), where we either draw directly to the clip
    // bit or first draw to client bits and then apply a cover pass. The complicating factor is that
    // we rely on path rendering and how the chosen path renderer uses the stencil buffer.
    aa = supported_aa(fRTC, aa);

    GrAAType pathAAType = aa == GrAA::kYes ? GrAAType::kMSAA : GrAAType::kNone;

    // This will be used to determine whether the clip shape can be rendered into the
    // stencil with arbitrary stencil settings.
    GrPathRenderer::StencilSupport stencilSupport;

    // Make path canonical with regards to fill type (inverse handled by stencil settings).
    bool fillInverted = path.isInverseFillType();
    SkTCopyOnFirstWrite<SkPath> clipPath(path);
    if (fillInverted) {
        clipPath.writable()->toggleInverseFillType();
    }

    GrStyledShape shape(*clipPath, GrStyle::SimpleFill());
    SkASSERT(!shape.inverseFilled());

    GrPathRenderer::CanDrawPathArgs canDrawArgs;
    canDrawArgs.fCaps = fContext->priv().caps();
    canDrawArgs.fProxy = fRTC->asRenderTargetProxy();
    canDrawArgs.fClipConservativeBounds = &fClip.fixedClip().scissorRect();
    canDrawArgs.fViewMatrix = &matrix;
    canDrawArgs.fShape = &shape;
    canDrawArgs.fPaint = nullptr;
    canDrawArgs.fAAType = pathAAType;
    canDrawArgs.fHasUserStencilSettings = false;
    canDrawArgs.fTargetIsWrappedVkSecondaryCB = fRTC->wrapsVkSecondaryCB();

    GrPathRenderer* pr =  fContext->priv().drawingManager()->getPathRenderer(
            canDrawArgs, false, GrPathRendererChain::DrawType::kStencil, &stencilSupport);
    if (!pr) {
        return false;
    }

    bool drawDirectToClip;
    auto passes = get_stencil_passes(op, stencilSupport, fillInverted, &drawDirectToClip);

    // Write to client bits if necessary
    if (!drawDirectToClip) {
        if (stencilSupport == GrPathRenderer::kNoRestriction_StencilSupport) {
            draw_path(fContext, fRTC, pr, fClip.fixedClip(), fClip.fixedClip().scissorRect(),
                      &kDrawToStencil, matrix, shape, aa);
        } else {
            stencil_path(fContext, fRTC, pr, fClip.fixedClip(), matrix, shape, aa);
        }
    }

    // Now modify the clip bit (either by rendering directly), or by covering the bounding box
    // of the clip
    for (GrUserStencilSettings const* const* pass = passes; *pass; ++pass) {
        if (drawDirectToClip) {
            draw_path(fContext, fRTC, pr, fClip, fClip.fixedClip().scissorRect(),
                      *pass, matrix, shape, aa);
        } else {
            draw_stencil_rect(fRTC, fClip, *pass, SkMatrix::I(),
                              SkRect::Make(fClip.fixedClip().scissorRect()), aa);
        }
    }

    return true;
}

bool GrStencilMaskHelper::drawShape(const GrShape& shape,
                                    const SkMatrix& matrix,
                                    SkRegion::Op op,
                                    GrAA aa) {
    if (shape.isRect() && !shape.inverted()) {
        this->drawRect(shape.rect(), matrix, op, aa);
        return true;
    } else {
        SkPath p;
        shape.asPath(&p);
        return this->drawPath(p, matrix, op, aa);
    }
}

void GrStencilMaskHelper::clear(bool insideStencil) {
    fRTC->priv().clearStencilClip(fClip.fixedClip(), insideStencil);
}

void GrStencilMaskHelper::finish() {
    fRTC->priv().setLastClip(fClip.stencilStackID(), fClip.fixedClip().scissorRect(), fNumFPs);
}
