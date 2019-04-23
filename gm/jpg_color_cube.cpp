/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageEncoder.h"

namespace skiagm {

class ColorCubeGM : public GM {
public:
    ColorCubeGM() {}

protected:
    SkString onShortName() override {
        return SkString("jpg-color-cube");
    }

    SkISize onISize() override {
        return SkISize::Make(512, 512);
    }

    void onOnceBeforeDraw() override {
        SkBitmap bmp;
        bmp.allocN32Pixels(512, 512, true);
        int bX = 0, bY = 0;
        for (int b = 0; b < 64; ++b) {
            for (int r = 0; r < 64; ++r) {
                for (int g = 0; g < 64; ++g) {
                    *bmp.getAddr32(bX + r, bY + g) = SkPackARGB32(255,
                                                                  SkTPin(r * 4, 0, 255),
                                                                  SkTPin(g * 4, 0, 255),
                                                                  SkTPin(b * 4, 0, 255));
                }
            }
            bX += 64;
            if (bX >= 512) {
                bX = 0;
                bY += 64;
            }
        }
        auto jpegData = SkEncodeBitmap(bmp, SkEncodedImageFormat::kJPEG, 100);
        fImage = SkImage::MakeFromEncoded(std::move(jpegData));
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
