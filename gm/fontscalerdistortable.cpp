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

    static constexpr int rows = 2;
    static constexpr int cols = 5;
    sk_sp<SkTypeface> typeface[rows][cols];
    void onOnceBeforeDraw() override {
        sk_sp<SkFontMgr> fontMgr(SkFontMgr::RefDefault());

        constexpr SkFourByteTag wght = SkSetFourByteTag('w','g','h','t');
        //constexpr SkFourByteTag wdth = SkSetFourByteTag('w','d','t','h');
        struct {
            sk_sp<SkTypeface> distortable;
            SkFourByteTag axisTag;
            SkScalar axisMin;
            SkScalar axisMax;
        } info = {
            MakeResourceAsTypeface("fonts/Distortable.ttf"), wght, 0.5f, 2.0f
            //SkTypeface::MakeFromFile("/Library/Fonts/Skia.ttf"), wght, 0.48f, 3.2f
            //SkTypeface::MakeFromName("Skia", SkFontStyle()), wdth, 0.62f, 1.3f
            //SkTypeface::MakeFromFile("/System/Library/Fonts/SFNS.ttf"), wght, 100.0f, 900.0f
            //SkTypeface::MakeFromName(".SF NS", SkFontStyle()), wght, 100.0f, 900.0f
        };
        std::unique_ptr<SkStreamAsset> distortableStream( info.distortable
                                                        ? info.distortable->openStream(nullptr)
                                                        : nullptr);
        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                SkScalar styleValue = SkScalarInterp(info.axisMin, info.axisMax,
                                                     SkScalar(row * cols + col) / (rows * cols));
                SkFontArguments::VariationPosition::Coordinate coordinates[] = {
                    {info.axisTag, styleValue},
                    {info.axisTag, styleValue}
                };
                SkFontArguments::VariationPosition position = {
                    coordinates, SK_ARRAY_COUNT(coordinates)
                };
                typeface[row][col] = [&]() -> sk_sp<SkTypeface> {
                    if (row == 0 && info.distortable) {
                        return info.distortable->makeClone(
                                SkFontArguments().setVariationDesignPosition(position));
                    }
                    if (distortableStream) {
                        return fontMgr->makeFromStream(distortableStream->duplicate(),
                                SkFontArguments().setVariationDesignPosition(position));
                    }
                    return nullptr;
                }();
            }
        }
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        SkPaint paint;
        paint.setAntiAlias(true);
        SkFont font;
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);

        const char* text = "abc";
        const size_t textLen = strlen(text);

        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                SkScalar x = SkIntToScalar(10);
                SkScalar y = SkIntToScalar(20);

                font.setTypeface(typeface[row][col] ? typeface[row][col] : nullptr);

                SkAutoCanvasRestore acr(canvas, true);
                canvas->translate(SkIntToScalar(30 + col * 100), SkIntToScalar(20));
                canvas->rotate(SkIntToScalar(col * 5), x, y * 10);

                {
                    SkPaint p;
                    p.setAntiAlias(true);
                    SkRect r;
                    r.setLTRB(x - 3, 15, x - 1, 280);
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
            font.setBaselineSnap(false);
        }
        return DrawResult::kOk;
    }
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new FontScalerDistortableGM; )

}
