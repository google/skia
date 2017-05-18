/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkPath.h"

DEF_SIMPLE_GM_BG(blurs, canvas, 700, 500, sk_tool_utils::color_to_565(0xFFDDDDDD)) {
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
        sk_tool_utils::set_portable_typeface(&paint);
        paint.setTextSize(SkIntToScalar(25));
        canvas->translate(SkIntToScalar(-40), SkIntToScalar(0));

        SkBlurMaskFilter::BlurFlags flags = SkBlurMaskFilter::kNone_BlurFlag;
        for (int j = 0; j < 2; j++) {
            canvas->save();
            paint.setColor(SK_ColorBLUE);
            for (size_t i = 0; i < SK_ARRAY_COUNT(gRecs); i++) {
                if (gRecs[i].fStyle != NONE) {
                    paint.setMaskFilter(SkBlurMaskFilter::Make(gRecs[i].fStyle,
                                           SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(20)),
                                           flags));
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
                paint.setMaskFilter(SkBlurMaskFilter::Make(kNormal_SkBlurStyle,
                                           SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(4)),
                                           flags));
                SkScalar x = SkIntToScalar(70);
                SkScalar y = SkIntToScalar(400);
                paint.setColor(SK_ColorBLACK);
                canvas->drawString("Hamburgefons Style", x, y, paint);
                canvas->drawString("Hamburgefons Style",
                                 x, y + SkIntToScalar(50), paint);
                paint.setMaskFilter(nullptr);
                paint.setColor(SK_ColorWHITE);
                x -= SkIntToScalar(2);
                y -= SkIntToScalar(2);
                canvas->drawString("Hamburgefons Style", x, y, paint);
            }
            canvas->restore();
            flags = SkBlurMaskFilter::kHighQuality_BlurFlag;
            canvas->translate(SkIntToScalar(350), SkIntToScalar(0));
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

        paint.setMaskFilter(SkBlurMaskFilter::Make(kNormal_SkBlurStyle, 2.3f));

        SkRect outer = SkRect::MakeXYWH(10.125f, 10.125f, 100.125f, 100);
        SkRect inner = SkRect::MakeXYWH(20.25f, 20.125f, 80, 80);
        SkPath path;
        path.addRect(outer, SkPath::kCW_Direction);
        path.addRect(inner, SkPath::kCCW_Direction);

        canvas->drawPath(path, paint);
        // important to translate by a factional amount to exercise a different "phase"
        // of the same path w.r.t. the pixel grid
        SkScalar dx = SkScalarRoundToScalar(path.getBounds().width()) + 14 + 0.25f;
        canvas->translate(dx, 0);
        canvas->drawPath(path, paint);
}

DEF_SIMPLE_GM(blur2rectsnonninepatch, canvas, 700, 500) {
        SkPaint paint;
        paint.setMaskFilter(SkBlurMaskFilter::Make(kNormal_SkBlurStyle, 4.3f));

        SkRect outer = SkRect::MakeXYWH(10, 110, 100, 100);
        SkRect inner = SkRect::MakeXYWH(50, 150, 10, 10);
        SkPath path;
        path.addRect(outer, SkPath::kCW_Direction);
        path.addRect(inner, SkPath::kCW_Direction);
        canvas->drawPath(path, paint);

        SkScalar dx = SkScalarRoundToScalar(path.getBounds().width()) + 40 + 0.25f;
        canvas->translate(dx, 0);
        canvas->drawPath(path, paint);

        // Translate to outside of clip bounds.
        canvas->translate(-dx, 0);
        canvas->translate(-30, -150);
        canvas->drawPath(path, paint);
}
