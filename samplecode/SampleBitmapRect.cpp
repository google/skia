
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkXfermode.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"

#include "SkOSFile.h"
#include "SkStream.h"

#if SK_SUPPORT_GPU
#include "SkGpuDevice.h"
#else
class GrContext;
#endif


static void make_bitmap(SkBitmap* bitmap, GrContext* ctx) {
    SkCanvas canvas;

#if SK_SUPPORT_GPU
    if (ctx) {
        SkDevice* dev = new SkGpuDevice(ctx, SkBitmap::kARGB_8888_Config, 64, 64);
        canvas.setDevice(dev)->unref();
        *bitmap = dev->accessBitmap(false);
    } else
#endif
    {
        bitmap->setConfig(SkBitmap::kARGB_8888_Config, 64, 64);
        bitmap->allocPixels();
        canvas.setBitmapDevice(*bitmap);
    }

    canvas.drawColor(SK_ColorRED);
    SkPaint paint;
    paint.setAntiAlias(true);
    const SkPoint pts[] = { { 0, 0 }, { 64, 64 } };
    const SkColor colors[] = { SK_ColorWHITE, SK_ColorBLUE };
    paint.setShader(SkGradientShader::CreateLinear(pts, colors, NULL, 2,
                                                   SkShader::kClamp_TileMode))->unref();
    canvas.drawCircle(32, 32, 32, paint);
}

class BitmapRectView : public SampleView {
public:
    BitmapRectView() {
        this->setBGColor(SK_ColorGRAY);
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "BitmapRect");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        GrContext* ctx = SampleCode::GetGr();

        const SkIRect src[] = {
            { 0, 0, 32, 32 },
            { 0, 0, 80, 80 },
            { 32, 32, 96, 96 },
            { -32, -32, 32, 32, }
        };

        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(ctx ? SK_ColorGREEN : SK_ColorYELLOW);

        SkBitmap bitmap;
        make_bitmap(&bitmap, ctx);

        SkRect dstR = { 0, 200, 128, 380 };

        canvas->translate(16, 40);
        for (size_t i = 0; i < SK_ARRAY_COUNT(src); i++) {
            SkRect srcR;
            srcR.set(src[i]);

            canvas->drawBitmap(bitmap, 0, 0, &paint);
            canvas->drawBitmapRect(bitmap, &src[i], dstR, &paint);

            canvas->drawRect(dstR, paint);
            canvas->drawRect(srcR, paint);

            canvas->translate(160, 0);
        }
    }

private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new BitmapRectView; }
static SkViewRegister reg(MyFactory);

