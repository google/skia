/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "Resources.h"
#include "SkCodec.h"
#include "SkImage.h"

namespace skiagm {

class BitmapImageGM : public GM {
public:
    BitmapImageGM() {}

protected:

    SkString onShortName() override {
        return SkString("bitmap-image-srgb-legacy");
    }

    SkISize onISize() override {
        return SkISize::Make(2*kSize, 2*kSize);
    }

    void onDraw(SkCanvas* canvas) override {
        // Create image.
        const char* path = "mandrill_512_q075.jpg";
        sk_sp<SkImage> image = GetResourceAsImage(path);
        if (!image) {
            SkDebugf("Failure: Is the resource path set properly?");
            return;
        }

        // Create matching bitmap.
        std::unique_ptr<SkCodec> codec(SkCodec::MakeFromStream(GetResourceAsStream(path)));
        SkBitmap bitmap;
        bitmap.allocPixels(codec->getInfo());
        codec->getPixels(codec->getInfo(), bitmap.getPixels(), bitmap.rowBytes());

        // The GM will be displayed in a 2x2 grid.
        // The top two squares show an sRGB image, then bitmap, drawn to a legacy canvas.
        SkImageInfo linearInfo = SkImageInfo::MakeN32(2*kSize, kSize, kOpaque_SkAlphaType);
        SkBitmap legacyBMCanvas;
        legacyBMCanvas.allocPixels(linearInfo);
        SkCanvas legacyCanvas(legacyBMCanvas);
        legacyCanvas.drawImage(image, 0.0f, 0.0f, nullptr);
        legacyCanvas.translate(SkScalar(kSize), 0.0f);
        legacyCanvas.drawBitmap(bitmap, 0.0f, 0.0f, nullptr);
        canvas->drawBitmap(legacyBMCanvas, 0.0f, 0.0f, nullptr);
        canvas->translate(0.0f, SkScalar(kSize));

        // The bottom two squares show an sRGB image, then bitmap, drawn to a srgb canvas.
        SkImageInfo srgbInfo = SkImageInfo::MakeS32(2*kSize, kSize, kOpaque_SkAlphaType);
        SkBitmap srgbBMCanvas;
        srgbBMCanvas.allocPixels(srgbInfo);
        SkCanvas srgbCanvas(srgbBMCanvas);
        srgbCanvas.drawImage(image, 0.0f, 0.0f, nullptr);
        srgbCanvas.translate(SkScalar(kSize), 0.0f);
        srgbCanvas.drawBitmap(bitmap, 0.0f, 0.0f, nullptr);
        canvas->drawBitmap(srgbBMCanvas, 0.0f, 0.0f, nullptr);
    }

private:
    static constexpr int kSize = 512;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new BitmapImageGM; }
static GMRegistry reg(MyFactory);

//////////////////////////////////////////////////////////////////////////////

// This reproduces the performance regression of b/70172912 .
class DrawScaledBitmapGM : public GM {
public:
    DrawScaledBitmapGM(SkColorType ct, SkFilterQuality fq) : fColorType(ct), fFilterQuality(fq) {}

protected:
    SkString onShortName() override {
        const char* colorTypeName = sk_tool_utils::colortype_name(fColorType);
        const char* filterQualityName;
        switch (fFilterQuality) {
            case kNone_SkFilterQuality:
                filterQualityName = "none";
                break;
            case kLow_SkFilterQuality:
                filterQualityName = "bilinear";
                break;
            default:
                filterQualityName = "bicubic";
        }
        return SkStringPrintf("bitmap_scale_%s_%s", filterQualityName, colorTypeName);
    }

    SkISize onISize() override {
        return SkISize::Make(1000, 1000);
    }

    bool runAsBench() const override { return true; }

    void onDraw(SkCanvas* canvas) override {
        const char* path = "mandrill_256.png";
        sk_sp<SkImage> image = GetResourceAsImage(path);
        if (!image) {
            SkDebugf("Failure: Is the resource path set properly?\n");
            return;
        }

        int w = image->width();
        int h = image->height();

        auto info = SkImageInfo::Make(w, h, fColorType, kOpaque_SkAlphaType);
        SkBitmap bitmap;
        bitmap.allocPixels(info);
        SkCanvas bitmapCanvas(bitmap);
        bitmapCanvas.drawImage(image.get(), 0, 0);

        auto infoScaled = SkImageInfo::Make(w * 3, h * 3, fColorType, kOpaque_SkAlphaType);
        SkBitmap bitmapScaled;
        bitmapScaled.allocPixels(infoScaled);
        SkCanvas bitmapCanvasScaled(bitmapScaled);

        SkPaint tmpPaint;
        tmpPaint.setFilterQuality(fFilterQuality);
        sk_sp<SkImage> tmpImage = SkImage::MakeFromBitmap(bitmap);
        SkRect srcRect = {0, 0, float(w), float(h)};
        SkRect dstRect = {0, 0, float(w) * 3, float(h) * 3};
        bitmapCanvasScaled.drawImageRect(tmpImage.get(), srcRect, dstRect, &tmpPaint,
                                         SkCanvas::kFast_SrcRectConstraint);

        canvas->drawBitmap(bitmapScaled, 0, 0);
    }

private:
    SkColorType     fColorType;
    SkFilterQuality fFilterQuality;
};

DEF_GM( return new DrawScaledBitmapGM(kRGB_565_SkColorType, kNone_SkFilterQuality); )
DEF_GM( return new DrawScaledBitmapGM(kN32_SkColorType,     kNone_SkFilterQuality); )
DEF_GM( return new DrawScaledBitmapGM(kRGB_565_SkColorType, kLow_SkFilterQuality); )
DEF_GM( return new DrawScaledBitmapGM(kN32_SkColorType,     kLow_SkFilterQuality); )
DEF_GM( return new DrawScaledBitmapGM(kRGB_565_SkColorType, kHigh_SkFilterQuality); )
DEF_GM( return new DrawScaledBitmapGM(kN32_SkColorType,     kHigh_SkFilterQuality); )

}

