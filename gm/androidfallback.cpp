/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

namespace skiagm {

class AndroidFallbackGM : public GM {
public:
    AndroidFallbackGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    virtual uint32_t onGetFlags() const SK_OVERRIDE {
        // TODO(scroggo): Undo this if we decide to fix skia:1763.
        return GM::kSkipPipe_Flag;
    }

    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("android_paint");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return make_isize(500, 500);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {

        SkPaint paint;
        paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
        paint.setTextSize(24);

#ifdef SK_BUILD_FOR_ANDROID
        SkPaintOptionsAndroid options = paint.getPaintOptionsAndroid();
        options.setUseFontFallbacks(true);
        paint.setPaintOptionsAndroid(options);
#endif

        // "א foo 免舌 bar क"
        const uint16_t unicodeStr[] = {0x05D0, 0x0020, 0x0066, 0x006F, 0x006F, 0x0020, 0x514D,
                                       0x820c, 0x0020, 0x0062, 0x0061, 0x0072, 0x0020, 0x0915};
        const int strLength = sizeof(unicodeStr) / sizeof(uint16_t);
        const int strByteLength = sizeof(unicodeStr);

        SkScalar posX[strLength];
        SkPoint posXY[strLength];

        for (int i = 0; i < strLength; ++i) {
            posX[i] = SkIntToScalar(i * 24);
            posXY[i].fX = posX[i];
            posXY[i].fY = SkIntToScalar(24 + i);
        }

        canvas->translate(SkIntToScalar(10), SkIntToScalar(25));
        // This currently causes the PDF backend to assert
        // canvas->drawText(unicodeStr, strByteLength, 0, 0, paint);

        canvas->translate(0, SkIntToScalar(75));
        canvas->drawPosTextH(unicodeStr, strByteLength, posX, 0, paint);

#ifdef SK_BUILD_FOR_ANDROID
        options.setLanguage("ja");
        paint.setPaintOptionsAndroid(options);
#endif

        canvas->translate(0, SkIntToScalar(75));
        canvas->drawPosText(unicodeStr, strByteLength, posXY, paint);

        SkPath path;
        path.moveTo(0, 0);
        path.quadTo(50.0f, 100.0f, 250.0f, 150.0f);

        canvas->translate(0, SkIntToScalar(75));
        canvas->drawTextOnPath(unicodeStr, strByteLength, path, NULL, paint);
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

#ifdef SK_BUILD_FOR_ANDROID
DEF_GM( return SkNEW(AndroidFallbackGM); )
#endif

}
