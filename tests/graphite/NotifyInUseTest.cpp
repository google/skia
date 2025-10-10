/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkBlurMaskFilter.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "tests/Test.h"

#include "src/gpu/graphite/Surface_Graphite.h"

using namespace skgpu::graphite;

namespace {

bool colors_are_similar(SkColor c1, SkColor c2, int tolerance) {
    if (std::abs((int)SkColorGetA(c1) - (int)SkColorGetA(c2)) > tolerance) return false;
    if (std::abs((int)SkColorGetR(c1) - (int)SkColorGetR(c2)) > tolerance) return false;
    if (std::abs((int)SkColorGetG(c1) - (int)SkColorGetG(c2)) > tolerance) return false;
    if (std::abs((int)SkColorGetB(c1) - (int)SkColorGetB(c2)) > tolerance) return false;
    return true;
}

bool layer_test(SkBitmap& bitmap, Context* context, SkBlendMode blendMode) {
    auto recorder = context->makeRecorder();
    SkImageInfo info = SkImageInfo::Make(256, 256, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), info);
    SkCanvas* canvas = surface->getCanvas();

    SkPaint blendPaint;
    blendPaint.setBlendMode(blendMode);

    SkPaint bluePaint;
    bluePaint.setColor(0xFF0099FF);
    canvas->clear(SK_ColorTRANSPARENT);
    canvas->drawRect(SkRect::MakeXYWH(5, 5, 45, 45), bluePaint);

    canvas->saveLayer(nullptr, &blendPaint);
    SkPaint greenPaint;
    greenPaint.setColor(0xCC33FF99);
    canvas->clear(SK_ColorTRANSPARENT);
    canvas->drawRect(SkRect::MakeXYWH(30, 15, 45, 45), greenPaint);
    canvas->restore();

    canvas->saveLayer(nullptr, &blendPaint);
    SkPaint redPaint;
    redPaint.setColor(0xFFFF3300);
    canvas->clear(SK_ColorTRANSPARENT);
    canvas->drawRect(SkRect::MakeXYWH(20, 25, 45, 45), redPaint);
    canvas->restore();

    bitmap.allocPixels(info);
    return canvas->readPixels(bitmap, 0, 0);
}

} // namespace

/*
    This test creates a dependency chain between different surfaces, by forcing a
    Device::makeImageCopy through makeImageSnapshot().

    Device::makeImageCopy first calls flushPendingWork(nullptr), then Image::Copy. However because
    flushPendingWork *only flushes tasks from B*, when Image::Copy adds the copy task directly to
    the recorder's root task list, A is not yet flushed. So when the recorder is snapped in
    readPixels, only B is copied into C.

    The incorrect ordering.
    =========== RECORDING ===========
    **** FLUSH TOKEN 1 Snap ****
    0: Draw Task (target=0x120e84570)   <-- B's drawTask
    └── RenderPass Task
    1: Copy TtoT Task                   <-- the copy task from B to C, A has drawn yet!
    2: Draw Task (target=0x120e834a0)   <-- A's drawTask
    └── RenderPass Task
    3: Draw Task (target=0x120e85690)   <-- C's drawTask
    └── RenderPass Task
    -------------- END --------------

    The correct ordering:
    =========== RECORDING ===========
    **** FLUSH TOKEN 1 Snap ****
    0: Draw Task (target=0x120e834a0)   <-- A's drawTask
    └── RenderPass Task
    1: Draw Task (target=0x120e84570)   <-- B's drawTask
    └── RenderPass Task
    2: Copy TtoT Task                   <-- the copy task from B to C
    3: Draw Task (target=0x120e85690)   <-- C's drawContext
    └── RenderPass Task
    -------------- END --------------
*/

