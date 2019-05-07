/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkCanvas.h"
#include "include/core/SkGraphics.h"
#include "include/utils/SkRandom.h"
#include "samplecode/Sample.h"

#include <pthread.h>

static void call_measure() {
    SkPaint paint;
    uint16_t text[32];
    SkRandom rand;

    paint.setAntiAlias(true);
    paint.setTextEncoding(SkTextEncoding::kUTF16);
    for (int j = 0; j < SK_ARRAY_COUNT(text); j++)
        text[j] = (uint16_t)((rand.nextU() & 0xFF) + 32);

    for (int i = 9; i < 36; i++) {
        SkFontMetrics m;

        paint.setTextSize(SkIntToScalar(i));
        paint.getFontMetrics(&m);
        paint.measureText(text, sizeof(text));
    }
}

static void call_draw(SkCanvas* canvas) {
    SkPaint paint;
    uint16_t text[32];
    SkRandom rand;

    paint.setAntiAlias(true);
    paint.setTextEncoding(SkTextEncoding::kUTF16);
    for (int j = 0; j < SK_ARRAY_COUNT(text); j++)
        text[j] = (uint16_t)((rand.nextU() & 0xFF) + 32);

    SkScalar x = SkIntToScalar(10);
    SkScalar y = SkIntToScalar(20);

    canvas->drawColor(SK_ColorWHITE);
    for (int i = 9; i < 36; i++)
    {
        SkFontMetrics m;

        paint.setTextSize(SkIntToScalar(i));
        paint.getFontMetrics(&m);
        canvas->drawText(text, sizeof(text), x, y, paint);
        y += m.fDescent - m.fAscent;
    }
}

static bool gDone;

static void* measure_proc(void* context) {
    while (!gDone) {
        call_measure();
    }
    return nullptr;
}

static void* draw_proc(void* context) {
    SkBitmap* bm = (SkBitmap*)context;
    SkCanvas    canvas(*bm);

    while (!gDone) {
        call_draw(&canvas);
    }
    return nullptr;
}

class FontCacheView : public Sample {
public:
    enum { N = 4 };

    pthread_t   fMThreads[N];
    pthread_t   fDThreads[N];
    SkBitmap    fBitmaps[N];

    FontCacheView() {
        gDone = false;
        for (int i = 0; i < N; i++) {
            int status;

            status = pthread_create(&fMThreads[i], nullptr,  measure_proc, nullptr);
            SkASSERT(0 == status);

            fBitmaps[i].allocPixels(SkImageInfo::Make(320, 240,
                                                      kRGB_565_SkColorType,
                                                      kOpaque_SkAlphaType));
            status = pthread_create(&fDThreads[i], nullptr,  draw_proc, &fBitmaps[i]);
            SkASSERT(0 == status);
        }
        this->setBGColor(0xFFDDDDDD);
    }

    virtual ~FontCacheView() {
        gDone = true;
        for (int i = 0; i < N; i++) {
            void* ret;
            int status = pthread_join(fMThreads[i], &ret);
            SkASSERT(0 == status);
            status = pthread_join(fDThreads[i], &ret);
            SkASSERT(0 == status);
        }
    }

protected:
    virtual bool onQuery(Sample::Event* evt) {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "FontCache");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        SkScalar x = 0;
        SkScalar y = 0;
        for (int i = 0; i < N; i++) {
            canvas->drawBitmap(fBitmaps[i], x, y);
            x += SkIntToScalar(fBitmaps[i].width());
        }
        this->inval(nullptr);
    }

private:
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static Sample* MyFactory() { return new FontCacheView; }
static SampleRegister reg(MyFactory);
