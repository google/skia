/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#include "Resources.h"
#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#endif
#include "SkCanvas.h"
#include "SkColorSpace_Base.h"
#include "SkImage.h"
#include "SkSurface.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

static void test_flatten(skiatest::Reporter* reporter, const SkImageInfo& info) {
    // Need a safe amount of 4-byte aligned storage.  Note that one of the test ICC profiles
    // is ~7500 bytes.
    const size_t storageBytes = 8000;
    SkAutoTMalloc<uint32_t> storage(storageBytes / sizeof(uint32_t));
    SkBinaryWriteBuffer wb(storage.get(), storageBytes);
    info.flatten(wb);
    SkASSERT(wb.bytesWritten() < storageBytes);

    SkReadBuffer rb(storage.get(), wb.bytesWritten());

    // pick a noisy byte pattern, so we ensure that unflatten sets all of our fields
    SkImageInfo info2 = SkImageInfo::Make(0xB8, 0xB8, (SkColorType) 0xB8, (SkAlphaType) 0xB8);

    info2.unflatten(rb);
    REPORTER_ASSERT(reporter, rb.offset() == wb.bytesWritten());

    REPORTER_ASSERT(reporter, info == info2);
}

DEF_TEST(ImageInfo_flattening, reporter) {
     sk_sp<SkData> data =
             SkData::MakeFromFileName(GetResourcePath("icc_profiles/HP_ZR30w.icc").c_str());
    sk_sp<SkColorSpace> space0 = SkColorSpace::MakeICC(data->data(), data->size());
    data = SkData::MakeFromFileName( GetResourcePath("icc_profiles/HP_Z32x.icc").c_str());
    sk_sp<SkColorSpace> space1 = SkColorSpace::MakeICC(data->data(), data->size());
    data = SkData::MakeFromFileName(GetResourcePath("icc_profiles/upperLeft.icc").c_str());
    sk_sp<SkColorSpace> space2 = SkColorSpace::MakeICC(data->data(), data->size());
    data = SkData::MakeFromFileName(GetResourcePath("icc_profiles/upperRight.icc").c_str());
    sk_sp<SkColorSpace> space3 = SkColorSpace::MakeICC(data->data(), data->size());

    sk_sp<SkColorSpace> spaces[] = {
        nullptr,
        SkColorSpace::MakeSRGB(),
        SkColorSpace_Base::MakeNamed(SkColorSpace_Base::kAdobeRGB_Named),
        space0,
        space1,
        space2,
        space3,
    };

    for (int ct = 0; ct <= kLastEnum_SkColorType; ++ct) {
        for (int at = 0; at <= kLastEnum_SkAlphaType; ++at) {
            for (auto& cs : spaces) {
                SkImageInfo info = SkImageInfo::Make(100, 200,
                                                     static_cast<SkColorType>(ct),
                                                     static_cast<SkAlphaType>(at),
                                                     cs);
                test_flatten(reporter, info);
            }
        }
    }
}

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

#if SK_SUPPORT_GPU

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ImageIsOpaqueTest_Gpu, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    SkImageInfo infoTransparent = SkImageInfo::MakeN32Premul(5, 5);
    auto surfaceTransparent(SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, infoTransparent));
    check_isopaque(reporter, surfaceTransparent, false);

    SkImageInfo infoOpaque = SkImageInfo::MakeN32(5, 5, kOpaque_SkAlphaType);
    auto surfaceOpaque(SkSurface::MakeRenderTarget(context,SkBudgeted::kNo, infoOpaque));

    check_isopaque(reporter, surfaceOpaque, true);
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "SkPictureRecorder.h"

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
        GetResourceAsImage("mandrill_128.png"),
        GetResourceAsImage("color_wheel.jpg"),
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
