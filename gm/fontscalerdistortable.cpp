/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm/gm.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkFixed.h"
#include "src/core/SkFontDescriptor.h"
#include "tools/Resources.h"

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

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        SkPaint paint;
        paint.setAntiAlias(true);
        SkFont font;
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        sk_sp<SkFontMgr> fontMgr(SkFontMgr::RefDefault());

        std::unique_ptr<SkStreamAsset> distortableStream(GetResourceAsStream("fonts/Distortable.ttf"));
        sk_sp<SkTypeface> distortable(MakeResourceAsTypeface("fonts/Distortable.ttf"));

        if (!distortableStream) {
            *errorMsg = "No distortableStream";
            return DrawResult::kFail;
        }
        const char* text = "abc";
        const size_t textLen = strlen(text);

        for (int j = 0; j < 2; ++j) {
            for (int i = 0; i < 5; ++i) {
                SkScalar x = SkIntToScalar(10);
                SkScalar y = SkIntToScalar(20);

                SkFourByteTag tag = SkSetFourByteTag('w','g','h','t');
                SkScalar styleValue = SkDoubleToScalar(0.5 + (5 * j + i) * ((2.0 - 0.5) / (2 * 5)));
                SkFontArguments::VariationPosition::Coordinate coordinates[] = {{tag, styleValue}};
                SkFontArguments::VariationPosition position =
                        { coordinates, SK_ARRAY_COUNT(coordinates) };
                if (j == 0 && distortable) {
                    sk_sp<SkTypeface> clone = distortable->makeClone(
                            SkFontArguments().setVariationDesignPosition(position));
                    font.setTypeface(clone ? std::move(clone) : distortable);
                } else {
                    font.setTypeface(fontMgr->makeFromStream(
                        distortableStream->duplicate(),
                        SkFontArguments().setVariationDesignPosition(position)));
                }

                SkAutoCanvasRestore acr(canvas, true);
                canvas->translate(SkIntToScalar(30 + i * 100), SkIntToScalar(20));
                canvas->rotate(SkIntToScalar(i * 5), x, y * 10);

                {
                    SkPaint p;
                    p.setAntiAlias(true);
                    SkRect r;
                    r.set(x - SkIntToScalar(3), SkIntToScalar(15),
                          x - SkIntToScalar(1), SkIntToScalar(280));
                    canvas->drawRect(r, p);
                }

                for (int ps = 6; ps <= 22; ps++) {
                    font.setSize(SkIntToScalar(ps));
                    canvas->drawSimpleText(text, textLen, kUTF8_SkTextEncoding, x, y, font, paint);
                    y += font.getMetrics(nullptr);
                }
            }
            canvas->translate(0, SkIntToScalar(360));
            font.setSubpixel(true);
            font.setLinearMetrics(true);
        }
        return DrawResult::kOk;
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new FontScalerDistortableGM; )

}
