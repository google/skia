/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/private/base/SkTemplates.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"

using namespace skia_private;

namespace skiagm {

static inline void read_into_pixmap(SkPixmap* dst, SkImageInfo dstInfo, void* dstPixels,
        sk_sp<SkImage> src) {
    dst->reset(dstInfo, dstPixels, dstInfo.minRowBytes());
    src->readPixels(nullptr, *dst, 0, 0, SkImage::CachingHint::kDisallow_CachingHint);
}

static inline sk_sp<SkImage> encode_pixmap_and_make_image(const SkPixmap& src,
        SkJpegEncoder::AlphaOption alphaOption) {
    SkDynamicMemoryWStream dst;
    SkJpegEncoder::Options options;
    options.fAlphaOption = alphaOption;
    SkJpegEncoder::Encode(&dst, src, options);
    return SkImages::DeferredFromEncodedData(dst.detachAsData());
}

class EncodeJpegAlphaOptsGM : public GM {
public:
    EncodeJpegAlphaOptsGM() {}

protected:
    SkString getName() const override { return SkString("encode-alpha-jpeg"); }

    SkISize getISize() override { return SkISize::Make(400, 200); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        sk_sp<SkImage> srcImg = ToolUtils::GetResourceAsImage("images/rainbow-gradient.png");
        if (!srcImg) {
            *errorMsg = "Could not load images/rainbow-gradient.png. "
                        "Did you forget to set the resourcePath?";
            return DrawResult::kFail;
        }
        fStorage.reset(srcImg->width() * srcImg->height() *
                SkColorTypeBytesPerPixel(kRGBA_F16_SkColorType));

        SkPixmap src;
        SkImageInfo info = SkImageInfo::MakeN32Premul(srcImg->width(), srcImg->height(),
                canvas->imageInfo().colorSpace() ? SkColorSpace::MakeSRGB() : nullptr);
        read_into_pixmap(&src, info, fStorage.get(), srcImg);

        // Encode 8888 premul.
        auto img0 = encode_pixmap_and_make_image(src, SkJpegEncoder::AlphaOption::kIgnore);
        auto img1 = encode_pixmap_and_make_image(src, SkJpegEncoder::AlphaOption::kBlendOnBlack);
        canvas->drawImage(img0, 0.0f, 0.0f);
        canvas->drawImage(img1, 0.0f, 100.0f);

        // Encode 8888 unpremul
        info = info.makeAlphaType(kUnpremul_SkAlphaType);
        read_into_pixmap(&src, info, fStorage.get(), srcImg);
        img0 = encode_pixmap_and_make_image(src, SkJpegEncoder::AlphaOption::kIgnore);
        img1 = encode_pixmap_and_make_image(src, SkJpegEncoder::AlphaOption::kBlendOnBlack);
        canvas->drawImage(img0, 100.0f, 0.0f);
        canvas->drawImage(img1, 100.0f, 100.0f);

        // Encode F16 premul
        info = SkImageInfo::Make(srcImg->width(), srcImg->height(), kRGBA_F16_SkColorType,
                kPremul_SkAlphaType, SkColorSpace::MakeSRGB());
        read_into_pixmap(&src, info, fStorage.get(), srcImg);
        img0 = encode_pixmap_and_make_image(src, SkJpegEncoder::AlphaOption::kIgnore);
        img1 = encode_pixmap_and_make_image(src, SkJpegEncoder::AlphaOption::kBlendOnBlack);
        canvas->drawImage(img0, 200.0f, 0.0f);
        canvas->drawImage(img1, 200.0f, 100.0f);

        // Encode F16 unpremul
        info = info.makeAlphaType(kUnpremul_SkAlphaType);
        read_into_pixmap(&src, info, fStorage.get(), srcImg);
        img0 = encode_pixmap_and_make_image(src, SkJpegEncoder::AlphaOption::kIgnore);
        img1 = encode_pixmap_and_make_image(src, SkJpegEncoder::AlphaOption::kBlendOnBlack);
        canvas->drawImage(img0, 300.0f, 0.0f);
        canvas->drawImage(img1, 300.0f, 100.0f);

        return DrawResult::kOk;
    }

private:
    AutoTMalloc<uint8_t> fStorage;

    using INHERITED = GM;
};

DEF_GM( return new EncodeJpegAlphaOptsGM; )

}  // namespace skiagm
