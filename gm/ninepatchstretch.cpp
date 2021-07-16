/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "tools/ToolUtils.h"

static sk_sp<SkSurface> make_surface(SkCanvas* root, int N) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(N, N);
    return ToolUtils::makeSurface(root, info);
}

static sk_sp<SkImage> make_image(SkCanvas* root, SkIRect* center) {
    const int kFixed = 28;
    const int kStretchy = 8;
    const int kSize = 2*kFixed + kStretchy;

    auto surface(make_surface(root, kSize));
    SkCanvas* canvas = surface->getCanvas();

    SkRect r = SkRect::MakeWH(SkIntToScalar(kSize), SkIntToScalar(kSize));
    const SkScalar strokeWidth = SkIntToScalar(6);
    const SkScalar radius = SkIntToScalar(kFixed) - strokeWidth/2;

    center->setXYWH(kFixed, kFixed, kStretchy, kStretchy);

    SkPaint paint;
    paint.setAntiAlias(true);

    paint.setColor(0xFFFF0000);
    canvas->drawRoundRect(r, radius, radius, paint);
    r.setXYWH(SkIntToScalar(kFixed), 0, SkIntToScalar(kStretchy), SkIntToScalar(kSize));
    paint.setColor(0x8800FF00);
    canvas->drawRect(r, paint);
    r.setXYWH(0, SkIntToScalar(kFixed), SkIntToScalar(kSize), SkIntToScalar(kStretchy));
    paint.setColor(0x880000FF);
    canvas->drawRect(r, paint);

    return surface->makeImageSnapshot();
}

class NinePatchStretchGM : public skiagm::GM {
public:
    sk_sp<SkImage>  fImage;
    SkIRect         fCenter;

    NinePatchStretchGM() {}

protected:
    SkString onShortName() override {
        return SkString("ninepatch-stretch");
    }

    SkISize onISize() override {
        return SkISize::Make(760, 800);
    }

    void onDraw(SkCanvas* canvas) override {
        if (!fImage || !fImage->isValid(canvas->recordingContext())) {
            fImage = make_image(canvas, &fCenter);
        }

        // amount of bm that should not be stretched (unless we have to)
        const SkScalar fixed = SkIntToScalar(fImage->width() - fCenter.width());

        const SkSize size[] = {
            { fixed * 4 / 5, fixed * 4 / 5 },   // shrink in both axes
            { fixed * 4 / 5, fixed * 4 },       // shrink in X
            { fixed * 4,     fixed * 4 / 5 },   // shrink in Y
            { fixed * 4,     fixed * 4 }
        };

        canvas->drawImage(fImage, 10, 10);

        SkScalar x = SkIntToScalar(100);
        SkScalar y = SkIntToScalar(100);

        SkPaint paint;
        for (auto fm : {SkFilterMode::kLinear, SkFilterMode::kNearest}) {
            for (int iy = 0; iy < 2; ++iy) {
                for (int ix = 0; ix < 2; ++ix) {
                    int i = ix * 2 + iy;
                    SkRect r = SkRect::MakeXYWH(x + ix * fixed, y + iy * fixed,
                                                size[i].width(), size[i].height());
                    canvas->drawImageNine(fImage.get(), fCenter, r, fm);
                }
            }
            canvas->translate(0, 400);
        }
    }

private:
    using INHERITED = skiagm::GM;
};
DEF_GM( return new NinePatchStretchGM; )
