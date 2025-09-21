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
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "tests/Test.h"

#include "src/gpu/graphite/Surface_Graphite.h"

using namespace skgpu::graphite;

DEF_GRAPHITE_TEST_FOR_CONTEXTS(NotifyInUseTestSnapshot, /*filter=*/nullptr, reporter, context,
                               testContext, CtsEnforcement::kApiLevel_202404) {
    auto recorder = context->makeRecorder();
    SkImageInfo info = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> scratchSurface = SkSurfaces::RenderTarget(recorder.get(), info);
    SkCanvas* scratchCanvas = scratchSurface->getCanvas();
    SkPaint greenPaint;
    greenPaint.setColor(SK_ColorGREEN);
    scratchCanvas->drawRect(SkRect::Make(info.bounds()), greenPaint);

    sk_sp<SkImage> scratchImage = SkSurfaces::AsImage(scratchSurface);
    REPORTER_ASSERT(reporter, scratchImage);

    sk_sp<SkSurface> mainSurface = SkSurfaces::RenderTarget(recorder.get(), info);
    SkCanvas* mainCanvas = mainSurface->getCanvas();
    mainCanvas->clear(SK_ColorBLACK);

    mainCanvas->drawImage(scratchImage, 0, 0);

    sk_sp<SkSurface> finalSurface = SkSurfaces::RenderTarget(recorder.get(), info);
    SkCanvas* finalCanvas = finalSurface->getCanvas();
    finalCanvas->clear(SK_ColorBLACK);

    /*
       Force a Device::makeImageCopy by calling makeImageSnapshot(). This in turn calls
       flushPendingWork(nullptr), then Image::Copy. *However* because flushPendingWork only flushes
       the tasks from mainSurface, when Image::Copy adds the copy task directly to the recorder's
       root task list, the scratchSurface is not yet flushed! So when the recorder snaps in
       readPixels, the initial draw which mainSurface depends on is recorded *after* the draw in
       mainSurface, resulting in an incorrect image:

        =========== RECORDING ===========
        **** FLUSH TOKEN 1 Snap ****
        0: Draw Task (target=0x120e84570)   <-- mainSurface's drawTask
        └── RenderPass Task
        1: Copy TtoT Task                   <-- the copy task from mainSurface to finalSurface
        2: Draw Task (target=0x120e834a0)   <-- scratchSuface's drawTask
        └── RenderPass Task
        3: Draw Task (target=0x120e85690)   <-- finalSurfaces's drawTask
        └── RenderPass Task
        -------------- END --------------

       The correct ordering:
        =========== RECORDING ===========
        **** FLUSH TOKEN 1 Snap ****
        0: Draw Task (target=0x120e834a0)   <-- scratchSuface's drawTask
        └── RenderPass Task
        1: Draw Task (target=0x120e84570)   <-- mainSurface's drawTask
        └── RenderPass Task
        2: Copy TtoT Task                   <-- the copy task from mainSurface to finalSurface
        3: Draw Task (target=0x120e85690)   <-- finalSurfaces's drawContext
        └── RenderPass Task
        -------------- END --------------
    */

    finalCanvas->drawImage(mainSurface->makeImageSnapshot(), 0, 0);

    SkBitmap bitmap;
    bitmap.allocPixels(info);
    REPORTER_ASSERT(reporter, finalSurface->readPixels(bitmap, 0, 0));

    SkColor topLeft = bitmap.getColor(0, 0);
    REPORTER_ASSERT(reporter, topLeft == SK_ColorGREEN,
                    "Expected green pixel from scratch surface, got 0x%08X", topLeft);
}

DEF_GRAPHITE_TEST_FOR_CONTEXTS(NotifyInUseTestBlend, /*filter=*/nullptr, reporter, context,
                               testContext, CtsEnforcement::kApiLevel_202404) {
    auto recorder = context->makeRecorder();
    SkImageInfo info = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> scratchSurface = SkSurfaces::RenderTarget(recorder.get(), info);
    SkCanvas* scratchCanvas = scratchSurface->getCanvas();
    SkPaint greenPaint;
    greenPaint.setColor(SK_ColorGREEN);
    scratchCanvas->drawRect(SkRect::Make(info.bounds()), greenPaint);

    sk_sp<SkImage> scratchImage = SkSurfaces::AsImage(scratchSurface);
    REPORTER_ASSERT(reporter, scratchImage);

    sk_sp<SkSurface> mainSurface = SkSurfaces::RenderTarget(recorder.get(), info);
    SkCanvas* mainCanvas = mainSurface->getCanvas();
    mainCanvas->clear(SK_ColorTRANSPARENT);

    /*
       This tests a scenario where a device lacking frame buffer fetch support flushes midway
       through Device::drawGeometry. Due to the SkBlendMode::Hue, a copy TtoT is required, but if
       the resulting task is not added as a child of the current drawTask, this test will fail.

       Correct ordering (for a device where shaderCaps->fFBFetchSupport = false):
        =========== RECORDING ===========
        **** FLUSH TOKEN 1 Snap ****
        0: Draw Task (target=0x11f495a50)    <-- scratchSuface's drawTask
        └── RenderPass Task
        1: Draw Task (target=0x11f494960)    <-- mainSurface's clear
        └── RenderPass Task
        2: Draw Task (target=0x11f495a50)    <-- mainSurface's draw
        │   Copy TtoT Task                   <-- copy requied by hue blend, child of draw task
        └── RenderPass Task
        3: Draw Task (target=0x11f497350)    <-- finalSurface's draw
        └── RenderPass Task
        -------------- END --------------
    */

    SkPaint blendPaint;
    blendPaint.setBlendMode(SkBlendMode::kHue);
    mainCanvas->drawImage(scratchImage, 0, 0, SkSamplingOptions(), &blendPaint);

    sk_sp<SkSurface> finalSurface = SkSurfaces::RenderTarget(recorder.get(), info);
    SkCanvas* finalCanvas = finalSurface->getCanvas();
    finalCanvas->clear(SK_ColorBLACK);

    finalCanvas->drawImage(SkSurfaces::AsImage(mainSurface), 0, 0);

    SkBitmap bitmap;
    bitmap.allocPixels(info);
    REPORTER_ASSERT(reporter, finalCanvas->readPixels(bitmap, 0, 0));

    SkColor topLeft = bitmap.getColor(0, 0);
    REPORTER_ASSERT(reporter, topLeft == SK_ColorGREEN,
                    "Expected green pixel from scratch surface, got 0x%08X", topLeft);
}
