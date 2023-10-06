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
#include "include/core/SkPicture.h"  // IWYU pragma: keep
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"

#include <cstdint>
#include <initializer_list>

struct GrContextOptions;

static void check_isopaque(skiatest::Reporter* reporter, const sk_sp<SkSurface>& surface,
                           bool expectedOpaque) {
    sk_sp<SkImage> image(surface->makeImageSnapshot());
    REPORTER_ASSERT(reporter, image->isOpaque() == expectedOpaque);
}

DEF_TEST(ImageIsOpaqueTest, reporter) {
    SkImageInfo infoTransparent = SkImageInfo::MakeN32Premul(5, 5);
    auto surfaceTransparent(SkSurfaces::Raster(infoTransparent));
    check_isopaque(reporter, surfaceTransparent, false);

    SkImageInfo infoOpaque = SkImageInfo::MakeN32(5, 5, kOpaque_SkAlphaType);
    auto surfaceOpaque(SkSurfaces::Raster(infoOpaque));
    check_isopaque(reporter, surfaceOpaque, true);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ImageIsOpaqueTest_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto context = ctxInfo.directContext();
    SkImageInfo infoTransparent = SkImageInfo::MakeN32Premul(5, 5);
    auto surfaceTransparent(
            SkSurfaces::RenderTarget(context, skgpu::Budgeted::kNo, infoTransparent));
    check_isopaque(reporter, surfaceTransparent, false);

    SkImageInfo infoOpaque = SkImageInfo::MakeN32(5, 5, kOpaque_SkAlphaType);
    auto surfaceOpaque(SkSurfaces::RenderTarget(context, skgpu::Budgeted::kNo, infoOpaque));

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
                 SkImages::RasterFromPixmapCopy(pmap),
                 ToolUtils::GetResourceAsImage("images/mandrill_128.png"),
                 ToolUtils::GetResourceAsImage("images/color_wheel.jpg"),
                 SkImages::DeferredFromPicture(make_picture(),
                                               {10, 10},
                                               nullptr,
                                               nullptr,
                                               SkImages::BitDepth::kU8,
                                               SkColorSpace::MakeSRGB()),
         }) {
        REPORTER_ASSERT(reporter, image->isAlphaOnly() == false);
    }

    REPORTER_ASSERT(
            reporter,
            SkImages::RasterFromPixmapCopy({SkImageInfo::MakeA8(1, 1), (uint8_t*)&pmColors, 1})
                            ->isAlphaOnly() == true);
}
