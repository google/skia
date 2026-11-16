/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/draw/DrawOrder.h"
#include "src/gpu/graphite/draw/DrawParams.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/NonMSAAClip.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Shape.h"
#include "src/gpu/graphite/geom/Transform.h"
#include "tests/Test.h"

#include <memory>
#include <optional>

using namespace skgpu::graphite;

// Directly verifies RenderStep::getScissor enforces (dstCopyBounds & params.scissor()) when a
// pass has non-empty dstCopyBounds (kTextureCopy) and DrawParams::readsDst() is true, shares
// dstCopyBounds across unclipped dst-reading draws and multi-step renderers without scissor
// thrashing, and restores wider scissors on non-dst draws.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(DstReadCopyScissorTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNever) {
    const RenderStep& step = context->priv().rendererProvider()->nonAABounds()->step(0);
    const Transform identity = Transform::Identity();
    const SkIRect deviceBounds = SkIRect::MakeWH(1600, 512);
    const SkIRect clipScissor = SkIRect::MakeLTRB(508, 0, 1600, 512);
    const SkIRect singleDstCopyBounds = SkIRect::MakeLTRB(508, 0, 1025, 512);

    const Rect shapeBounds(508.0f, 0.0f, 1024.2f, 512.0f);
    const Geometry geom{Shape{shapeBounds}};
    const Clip clip(shapeBounds, shapeBounds, clipScissor, NonMSAAClip{}, /*shader=*/nullptr);

    const DrawParams dstReadParams(identity,
                                   geom,
                                   clip,
                                   DrawOrder(PaintersDepth::First()),
                                   /*stroke=*/nullptr,
                                   BarrierType::kNone,
                                   /*readsDst=*/true);

    // When dstCopyBounds is empty (e.g. kFramebufferFetch / kReadFromInput), getScissor elides the
    // scissor change because shapeBounds clipped to clipScissor already equals drawBounds.
    REPORTER_ASSERT(reporter,
                    !step.getScissor(dstReadParams, clipScissor, deviceBounds, SkIRect::MakeEmpty())
                             .has_value());

    // When dstCopyBounds is non-empty (kTextureCopy), getScissor tightens to (dstCopyBounds &
    // scissor).
    std::optional<SkIRect> newScissor =
            step.getScissor(dstReadParams, clipScissor, deviceBounds, singleDstCopyBounds);
    REPORTER_ASSERT(reporter, newScissor.has_value() && *newScissor == singleDstCopyBounds);

    // Subsequent steps/draws with the same scissor already active require no state change.
    REPORTER_ASSERT(
            reporter,
            !step.getScissor(dstReadParams, singleDstCopyBounds, deviceBounds, singleDstCopyBounds)
                     .has_value());

    // Two disjoint unclipped dst-reading draws sharing aggregate dstCopyBounds [508, 0, 1500, 512]
    // share one SetScissor with zero scissor thrashing between them, while a draw with a tighter
    // clip scissor [600, 50, 900, 400] still respects its own clip scissor.
    const SkIRect unionDstCopyBounds = SkIRect::MakeLTRB(508, 0, 1500, 512);
    const Rect secondDstBounds(1400.0f, 0.0f, 1500.0f, 512.0f);
    const Clip secondDstClip(
            secondDstBounds, secondDstBounds, deviceBounds, NonMSAAClip{}, /*shader=*/nullptr);
    const DrawParams secondDstParams(identity,
                                     Geometry{Shape{secondDstBounds}},
                                     secondDstClip,
                                     DrawOrder(PaintersDepth::First()),
                                     /*stroke=*/nullptr,
                                     BarrierType::kNone,
                                     /*readsDst=*/true);
    std::optional<SkIRect> firstUnionScissor =
            step.getScissor(dstReadParams, deviceBounds, deviceBounds, unionDstCopyBounds);
    REPORTER_ASSERT(reporter,
                    firstUnionScissor.has_value() && *firstUnionScissor == unionDstCopyBounds);
    REPORTER_ASSERT(
            reporter,
            !step.getScissor(secondDstParams, unionDstCopyBounds, deviceBounds, unionDstCopyBounds)
                     .has_value());

    const SkIRect tighterClipScissor = SkIRect::MakeLTRB(600, 50, 900, 400);
    const Rect tighterBounds(600.0f, 50.0f, 900.0f, 400.0f);
    const Clip tighterClip(
            tighterBounds, tighterBounds, tighterClipScissor, NonMSAAClip{}, /*shader=*/nullptr);
    const DrawParams tighterDstParams(identity,
                                      Geometry{Shape{tighterBounds}},
                                      tighterClip,
                                      DrawOrder(PaintersDepth::First()),
                                      /*stroke=*/nullptr,
                                      BarrierType::kNone,
                                      /*readsDst=*/true);
    std::optional<SkIRect> tighterScissor =
            step.getScissor(tighterDstParams, unionDstCopyBounds, deviceBounds, unionDstCopyBounds);
    REPORTER_ASSERT(reporter, tighterScissor.has_value() && *tighterScissor == tighterClipScissor);

    // A subsequent non-dst-reading draw outside singleDstCopyBounds restores deviceBounds.
    const Rect otherBounds(1200.0f, 100.0f, 1300.0f, 200.0f);
    const Geometry otherGeom{Shape{otherBounds}};
    const Clip otherClip(otherBounds, otherBounds, deviceBounds, NonMSAAClip{}, /*shader=*/nullptr);
    const DrawParams nonDstReadParams(identity,
                                      otherGeom,
                                      otherClip,
                                      DrawOrder(PaintersDepth::First()),
                                      /*stroke=*/nullptr,
                                      BarrierType::kNone,
                                      /*readsDst=*/false);
    std::optional<SkIRect> restoredScissor = step.getScissor(
            nonDstReadParams, singleDstCopyBounds, deviceBounds, singleDstCopyBounds);
    REPORTER_ASSERT(reporter, restoredScissor.has_value() && *restoredScissor == deviceBounds);

    // Multi-step regular fill (StencilTessellatedWedges[winding]): both Step 0 (stencil) and
    // Step 1 (cover) share singleDstCopyBounds with no redundant SetScissor on Step 1.
    const RendererProvider* renderers = context->priv().rendererProvider();
    const Renderer* regularWedges = renderers->stencilTessellatedWedges(SkPathFillType::kWinding);
    REPORTER_ASSERT(reporter, regularWedges->numRenderSteps() == 2);
    std::optional<SkIRect> regularStep0Scissor = regularWedges->step(0).getScissor(
            dstReadParams, deviceBounds, deviceBounds, singleDstCopyBounds);
    REPORTER_ASSERT(reporter,
                    regularStep0Scissor.has_value() && *regularStep0Scissor == singleDstCopyBounds);
    REPORTER_ASSERT(
            reporter,
            !regularWedges->step(1)
                     .getScissor(
                             dstReadParams, singleDstCopyBounds, deviceBounds, singleDstCopyBounds)
                     .has_value());

    // Multi-step inverse fill (StencilTessellatedWedges[inverse-winding]): inverse fill contributes
    // invClipScissor to dstCopyBounds, so both Step 0 (kIgnoreInverseFill) and Step 1
    // (kInverseFillsScissor) share invClipScissor with zero step-to-step scissor thrashing.
    const Renderer* inverseWedges =
            renderers->stencilTessellatedWedges(SkPathFillType::kInverseWinding);
    REPORTER_ASSERT(reporter, inverseWedges->numRenderSteps() == 2);
    const SkIRect invClipScissor = SkIRect::MakeLTRB(508, 0, 1400, 512);
    const Rect invShapeBounds(600.2f, 50.0f, 900.8f, 400.0f);
    Shape invShape{invShapeBounds};
    invShape.setInverted(true);
    const Clip invClip(Rect(invClipScissor),
                       invShapeBounds,
                       invClipScissor,
                       NonMSAAClip{},
                       /*shader=*/nullptr);
    const DrawParams invParams(identity,
                               Geometry{invShape},
                               invClip,
                               DrawOrder(PaintersDepth::First()),
                               /*stroke=*/nullptr,
                               BarrierType::kNone,
                               /*readsDst=*/true);

    std::optional<SkIRect> invStep0Scissor = inverseWedges->step(0).getScissor(
            invParams, deviceBounds, deviceBounds, invClipScissor);
    REPORTER_ASSERT(reporter, invStep0Scissor.has_value() && *invStep0Scissor == invClipScissor);
    REPORTER_ASSERT(reporter,
                    !inverseWedges->step(1)
                             .getScissor(invParams, invClipScissor, deviceBounds, invClipScissor)
                             .has_value());
}

