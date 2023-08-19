/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "tools/ToolUtils.h"

#define WIDTH 700
#define HEIGHT 560

namespace skiagm {

class MorphologyGM : public GM {
public:
    MorphologyGM() {
        this->setBGColor(0xFF000000);
    }

protected:
    SkString getName() const override { return SkString("morphology"); }

    void onOnceBeforeDraw() override {
        auto surf = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(135, 135));

        SkFont  font(ToolUtils::create_portable_typeface(), 64.0f);
        SkPaint paint;
        paint.setColor(0xFFFFFFFF);
        surf->getCanvas()->drawString("ABC", 10, 55,  font, paint);
        surf->getCanvas()->drawString("XYZ", 10, 110, font, paint);

        fImage = surf->makeImageSnapshot();
    }

    SkISize getISize() override { return SkISize::Make(WIDTH, HEIGHT); }

    void drawClippedBitmap(SkCanvas* canvas, const SkPaint& paint, int x, int y) {
        canvas->save();
        canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
        canvas->clipIRect(fImage->bounds());
        canvas->drawImage(fImage, 0, 0, SkSamplingOptions(), &paint);
        canvas->restore();
    }

    void onDraw(SkCanvas* canvas) override {
        struct {
            int fWidth, fHeight;
            int fRadiusX, fRadiusY;
        } samples[] = {
            { 140, 140,   0,   0 },
            { 140, 140,   0,   2 },
            { 140, 140,   2,   0 },
            { 140, 140,   2,   2 },
            {  24,  24,  25,  25 },
        };
        SkPaint paint;
        SkIRect cropRect = SkIRect::MakeXYWH(25, 20, 100, 80);

        for (unsigned j = 0; j < 4; ++j) {
            for (unsigned i = 0; i < std::size(samples); ++i) {
                const SkIRect* cr = j & 0x02 ? &cropRect : nullptr;
                if (j & 0x01) {
                    paint.setImageFilter(SkImageFilters::Erode(
                            samples[i].fRadiusX, samples[i].fRadiusY, nullptr, cr));
                } else {
                    paint.setImageFilter(SkImageFilters::Dilate(
                            samples[i].fRadiusX, samples[i].fRadiusY, nullptr, cr));
                }
                this->drawClippedBitmap(canvas, paint, i * 140, j * 140);
            }
        }
    }

private:
    sk_sp<SkImage> fImage;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new MorphologyGM;)

}  // namespace skiagm
