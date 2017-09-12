/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "sk_tool_utils.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkImageEncoder.h"

namespace skiagm {

class ColorCubeGM : public GM {
public:
    ColorCubeGM() {}

protected:
    SkString onShortName() override {
        return SkString("jpg-color-cube");
    }

    SkISize onISize() override {
        return SkISize::Make(18 * 18, 18);
    }

    void onOnceBeforeDraw() override {
        SkBitmap bmp;
        bmp.allocN32Pixels(18 * 18, 18, true);
        for (int b = 0; b < 18; ++b) {
            for (int r = 0; r < 18; ++r) {
                for (int g = 0; g < 18; ++g) {
                    *bmp.getAddr32(b * 18 + r, g) = SkPackARGB32(255, r * 15, g * 15, b * 15);
                }
            }
        }
        auto jpegData(sk_tool_utils::EncodeImageToData(bmp, SkEncodedImageFormat::kJPEG, 100));
        fImage = SkImage::MakeFromEncoded(jpegData);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->drawImage(fImage, 0, 0);
    }

private:
    sk_sp<SkImage> fImage;

    typedef GM INHERITED;
};

DEF_GM( return new ColorCubeGM; )
}
