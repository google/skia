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
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "tools/Resources.h"

#include <memory>

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

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        // Create image.
        const char* path = "images/mandrill_512_q075.jpg";
        sk_sp<SkImage> image = GetResourceAsImage(path);
        if (!image) {
            *errorMsg = "Couldn't load images/mandrill_512_q075.jpg. "
                        "Did you forget to set the resource path?";
            return DrawResult::kFail;
        }

        // Create matching bitmap.
        std::unique_ptr<SkCodec> codec(SkCodec::MakeFromStream(GetResourceAsStream(path)));
        auto [codecImage, _] = codec->getImage();

        // The GM will be displayed in a 2x2 grid.
        // The top two squares show an sRGB image, then bitmap, drawn to a legacy canvas.
        SkImageInfo linearInfo = SkImageInfo::MakeN32(2*kSize, kSize, kOpaque_SkAlphaType);
        SkBitmap legacyBMCanvas;
        legacyBMCanvas.allocPixels(linearInfo);
        SkCanvas legacyCanvas(legacyBMCanvas);
        legacyCanvas.drawImage(image, 0.0f, 0.0f);
        legacyCanvas.translate(SkScalar(kSize), 0.0f);
        legacyCanvas.drawImage(codecImage, 0.0f, 0.0f);
        canvas->drawImage(legacyBMCanvas.asImage(), 0.0f, 0.0f);
        canvas->translate(0.0f, SkScalar(kSize));

        // The bottom two squares show an sRGB image, then bitmap, drawn to a srgb canvas.
        SkImageInfo srgbInfo = SkImageInfo::MakeS32(2*kSize, kSize, kOpaque_SkAlphaType);
        SkBitmap srgbBMCanvas;
        srgbBMCanvas.allocPixels(srgbInfo);
        SkCanvas srgbCanvas(srgbBMCanvas);
        srgbCanvas.drawImage(image, 0.0f, 0.0f);
        srgbCanvas.translate(SkScalar(kSize), 0.0f);
        srgbCanvas.drawImage(codecImage, 0.0f, 0.0f);
        canvas->drawImage(srgbBMCanvas.asImage(), 0.0f, 0.0f);
        return DrawResult::kOk;
    }

private:
    inline static constexpr int kSize = 512;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new BitmapImageGM; )

}  // namespace skiagm
