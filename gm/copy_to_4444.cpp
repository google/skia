/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "tools/ToolUtils.h"

#include "include/core/SkCanvas.h"
#include "src/core/SkOSFile.h"
#include "tools/Resources.h"

namespace skiagm {

/**
 *  Test copying an image from 8888 to 4444.
 */
class CopyTo4444GM : public GM {
public:
    CopyTo4444GM() {}

protected:
    virtual SkString onShortName() {
        return SkString("copyTo4444");
    }

    virtual SkISize onISize() {
        return SkISize::Make(360, 180);
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) {
        SkBitmap bm, bm4444;
        if (!GetResourceAsBitmap("images/dog.jpg", &bm)) {
            *errorMsg = "Could not decode the file. Did you forget to set the resourcePath?";
            return DrawResult::kFail;
        }
        canvas->drawBitmap(bm, 0, 0);

        // This should dither or we will see artifacts in the background of the image.
        SkAssertResult(ToolUtils::copy_to(&bm4444, kARGB_4444_SkColorType, bm));
        canvas->drawBitmap(bm4444, SkIntToScalar(bm.width()), 0);
        return DrawResult::kOk;
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new CopyTo4444GM; )

}

DEF_SIMPLE_GM(format4444, canvas, 64, 64) {
    canvas->scale(16, 16);
    SkBitmap bitmap;
    SkImageInfo imageInfo = SkImageInfo::Make(1, 1, kARGB_4444_SkColorType, kPremul_SkAlphaType);
    bitmap.allocPixels(imageInfo);
    SkCanvas offscreen(bitmap);
    offscreen.clear(SK_ColorRED);
    canvas->drawBitmap(bitmap, 0, 0);
    offscreen.clear(SK_ColorBLUE);
    canvas->drawBitmap(bitmap, 1, 1);
    auto pack4444 = [](unsigned a, unsigned r, unsigned g, unsigned b) -> uint16_t {
        return (a << 0) | (b << 4) | (g << 8) | (r << 12);
    };
    uint16_t red4444 = pack4444(0xF, 0xF, 0x0, 0x0);
    uint16_t blue4444 = pack4444(0xF, 0x0, 0x0, 0x0F);
    SkPixmap redPixmap(imageInfo, &red4444, 2);
    if (bitmap.writePixels(redPixmap, 0, 0)) {
        canvas->drawBitmap(bitmap, 2, 2);
    }
    SkPixmap bluePixmap(imageInfo, &blue4444, 2);
    if (bitmap.writePixels(bluePixmap, 0, 0)) {
        canvas->drawBitmap(bitmap, 3, 3);
    }
}
