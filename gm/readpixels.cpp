/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "modules/skcms/skcms.h"
#include "tools/Resources.h"

#include <string.h>
#include <memory>
#include <utility>

static const int kWidth = 64;
static const int kHeight = 64;

static sk_sp<SkImage> make_raster_image(SkColorType colorType) {
    std::unique_ptr<SkStream> stream(GetResourceAsStream("images/google_chrome.ico"));
    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromStream(std::move(stream));
    if (!codec) {
        return nullptr;
    }

    SkImageInfo info = codec->getInfo().makeWH(kWidth, kHeight)
                                       .makeColorType(colorType)
                                       .makeAlphaType(kPremul_SkAlphaType);
    return std::get<0>(codec->getImage(info));
}

static sk_sp<SkImage> make_codec_image() {
    sk_sp<SkData> encoded = GetResourceAsData("images/randPixels.png");
    return SkImages::DeferredFromEncodedData(encoded);
}

static void draw_contents(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(20);
    paint.setColor(0xFF800000);
    canvas->drawCircle(40, 40, 35, paint);
    paint.setColor(0xFF008000);
    canvas->drawCircle(50, 50, 35, paint);
    paint.setColor(0xFF000080);
    canvas->drawCircle(60, 60, 35, paint);
}

static sk_sp<SkImage> make_picture_image() {
    SkPictureRecorder recorder;
    draw_contents(recorder.beginRecording(SkRect::MakeIWH(kWidth, kHeight)));
    return SkImages::DeferredFromPicture(recorder.finishRecordingAsPicture(),
                                         SkISize::Make(kWidth, kHeight),
                                         nullptr,
                                         nullptr,
                                         SkImages::BitDepth::kU8,
                                         SkColorSpace::MakeSRGB());
}

static sk_sp<SkColorSpace> make_parametric_transfer_fn(const SkColorSpacePrimaries& primaries) {
    skcms_Matrix3x3 toXYZD50;
    SkAssertResult(primaries.toXYZD50(&toXYZD50));
    skcms_TransferFunction fn = { 1.8f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f };
    return SkColorSpace::MakeRGB(fn, toXYZD50);
}

static sk_sp<SkColorSpace> make_wide_gamut() {
    // ProPhoto
    SkColorSpacePrimaries primaries;
    primaries.fRX = 0.7347f;
    primaries.fRY = 0.2653f;
    primaries.fGX = 0.1596f;
    primaries.fGY = 0.8404f;
    primaries.fBX = 0.0366f;
    primaries.fBY = 0.0001f;
    primaries.fWX = 0.34567f;
    primaries.fWY = 0.35850f;
    return make_parametric_transfer_fn(primaries);
}

static sk_sp<SkColorSpace> make_small_gamut() {
    SkColorSpacePrimaries primaries;
    primaries.fRX = 0.50f;
    primaries.fRY = 0.33f;
    primaries.fGX = 0.30f;
    primaries.fGY = 0.50f;
    primaries.fBX = 0.25f;
    primaries.fBY = 0.16f;
    primaries.fWX = 0.3127f;
    primaries.fWY = 0.3290f;
    return make_parametric_transfer_fn(primaries);
}

static void draw_image(GrDirectContext* dContext, SkCanvas* canvas, SkImage* image,
                       SkColorType dstColorType, SkAlphaType dstAlphaType,
                       sk_sp<SkColorSpace> dstColorSpace, SkImage::CachingHint hint) {
    size_t rowBytes = image->width() * SkColorTypeBytesPerPixel(dstColorType);
    sk_sp<SkData> data = SkData::MakeUninitialized(rowBytes * image->height());
    SkImageInfo dstInfo = SkImageInfo::Make(image->width(), image->height(), dstColorType,
                                            dstAlphaType, dstColorSpace);
    if (!image->readPixels(dContext, dstInfo, data->writable_data(), rowBytes, 0, 0, hint)) {
        memset(data->writable_data(), 0, rowBytes * image->height());
    }

    // Now that we have called readPixels(), dump the raw pixels into an srgb image.
    sk_sp<SkColorSpace> srgb = SkColorSpace::MakeSRGB();
    sk_sp<SkImage> raw = SkImages::RasterFromData(dstInfo.makeColorSpace(srgb), data, rowBytes);
    canvas->drawImage(raw.get(), 0.0f, 0.0f);
}

class ReadPixelsGM : public skiagm::GM {
public:
    ReadPixelsGM() {}

protected:
    SkString getName() const override { return SkString("readpixels"); }

    SkISize getISize() override { return SkISize::Make(6 * kWidth, 9 * kHeight); }

