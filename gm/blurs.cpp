/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFont.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "src/core/SkBlurMask.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

DEF_SIMPLE_GM_BG(blurs, canvas, 700, 500, 0xFFDDDDDD) {
    SkBlurStyle NONE = SkBlurStyle(-999);
    const struct {
        SkBlurStyle fStyle;
        int         fCx, fCy;
    } gRecs[] = {
        { NONE,                 0,  0 },
        { kInner_SkBlurStyle,  -1,  0 },
        { kNormal_SkBlurStyle,  0,  1 },
        { kSolid_SkBlurStyle,   0, -1 },
        { kOuter_SkBlurStyle,   1,  0 },
    };

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLUE);

    canvas->translate(SkIntToScalar(-40), SkIntToScalar(0));

    for (size_t i = 0; i < std::size(gRecs); i++) {
        if (gRecs[i].fStyle != NONE) {
            paint.setMaskFilter(SkMaskFilter::MakeBlur(gRecs[i].fStyle,
                                   SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(20))));
        } else {
            paint.setMaskFilter(nullptr);
        }
        canvas->drawCircle(SkIntToScalar(200 + gRecs[i].fCx*100),
                           SkIntToScalar(200 + gRecs[i].fCy*100),
                           SkIntToScalar(50),
                           paint);
    }
    // draw text
    {
        SkFont font(ToolUtils::DefaultPortableTypeface(), 25);
        paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle,
                                   SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(4))));
        SkScalar x = SkIntToScalar(70);
        SkScalar y = SkIntToScalar(400);
        paint.setColor(SK_ColorBLACK);
        canvas->drawString("Hamburgefons Style", x, y, font, paint);
        canvas->drawString("Hamburgefons Style",
                         x, y + SkIntToScalar(50), font, paint);
        paint.setMaskFilter(nullptr);
        paint.setColor(SK_ColorWHITE);
        x -= SkIntToScalar(2);
        y -= SkIntToScalar(2);
        canvas->drawString("Hamburgefons Style", x, y, font, paint);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////

// exercise a special-case of blurs, which is two nested rects. These are drawn specially,
// and possibly cached.
//
// in particular, we want to notice that the 2nd rect draws slightly differently, since it
// is translated a fractional amount.
//
DEF_SIMPLE_GM(blur2rects, canvas, 700, 500) {
    SkPaint paint;

    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 2.3f));

    SkRect outer = SkRect::MakeXYWH(10.125f, 10.125f, 100.125f, 100);
    SkRect inner = SkRect::MakeXYWH(20.25f, 20.125f, 80, 80);
    SkPath path = SkPathBuilder().addRect(outer, SkPathDirection::kCW)
                                 .addRect(inner, SkPathDirection::kCCW)
                                 .detach();

    canvas->drawPath(path, paint);
    // important to translate by a factional amount to exercise a different "phase"
    // of the same path w.r.t. the pixel grid
    SkScalar dx = SkScalarRoundToScalar(path.getBounds().width()) + 14 + 0.25f;
    canvas->translate(dx, 0);
    canvas->drawPath(path, paint);
}

DEF_SIMPLE_GM(blur2rectsnonninepatch, canvas, 700, 500) {
    SkPaint paint;
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 4.3f));

    SkRect outer = SkRect::MakeXYWH(10, 110, 100, 100);
    SkRect inner = SkRect::MakeXYWH(50, 150, 10, 10);
    SkPath path = SkPathBuilder().addRect(outer, SkPathDirection::kCW)
                                 .addRect(inner, SkPathDirection::kCW)
                                 .detach();
    canvas->drawPath(path, paint);

    SkScalar dx = SkScalarRoundToScalar(path.getBounds().width()) + 40 + 0.25f;
    canvas->translate(dx, 0);
    canvas->drawPath(path, paint);

    // Translate to outside of clip bounds.
    canvas->translate(-dx, 0);
    canvas->translate(-30, -150);
    canvas->drawPath(path, paint);
}

DEF_SIMPLE_GM(BlurDrawImage, canvas, 256, 256) {
    SkPaint paint;
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 10));
    canvas->clear(0xFF88FF88);
    if (auto image = ToolUtils::GetResourceAsImage("images/mandrill_512_q075.jpg")) {
        canvas->scale(0.25, 0.25);
        canvas->drawImage(image, 256, 256, SkSamplingOptions(), &paint);
    }
}

DEF_SIMPLE_GM(BlurBigSigma, canvas, 1024, 1024) {
    SkPaint layerPaint, p;

    p.setImageFilter(SkImageFilters::Blur(500, 500, nullptr));

    canvas->drawRect(SkRect::MakeWH(700, 800), p);
}

DEF_SIMPLE_GM(BlurSmallSigma, canvas, 512, 256) {
    {
        // Normal sigma on x-axis, a small but non-zero sigma on y-axis that should
        // be treated as identity.
        SkPaint paint;
        paint.setImageFilter(SkImageFilters::Blur(16.f, 1e-5f, nullptr));
        canvas->drawRect(SkRect::MakeLTRB(64, 64, 192, 192), paint);
    }

    {
        // Small sigma on both axes, should be treated as identity and no red should show
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        SkRect rect = SkRect::MakeLTRB(320, 64, 448, 192);
        canvas->drawRect(rect, paint);
        paint.setColor(SK_ColorBLACK);
        paint.setImageFilter(SkImageFilters::Blur(1e-5f, 1e-5f, nullptr));
        canvas->drawRect(rect, paint);
    }
}

// Modeled after crbug.com/1500021, incorporates manual tiling to emulate Chrome's raster tiles
// or the tiled rendering mode in Viewer.
DEF_SIMPLE_GM(TiledBlurBigSigma, canvas, 1024, 768) {
    static constexpr int kTileWidth = 342;
    static constexpr int kTileHeight = 256;

    SkM44 origCTM = canvas->getLocalToDevice();

    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            // Define tiled grid in the canvas pixel space
            canvas->save();
                canvas->resetMatrix();

                canvas->clipIRect(SkIRect::MakeXYWH(x*kTileWidth, y*kTileHeight,
                                                    kTileWidth, kTileHeight));
                canvas->setMatrix(origCTM);

                auto flood = SkImageFilters::ColorFilter(SkColorFilters::Blend(
                        SK_ColorBLACK, SkBlendMode::kSrc), nullptr);
                auto blend = SkImageFilters::Blend(SkBlendMode::kSrcOver,
                                                   std::move(flood), nullptr);
                auto blur = SkImageFilters::Blur(206.f, 206.f, std::move(blend));

                SkPaint p;
                p.setImageFilter(std::move(blur));

                canvas->clipRect({0, 0, 1970, 1223});
                canvas->saveLayer(nullptr, &p);
                    SkPaint fill;
                    fill.setColor(SK_ColorBLUE);
                    canvas->drawCircle(600, 150, 350, fill);
                canvas->restore();
            canvas->restore();
        }
    }
}
