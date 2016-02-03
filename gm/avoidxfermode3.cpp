/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "gm.h"

#include "SkAvoidXfermode.h"

// This GM exercises how the avoid xfer mode interacts with partial coverage
class AvoidXfermode3GM : public skiagm::GM {
public:
    AvoidXfermode3GM() { }

protected:
    SkString onShortName() override {
        return SkString("avoidxfermode3");
    }

    SkISize onISize() override { return SkISize::Make(128, 128); }

    void onDraw(SkCanvas* canvas) override {
        canvas->drawARGB(255, 0, 0, 0);

        SkRect r = SkRect::MakeXYWH(16.5f, 16.5f, 96.0f, 96.0f);

        SkPaint p0;
        p0.setColor(SK_ColorWHITE);
        p0.setAntiAlias(true);

        canvas->drawRect(r, p0);

        r = SkRect::MakeXYWH(3.5f, 3.5f, 59.0f, 59.0f);

        // UL corner: replace white with green with a tight tolerance
        SkPaint p1;
        p1.setColor(SK_ColorGREEN);
        p1.setAntiAlias(true);
        p1.setXfermode(SkAvoidXfermode::Create(SK_ColorWHITE,
                                               5,
                                               SkAvoidXfermode::kTargetColor_Mode))->unref();

        canvas->drawRect(r, p1);

        r.offsetTo(65.5f, 3.5f);

        // UR corner: draw red everywhere except white with a tight tolerance
        SkPaint p2;
        p2.setColor(SK_ColorRED);
        p2.setAntiAlias(true);
        p2.setXfermode(SkAvoidXfermode::Create(SK_ColorWHITE,
                                               250,
                                               SkAvoidXfermode::kAvoidColor_Mode))->unref();

        canvas->drawRect(r, p2);

        r.offsetTo(3.5f, 65.5f);

        // LL corner: replace white with blue with a loose tolerance
        SkPaint p3;
        p3.setColor(SK_ColorBLUE);
        p3.setAntiAlias(true);
        p3.setXfermode(SkAvoidXfermode::Create(SK_ColorWHITE,
                                               250,
                                               SkAvoidXfermode::kTargetColor_Mode))->unref();

        canvas->drawRect(r, p3);

        r.offsetTo(65.5f, 65.5f);

        // LR corner: draw yellow everywhere except white with a loose tolerance
        SkPaint p4;
        p4.setColor(SK_ColorYELLOW);
        p4.setAntiAlias(true);
        p4.setXfermode(SkAvoidXfermode::Create(SK_ColorWHITE,
                                               5,
                                               SkAvoidXfermode::kAvoidColor_Mode))->unref();

        canvas->drawRect(r, p4);
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new AvoidXfermode3GM;)
