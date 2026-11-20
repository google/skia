/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/Swizzle.h"

namespace skgpu {

DEF_TEST(SwizzleInvertTest, r) {
    struct {
        const char* fTest;
        const char* fExpectedInverse;
        bool fTrueInverse;
    } kSwizzleTests[] = {
        // These are true inverses since they use each of r,g,b,a exactly once
        {.fTest="rgba", .fExpectedInverse="rgba", .fTrueInverse=true},
        {.fTest="bgra", .fExpectedInverse="bgra", .fTrueInverse=true},
        {.fTest="garb", .fExpectedInverse="brag", .fTrueInverse=true},
        {.fTest="argb", .fExpectedInverse="gbar", .fTrueInverse=true},
        {.fTest="abgr", .fExpectedInverse="abgr", .fTrueInverse=true},
        // These are true inverses because the channels with constants are not referenced by the
        // other channels being swizzled
        {.fTest="000r", .fExpectedInverse="a001", .fTrueInverse=true},
        {.fTest="rgb1", .fExpectedInverse="rgb1", .fTrueInverse=true},
        {.fTest="bgr1", .fExpectedInverse="bgr1", .fTrueInverse=true},
        // These are not true inverses because not every channel is used, and the missing channels
        // are not set to a constant 0 or 1
        {.fTest="rrr1", .fExpectedInverse="r001", .fTrueInverse=false},
        {.fTest="rrra", .fExpectedInverse="r00a", .fTrueInverse=false},
        // This tests that a non-default constant is preserved if the channel is not in conflict
        // with any other swizzle component (e.g. B=1, but nothing references 'b' in rr11 so
        // preserve B=1 instead of falling back to the default B=0. At the same time, the inverse
        // G = 0 because had set it to 'r' and nothing references 'g' to provide another value).
        {.fTest="rr11", .fExpectedInverse="r011", .fTrueInverse=false},
        // This one is subtle, because both a000 and a001 map to 000r, but 000r maps back to a001
        {.fTest="a000", .fExpectedInverse="000r", .fTrueInverse=false},
    };

    for (auto t : kSwizzleTests) {
        Swizzle actualInverse = Swizzle(t.fTest).invert();
        REPORTER_ASSERT(r, Swizzle(actualInverse) == Swizzle(t.fExpectedInverse),
                        "Expected inverse(%s) == %s, but was %s",
                        t.fTest, t.fExpectedInverse, actualInverse.asString().c_str());
        if (t.fTrueInverse) {
            // For a true inverse, the inverse of the inverse should be the original
            Swizzle original = actualInverse.invert();
            REPORTER_ASSERT(r, Swizzle(t.fTest) == original,
                            "Expected inverse(inverse(%s)) == inverse(%s) == %s, but was %s",
                            t.fTest, actualInverse.asString().c_str(), t.fTest,
                            original.asString().c_str());
        }
    }
}

} // namespace skgpu

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SwizzleTest_WriteSwizzleClear,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNextRelease) {
    if (context->supportsProtectedContent()) {
        return;
    }

    constexpr int kW = 64;
    constexpr int kH = 64;

    // Choose kAlpha_8_SkColorType
    std::unique_ptr<skgpu::graphite::Recorder> recorder = context->makeRecorder();
    const SkImageInfo ii = SkImageInfo::Make(kW, kH, kAlpha_8_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), ii);
    if (!surface) {
        ERRORF(reporter, "Could not create surface.");
        return;
    }

    SkBitmap bm;
    bm.allocPixels(ii);

    // Reads back the surface and returns alpha at a pixel
    auto readSurfaceA = [&](int srcX, int srcY) {
        bm.eraseColor(0);
        REPORTER_ASSERT(reporter, surface->readPixels(bm, 0, 0));
        return SkColorGetA(bm.getColor(srcX, srcY));
    };

    SkCanvas* canvas = surface->getCanvas();

    // Partial alpha clear: Clearing with a=128 should read back as 128 in a
    canvas->clear(SkColorSetARGB(128, 0, 64, 128));
    REPORTER_ASSERT(reporter, readSurfaceA(32, 32) == 128);

    // Transparent clear: Clearing with a=0 should read back as 0 in a
    canvas->clear(SkColorSetARGB(0, 255, 255, 255));
    REPORTER_ASSERT(reporter, readSurfaceA(32, 32) == 0);

    // Unclipped solid-color drawPaint: Promotes to clear; clearing with black should
    // read back as 255 in a
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    canvas->drawPaint(paint);
    REPORTER_ASSERT(reporter, readSurfaceA(32, 32) == 255);
}