DEF_GRAPHITE_TEST_FOR_CONTEXTS(NotifyInUseTestSnapshot, /*filter=*/nullptr, reporter, context,
                               testContext, CtsEnforcement::kNextRelease) {
    auto recorder = context->makeRecorder();
    SkImageInfo info = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    // "A"
    sk_sp<SkSurface> surfaceA = SkSurfaces::RenderTarget(recorder.get(), info);
    SkCanvas* canvasA = surfaceA->getCanvas();
    SkPaint greenPaint;
    greenPaint.setColor(SK_ColorGREEN);
    canvasA->drawRect(SkRect::Make(info.bounds()), greenPaint);
    sk_sp<SkImage> imageA = SkSurfaces::AsImage(surfaceA);
    REPORTER_ASSERT(reporter, imageA);

    // "B"
    sk_sp<SkSurface> surfaceB = SkSurfaces::RenderTarget(recorder.get(), info);
    SkCanvas* canvasB = surfaceB->getCanvas();
    canvasB->clear(SK_ColorBLACK);
    canvasB->drawImage(imageA, 0, 0);

    // "C"
    sk_sp<SkSurface> surfaceC = SkSurfaces::RenderTarget(recorder.get(), info);
    SkCanvas* canvasC = surfaceC->getCanvas();
    canvasC->clear(SK_ColorBLACK);
    canvasC->drawImage(surfaceB->makeImageSnapshot(), 0, 0);

    // Recorder snap inside readPixels
    SkBitmap bitmap;
    bitmap.allocPixels(info);
    REPORTER_ASSERT(reporter, surfaceC->readPixels(bitmap, 0, 0));

    SkColor topLeft = bitmap.getColor(0, 0);
    REPORTER_ASSERT(reporter, topLeft == SK_ColorGREEN,
                    "Expected green pixel from surface C, got 0x%08X", topLeft);
}

