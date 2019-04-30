/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkArithmeticImageFilter.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageSource.h"
#include "tools/ToolUtils.h"

#include <utility>

#define WW  100
#define HH  32

static sk_sp<SkImage> make_src() {
    sk_sp<SkSurface> surface(SkSurface::MakeRasterN32Premul(WW, HH));
    SkCanvas* canvas = surface->getCanvas();

    SkPaint paint;
    SkPoint pts[] = { {0, 0}, {SkIntToScalar(WW), SkIntToScalar(HH)} };
    SkColor colors[] = {
        SK_ColorTRANSPARENT, SK_ColorGREEN, SK_ColorCYAN,
        SK_ColorRED, SK_ColorMAGENTA, SK_ColorWHITE,
    };
    paint.setShader(SkGradientShader::MakeLinear(pts, colors, nullptr, SK_ARRAY_COUNT(colors),
                                                 SkTileMode::kClamp));
    canvas->drawPaint(paint);
    return surface->makeImageSnapshot();
}

static sk_sp<SkImage> make_dst() {
    sk_sp<SkSurface> surface(SkSurface::MakeRasterN32Premul(WW, HH));
    SkCanvas* canvas = surface->getCanvas();

    SkPaint paint;
    SkPoint pts[] = { {0, SkIntToScalar(HH)}, {SkIntToScalar(WW), 0} };
    SkColor colors[] = {
        SK_ColorBLUE, SK_ColorYELLOW, SK_ColorBLACK, SK_ColorGREEN,
        SK_ColorGRAY,
    };
    paint.setShader(SkGradientShader::MakeLinear(pts, colors, nullptr, SK_ARRAY_COUNT(colors),
                                                 SkTileMode::kClamp));
    canvas->drawPaint(paint);
    return surface->makeImageSnapshot();
}

static void show_k_text(SkCanvas* canvas, SkScalar x, SkScalar y, const SkScalar k[]) {
    SkFont font(ToolUtils::create_portable_typeface(), 24);
    font.setEdging(SkFont::Edging::kAntiAlias);
    SkPaint paint;
    paint.setAntiAlias(true);
    for (int i = 0; i < 4; ++i) {
        SkString str;
        str.appendScalar(k[i]);
        SkScalar width = font.measureText(str.c_str(), str.size(), kUTF8_SkTextEncoding);
        canvas->drawString(str, x, y + font.getSize(), font, paint);
        x += width + SkIntToScalar(10);
    }
}

class ArithmodeGM : public skiagm::GM {
public:
    ArithmodeGM () {}

protected:

    virtual SkString onShortName() {
        return SkString("arithmode");
    }

    virtual SkISize onISize() { return SkISize::Make(640, 572); }

    virtual void onDraw(SkCanvas* canvas) {
        sk_sp<SkImage> src = make_src();
        sk_sp<SkImage> dst = make_dst();
        sk_sp<SkImageFilter> srcFilter = SkImageSource::Make(src);
        sk_sp<SkImageFilter> dstFilter = SkImageSource::Make(dst);

        constexpr SkScalar one = SK_Scalar1;
        constexpr SkScalar K[] = {
            0, 0, 0, 0,
            0, 0, 0, one,
            0, one, 0, 0,
            0, 0, one, 0,
            0, one, one, 0,
            0, one, -one, 0,
            0, one/2, one/2, 0,
            0, one/2, one/2, one/4,
            0, one/2, one/2, -one/4,
            one/4, one/2, one/2, 0,
            -one/4, one/2, one/2, 0,
        };

        const SkScalar* k = K;
        const SkScalar* stop = k + SK_ARRAY_COUNT(K);
        const SkRect rect = SkRect::MakeWH(WW, HH);
        SkScalar gap = SkIntToScalar(WW + 20);
        while (k < stop) {
            {
                SkAutoCanvasRestore acr(canvas, true);
                canvas->drawImage(src, 0, 0);
                canvas->translate(gap, 0);
                canvas->drawImage(dst, 0, 0);
                canvas->translate(gap, 0);
                SkPaint paint;
                paint.setImageFilter(SkArithmeticImageFilter::Make(k[0], k[1], k[2], k[3], true,
                                                                   dstFilter, srcFilter, nullptr));
                canvas->saveLayer(&rect, &paint);
                canvas->restore();

                canvas->translate(gap, 0);
                show_k_text(canvas, 0, 0, k);
            }

            k += 4;
            canvas->translate(0, HH + 12);
        }

        // Draw two special cases to test enforcePMColor. In these cases, we
        // draw the dst bitmap twice, the first time it is halved and inverted,
        // leading to invalid premultiplied colors. If we enforcePMColor, these
        // invalid values should be clamped, and will not contribute to the
        // second draw.
        for (int i = 0; i < 2; i++) {
            const bool enforcePMColor = (i == 0);

            {
                SkAutoCanvasRestore acr(canvas, true);
                canvas->translate(gap, 0);
                canvas->drawImage(dst, 0, 0);
                canvas->translate(gap, 0);

                sk_sp<SkImageFilter> bg =
                        SkArithmeticImageFilter::Make(0, 0, -one / 2, 1, enforcePMColor, dstFilter,
                                                      nullptr, nullptr);
                SkPaint p;
                p.setImageFilter(SkArithmeticImageFilter::Make(0, one / 2, -one, 1, true,
                                                               std::move(bg), dstFilter, nullptr));
                canvas->saveLayer(&rect, &p);
                canvas->restore();
                canvas->translate(gap, 0);

                // Label
                SkFont   font(ToolUtils::create_portable_typeface(), 24);
                SkString str(enforcePMColor ? "enforcePM" : "no enforcePM");
                canvas->drawString(str, 0, font.getSize(), font, SkPaint());
            }
            canvas->translate(0, HH + 12);
        }
    }

private:
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new ArithmodeGM; )
