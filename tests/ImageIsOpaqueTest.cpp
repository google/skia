/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <cstdint>
#include <initializer_list>

class SkPicture;
struct GrContextOptions;

static void check_isopaque(skiatest::Reporter* reporter, const sk_sp<SkSurface>& surface,
                           bool expectedOpaque) {
    sk_sp<SkImage> image(surface->makeImageSnapshot());
    REPORTER_ASSERT(reporter, image->isOpaque() == expectedOpaque);
}

DEF_TEST(ImageIsOpaqueTest, reporter) {
    SkImageInfo infoTransparent = SkImageInfo::MakeN32Premul(5, 5);
    auto surfaceTransparent(SkSurface::MakeRaster(infoTransparent));
    check_isopaque(reporter, surfaceTransparent, false);

    SkImageInfo infoOpaque = SkImageInfo::MakeN32(5, 5, kOpaque_SkAlphaType);
    auto surfaceOpaque(SkSurface::MakeRaster(infoOpaque));
    check_isopaque(reporter, surfaceOpaque, true);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ImageIsOpaqueTest_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto context = ctxInfo.directContext();
    SkImageInfo infoTransparent = SkImageInfo::MakeN32Premul(5, 5);
    auto surfaceTransparent(SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, infoTransparent));
    check_isopaque(reporter, surfaceTransparent, false);

    SkImageInfo infoOpaque = SkImageInfo::MakeN32(5, 5, kOpaque_SkAlphaType);
    auto surfaceOpaque(SkSurface::MakeRenderTarget(context,SkBudgeted::kNo, infoOpaque));

    check_isopaque(reporter, surfaceOpaque, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "include/core/SkPictureRecorder.h"

static sk_sp<SkPicture> make_picture() {
    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording({ 0, 0, 10, 10 });
    canvas->drawColor(SK_ColorRED);
    return recorder.finishRecordingAsPicture();
}

DEF_TEST(Image_isAlphaOnly, reporter) {
    SkPMColor pmColors = 0;
    SkPixmap pmap = {
        SkImageInfo::MakeN32Premul(1, 1),
        &pmColors,
        sizeof(pmColors)
    };
    for (auto& image : {
        SkImage::MakeRasterCopy(pmap),
        GetResourceAsImage("images/mandrill_128.png"),
        GetResourceAsImage("images/color_wheel.jpg"),
        SkImage::MakeFromPicture(make_picture(), { 10, 10 }, nullptr, nullptr,
                                 SkImage::BitDepth::kU8,
                                 SkColorSpace::MakeSRGB()),
    })
    {
        REPORTER_ASSERT(reporter, image->isAlphaOnly() == false);
    }

    REPORTER_ASSERT(reporter, SkImage::MakeRasterCopy({
        SkImageInfo::MakeA8(1, 1), (uint8_t*)&pmColors, 1})->isAlphaOnly() == true);
}