/*
    This test confirms that mixing reads from an image view of a surface (SkSurfaces::AsImage) with
    writes to that surface's canvas order tasks correctly. At a high level, the flow of draws and
    flushes should be:

    DrawBlue to A
    DrawA to left of B (flushes A's tasks)
    DrawRed to A
    DrawA to right of B (flushes B's tasks, then A's tasks)

    This should produce the following task graph:

    =========== RECORDING 1 ===========
    **** FLUSH TOKEN 1 Recorder::Snap ****
    0: Draw Task=0x600000433e80 (Target=0x600002e04690) (Label=SkSurfaceRenderTarget)
    └── RenderPass Task (A1)
    1: Draw Task=0x600000433f00 (Target=0x600002e045a0) (Label=SkSurfaceRenderTarget)
    └── RenderPass Task (B1)
    2: Draw Task=0x600000433fc0 (Target=0x600002e04690) (Label=SkSurfaceRenderTarget)
    └── RenderPass Task (A2)
    3: Draw Task=0x60000041dd00 (Target=0x600002e045a0) (Label=SkSurfaceRenderTarget)
    └── RenderPass Task (B2)
    --------------- END ---------------

    Without tracking the fact that B depends on A's prior contents, when notifyInUse() incorrectly
    was only flushing A's tasks, the task graph would be the following. This results in the final
    contents of A being used for both of its draws into B:

    =========== RECORDING 1 ===========
    **** FLUSH TOKEN 1 Recorder::Snap ****
    0: Draw Task=0x600001a27bc0 (Target=0x6000030084b0) (Label=SkSurfaceRenderTarget)
    └── RenderPass Task (A1)
    1: Draw Task=0x600001a27d00 (Target=0x6000030084b0) (Label=SkSurfaceRenderTarget)
    └── RenderPass Task (A2)
    2: Draw Task=0x600001a27c40 (Target=0x6000030083c0) (Label=SkSurfaceRenderTarget)
    └── RenderPass Task (B1+B2, only samples A2's state)
    --------------- END ---------------

*/
DEF_GRAPHITE_TEST_FOR_CONTEXTS(NotifyInUseTestAsImage, /*filter=*/nullptr, reporter, context,
                               testContext, CtsEnforcement::kNextRelease) {
    auto recorder = context->makeRecorder();
    SkImageInfo aInfo = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkImageInfo bInfo = SkImageInfo::Make(20, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    // "A1"
    sk_sp<SkSurface> surfaceA = SkSurfaces::RenderTarget(recorder.get(), aInfo);
    surfaceA->getCanvas()->clear(SK_ColorBLUE);
    sk_sp<SkImage> imageA = SkSurfaces::AsImage(surfaceA);
    REPORTER_ASSERT(reporter, imageA);

    // "B1"
    sk_sp<SkSurface> surfaceB = SkSurfaces::RenderTarget(recorder.get(), bInfo);
    SkCanvas* canvasB = surfaceB->getCanvas();
    canvasB->clear(SK_ColorBLACK);
    canvasB->drawImage(imageA, 0, 0); // Should see A's blue clear in left half of B

    // "A2"
    surfaceA->getCanvas()->clear(SK_ColorRED);

    // "B2"
    canvasB->drawImage(imageA, 10, 0); // Should see A's now-red clear in right half of B

    // Recorder snaps inside readPixels
    SkBitmap bitmap;
    bitmap.allocPixels(bInfo);
    REPORTER_ASSERT(reporter, surfaceB->readPixels(bitmap, 0, 0));


    SkColor leftB = bitmap.getColor(5, 5);
    REPORTER_ASSERT(reporter, leftB == SK_ColorBLUE,
                    "Expected blue pixel from surface B's left half, got 0x%08X", leftB);
    SkColor rightB = bitmap.getColor(15, 5);
    REPORTER_ASSERT(reporter, rightB == SK_ColorRED,
                    "Expected red pixel from surface B's right half, got 0x%08X", rightB);
}

/*
    These tests replicate the compositing behavior in blink's canvas2d. Due to the use of save
    layers, the final order of draw tasks on the root task list does not match the order of draw
    calls in the code. At a high level the flow of draws and flushes looks like this:

    DrawBlue
        DrawGeometry DrawContext0
    SaveLayer SkBlendMode::kMultiply

    DrawGreen
        DrawGeometry DrawContext1
    Restore0
        SetImmutable FlushPendingWork
        DrawSpecial DrawGeometry DrawContext0
    SaveLayer SkBlendMode::kMultiply

    DrawRed
        DrawGeometry DrawContext2
    Restore1
        SetImmutable FlushPendingWork
        DrawSpecial DrawGeometry DrawContext0

    Snap
        FlushTrackedDevices

    ------------------------------------------------------------------------------------------------

    Some blends, like multiply, depend on prior draws. However, on devices without frame buffer
    fetch, this creates a dependency hazard because the destination surface must be copied to be
    read in for the blending. This means flushing prior draws to the root task list *before* the
    current draw---flushBeforeDraw. Dependency on prior draws is communicated through
    Image_Base::notifyInUse, but correct dependency ordering is contingent on the ordering of
    flushBeforeDraw and notifyInUse relative to each other.

    Calling notifyInUse prior to flushBeforeDraw causes the dependency draw task to be added as a
    child task of the current draw instead of flushing to the root task list. In many cases, adding
    the draw as a child of the dependent draw task is ok because child tasks execute prior to their
    parent task. However, if a later draw *also* depends on the now child draw, an incorrect draw
    order may arise.

    Using the SkBlendMode::Multiply version of the test as an example:

    Incorrect ordering:
    1) Green flushed to the root task list by restore0 setImmutable

    2) Green gets added as child of Blue restore0 drawSpecial
    3) Blue+Green gets flushed as flushBeforeDraw restore0 drawSpecial

    4) Red flushed to the root task list by restore1 setImmutable

    5) Red gets added as child of Blue+Green+Red+Blend restore1 drawSpecial
    6) Blue+Green+Red+Blend gets flushed as flushBeforeDraw restore1 drawSpecial

    7) Malformed gets flushed as flushTrackedDevices snap

    Correct ordering:
    1) Green flushed to the root task list by restore0 setImmutable

    2) Blue gets flushed as flushBeforeDraw restore0 drawSpecial
    3) Green added as child of Blue+Green+Blend restore0 drawSpecial

    4) Red flushed to the root task list by restore1 setImmutable

    5) Blue+Green+Blend flushed as flushBeforeDraw restore1 drawSpecial
    6) Red added as child of Blue+Green+Red+Blend restore1 drawSpecial

    7) Blue+Green+Red+Blend flushed as flushTrackedDevices snap

    SkBlendMode::Multiply incorrect ordering:
    =========== RECORDING 1 ===========
    **** FLUSH TOKEN 154 Recorder::Snap ****
    0: Draw Task (target=0x17252f6c0) (DC=0x17253b7d0)  <-- Draw Green into layer
    └── RenderPass Task
    1: Draw Task (target=0x172544bf0) (DC=0x1725290d0)  <-- Draw blue into main layer
    │   Draw Task (target=0x17252f6c0) (DC=0x17253b7d0) <-- Green is added as child of blue
    │   └── RenderPass Task
    └── RenderPass Task
    2: Draw Task (target=0x17254c3d0) (DC=0x172530cc0)  <-- Draw red into layer
    └── RenderPass Task
    3: Draw Task (target=0x172544bf0) (DC=0x1725290d0)  <-- Blend blue with red!
    │   Draw Task (target=0x17254c3d0) (DC=0x172530cc0) <-- Red is added as child of blend
    │   └── RenderPass Task
    │   Copy TtoT Task: Src=0x172544bf0 Dst=0x172547880 <-- Texture Copy because no FbFetch
    └── RenderPass Task
    4: Draw Task (target=0x172544bf0) (DC=0x1725290d0)  <-- ATTEMPT to blend blue+red with green!
    │   Copy TtoT Task: Src=0x172544bf0 Dst=0x17254c130 <-- Copy but there's no lastDrawTask !
    └── RenderPass Task
    --------------- END ---------------

    The correct ordering:
    =========== RECORDING 1 ===========
    **** FLUSH TOKEN 181 Recorder::Snap ****
    0: Draw Task (target=0x349db7690) (DC=0x349d856b0)  <-- Draw green into layer
    └── RenderPass Task
    1: Draw Task (target=0x349db7010) (DC=0x349d98a80)  <-- Draw blue into main layer
    └── RenderPass Task
    2: Draw Task (target=0x349d99f90) (DC=0x349d981d0)  <-- Draw red into layer
    └── RenderPass Task
    3: Draw Task (target=0x349db7010) (DC=0x349d98a80)  <-- Blend blue with green
    │   Draw Task (target=0x349db7690) (DC=0x349d856b0) <-- Green is correctly added as child
    │   └── RenderPass Task
    │   Copy TtoT Task: Src=0x349db7010 Dst=0x349d96b90 <-- Texture Copy because no FbFetch
    └── RenderPass Task
    4: Draw Task (target=0x349db7010) (DC=0x349d98a80)  <-- Blend blue+green with red
    │   Draw Task (target=0x349d99f90) (DC=0x349d981d0) <-- Red is correctly added as child
    │   └── RenderPass Task
    │   Copy TtoT Task: Src=0x349db7010 Dst=0x349d988a0 <-- Texture Copy because no FbFetch
    └── RenderPass Task
    --------------- END ---------------
*/

#define DEFINE_LAYER_BLEND_TEST(TestName, BlendMode, ExpectedBlueOnly, ExpectedGreenOnly,        \
                                ExpectedBlueAndGreen, ExpectedRedOnly, ExpectedAllOverlap)       \
