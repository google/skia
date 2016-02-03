/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkAvoidXfermode.h"

class AvoidXfermode2GM : public skiagm::GM {
public:
    AvoidXfermode2GM() { }

protected:
    SkString onShortName() override {
        return SkString("avoidxfermode2");
    }

    SkISize onISize() override { return SkISize::Make(128, 128); }

    void onDraw(SkCanvas* canvas) override {
        canvas->drawARGB(255, 0, 0, 0);

        SkRect r = SkRect::MakeXYWH(32, 32, 64, 64);

        SkPaint p0;
        p0.setColor(SK_ColorGREEN);
        p0.setAntiAlias(false);

        canvas->drawRect(r, p0);

        r = SkRect::MakeIWH(64, 64);

        // UL corner: replace the green with a tight tolerance
        SkPaint p1;
        p1.setColor(SK_ColorRED);
        p1.setXfermode(SkAvoidXfermode::Create(SK_ColorGREEN,
                                               55,
                                               SkAvoidXfermode::kTargetColor_Mode))->unref();

        canvas->drawRect(r, p1);

        r.offsetTo(64, 0.0f);

        // UR corner: avoid the green with a tight tolerance
        SkPaint p2;
        p2.setColor(SK_ColorRED);
        p2.setXfermode(SkAvoidXfermode::Create(SK_ColorGREEN,
                                               200,
                                               SkAvoidXfermode::kAvoidColor_Mode))->unref();

        canvas->drawRect(r, p2);

        r.offsetTo(0.0f, 64);

        // LL corner: replace the green with a loose tolerance
        SkPaint p3;
        p3.setColor(SK_ColorRED);
        p3.setXfermode(SkAvoidXfermode::Create(SK_ColorGREEN,
                                               200,
                                               SkAvoidXfermode::kTargetColor_Mode))->unref();

        canvas->drawRect(r, p3);

        r.offsetTo(64, 64);

        // LR corner: avoid the green with a loose tolerance
        SkPaint p4;
        p4.setColor(SK_ColorRED);
        p4.setXfermode(SkAvoidXfermode::Create(SK_ColorGREEN,
                                               55,
                                               SkAvoidXfermode::kAvoidColor_Mode))->unref();

        canvas->drawRect(r, p4);
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new AvoidXfermode2GM;)
