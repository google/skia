/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkGradientShader.h"
#include "SkLumaColorFilter.h"
#include "SkTableColorFilter.h"
#include "Resources.h"

namespace {

static constexpr struct {
    SkColor fTint0, fTint1;
} gTints[] = {
    { 0x00000000, 0xffffffff },
    { 0xff400000, 0xffc00000 },
    { 0xff000000, 0xff00ff00 },
    { 0xff7f7f00, 0xffa0a000 },
};

class TintCFGM final : public skiagm::GM {
public:
    TintCFGM(const SkSize& tileSize, size_t tileCount)
        : fTileSize(tileSize)
        , fTileCount(tileCount) {}

protected:
    SkString onShortName() override {
        return SkString("tintCF");
    }

    SkISize onISize() override {
        return SkISize::Make(fTileSize.width()  * 1.2f * fTileCount,
                             fTileSize.height() * 1.2f * SK_ARRAY_COUNT(gTints));
    }

    void onOnceBeforeDraw() override {
        fImage = GetResourceAsImage("images/mandrill_256.png");
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;

        for (size_t i = 0; i < SK_ARRAY_COUNT(gTints); ++i) {
            canvas->translate(0, fTileSize.height() * 0.1f);
            {
                SkAutoCanvasRestore acr(canvas, true);
                for (size_t j = 0; j < fTileCount; ++j) {
                    paint.setColorFilter(SkColorFilter::MakeTint(gTints[i].fTint0,
                                                                 gTints[i].fTint1,
                                                                 j / (fTileCount - 1.0f)));
                    canvas->translate(fTileSize.width() * 0.1f, 0);

                    canvas->drawImageRect(fImage,
                                          SkRect::MakeWH(fTileSize.width(), fTileSize.height()),
                                          &paint);
                    canvas->translate(fTileSize.width() * 1.1f, 0);
                }
            }
            canvas->translate(0, fTileSize.height() * 1.1f);
        }

    }

private:
    const SkSize fTileSize;
    const size_t fTileCount;

    sk_sp<SkImage> fImage;

    using INHERITED = skiagm::GM;
};

} // namespace

DEF_GM( return new TintCFGM(SkSize::Make(200, 200), 5); )