DEF_GRAPHITE_TEST_FOR_CONTEXTS(NotifyInUseTestLayer ## TestName, /*filter=*/nullptr, reporter,   \
                               context, testContext, CtsEnforcement::kNextRelease) {             \
    SkBitmap bitmap;                                                                             \
    REPORTER_ASSERT(reporter, layer_test(bitmap, context, BlendMode),                            \
                        "Failed to read pixels for BlendMode '%s'",                              \
                        SkBlendMode_Name(BlendMode));                                            \
                                                                                                 \
    constexpr int kTolerance = 2;                                                                \
    SkColor blueOnly = bitmap.getColor(10, 10);                                                  \
    REPORTER_ASSERT(reporter, colors_are_similar(blueOnly, ExpectedBlueOnly, kTolerance),        \
                    "Pt(10,10) BlendMode '%s': Expected blue-only ~0x%08X, got 0x%08X",          \
                    SkBlendMode_Name(BlendMode), (uint32_t)ExpectedBlueOnly, blueOnly);          \
                                                                                                 \
    SkColor greenOnly = bitmap.getColor(70, 30);                                                 \
    REPORTER_ASSERT(reporter, colors_are_similar(greenOnly, ExpectedGreenOnly, kTolerance),      \
                    "Pt(70,30) BlendMode '%s': Expected green-only ~0x%08X, got 0x%08X",         \
                    SkBlendMode_Name(BlendMode), (uint32_t)ExpectedGreenOnly, greenOnly);        \
                                                                                                 \
    SkColor blueAndGreen = bitmap.getColor(40, 20);                                              \
    REPORTER_ASSERT(reporter, colors_are_similar(blueAndGreen, ExpectedBlueAndGreen, kTolerance),\
                    "Pt(40,20) BlendMode '%s': Expected blue/green ~0x%08X, got 0x%08X",         \
                    SkBlendMode_Name(BlendMode), (uint32_t)ExpectedBlueAndGreen, blueAndGreen);  \
                                                                                                 \
    SkColor redOnly = bitmap.getColor(25, 68);                                                   \
    REPORTER_ASSERT(reporter, colors_are_similar(redOnly, ExpectedRedOnly, kTolerance),          \
                    "Pt(25,68) BlendMode '%s': Expected red-only ~0x%08X, got 0x%08X",           \
                    SkBlendMode_Name(BlendMode), (uint32_t)ExpectedRedOnly, redOnly);            \
                                                                                                 \
    SkColor allOverlap = bitmap.getColor(40, 40);                                                \
    REPORTER_ASSERT(reporter, colors_are_similar(allOverlap, ExpectedAllOverlap, kTolerance),    \
                    "Pt(40,40) BlendMode '%s': Expected all-overlap ~0x%08X, got 0x%08X",        \
                    SkBlendMode_Name(BlendMode), (uint32_t)ExpectedAllOverlap, allOverlap);      \
}