// End-to-end GPU verification on DstReadStrategy::kTextureCopy backends that dstCopyBounds
// scissoring prevents overhanging GPU fragments from sampling uninitialized padding of a recycled
// approximate-fit DstCopy texture.
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(DstReadCopyBoundsClampTest,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    if (context->supportsProtectedContent()) {
        return;
    }

    const Caps* caps = context->priv().caps();
    if (caps->getDstReadStrategy() != DstReadStrategy::kTextureCopy) {
        return;
    }

    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    REPORTER_ASSERT(reporter, recorder);

    // Step 1: Populate a 1024x512 DstCopy scratch texture in ResourceCache with a sentinel color
    // (SK_ColorGREEN) across all 1024x512 texels, then release it back to the cache.
    {
        const SkImageInfo seedII =
                SkImageInfo::Make(1024, 512, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        sk_sp<SkSurface> seedSurface = SkSurfaces::RenderTarget(recorder.get(), seedII);
        REPORTER_ASSERT(reporter, seedSurface);

        SkCanvas* seedCanvas = seedSurface->getCanvas();
        seedCanvas->clear(SK_ColorGREEN);

        SkPaint multiplyWhite;
        multiplyWhite.setColor(SK_ColorWHITE);
        multiplyWhite.setBlendMode(SkBlendMode::kMultiply);
        seedCanvas->drawRect(SkRect::MakeWH(1024, 512), multiplyWhite);

        std::unique_ptr<Recording> seedRec = recorder->snap();
        REPORTER_ASSERT(reporter, seedRec);
        REPORTER_ASSERT(reporter, context->insertRecording({seedRec.get()}));
        REPORTER_ASSERT(reporter, context->submit(SyncToCpu::kYes));
    }

    // Step 2: On a fresh 1600x512 target cleared to SK_ColorBLUE, draw a convex conic at huge local
    // X = 4294978560 (ULP = 512) translated by (-X + 1024, 0) with a clip [508, 0, 1600, 512].
    //   - CPU dstCopyBounds = [508, 0, 1025, 512] (517x512 -> GetApproxSize allocates 1024x512,
    //     reusing the SK_ColorGREEN scratch texture and writing only columns 0..516).
    //   - Hardware scissoring to dstCopyBounds must discard overhanging GPU fragments past x = 1025
    //     so zero pixels sample SK_ColorGREEN from columns 517..1023.
    constexpr int kTargetW = 1600;
    constexpr int kTargetH = 512;
    const SkImageInfo targetII =
            SkImageInfo::Make(kTargetW, kTargetH, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> targetSurface = SkSurfaces::RenderTarget(recorder.get(), targetII);
    REPORTER_ASSERT(reporter, targetSurface);

    SkCanvas* canvas = targetSurface->getCanvas();
    canvas->clear(SK_ColorBLUE);

    constexpr float kHugeX = 4294978560.0f;  // 2^32 + 22 * 512, ULP = 512
    SkPathBuilder pb;
    pb.moveTo(kHugeX - 1024.0f, 0.0f);
    pb.lineTo(kHugeX, 0.0f);
    pb.conicTo({kHugeX, 256.0f}, {kHugeX, 512.0f}, 0.7406733f);
    pb.lineTo(kHugeX - 1024.0f, 512.0f);
    pb.close();
    SkPath path = pb.detach();

    canvas->save();
    canvas->clipRect(SkRect::MakeLTRB(508, 0, kTargetW, kTargetH));
    canvas->translate(-kHugeX + 1024.0f, 0.0f);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorWHITE);
    paint.setBlendMode(SkBlendMode::kMultiply);
    canvas->drawPath(path, paint);
    canvas->restore();

    std::unique_ptr<Recording> rec = recorder->snap();
    REPORTER_ASSERT(reporter, rec);
    REPORTER_ASSERT(reporter, context->insertRecording({rec.get()}));

    SkBitmap readback;
    readback.allocPixels(targetII);
    REPORTER_ASSERT(reporter, targetSurface->readPixels(readback, 0, 0));

    int leakedPixels = 0;
    for (int y = 0; y < kTargetH; ++y) {
        for (int x = 0; x < kTargetW; ++x) {
            SkColor c = readback.getColor(x, y);
            if (c != SK_ColorBLUE) {
                ++leakedPixels;
            }
        }
    }
    REPORTER_ASSERT(reporter,
                    leakedPixels == 0,
                    "Expected all pixels to remain SK_ColorBLUE (no stale DstCopy read), "
                    "but %d pixels differed (sample at (1100, 256) = 0x%08x)",
                    leakedPixels,
                    readback.getColor(1100, 256));

    // Step 3: Verify multiple dst-reading draws followed by a non-dst-reading draw.
    {
        canvas->clear(SK_ColorWHITE);

        canvas->save();
        canvas->clipRect(SkRect::MakeLTRB(508, 0, kTargetW, kTargetH));
        canvas->translate(-kHugeX + 1024.0f, 0.0f);
        SkPaint redMultiply;
        redMultiply.setAntiAlias(true);
        redMultiply.setColor(SK_ColorRED);
        redMultiply.setBlendMode(SkBlendMode::kMultiply);
        canvas->drawPath(path, redMultiply);
        canvas->restore();

        SkPaint greenMultiply;
        greenMultiply.setColor(SK_ColorGREEN);
        greenMultiply.setBlendMode(SkBlendMode::kMultiply);
        canvas->drawRect(SkRect::MakeLTRB(1400, 0, 1500, kTargetH), greenMultiply);

        // Non-dst-reading draw outside dstCopyBounds must not be clipped by the dstCopy scissor.
        SkPaint yellowSrcOver;
        yellowSrcOver.setColor(SK_ColorYELLOW);
        canvas->drawRect(SkRect::MakeLTRB(1200, 100, 1300, 200), yellowSrcOver);

        std::unique_ptr<Recording> multiRec = recorder->snap();
        REPORTER_ASSERT(reporter, multiRec);
        REPORTER_ASSERT(reporter, context->insertRecording({multiRec.get()}));
        REPORTER_ASSERT(reporter, targetSurface->readPixels(readback, 0, 0));

        REPORTER_ASSERT(reporter, readback.getColor(800, 256) == SK_ColorRED);
        REPORTER_ASSERT(reporter, readback.getColor(1100, 256) == SK_ColorWHITE);
        REPORTER_ASSERT(reporter, readback.getColor(1250, 150) == SK_ColorYELLOW);
        REPORTER_ASSERT(reporter, readback.getColor(1450, 256) == SK_ColorGREEN);
    }

    // Step 4: Verify an inverse-filled multi-step tessellated conic draw under kTextureCopy:
    //   - Both Step 0 and Step 1 are scissored to (dstCopyBounds & scissor) == [256, 0, 780, 512],
    //     so neither Step 0's overhanging stencil nor Step 1's cover quad can escape past x = 780.
    //   - Follow immediately with a non-dst-reading stenciled path over [800, 950] in the same
    //     DrawPass to verify no dirty stencil bits leaked past x = 780.
    {
        canvas->clear(SK_ColorWHITE);

        SkPathBuilder invPB;
        invPB.setFillType(SkPathFillType::kInverseWinding);
        invPB.moveTo(kHugeX - 512.0f, 100.0f);
        invPB.lineTo(kHugeX, 100.0f);
        invPB.conicTo({kHugeX, 250.0f}, {kHugeX, 400.0f}, 0.7406733f);
        invPB.lineTo(kHugeX - 512.0f, 400.0f);
        invPB.close();

        canvas->save();
        canvas->clipRect(SkRect::MakeLTRB(256, 0, 780, kTargetH));
        canvas->translate(-kHugeX + 512.0f, 0.0f);  // Cutout sits at device [0, 100, 512, 400]
        SkPaint redMultiply;
        redMultiply.setAntiAlias(true);
        redMultiply.setColor(SK_ColorRED);
        redMultiply.setBlendMode(SkBlendMode::kMultiply);
        canvas->drawPath(invPB.detach(), redMultiply);
        canvas->restore();

        // Concave non-dst-reading path in [800, 950] x [150, 350] uses stencil in the same pass;
        // if Step 0 above leaked stencil into [780, 1024), this fill would be corrupted.
        SkPathBuilder checkPB;
        checkPB.moveTo(800.0f, 150.0f);
        checkPB.lineTo(950.0f, 150.0f);
        checkPB.lineTo(900.0f, 250.0f);
        checkPB.lineTo(950.0f, 350.0f);
        checkPB.lineTo(800.0f, 350.0f);
        checkPB.close();
        SkPaint blueSrcOver;
        blueSrcOver.setAntiAlias(true);
        blueSrcOver.setColor(SK_ColorBLUE);
        canvas->drawPath(checkPB.detach(), blueSrcOver);

        std::unique_ptr<Recording> invRec = recorder->snap();
        REPORTER_ASSERT(reporter, invRec);
        REPORTER_ASSERT(reporter, context->insertRecording({invRec.get()}));
        REPORTER_ASSERT(reporter, targetSurface->readPixels(readback, 0, 0));

        // Inside the cutout [256, 512) x [100, 400): untouched white.
        REPORTER_ASSERT(reporter, readback.getColor(400, 250) == SK_ColorWHITE);
        // Above the cutout inside clip [256, 780) x [0, 100): inverse fill painted red.
        REPORTER_ASSERT(reporter, readback.getColor(650, 50) == SK_ColorRED);
        // Subsequent stenciled path in [800, 900) x [150, 350): cleanly painted blue.
        REPORTER_ASSERT(reporter, readback.getColor(850, 250) == SK_ColorBLUE);
        // Outside both draws at (1100, 250): untouched white (no DstCopy padding leak).
        REPORTER_ASSERT(reporter, readback.getColor(1100, 250) == SK_ColorWHITE);
    }

    // Step 5: Verify exact in-bounds texel mapping at all corners of a non-power-of-two DstCopy
    // subset (offset (37, 29), size 300x200 -> approximate backing 512x256).
    {
        const SkImageInfo subII =
                SkImageInfo::Make(400, 300, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        sk_sp<SkSurface> subSurface = SkSurfaces::RenderTarget(recorder.get(), subII);
        REPORTER_ASSERT(reporter, subSurface);

        SkCanvas* subCanvas = subSurface->getCanvas();
        subCanvas->clear(SK_ColorBLACK);

        // Paint distinct colors at the exact 4 corner pixels of [37, 29, 337, 229).
        SkPaint srcPaint;
        srcPaint.setBlendMode(SkBlendMode::kSrc);
        srcPaint.setColor(SkColorSetARGB(255, 200, 0, 0));
        subCanvas->drawIRect(SkIRect::MakeXYWH(37, 29, 1, 1), srcPaint);
        srcPaint.setColor(SkColorSetARGB(255, 0, 200, 0));
        subCanvas->drawIRect(SkIRect::MakeXYWH(336, 29, 1, 1), srcPaint);
        srcPaint.setColor(SkColorSetARGB(255, 0, 0, 200));
        subCanvas->drawIRect(SkIRect::MakeXYWH(37, 228, 1, 1), srcPaint);
        srcPaint.setColor(SkColorSetARGB(255, 200, 200, 0));
        subCanvas->drawIRect(SkIRect::MakeXYWH(336, 228, 1, 1), srcPaint);

        // Blend RGB(100, 100, 100) with kLighten over [37, 29, 337, 229) so each corner's output
        // depends on sampling its exact corresponding dst texel.
        SkPaint lightenPaint;
        lightenPaint.setColor(SkColorSetARGB(255, 100, 100, 100));
        lightenPaint.setBlendMode(SkBlendMode::kLighten);
        subCanvas->drawIRect(SkIRect::MakeXYWH(37, 29, 300, 200), lightenPaint);

        std::unique_ptr<Recording> subRec = recorder->snap();
        REPORTER_ASSERT(reporter, subRec);
        REPORTER_ASSERT(reporter, context->insertRecording({subRec.get()}));

        SkBitmap subBM;
        subBM.allocPixels(subII);
        REPORTER_ASSERT(reporter, subSurface->readPixels(subBM, 0, 0));

        REPORTER_ASSERT(reporter, subBM.getColor(37, 29) == SkColorSetARGB(255, 200, 100, 100));
        REPORTER_ASSERT(reporter, subBM.getColor(336, 29) == SkColorSetARGB(255, 100, 200, 100));
        REPORTER_ASSERT(reporter, subBM.getColor(37, 228) == SkColorSetARGB(255, 100, 100, 200));
        REPORTER_ASSERT(reporter, subBM.getColor(336, 228) == SkColorSetARGB(255, 200, 200, 100));
        // Interior black pixel becomes (100, 100, 100)
        REPORTER_ASSERT(reporter, subBM.getColor(100, 100) == SkColorSetARGB(255, 100, 100, 100));
        // Outside pixel at (337, 228) untouched black
        REPORTER_ASSERT(reporter, subBM.getColor(337, 228) == SK_ColorBLACK);
    }
}
