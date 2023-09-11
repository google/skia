/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/private/base/SkTPin.h"

#include <utility>

namespace skiagm {

class ColorCubeGM : public GM {
public:
    ColorCubeGM() {}

protected:
    SkString getName() const override { return SkString("jpg-color-cube"); }

    SkISize getISize() override { return SkISize::Make(512, 512); }

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
        SkDynamicMemoryWStream stream;
        SkASSERT_RELEASE(SkJpegEncoder::Encode(&stream, bmp.pixmap(), {}));
        fImage = SkImages::DeferredFromEncodedData(stream.detachAsData());
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->drawImage(fImage, 0, 0);
    }

private:
    sk_sp<SkImage> fImage;

    using INHERITED = GM;
};

DEF_GM( return new ColorCubeGM; )
}  // namespace skiagm