// Basic Porter-Duff blend modes
DEFINE_LAYER_BLEND_TEST(Clear, SkBlendMode::kClear, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                                    0x00000000)
DEFINE_LAYER_BLEND_TEST(Src, SkBlendMode::kSrc, 0x00000000, 0x00000000, 0x00000000, 0xFFFF3300,
                                                0xFFFF3300)
DEFINE_LAYER_BLEND_TEST(Dst, SkBlendMode::kDst, 0xFF0099FF, 0x00000000, 0xFF0099FF, 0x00000000,
                                                0xFF0099FF)
DEFINE_LAYER_BLEND_TEST(SrcOver, SkBlendMode::kSrcOver, 0xFF0099FF, 0xCC33FF99, 0xFF29EBAD,
                                                        0xFFFF3300, 0xFFFF3300)
DEFINE_LAYER_BLEND_TEST(DstOver, SkBlendMode::kDstOver, 0xFF0099FF, 0xCC33FF99, 0xFF0099FF,
                                                        0xFFFD3302, 0xFF0099FF)
DEFINE_LAYER_BLEND_TEST(SrcIn, SkBlendMode::kSrcIn, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                                    0xCCFF3300)
DEFINE_LAYER_BLEND_TEST(DstIn, SkBlendMode::kDstIn, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                                    0xCC0099FF)
DEFINE_LAYER_BLEND_TEST(SrcOut, SkBlendMode::kSrcOut, 0x00000000, 0x00000000, 0x00000000,
                                                      0xFDFF3300, 0xFFFF3300)
DEFINE_LAYER_BLEND_TEST(DstOut, SkBlendMode::kDstOut, 0xFF0099FF, 0x00000000, 0x33009BFF,
                                                      0x00000000, 0x00000000)
DEFINE_LAYER_BLEND_TEST(SrcATop, SkBlendMode::kSrcATop, 0xFF0099FF, 0x00000000, 0xFF29EBAD,
                                                        0x00000000, 0xFFFF3300)
DEFINE_LAYER_BLEND_TEST(DstATop, SkBlendMode::kDstATop, 0x00000000, 0x00000000, 0x00000000,
                                                        0xFFFD3302, 0xFF3384CC)
DEFINE_LAYER_BLEND_TEST(Xor, SkBlendMode::kXor, 0xFF0099FF, 0xCC33FF99, 0x33009BFF, 0xFDFF3300,
                                                0xCCFF3300)
