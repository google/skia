/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkDrawLooper.h"
#include "Test.h"

/* Tests for SkDrawLooper -related APIs and implementations. */

namespace {
/*
 *  Subclass that caused an assert at the time of writing.
 */
class GetSaveCountAssertLooper : public SkDrawLooper {
public:

    SkDrawLooper::Context* createContext(SkCanvas*, void* storage) const override {
        return SkNEW_PLACEMENT(storage, GetSaveCountAssertLooperContext);
    }

    size_t contextSize() const override { return sizeof(GetSaveCountAssertLooperContext); }

#ifndef SK_IGNORE_TO_STRING
    void toString(SkString* str) const override {
        str->append("GetSaveCountAssertLooper:");
    }
#endif

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(GetSaveCountAssertLooper);

private:
    class GetSaveCountAssertLooperContext : public SkDrawLooper::Context {
    public:
        GetSaveCountAssertLooperContext() : fOnce(0) {}
        bool next(SkCanvas* canvas, SkPaint* p) override {
            // Getting the save count would assert in SkCanvas at the time of writing.
            canvas->getSaveCount();

            SkASSERT(p->getColor() == SK_ColorRED);
            // Set the color green so the test knows payload was run. We use this color in order to
            // try to express the expectation that Skia can not away the color filter. Due to
            // non-pm-to-pm-to-non-pm conversions, this probably is not exactly correct.
            p->setColor(SkColorSetARGB(255, 0, 254, 0));
            return fOnce++ < 1;
        }
    private:
        int fOnce;
    };
};

SkFlattenable* GetSaveCountAssertLooper::CreateProc(SkReadBuffer&) {
    return SkNEW(GetSaveCountAssertLooper);
}

}

DEF_TEST(SkCanvas_GetSaveCountInDrawLooperAssert, reporter) {
    SkBitmap dst;
    dst.allocN32Pixels(10, 10);
    dst.eraseColor(SK_ColorTRANSPARENT);

    SkCanvas canvas(dst);
    SkPaint  paint;
    {
        SkAutoTUnref<SkColorFilter> addGreenCF(
            SkColorFilter::CreateModeFilter(SkColorSetARGB(64, 0, 4, 0), SkXfermode::kPlus_Mode));
        SkAutoTUnref<SkImageFilter> addGreenIF(
            SkColorFilterImageFilter::Create(addGreenCF));

        // This would trigger the assert upon a draw. It is a color filter that adds (roughly) 1 to
        // the green component.
        paint.setImageFilter(addGreenIF);

        SkAutoTUnref<SkDrawLooper> looper(SkNEW(GetSaveCountAssertLooper));
        paint.setLooper(looper);
    }
    paint.setColor(SK_ColorRED);
    paint.setStrokeWidth(1);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas.drawPoint(0, 0, paint);

    uint32_t pixel = 0;
    SkImageInfo info = SkImageInfo::Make(1, 1, kBGRA_8888_SkColorType, kUnpremul_SkAlphaType);
    canvas.readPixels(info, &pixel, 4, 0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);
}
