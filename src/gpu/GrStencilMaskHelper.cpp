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
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/geometry/GrShape.h"
#include "src/gpu/geometry/GrStyledShape.h"

namespace {

////////////////////////////////////////////////////////////////////////////////
// Stencil Rules for Merging user stencil space into clip
//

///////
// Replace
static constexpr GrUserStencilSettings gUserToClipReplace(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kNotEqual,
        0xffff,
        GrUserStencilOp::kSetClipAndReplaceUserBits,
        GrUserStencilOp::kZeroClipAndUserBits,
        0xffff>()
);

static constexpr GrUserStencilSettings gInvUserToClipReplace(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kEqual,
        0xffff,
        GrUserStencilOp::kSetClipAndReplaceUserBits,
        GrUserStencilOp::kZeroClipAndUserBits,
        0xffff>()
);

///////
// Intersect
static constexpr GrUserStencilSettings gUserToClipIsect(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kLessIfInClip, // "0 < userBits" is equivalent to "0 != userBits".
        0xffff,
        GrUserStencilOp::kSetClipAndReplaceUserBits,
        GrUserStencilOp::kZeroClipAndUserBits,
        0xffff>()
);

///////
// Difference
static constexpr GrUserStencilSettings gUserToClipDiff(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kEqualIfInClip,
        0xffff,
        GrUserStencilOp::kSetClipAndReplaceUserBits,
        GrUserStencilOp::kZeroClipAndUserBits,
        0xffff>()
);

///////
// Union
static constexpr GrUserStencilSettings gUserToClipUnion(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kNotEqual,
        0xffff,
        GrUserStencilOp::kSetClipAndReplaceUserBits,
        GrUserStencilOp::kKeep,
        0xffff>()
);

static constexpr GrUserStencilSettings gInvUserToClipUnionPass0( // Does not zero user bits.
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kEqual,
        0xffff,
        GrUserStencilOp::kSetClipBit,
        GrUserStencilOp::kKeep,
        0x0000>()
);

///////
// Xor
static constexpr GrUserStencilSettings gUserToClipXorPass0( // Does not zero user bits.
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kNotEqual,
        0xffff,
        GrUserStencilOp::kInvertClipBit,
        GrUserStencilOp::kKeep,
        0x0000>()
);

static constexpr GrUserStencilSettings gInvUserToClipXorPass0( // Does not zero user bits.
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kEqual,
        0xffff,
        GrUserStencilOp::kInvertClipBit,
        GrUserStencilOp::kKeep,
        0x0000>()
);

///////
// Reverse Diff
static constexpr GrUserStencilSettings gUserToClipRDiffPass0( // Does not zero user bits.
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kNotEqual,
        0xffff,
        GrUserStencilOp::kInvertClipBit,
        GrUserStencilOp::kZeroClipBit,
        0x0000>()
);

static constexpr GrUserStencilSettings gInvUserToClipRDiffPass0( // Does not zero user bits.
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kEqual,
        0xffff,
        GrUserStencilOp::kInvertClipBit,
        GrUserStencilOp::kZeroClipBit,
        0x0000>()
);

///////
// Second pass to clear user bits (only needed sometimes)
static constexpr GrUserStencilSettings gZeroUserBits(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kNotEqual,
        0xffff,
        GrUserStencilOp::kZero,
        GrUserStencilOp::kKeep,
        0xffff>()
);

static constexpr const GrUserStencilSettings* gUserToClipTable[2][1 + SkRegion::kLastOp][3] = {
    {  /* Normal fill. */
        {&gUserToClipDiff,           nullptr,         nullptr},  // kDifference_Op.
        {&gUserToClipIsect,          nullptr,         nullptr},  // kIntersect_Op.
        {&gUserToClipUnion,          nullptr,         nullptr},  // kUnion_Op.
        {&gUserToClipXorPass0,       &gZeroUserBits,  nullptr},  // kXOR_Op.
        {&gUserToClipRDiffPass0,     &gZeroUserBits,  nullptr},  // kReverseDifference_Op.
        {&gUserToClipReplace,        nullptr,         nullptr}   // kReplace_Op.

    }, /* Inverse fill. */ {
        {&gUserToClipIsect,          nullptr,         nullptr},  // ~diff (aka isect).
        {&gUserToClipDiff,           nullptr,         nullptr},  // ~isect (aka diff).
        {&gInvUserToClipUnionPass0,  &gZeroUserBits,  nullptr},  // ~union.
        {&gInvUserToClipXorPass0,    &gZeroUserBits,  nullptr},  // ~xor.
        {&gInvUserToClipRDiffPass0,  &gZeroUserBits,  nullptr},  // ~reverse diff.
        {&gInvUserToClipReplace,     nullptr,         nullptr}   // ~replace.
    }
};

///////
// Direct to Stencil

// We can render a clip element directly without first writing to the client
// portion of the clip when the fill is not inverse and the set operation will
// only modify the in/out status of samples covered by the clip element.

// this one only works if used right after stencil clip was cleared.
// Our clip mask creation code doesn't allow midstream replace ops.
static constexpr GrUserStencilSettings gReplaceClip(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kAlways,
        0xffff,
        GrUserStencilOp::kSetClipBit,
        GrUserStencilOp::kSetClipBit,
        0x0000>()
);

static constexpr GrUserStencilSettings gUnionClip(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kAlwaysIfInClip,
        0xffff,
        GrUserStencilOp::kKeep,
        GrUserStencilOp::kSetClipBit,
        0x0000>()
);

static constexpr GrUserStencilSettings gXorClip(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kAlways,
        0xffff,
        GrUserStencilOp::kInvertClipBit,
        GrUserStencilOp::kInvertClipBit,
        0x0000>()
);

static constexpr GrUserStencilSettings gDiffClip(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kAlwaysIfInClip,
        0xffff,
        GrUserStencilOp::kZeroClipBit,
        GrUserStencilOp::kKeep,
        0x0000>()
);

static constexpr const GrUserStencilSettings* gDirectDrawTable[1 + SkRegion::kLastOp][2] = {
    {&gDiffClip,     nullptr},  // kDifference_Op.
    {nullptr,        nullptr},  // kIntersect_Op.
    {&gUnionClip,    nullptr},  // kUnion_Op.
    {&gXorClip,      nullptr},  // kXOR_Op.
    {nullptr,        nullptr},  // kReverseDifference_Op.
    {&gReplaceClip,  nullptr}   // kReplace_Op.
};

static_assert(0 == SkRegion::kDifference_Op);
static_assert(1 == SkRegion::kIntersect_Op);
static_assert(2 == SkRegion::kUnion_Op);
static_assert(3 == SkRegion::kXOR_Op);
static_assert(4 == SkRegion::kReverseDifference_Op);
static_assert(5 == SkRegion::kReplace_Op);

// Settings used to when not allowed to draw directly to the clip to fill the user stencil bits
// before applying the covering clip stencil passes.
static constexpr GrUserStencilSettings gDrawToStencil(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kAlways,
        0xffff,
        GrUserStencilOp::kIncMaybeClamp,
        GrUserStencilOp::kIncMaybeClamp,
        0xffff>()
);

// Get the stencil settings per-pass to achieve the given fill+region op effect on the
// stencil buffer.
//
// If drawDirectToClip comes back false, the caller must first draw the element into the user
// stencil bits, and then cover the clip area with multiple passes using the returned
// stencil settings.

// If drawDirectToClip is true, the returned array will only have one pass and the
// caller should use those stencil settings while drawing the element directly.
//
// This returns a null-terminated list of const GrUserStencilSettings*
static GrUserStencilSettings const* const* get_stencil_passes(
        SkRegion::Op op, GrPathRenderer::StencilSupport stencilSupport, bool fillInverted,
        bool* drawDirectToClip) {
    bool canRenderDirectToStencil =
            GrPathRenderer::kNoRestriction_StencilSupport == stencilSupport;

    // TODO: inverse fill + intersect op can be direct.
    // TODO: this can be greatly simplified when we only need intersect and difference ops and
    //       none of the paths will be inverse-filled (just toggle the op instead).
    SkASSERT((unsigned)op <= SkRegion::kLastOp);
    if (canRenderDirectToStencil && !fillInverted) {
        GrUserStencilSettings const* const* directPass = gDirectDrawTable[op];
        if (directPass[0]) {
            *drawDirectToClip = true;
            return directPass;
        }
    }
    *drawDirectToClip = false;
    return gUserToClipTable[fillInverted][op];
}

static void draw_stencil_rect(GrSurfaceDrawContext* rtc, const GrHardClip& clip,
                              const GrUserStencilSettings* ss, const SkMatrix& matrix,
                              const SkRect& rect, GrAA aa) {
    GrPaint paint;
    paint.setXPFactory(GrDisableColorXPFactory::Get());
    rtc->stencilRect(&clip, ss, std::move(paint), aa, matrix, rect);
}

static void draw_path(GrRecordingContext* context,
                      GrSurfaceDrawContext* rtc,
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

static void stencil_path(GrRecordingContext* context,
                         GrSurfaceDrawContext* rtc,
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

static GrAA supported_aa(GrSurfaceDrawContext* rtc, GrAA aa) {
    // MIXED SAMPLES TODO: We can use stencil with mixed samples as well.
    if (rtc->numSamples() > 1) {
        if (rtc->caps()->multisampleDisableSupport()) {
            return aa;
        } else {
            return GrAA::kYes;
        }
    } else {
        return GrAA::kNo;
    }
}

}  // namespace

bool GrStencilMaskHelper::init(const SkIRect& bounds, uint32_t genID,
                               const GrWindowRectangles& windowRects, int numFPs) {
    if (!fRTC->mustRenderClip(genID, bounds, numFPs)) {
        return false;
    }

    fClip.setStencilClip(genID);
    // Should have caught bounds not intersecting the render target much earlier in clip application
    SkAssertResult(fClip.fixedClip().setScissor(bounds));
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
        draw_stencil_rect(fRTC, fClip.fixedClip(), &gDrawToStencil, matrix, rect, aa);
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
                      &gDrawToStencil, matrix, shape, aa);
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
    if (fClip.fixedClip().hasWindowRectangles()) {
        // Use a draw to benefit from window rectangles when resetting the stencil buffer; for
        // large buffers with MSAA this can be significant.
        draw_stencil_rect(fRTC, fClip.fixedClip(),
                          GrStencilSettings::SetClipBitSettings(insideStencil), SkMatrix::I(),
                          SkRect::Make(fClip.fixedClip().scissorRect()), GrAA::kNo);
    } else {
        fRTC->clearStencilClip(fClip.fixedClip().scissorRect(), insideStencil);
    }
}

void GrStencilMaskHelper::finish() {
    fRTC->setLastClip(fClip.stencilStackID(), fClip.fixedClip().scissorRect(), fNumFPs);
}
