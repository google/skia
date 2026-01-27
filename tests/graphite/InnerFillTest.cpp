/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBlendMode.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/TextureInfoPriv.h"

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
        device->drawRRect(rrect, paint);
        int actualRenderSteps = device->testingOnly_pendingRenderSteps();

        if (actualRenderSteps == 1) {
            // This either represents the first draw of the tests, or it represents the draw after
            // a dst-read texture copy flush (which should only happen if we are not expecting an
            // inner fill draw as well).
            SkASSERT(initialRenderSteps == 0 || !innerFillExpected);
            initialRenderSteps = 0;
        }

        // TODO(michaelludwig): After migrating to the layer draw tracking system, having a way to
        // traverse the recorded draws and inspect them could let these checks be more robust, e.g.
        // one of the steps is actually the CoverBoundsRenderStep AND it's in the front.
        int expectedRenderSteps = initialRenderSteps + (innerFillExpected ? 2 : 1);
        REPORTER_ASSERT(reporter, actualRenderSteps == expectedRenderSteps,
                        "Expected (%d) vs Actual (%d)", expectedRenderSteps, actualRenderSteps);
    };

    const bool srcWithCoverageIsHW =
            CanUseHardwareBlending(context->priv().caps(),
                                   TextureInfoPriv::ViewFormat(device->target()->textureInfo()),
                                   SkBlendMode::kSrc,
                                   Coverage::kSingleChannel);

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
    // 4. An opaque paint but with an analytic clip
    {
        skiatest::ReporterContext label{reporter, "analytic clip"};
        device->pushClipStack();
        device->clipShader(SkShaders::Color({0.1f, 0.2f, 0.3f, 0.5f}, nullptr),
                           SkClipOp::kIntersect);
        testRRect(SkPaint(), /*innerFillExpected=*/false);
        device->popClipStack();
    }
    // 5. A kSrc paint but the device requires shader blending since there's coverage
    if (!srcWithCoverageIsHW) {
        skiatest::ReporterContext label{reporter, "src w/o dual-src blending"};
        SkPaint srcPaint;
        srcPaint.setBlendMode(SkBlendMode::kSrc);
        testRRect(srcPaint, /*innerFillExpected=*/false);
    }

    // ** Test cases that should produce an inner fill:
    // 1. A kSrcOver paint with opaque color (or shader)
    {
        skiatest::ReporterContext label{reporter, "opaque src-over"};
        testRRect(SkPaint(), /*innerFillExpected=*/true);
    }
    // 2. A kSrc paint when the device supports HW blending regardless of coverage
    if (srcWithCoverageIsHW) {
        skiatest::ReporterContext label{reporter, "src w/ dual-src blending"};
        SkPaint srcPaint;
        srcPaint.setBlendMode(SkBlendMode::kSrc);
        srcPaint.setAlphaf(0.5f); // emphasize it's src color is not opaque
        testRRect(srcPaint, /*innerFillExpected=*/true);
    }

    // We don't actually care about rendering, so just throw everything away
}
