/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "Resources.h"

#include "SkImageDecoder.h"
#include "SkAvoidXfermode.h"
#include "SkStream.h"

class AvoidXfermodeGM : public skiagm::GM {
public:
    AvoidXfermodeGM() { }

protected:
    SkString onShortName() override {
        return SkString("avoidxfermode");
    }

    SkISize onISize() override { return SkISize::Make(128, 128); }

    void onOnceBeforeDraw() override {
        SkImageDecoder* codec = nullptr;
        SkString resourcePath = GetResourcePath("color_wheel.png");
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

        SkRect r = SkRect::MakeIWH(64, 64);

        // UL corner: replace white with black with a tight tolerance
        SkPaint p1;
        p1.setColor(SK_ColorBLACK);
        p1.setXfermode(SkAvoidXfermode::Create(SK_ColorWHITE,
                                               5,
                                               SkAvoidXfermode::kTargetColor_Mode))->unref();

        canvas->drawRect(r, p1);

        r.offsetTo(64, 0.0f);

        // UR corner: draw black everywhere except white with a tight tolerance
        SkPaint p2;
        p2.setColor(SK_ColorBLACK);
        p2.setXfermode(SkAvoidXfermode::Create(SK_ColorWHITE,
                                               250,
                                               SkAvoidXfermode::kAvoidColor_Mode))->unref();

        canvas->drawRect(r, p2);

        r.offsetTo(0.0f, 64);

        // LL corner: replace red with transparent blue with a loose tolerance
        SkPaint p3;
        p3.setColor(0x7F0000FF);
        p3.setXfermode(SkAvoidXfermode::Create(SK_ColorRED,
                                               250,
                                               SkAvoidXfermode::kTargetColor_Mode))->unref();

        canvas->drawRect(r, p3);

        r.offsetTo(64, 64);

        // LR corner: draw black everywhere except red with a loose tolerance
        SkPaint p4;
        p4.setColor(SK_ColorBLACK);
        p4.setXfermode(SkAvoidXfermode::Create(SK_ColorRED,
                                               5,
                                               SkAvoidXfermode::kAvoidColor_Mode))->unref();

        canvas->drawRect(r, p4);
    }

private:
    SkBitmap fBM;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new AvoidXfermodeGM;)
