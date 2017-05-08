/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkImage.h"
#include "SkJpegEncoder.h"

#include "Resources.h"

namespace skiagm {

static inline void set_pixmap(SkPixmap* dst, SkImageInfo dstInfo, void* dstPixels,
        sk_sp<SkImage> src) {
    dst->reset(dstInfo, dstPixels, dstInfo.minRowBytes());
    src->readPixels(*dst, 0, 0, SkImage::CachingHint::kDisallow_CachingHint);
}

static inline sk_sp<SkImage> encoded_image(const SkPixmap& src,
        SkJpegEncoder::AlphaOption alphaOption, SkTransferFunctionBehavior blendBehavior) {
    SkDynamicMemoryWStream dst;
    SkJpegEncoder::Options options;
    options.fAlphaOption = alphaOption;
    options.fBlendBehavior = blendBehavior;
    SkJpegEncoder::Encode(&dst, src, options);
    return SkImage::MakeFromEncoded(dst.detachAsData());
}

class EncodeJpegAlphaOptsGM : public GM {
public:
    EncodeJpegAlphaOptsGM() {}

protected:
    SkString onShortName() override {
        return SkString("encode-alpha-jpeg");
    }

    SkISize onISize() override {
        return SkISize::Make(400, 200);
    }

    void onDraw(SkCanvas* canvas) override {
        // Set up the |src| pixmap.
        sk_sp<SkImage> srcImg = GetResourceAsImage("rainbow-gradient.png");
        fStorage.reset(srcImg->width() * srcImg->height() *
                SkColorTypeBytesPerPixel(kRGBA_F16_SkColorType));

        SkPixmap src;
        SkImageInfo info = SkImageInfo::MakeN32Premul(srcImg->width(), srcImg->height(),
                canvas->imageInfo().colorSpace() ? SkColorSpace::MakeSRGB() : nullptr);
        set_pixmap(&src, info, fStorage.get(), srcImg);

        SkTransferFunctionBehavior behavior = canvas->imageInfo().colorSpace() ?
                SkTransferFunctionBehavior::kRespect : SkTransferFunctionBehavior::kIgnore;

        sk_sp<SkImage> img0 = encoded_image(src, SkJpegEncoder::AlphaOption::kIgnore, behavior);
        sk_sp<SkImage> img1 = encoded_image(src, SkJpegEncoder::AlphaOption::kBlendOnBlack, behavior);
        canvas->drawImage(img0, 0.0f, 0.0f);
        canvas->drawImage(img1, 0.0f, 100.0f);

        info = info.makeAlphaType(kUnpremul_SkAlphaType);
        set_pixmap(&src, info, fStorage.get(), srcImg);
        img0 = encoded_image(src, SkJpegEncoder::AlphaOption::kIgnore, behavior);
        img1 = encoded_image(src, SkJpegEncoder::AlphaOption::kBlendOnBlack, behavior);
        canvas->drawImage(img0, 100.0f, 0.0f);
        canvas->drawImage(img1, 100.0f, 100.0f);

        if (canvas->imageInfo().colorSpace()) {
            info = SkImageInfo::Make(srcImg->width(), srcImg->height(), kRGBA_F16_SkColorType,
                    kPremul_SkAlphaType, SkColorSpace::MakeSRGBLinear());
            set_pixmap(&src, info, fStorage.get(), srcImg);
            img0 = encoded_image(src, SkJpegEncoder::AlphaOption::kIgnore, behavior);
            img1 = encoded_image(src, SkJpegEncoder::AlphaOption::kBlendOnBlack, behavior);
            canvas->drawImage(img0, 200.0f, 0.0f);
            canvas->drawImage(img1, 200.0f, 100.0f);

            info = info.makeAlphaType(kUnpremul_SkAlphaType);
            set_pixmap(&src, info, fStorage.get(), srcImg);
            img0 = encoded_image(src, SkJpegEncoder::AlphaOption::kIgnore, behavior);
            img1 = encoded_image(src, SkJpegEncoder::AlphaOption::kBlendOnBlack, behavior);
            canvas->drawImage(img0, 300.0f, 0.0f);
            canvas->drawImage(img1, 300.0f, 100.0f);
        }
    }

private:
    SkAutoTMalloc<uint8_t> fStorage;

    typedef GM INHERITED;
};

DEF_GM( return new EncodeJpegAlphaOptsGM; )

};
