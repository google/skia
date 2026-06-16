/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBlendMode.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/TextureProxyView.h"

using namespace skgpu;
using namespace skgpu::graphite;

// These are unit tests that ensure the secondary "inner fill" draw is recorded when possible.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(InnerFillTest, reporter, context,
                                   CtsEnforcement::kNextRelease) {

    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    sk_sp<Device> device = Device::Make(recorder.get(),
                                        SkImageInfo::Make(512, 512,
                                                          kRGBA_8888_SkColorType,
                                                          kPremul_SkAlphaType),
                                        Budgeted::kYes,
                                        Mipmapped::kNo,
                                        SkBackingFit::kExact,
                                        /*surfaceProps=*/{},
                                        LoadOp::kClear,
                                        "inner-fill");

    // Default rrect size is sufficient to avoid the small-size exclusion criteria
    auto testRRect = [&](const SkPaint& paint, bool innerFillExpected, float rrectSize = 128.f) {
        SkRRect rrect = SkRRect::MakeRectXY(SkRect::MakeXYWH(0.f, 0.f, rrectSize, rrectSize),
                                            0.1f * rrectSize, 0.1f * rrectSize);
        int initialRenderSteps = device->testingOnly_pendingRenderSteps();
        int initialTasks = recorder->priv().numRootTasks();
        device->drawRRect(rrect, paint);

        int actualRenderSteps = device->testingOnly_pendingRenderSteps();
        if (recorder->priv().numRootTasks() != initialTasks) {
            // Flushed for a dst read copy before appending the new draw and possibly inner fill.
            initialRenderSteps = 0;
        }

        // TODO(michaelludwig): After migrating to the layer draw tracking system, having a way to
        // traverse the recorded draws and inspect them could let these checks be more robust, e.g.
        // one of the steps is actually the CoverBoundsRenderStep AND it's in the front.
        int expectedRenderSteps = initialRenderSteps + (innerFillExpected ? 2 : 1);
        REPORTER_ASSERT(reporter, actualRenderSteps == expectedRenderSteps,
                        "Expected (%d) vs Actual (%d)", expectedRenderSteps, actualRenderSteps);
    };

    // ** Test cases that should not produce an inner fill:
    // 1. A transparent paint, so the draw is not eligible for an inner fill
    {
        skiatest::ReporterContext label{reporter, "transparent paint"};
        SkPaint transparentPaint;
        transparentPaint.setAlphaf(0.5f);
        testRRect(transparentPaint, /*innerFillExpected=*/false);
    }
    // 2. A very small rounded rectangle that would otherwise have an inner fill
    {
        skiatest::ReporterContext label{reporter, "small rrect"};
        testRRect(SkPaint(), /*innerFillExpected=*/false, /*rrectSize=*/8.f);
    }
    // 3. An opaque paint that has a blend mode that still mixes with the dst
    {
        skiatest::ReporterContext label{reporter, "fancy blend"};
        SkPaint opaqueBlendedPaint;
        opaqueBlendedPaint.setBlendMode(SkBlendMode::kSrcIn);
        testRRect(opaqueBlendedPaint, /*innerFillExpected=*/false);
    }

    // ** Test cases that shouldn't produce an inner fill due to analytic clipping
    device->pushClipStack();
    device->clipShader(SkShaders::Color({0.1f, 0.2f, 0.3f, 0.5f}, nullptr),
                        SkClipOp::kIntersect);
    // 4. An opaque paint but with an analytic clip
    {
        skiatest::ReporterContext label{reporter, "opaque src-over analytic clip"};
        testRRect(SkPaint(), /*innerFillExpected=*/false);
    }
    // 5. A kSrc paint but there is analytic clip
    {
        skiatest::ReporterContext label{reporter, "src w/ analytic clip"};
        SkPaint srcPaint;
        srcPaint.setBlendMode(SkBlendMode::kSrc);
        testRRect(srcPaint, /*innerFillExpected=*/false);
    }
    device->popClipStack();

    // ** Test cases that should produce an inner fill:
    // 1. A kSrcOver paint with opaque color (or shader)
    {
        skiatest::ReporterContext label{reporter, "opaque src-over"};
        testRRect(SkPaint(), /*innerFillExpected=*/true);
    }
    // 2. A kSrc paint when the device supports HW blending regardless of coverage
    {
        skiatest::ReporterContext label{reporter, "src"};
        SkPaint srcPaint;
        srcPaint.setBlendMode(SkBlendMode::kSrc);
        srcPaint.setAlphaf(0.5f); // emphasize it's src color is not opaque
        testRRect(srcPaint, /*innerFillExpected=*/true);
    }

    // We don't actually care about rendering, so just throw everything away
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(OptimizeForOpacity,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNextRelease) {
    static constexpr SkColor4f kSemiTransparent{1.f, 1.f, 1.f, 0.5f};
    static constexpr SkColor4f kOpaque{1.f, 1.f, 1.f, 1.f};

    const Caps* caps = context->priv().caps();
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    // We need a DrawContext to participate in uniform extraction and key generation
    SkColorInfo targetInfo{kRGBA_8888_SkColorType, kPremul_SkAlphaType, /*cs=*/nullptr};
    sk_sp<TextureProxy> target = TextureProxy::Make(
            caps, recorder->priv().resourceProvider(),
            /*dimensions=*/{16, 16},
            caps->getDefaultSampledTextureInfo(targetInfo.colorType(),
                                               Mipmapped::kNo,
                                               Protected::kNo,
                                               Renderable::kYes),
            "OptimizeForOpacityTarget",
            Budgeted::kYes);
    sk_sp<DrawContext> drawContext = DrawContext::Make(
            caps, std::move(target), {16, 16}, targetInfo, {});

    auto genPaintID = [&](const PaintParams& paint,
                          Coverage rendererCoverage,
                          SkEnumBitMask<DstUsage> expectedDstUsage,
                          const NonMSAAClip& clip={}) {
        ShadingParams shading{caps, paint, clip, /*clipShader=*/nullptr,
                              rendererCoverage, TextureFormat::kRGBA8};

        auto keyAndDataBuilder = recorder->priv().popOrCreateKeyAndDataBuilder();
        // This opts into kPreferFixedSrcBlend as if we were drawing a rect or quad that supports
        // inner fills (and assuming that `rendererCoverage` is set to match either the AA or the
        // pixel-aligned case appropriately).
        KeyContext keyContext{recorder.get(),
                              drawContext.get(),
                              recorder->priv().floatStorageManager(),
                              &keyAndDataBuilder->second,
                              &keyAndDataBuilder->first,
                              SkM44(),
                              SkRect::MakeIWH(16, 16),
                              targetInfo,
                              KeyGenFlags::kPreferFixedSrcBlend,
                              paint.color()};

        auto [paintID, dstUsage] = *shading.toKey(keyContext);
        // The expected usages should be a subset of what was actually returned
        REPORTER_ASSERT(reporter, (dstUsage & expectedDstUsage) == expectedDstUsage);

        std::optional<UniquePaintParamsID> opaqueID;
        if (dstUsage == DstUsage::kNone || (dstUsage & DstUsage::kDstOnlyUsedByRenderer)) {
            opaqueID = shading.optimizeForOpacity(keyContext, paintID);
        }

        keyAndDataBuilder->first.resetForDraw();
        keyAndDataBuilder->second.resetForDraw();
        recorder->priv().pushKeyAndDataBuilder(std::move(keyAndDataBuilder));
        return std::make_pair(paintID, opaqueID);
    };

    // Construct an unclipped solid color + src paint without coverage, e.g. the optimized
    // paint params key for an inner fill.
    auto [nonAASrcID, opaqueNonAASrcID] =
            genPaintID(PaintParams{kSemiTransparent, SkBlendMode::kSrc},
                       Coverage::kNone,
                       DstUsage::kNone);
    REPORTER_ASSERT(reporter, opaqueNonAASrcID.has_value(), "Non-AA + kSrc not detected as opaque");
    REPORTER_ASSERT(reporter, nonAASrcID == *opaqueNonAASrcID,
                    "optimizeForOpacity() should be a no-op for non-AA+kSrc");

    // Case 1: src-over + opaque color without coverage is optimized up front to match kSrc
    {
        auto [nonAASrcOverID, opaqueNonAASrcOverID] =
                genPaintID(PaintParams{kOpaque, SkBlendMode::kSrcOver},
                           Coverage::kNone,
                           DstUsage::kNone);
        REPORTER_ASSERT(reporter, opaqueNonAASrcOverID.has_value(),
                        "Non-AA + kSrcOver not detected as opaque");
        REPORTER_ASSERT(reporter, nonAASrcOverID == *opaqueNonAASrcOverID,
                        "optimizeForOpacity() should be a no-op for non-AA+kSrcOver");
        REPORTER_ASSERT(reporter, nonAASrcOverID == nonAASrcID,
                        "Opaque paint ID should match between non-AA+kSrcOver and non-AA+kSrc");
    }

    // Case 2: src with coverage is optimized in second pass for inner fill, although the paint
    // ID works out the same (the change comes later with a different RenderStep combination).
    {
        auto [aaSrcID, opaqueAASrcID] =
                genPaintID(PaintParams{kOpaque, SkBlendMode::kSrc},
                           Coverage::kSingleChannel,
                           DstUsage::kDependsOnDst | DstUsage::kDstOnlyUsedByRenderer);
        REPORTER_ASSERT(reporter, opaqueAASrcID.has_value(),
                        "AA + kSrc should have opaque variant");
        REPORTER_ASSERT(reporter, aaSrcID == *opaqueAASrcID,
                        "optimizeForOpacity() should be a no-op for AA+kSrc");
        REPORTER_ASSERT(reporter, *opaqueAASrcID == nonAASrcID,
                        "Opaque paint ID should match between AA+kSrc and non-AA+kSrc");
    }

    // Case 3: src-over + opaque color with coverage is optimized in second pass for inner fill
    {
        auto [aaSrcOverID, opaqueAASrcOverID] =
                genPaintID(PaintParams{kOpaque, SkBlendMode::kSrcOver},
                           Coverage::kSingleChannel,
                           DstUsage::kDependsOnDst | DstUsage::kDstOnlyUsedByRenderer);
        REPORTER_ASSERT(reporter, opaqueAASrcOverID.has_value(),
                        "AA + kSrcOver should have opaque variant");
        REPORTER_ASSERT(reporter, aaSrcOverID != *opaqueAASrcOverID,
                        "optimizeForOpacity() should be different for AA+kSrcOver");
        REPORTER_ASSERT(reporter, *opaqueAASrcOverID == nonAASrcID,
                        "Opaque paint ID should match between AA+kSrcOver and non-AA+kSrc");
    }

    // Case 4: src-over + transparent color is not optimized and has no inner fill follow-up
    {
        auto [_, opaqueAASrcOverID] =
                genPaintID(PaintParams{kSemiTransparent, SkBlendMode::kSrcOver},
                           Coverage::kSingleChannel,
                           DstUsage::kDependsOnDst);
        REPORTER_ASSERT(reporter, !opaqueAASrcOverID.has_value(),
                        "AA + kSrcOver should not have an opaque variant");
    }

    AnalyticClip clip;
    clip.fBounds = { 1.f, 1.f, 15.f, 15.f };

    // Case 5: src without coverage but an analytic clip cannot have an inner fill
    {
        auto [_, clippedOpaqueNonAASrcID] =
        genPaintID(PaintParams{kSemiTransparent, SkBlendMode::kSrc},
                    Coverage::kNone,
                    DstUsage::kDependsOnDst,
                    NonMSAAClip{clip, {}});
        REPORTER_ASSERT(reporter, !clippedOpaqueNonAASrcID.has_value(),
                        "Non-AA + kSrc + AnalyticClip should not have an opaque variant");
    }
    // Case 6: src-over + opaque without coverage but an analytic clip cannot have an inner fill
    {
        auto [_, clippedOpaqueNonAASrcOverID] =
                genPaintID(PaintParams{kOpaque, SkBlendMode::kSrcOver},
                           Coverage::kNone,
                           DstUsage::kDependsOnDst,
                           NonMSAAClip{clip, {}});
        REPORTER_ASSERT(reporter, !clippedOpaqueNonAASrcOverID.has_value(),
                        "Non-AA + kSrcOver + AnalyticClip should not have an opaque variant");
    }
    // Case 7: src with coverage and an analytic clip cannot have an inner fill
    {
        auto [_, clippedOpaqueAASrcID] =
                genPaintID(PaintParams{kOpaque, SkBlendMode::kSrc},
                           Coverage::kSingleChannel,
                           DstUsage::kDependsOnDst,
                           NonMSAAClip{clip, {}});
        REPORTER_ASSERT(reporter, !clippedOpaqueAASrcID.has_value(),
                        "AA + kSrc + AnalyticClip should not have an opaque variant");
    }
    // Case 8: src-over + opaque with coverage and an analytic clip cannot have an inner fill
    {
        auto [_, clippedOpaqueAASrcOverID] =
                genPaintID(PaintParams{kOpaque, SkBlendMode::kSrcOver},
                           Coverage::kSingleChannel,
                           DstUsage::kDependsOnDst,
                           NonMSAAClip{clip, {}});
        REPORTER_ASSERT(reporter, !clippedOpaqueAASrcOverID.has_value(),
                        "AA + kSrcOver + AnalyticClip should not have an opaque variant");
    }
}
