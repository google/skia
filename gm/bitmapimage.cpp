/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
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
        std::unique_ptr<SkCodec> codec(SkCodec::NewFromStream(GetResourceAsStream(path)));
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

}
