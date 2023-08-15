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
#include "tools/SkMetaData.h"
#include "tools/ToolUtils.h"

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
    SkString getName() const override { return SkString("fontscalerdistortable"); }

    SkISize getISize() override { return SkISize::Make(550, 700); }

    bool fDirty = true;
    bool fOverride = false;

    ToolUtils::VariationSliders fVariationSliders;

    bool onGetControls(SkMetaData* controls) override {
        controls->setBool("Override", fOverride);
        return fVariationSliders.writeControls(controls);
    }

    void onSetControls(const SkMetaData& controls) override {
        bool oldOverride = fOverride;
        controls.findBool("Override", &fOverride);
        if (fOverride != oldOverride) {
            fDirty = true;
        }

        return fVariationSliders.readControls(controls, &fDirty);
    }

    struct Info {
        sk_sp<SkTypeface> distortable;
        SkFourByteTag axisTag;
        SkScalar axisMin;
        SkScalar axisMax;
    } fInfo;

    void onOnceBeforeDraw() override {
        constexpr SkFourByteTag wght = SkSetFourByteTag('w','g','h','t');
        //constexpr SkFourByteTag wdth = SkSetFourByteTag('w','d','t','h');
        fInfo = {
            MakeResourceAsTypeface("fonts/Distortable.ttf"), wght, 0.5f, 2.0f
            //SkTypeface::MakeFromFile("/Library/Fonts/Skia.ttf"), wght, 0.48f, 3.2f
            //SkTypeface::MakeFromName("Skia", SkFontStyle()), wdth, 0.62f, 1.3f
            //SkTypeface::MakeFromFile("/System/Library/Fonts/SFNS.ttf"), wght, 100.0f, 900.0f
            //SkTypeface::MakeFromName(".SF NS", SkFontStyle()), wght, 100.0f, 900.0f
        };

        if (fInfo.distortable) {
            fVariationSliders = ToolUtils::VariationSliders(fInfo.distortable.get());
        }
    }

    inline static constexpr int rows = 2;
    inline static constexpr int cols = 5;
    sk_sp<SkTypeface> typeface[rows][cols];

    void updateTypefaces() {
        sk_sp<SkFontMgr> fontMgr(SkFontMgr::RefDefault());

        std::unique_ptr<SkStreamAsset> distortableStream( fInfo.distortable
                                                        ? fInfo.distortable->openStream(nullptr)
                                                        : nullptr);
        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                using Coordinate = SkFontArguments::VariationPosition::Coordinate;
                SkFontArguments::VariationPosition position;
                Coordinate coordinates[2];

                if (fOverride) {
                    SkSpan<const Coordinate> user_coordinates = fVariationSliders.getCoordinates();
                    position = {user_coordinates.data(), static_cast<int>(user_coordinates.size())};

                } else {
                    const int coordinateCount = 2;
                    SkScalar styleValue = SkScalarInterp(fInfo.axisMin, fInfo.axisMax,
                                                         SkScalar(row*cols + col) / (rows*cols));
                    coordinates[0] = {fInfo.axisTag, styleValue};
                    coordinates[1] = {fInfo.axisTag, styleValue};
                    position = {coordinates, static_cast<int>(coordinateCount)};
                }

                typeface[row][col] = [&]() -> sk_sp<SkTypeface> {
                    if (row == 0 && fInfo.distortable) {
                        return fInfo.distortable->makeClone(
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
        fDirty = false;
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        if (fDirty) {
            this->updateTypefaces();
        }

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

}  // namespace skiagm
