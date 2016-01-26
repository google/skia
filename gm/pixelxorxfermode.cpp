/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "Resources.h"

#include "SkImageDecoder.h"
#include "SkPixelXorXfermode.h"
#include "SkStream.h"

class PixelXorXfermodeGM : public skiagm::GM {
public:
    PixelXorXfermodeGM() { }

protected:
    SkString onShortName() override {
        return SkString("pixelxorxfermode");
    }

    SkISize onISize() override { return SkISize::Make(512, 512); }

    void onOnceBeforeDraw() override {
        SkImageDecoder* codec = nullptr;
        SkString resourcePath = GetResourcePath("mandrill_512.png");
        SkFILEStream stream(resourcePath.c_str());
        if (stream.isValid()) {
            codec = SkImageDecoder::Factory(&stream);
        }
        if (codec) {
            stream.rewind();
            codec->decode(&stream, &fBM, kN32_SkColorType, SkImageDecoder::kDecodePixels_Mode);
            delete codec;
        } else {
            fBM.allocN32Pixels(1, 1);
            fBM.eraseARGB(255, 255, 0 , 0); // red == bad
        }
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->drawBitmap(fBM, 0, 0);

        SkRect r = SkRect::MakeIWH(256, 256);

        // Negate the red channel of the dst (via the ancillary color) but leave
        // the green & blue channels alone
        SkPaint p1;
        p1.setColor(SK_ColorBLACK); // noop
        p1.setXfermode(SkPixelXorXfermode::Create(0x7FFF0000))->unref();

        canvas->drawRect(r, p1);

        r.offsetTo(256.0f, 0.0f);

        // Negate the dst color via the src color
        SkPaint p2;
        p2.setColor(SK_ColorWHITE);
        p2.setXfermode(SkPixelXorXfermode::Create(SK_ColorBLACK))->unref(); // noop

        canvas->drawRect(r, p2);

        r.offsetTo(0.0f, 256.0f);

        // Just return the original color
        SkPaint p3;
        p3.setColor(SK_ColorBLACK); // noop
        p3.setXfermode(SkPixelXorXfermode::Create(SK_ColorBLACK))->unref(); // noop

        canvas->drawRect(r, p3);

        r.offsetTo(256.0f, 256.0f);

        // Negate the red & green channels (via the ancillary color) but leave
        // the blue channel alone
        SkPaint p4;
        p4.setColor(SK_ColorBLACK); // noop
        p4.setXfermode(SkPixelXorXfermode::Create(SK_ColorYELLOW))->unref();

        canvas->drawRect(r, p4);
    }

private:
    SkBitmap fBM;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new PixelXorXfermodeGM;)
