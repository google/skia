/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "Resources.h"
#include "SkFixed.h"
#include "SkFontDescriptor.h"
#include "SkFontMgr.h"
#include "SkTypeface.h"

namespace skiagm {

class FontScalerDistortableGM : public GM {
public:
    FontScalerDistortableGM() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:

    SkString onShortName() override {
        return SkString("fontscalerdistortable");
    }

    SkISize onISize() override {
        return SkISize::Make(550, 700);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setLCDRenderText(true);
        paint.setTextEncoding(SkPaint::kUTF32_TextEncoding);
        paint.setSubpixelText(true);
        paint.setHinting(SkPaint::kNo_Hinting);

        sk_sp<SkFontMgr> fontMgr(SkFontMgr::RefDefault());
        std::unique_ptr<SkStreamAsset> distortable(
                GetResourceAsStream("/fonts/DistortableAdvance.ttf"));
        if (!distortable) {
            return;
        }
        paint.setTextSize(100);
        uint32_t character = 0x0061; // A
        SkFourByteTag tag = SkSetFourByteTag('w','g','h','t');


        SkScalar thinStyleValue = 0.5;
        SkFontMgr::FontParameters::Axis thinAxes[] = { { tag, thinStyleValue } };
        paint.setTypeface(sk_sp<SkTypeface>(fontMgr->createFromStream(
            distortable->duplicate(), SkFontMgr::FontParameters().setAxes(thinAxes, 1))));

        SkScalar thinAdvance;
        paint.getTextWidths(&character, sizeof(character), &thinAdvance, nullptr);
        paint.setColor(SK_ColorBLACK);
        canvas->drawText(&character, 1, 10, 150, paint);
        paint.setColor(SK_ColorGREEN);
        canvas->drawRect(SkRect::MakeXYWH(10, 150, thinAdvance, 4), paint);


        SkScalar heavyStyleValue = 2.0;
        SkFontMgr::FontParameters::Axis heavyAxes[] = { { tag, heavyStyleValue } };
        paint.setTypeface(sk_sp<SkTypeface>(fontMgr->createFromStream(
            distortable->duplicate(), SkFontMgr::FontParameters().setAxes(heavyAxes, 1))));

        SkScalar heavyAdvance;
        paint.getTextWidths(&character, sizeof(character), &heavyAdvance, nullptr);
        paint.setColor(SK_ColorBLACK);
        canvas->drawText(&character, 1, 10, 400, paint);
        paint.setColor(SK_ColorGREEN);
        canvas->drawRect(SkRect::MakeXYWH(10, 400, heavyAdvance, 4), paint);


        paint.setColor(SK_ColorRED);
        canvas->drawRect(SkRect::MakeXYWH(10 + thinAdvance, 145, 2, 263), paint);
        canvas->drawRect(SkRect::MakeXYWH(10 + heavyAdvance, 145, 2, 263), paint);
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new FontScalerDistortableGM; }
static GMRegistry reg(MyFactory);

}
