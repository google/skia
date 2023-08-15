/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "tools/ToolUtils.h"

// Create a square atlas of:
//   opaque white  |     opaque red
//  ------------------------------------
//   opaque green  |  transparent black
//
static sk_sp<SkImage> make_atlas(SkCanvas* caller, int atlasSize) {
    const int kBlockSize = atlasSize/2;

    SkImageInfo info = SkImageInfo::MakeN32Premul(atlasSize, atlasSize);
    auto        surface(ToolUtils::makeSurface(caller, info));
    SkCanvas* canvas = surface->getCanvas();

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);

    paint.setColor(SK_ColorWHITE);
    SkRect r = SkRect::MakeXYWH(0, 0,
                                SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));
    canvas->drawRect(r, paint);

    paint.setColor(SK_ColorRED);
    r = SkRect::MakeXYWH(SkIntToScalar(kBlockSize), 0,
                         SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));
    canvas->drawRect(r, paint);

    paint.setColor(SK_ColorGREEN);
    r = SkRect::MakeXYWH(0, SkIntToScalar(kBlockSize),
                         SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));
    canvas->drawRect(r, paint);

    paint.setColor(SK_ColorTRANSPARENT);
    r = SkRect::MakeXYWH(SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize),
                         SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));
    canvas->drawRect(r, paint);

    return surface->makeImageSnapshot();
}

// This GM tests the drawAtlas API with colors, different xfer modes
// and transparency in the atlas image
class DrawAtlasColorsGM : public skiagm::GM {
public:
    DrawAtlasColorsGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    SkString getName() const override { return SkString("draw-atlas-colors"); }

    SkISize getISize() override {
        return SkISize::Make(kNumXferModes * (kAtlasSize + kPad) + kPad,
                             2 * kNumColors * (kAtlasSize + kPad) + kTextPad + kPad);
    }

    void onDraw(SkCanvas* canvas) override {
        const SkRect target = SkRect::MakeWH(SkIntToScalar(kAtlasSize), SkIntToScalar(kAtlasSize));

        auto atlas = make_atlas(canvas, kAtlasSize);

        const SkBlendMode gModes[] = {
            SkBlendMode::kClear,
            SkBlendMode::kSrc,
            SkBlendMode::kDst,
            SkBlendMode::kSrcOver,
            SkBlendMode::kDstOver,
            SkBlendMode::kSrcIn,
            SkBlendMode::kDstIn,
            SkBlendMode::kSrcOut,
            SkBlendMode::kDstOut,
            SkBlendMode::kSrcATop,
            SkBlendMode::kDstATop,
            SkBlendMode::kXor,
            SkBlendMode::kPlus,
            SkBlendMode::kModulate,
            SkBlendMode::kScreen,
            SkBlendMode::kOverlay,
            SkBlendMode::kDarken,
            SkBlendMode::kLighten,
            SkBlendMode::kColorDodge,
            SkBlendMode::kColorBurn,
            SkBlendMode::kHardLight,
            SkBlendMode::kSoftLight,
            SkBlendMode::kDifference,
            SkBlendMode::kExclusion,
            SkBlendMode::kMultiply,
            SkBlendMode::kHue,
            SkBlendMode::kSaturation,
            SkBlendMode::kColor,
            SkBlendMode::kLuminosity,
        };

        SkColor gColors[] = {
            SK_ColorWHITE,
            SK_ColorRED,
            0x88888888,         // transparent grey
            0x88000088          // transparent blue
        };

        const int numModes = std::size(gModes);
        SkASSERT(numModes == kNumXferModes);
        const int numColors = std::size(gColors);
        SkASSERT(numColors == kNumColors);
        SkRSXform xforms[numColors];
        SkRect rects[numColors];
        SkColor quadColors[numColors];

        SkPaint paint;
        paint.setAntiAlias(true);

        for (int i = 0; i < numColors; ++i) {
            xforms[i].set(1.0f, 0.0f, SkIntToScalar(kPad), i*(target.width()+kPad));
            rects[i] = target;
            quadColors[i] = gColors[i];
        }

        SkFont font(ToolUtils::create_portable_typeface(), kTextPad);

        for (int i = 0; i < numModes; ++i) {
            const char* label = SkBlendMode_Name(gModes[i]);
            canvas->drawString(label, i*(target.width()+kPad)+kPad, SkIntToScalar(kTextPad),
                               font, paint);
        }

        for (int i = 0; i < numModes; ++i) {
            canvas->save();
            canvas->translate(SkIntToScalar(i*(target.height()+kPad)),
                              SkIntToScalar(kTextPad+kPad));
            // w/o a paint
            canvas->drawAtlas(atlas.get(), xforms, rects, quadColors, numColors,
                              gModes[i], SkSamplingOptions(), nullptr, nullptr);
            canvas->translate(0.0f, numColors*(target.height()+kPad));
            // w a paint
            canvas->drawAtlas(atlas.get(), xforms, rects, quadColors, numColors,
                              gModes[i], SkSamplingOptions(), nullptr, &paint);
            canvas->restore();
        }
    }

private:
    inline static constexpr int kNumXferModes = 29;
    inline static constexpr int kNumColors = 4;
    inline static constexpr int kAtlasSize = 30;
    inline static constexpr int kPad = 2;
    inline static constexpr int kTextPad = 8;

    using INHERITED = GM;
};
DEF_GM( return new DrawAtlasColorsGM; )