DEFINE_LAYER_BLEND_TEST(Plus, SkBlendMode::kPlus, 0xFF0099FF, 0xCC33FF99, 0xFF29FFFF, 0xFFFF3302,
                                                  0xFFFFFFFF)
DEFINE_LAYER_BLEND_TEST(Modulate, SkBlendMode::kModulate, 0x00000000, 0x00000000, 0x00000000,
                                                          0x00000000, 0xCC001E00)
DEFINE_LAYER_BLEND_TEST(Screen, SkBlendMode::kScreen, 0xFF0099FF, 0xCC33FF99, 0xFF29EBFF,
                                                      0xFFFF3302, 0xFFFFEFFF)

// Advanced, non-separable blend modes
DEFINE_LAYER_BLEND_TEST(Overlay, SkBlendMode::kOverlay, 0xFF0099FF, 0xCC33FF99, 0xFF00EBFF,
                                                        0xFFFD3302, 0xFF00DFFF)
DEFINE_LAYER_BLEND_TEST(Darken, SkBlendMode::kDarken, 0xFF0099FF, 0xCC33FF99, 0xFF0099AD,
                                                      0xFFFD3300, 0xFF003300)
DEFINE_LAYER_BLEND_TEST(Lighten, SkBlendMode::kLighten, 0xFF0099FF, 0xCC33FF99, 0xFF29EBFF,
                                                        0xFFFF3302, 0xFFFFEBFF)
DEFINE_LAYER_BLEND_TEST(ColorDodge, SkBlendMode::kColorDodge, 0xFF0099FF, 0xCC33FF99, 0xFF00EBFF,
                                                              0xFFFD3302, 0xFF00FFFF)
DEFINE_LAYER_BLEND_TEST(ColorBurn, SkBlendMode::kColorBurn, 0xFF0099FF, 0xCC33FF99, 0xFF0099FF,
                                                            0xFFFD3302, 0xFF0000FF)
DEFINE_LAYER_BLEND_TEST(HardLight, SkBlendMode::kHardLight, 0xFF0099FF, 0xCC33FF99, 0xFF00EBFF,
                                                            0xFFFF3300, 0xFFFF5E00)
DEFINE_LAYER_BLEND_TEST(SoftLight, SkBlendMode::kSoftLight, 0xFF0099FF, 0xCC33FF99, 0xFF00BDFF,
                                                            0xFFFD3302, 0xFF00A0FF)
DEFINE_LAYER_BLEND_TEST(Difference, SkBlendMode::kDifference, 0xFF0099FF, 0xCC33FF99, 0xFF297085,
                                                              0xFFFF3302, 0xFFD63D85)
DEFINE_LAYER_BLEND_TEST(Exclusion, SkBlendMode::kExclusion, 0xFF0099FF, 0xCC33FF99, 0xFF297085,
                                                            0xFFFF3302, 0xFFD67685)
DEFINE_LAYER_BLEND_TEST(Multiply, SkBlendMode::kMultiply, 0xFF0099FF, 0xCC33FF99, 0xFF0099AD,
                                                          0xFFFD3300, 0xFF001F00)

// HSL component blend modes
DEFINE_LAYER_BLEND_TEST(Hue, SkBlendMode::kHue, 0xFF0099FF, 0xCC33FF99, 0xFF00B17C, 0xFFFE3300,
                                                0xFFDD4F2C)
DEFINE_LAYER_BLEND_TEST(Saturation, SkBlendMode::kSaturation, 0xFF0099FF, 0xCC33FF99, 0xFF1393E9,
                                                              0xFFFD3302, 0xFF0099FF)
DEFINE_LAYER_BLEND_TEST(Color, SkBlendMode::kColor, 0xFF0099FF, 0xCC33FF99, 0xFF00B17C, 0xFFFE3300,
                                                    0xFFFF4314)
DEFINE_LAYER_BLEND_TEST(Luminosity, SkBlendMode::kLuminosity, 0xFF0099FF, 0xCC33FF99, 0xFF60BFFF,
                                                              0xFFFE3302, 0xFF2180C0)
