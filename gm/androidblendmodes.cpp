/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/utils/SkTextUtils.h"
#include "tools/ToolUtils.h"

#include <initializer_list>

namespace skiagm {

// This GM recreates the blend mode images from the Android documentation
class AndroidBlendModesGM : public GM {
public:
    AndroidBlendModesGM() {
        this->setBGColor(SK_ColorBLACK);
    }

protected:
    SkString onShortName() override {
        return SkString("androidblendmodes");
    }

    SkISize onISize() override {
        return SkISize::Make(kNumCols * kBitmapSize, kNumRows * kBitmapSize);
    }

    void onOnceBeforeDraw() override {
        SkImageInfo ii = SkImageInfo::MakeN32Premul(kBitmapSize, kBitmapSize);
        {
            fCompositeSrc.allocPixels(ii);
            SkCanvas tmp(fCompositeSrc);
            tmp.clear(SK_ColorTRANSPARENT);
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(ToolUtils::color_to_565(kBlue));
            tmp.drawRect(SkRect::MakeLTRB(16, 96, 160, 240), p);
        }

        {
            fCompositeDst.allocPixels(ii);
            SkCanvas tmp(fCompositeDst);
            tmp.clear(SK_ColorTRANSPARENT);
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(ToolUtils::color_to_565(kRed));
            tmp.drawCircle(160, 95, 80, p);
        }
    }

    void drawTile(SkCanvas* canvas, int xOffset, int yOffset, SkBlendMode mode) {
        canvas->translate(xOffset, yOffset);

        canvas->clipRect(SkRect::MakeXYWH(0, 0, 256, 256));

        canvas->saveLayer(nullptr, nullptr);

        SkPaint p;
        canvas->drawBitmap(fCompositeDst, 0, 0, &p);
        p.setBlendMode(mode);
        canvas->drawBitmap(fCompositeSrc, 0, 0, &p);
    }

    void onDraw(SkCanvas* canvas) override {
        SkFont font(ToolUtils::create_portable_typeface());

        ToolUtils::draw_checkerboard(canvas, kWhite, kGrey, 32);

        int xOffset = 0, yOffset = 0;

        // Android doesn't expose all the blend modes
        // Note that the Android documentation calls:
        //    Skia's kPlus,     add
        //    Skia's kModulate, multiply
        for (SkBlendMode mode : { SkBlendMode::kPlus /* add */, SkBlendMode::kClear,
                                  SkBlendMode::kDarken, SkBlendMode::kDst,
                                  SkBlendMode::kDstATop, SkBlendMode::kDstIn,
                                  SkBlendMode::kDstOut, SkBlendMode::kDstOver,
                                  SkBlendMode::kLighten, SkBlendMode::kModulate /* multiply */,
                                  SkBlendMode::kOverlay, SkBlendMode::kScreen,
                                  SkBlendMode::kSrc, SkBlendMode::kSrcATop,
                                  SkBlendMode::kSrcIn, SkBlendMode::kSrcOut,
                                  SkBlendMode::kSrcOver, SkBlendMode::kXor } ) {

            int saveCount = canvas->save();
            this->drawTile(canvas, xOffset, yOffset, mode);
            canvas->restoreToCount(saveCount);

            SkTextUtils::DrawString(canvas, SkBlendMode_Name(mode),
                               xOffset + kBitmapSize/2.0f,
                               yOffset + kBitmapSize,
                               font, SkPaint(), SkTextUtils::kCenter_Align);

            xOffset += 256;
            if (xOffset >= 1024) {
                xOffset = 0;
                yOffset += 256;
            }
        }
    }

private:
    static const int kBitmapSize = 256;
    static const int kNumRows = 5;
    static const int kNumCols = 4;

    static const SkColor  kBlue  = SkColorSetARGB(255, 22, 150, 243);
    static const SkColor  kRed   = SkColorSetARGB(255, 233, 30, 99);
    static const SkColor  kWhite = SkColorSetARGB(255, 243, 243, 243);
    static const SkColor  kGrey  = SkColorSetARGB(255, 222, 222, 222);

    SkBitmap fCompositeSrc;
    SkBitmap fCompositeDst;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new AndroidBlendModesGM;)
}
