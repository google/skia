/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontArguments.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "tools/Resources.h"

#include <string.h>
#include <memory>
#include <utility>

namespace skiagm {

class FontScalerDistortableGM : public GM {
public:
    FontScalerDistortableGM() {
        this->setBGColor(0xFFFFFFFF);
    }

private:

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

        SkFourByteTag tag = SkSetFourByteTag('w','g','h','t');
        constexpr SkScalar tagMin = 0.5f;
        constexpr SkScalar tagMax = 2.0f;
        constexpr int rows = 2;
        constexpr int cols = 5;
        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                SkScalar x = SkIntToScalar(10);
                SkScalar y = SkIntToScalar(20);

                SkScalar styleValue = SkScalarInterp(tagMin, tagMax,
                                                     SkScalar(row * cols + col) / (rows * cols));
                SkFontArguments::VariationPosition::Coordinate coordinates[] = {{tag, styleValue}};
                SkFontArguments::VariationPosition position =
                        { coordinates, SK_ARRAY_COUNT(coordinates) };
                if (row == 0 && distortable) {
                    sk_sp<SkTypeface> clone = distortable->makeClone(
                            SkFontArguments().setVariationDesignPosition(position));
                    font.setTypeface(clone ? std::move(clone) : distortable);
                } else {
                    font.setTypeface(fontMgr->makeFromStream(
                        distortableStream->duplicate(),
                        SkFontArguments().setVariationDesignPosition(position)));
                }

                SkAutoCanvasRestore acr(canvas, true);
                canvas->translate(SkIntToScalar(30 + col * 100), SkIntToScalar(20));
                canvas->rotate(SkIntToScalar(col * 5), x, y * 10);

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
                    canvas->drawSimpleText(text, textLen, SkTextEncoding::kUTF8, x, y, font, paint);
                    y += font.getMetrics(nullptr);
                }
            }
            canvas->translate(0, SkIntToScalar(360));
            font.setSubpixel(true);
            font.setLinearMetrics(true);
        }
        return DrawResult::kOk;
    }
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new FontScalerDistortableGM; )

}