    void onDraw(SkCanvas* canvas) override {
        const SkAlphaType alphaTypes[] = {
                kUnpremul_SkAlphaType,
                kPremul_SkAlphaType,
        };
        const SkColorType colorTypes[] = {
                kRGBA_8888_SkColorType,
                kBGRA_8888_SkColorType,
                kRGBA_F16_SkColorType,
        };
        const sk_sp<SkColorSpace> colorSpaces[] = {
                make_wide_gamut(),
                SkColorSpace::MakeSRGB(),
                make_small_gamut(),
        };

        for (const sk_sp<SkColorSpace>& dstColorSpace : colorSpaces) {
            for (SkColorType srcColorType : colorTypes) {
                canvas->save();
                sk_sp<SkImage> image = make_raster_image(srcColorType);
                if (!image) {
                    continue;
                }
                auto dContext = GrAsDirectContext(canvas->recordingContext());
                if (dContext) {
                    image = SkImages::TextureFromImage(dContext, image);
                }
                if (image) {
                    for (SkColorType dstColorType : colorTypes) {
                        for (SkAlphaType dstAlphaType : alphaTypes) {
                            draw_image(dContext, canvas, image.get(), dstColorType, dstAlphaType,
                                       dstColorSpace, SkImage::kAllow_CachingHint);
                            canvas->translate((float)kWidth, 0.0f);
                        }
                    }
                }
                canvas->restore();
                canvas->translate(0.0f, (float) kHeight);
            }
        }
    }

private:
    using INHERITED = skiagm::GM;
};
DEF_GM( return new ReadPixelsGM; )

class ReadPixelsCodecGM : public skiagm::GM {
public:
    ReadPixelsCodecGM() {}

protected:
    SkString getName() const override { return SkString("readpixelscodec"); }

    SkISize getISize() override {
        return SkISize::Make(3 * (kEncodedWidth + 1), 12 * (kEncodedHeight + 1));
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        if (!canvas->imageInfo().colorSpace()) {
            *errorMsg = "This gm is only interesting in color correct modes.";
            return DrawResult::kSkip;
        }

        const SkAlphaType alphaTypes[] = {
                kUnpremul_SkAlphaType,
                kPremul_SkAlphaType,
        };
        const SkColorType colorTypes[] = {
                kRGBA_8888_SkColorType,
                kBGRA_8888_SkColorType,
                kRGBA_F16_SkColorType,
        };
        const sk_sp<SkColorSpace> colorSpaces[] = {
                make_wide_gamut(),
                SkColorSpace::MakeSRGB(),
                make_small_gamut(),
        };
        const SkImage::CachingHint hints[] = {
                SkImage::kAllow_CachingHint,
                SkImage::kDisallow_CachingHint,
        };

        sk_sp<SkImage> image = make_codec_image();
        for (const sk_sp<SkColorSpace>& dstColorSpace : colorSpaces) {
            canvas->save();
            for (SkColorType dstColorType : colorTypes) {
                for (SkAlphaType dstAlphaType : alphaTypes) {
                    for (SkImage::CachingHint hint : hints) {
                        draw_image(nullptr, canvas, image.get(), dstColorType, dstAlphaType,
                                   dstColorSpace, hint);
                        canvas->translate(0.0f, (float) kEncodedHeight + 1);
                    }
                }
            }
            canvas->restore();
            canvas->translate((float) kEncodedWidth + 1, 0.0f);
        }
        return DrawResult::kOk;
    }

private:
    static const int kEncodedWidth = 8;
    static const int kEncodedHeight = 8;

    using INHERITED = skiagm::GM;
};
DEF_GM( return new ReadPixelsCodecGM; )

class ReadPixelsPictureGM : public skiagm::GM {
public:
    ReadPixelsPictureGM() {}

protected:
    SkString getName() const override { return SkString("readpixelspicture"); }

    SkISize getISize() override { return SkISize::Make(3 * kWidth, 12 * kHeight); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        if (!canvas->imageInfo().colorSpace()) {
            *errorMsg = "This gm is only interesting in color correct modes.";
            return DrawResult::kSkip;
        }

        const sk_sp<SkImage> images[] = {
                make_picture_image(),
        };
        const SkAlphaType alphaTypes[] = {
                kUnpremul_SkAlphaType,
                kPremul_SkAlphaType,
        };
        const SkColorType colorTypes[] = {
                kRGBA_8888_SkColorType,
                kBGRA_8888_SkColorType,
                kRGBA_F16_SkColorType,
        };
        const sk_sp<SkColorSpace> colorSpaces[] = {
                make_wide_gamut(),
                SkColorSpace::MakeSRGB(),
                make_small_gamut(),
        };
        const SkImage::CachingHint hints[] = {
                SkImage::kAllow_CachingHint,
                SkImage::kDisallow_CachingHint,
        };

        for (const sk_sp<SkImage>& image : images) {
            for (const sk_sp<SkColorSpace>& dstColorSpace : colorSpaces) {
                canvas->save();
                for (SkColorType dstColorType : colorTypes) {
                    for (SkAlphaType dstAlphaType : alphaTypes) {
                        for (SkImage::CachingHint hint : hints) {
                            draw_image(nullptr, canvas, image.get(), dstColorType, dstAlphaType,
                                       dstColorSpace, hint);
                            canvas->translate(0.0f, (float) kHeight);
                        }
                    }
                }
                canvas->restore();
                canvas->translate((float) kWidth, 0.0f);
            }
        }
        return DrawResult::kOk;
    }

private:

    using INHERITED = skiagm::GM;
};
DEF_GM( return new ReadPixelsPictureGM; )
