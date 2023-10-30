/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkTypes.h"
#include "tools/fonts/FontToolUtils.h"

// This GM shows off a flaw in delta-based rasterizers (DAA, CCPR, etc.).
// See also the bottom of dashing4 and skia:6886.

static const int K = 49;

DEF_SIMPLE_GM(daa, canvas, K+350, 5*K) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkFont font = ToolUtils::DefaultPortableFont();

    {
        paint.setColor(SK_ColorBLACK);
        canvas->drawString("Should be a green square with no red showing through.",
                           K*1.5f, K*0.5f, font, paint);

        paint.setColor(SK_ColorRED);
        canvas->drawRect({0,0,K,K}, paint);

        SkPoint tri1[] = {{0,0},{K,K},{0,K},{0,0}};
        SkPoint tri2[] = {{0,0},{K,K},{K,0},{0,0}};
        SkPath path = SkPathBuilder().addPolygon(tri1, std::size(tri1), false)
                                     .addPolygon(tri2, std::size(tri2), false)
                                     .detach();

        paint.setColor(SK_ColorGREEN);
        canvas->drawPath(path, paint);
    }

    canvas->translate(0,K);
    {
        paint.setColor(SK_ColorBLACK);
        canvas->drawString("Adjacent rects, two draws.  Blue then green, no red?",
                           K*1.5f, K*0.5f, font, paint);

        paint.setColor(SK_ColorRED);
        canvas->drawRect({0,0,K,K}, paint);

        {
            SkPath path = SkPath::Polygon({{0,0},{0,K},{K*0.5f,K},{K*0.5f,0}}, false);
            paint.setColor(SK_ColorBLUE);
            canvas->drawPath(path, paint);
        }

        {
            SkPath path = SkPath::Polygon({{K*0.5f,0},{K*0.5f,K},{K,K},{K,0}}, false);
            paint.setColor(SK_ColorGREEN);
            canvas->drawPath(path, paint);
        }
    }

    canvas->translate(0,K);
    {
        paint.setColor(SK_ColorBLACK);
        canvas->drawString("Adjacent rects, wound together.  All green?",
                           K*1.5f, K*0.5f, font, paint);

        paint.setColor(SK_ColorRED);
        canvas->drawRect({0,0,K,K}, paint);

        {
            SkPath path = SkPathBuilder().addPolygon({{0,0},{0,K},{K*0.5f,K},{K*0.5f,0}}, false)
                                         .addPolygon({{K*0.5f,0},{K*0.5f,K},{K,K},{K,0}}, false)
                                         .detach();

            paint.setColor(SK_ColorGREEN);
            canvas->drawPath(path, paint);
        }
    }

    canvas->translate(0,K);
    {
        paint.setColor(SK_ColorBLACK);
        canvas->drawString("Adjacent rects, wound opposite.  All green?",
                           K*1.5f, K*0.5f, font, paint);

        paint.setColor(SK_ColorRED);
        canvas->drawRect({0,0,K,K}, paint);

        {
            SkPath path = SkPathBuilder().addPolygon({{0,0},{0,K},{K*0.5f,K},{K*0.5f,0}}, false)
                                         .addPolygon({{K*0.5f,0},{K,0},{K,K},{K*0.5f,K}}, false)
                                         .detach();

            paint.setColor(SK_ColorGREEN);
            canvas->drawPath(path, paint);
        }
    }

    canvas->translate(0,K);
    {
        paint.setColor(SK_ColorBLACK);
        canvas->drawString("One poly, wound opposite.  All green?",
                           K*1.5f, K*0.5f, font, paint);

        paint.setColor(SK_ColorRED);
        canvas->drawRect({0,0,K,K}, paint);

        SkPath path = SkPath::Polygon({{K*0.5f,0},{0,0},{0,K},{K*0.5f,K},
                                       {K*0.5f,0},{K,0},{K,K},{K*0.5f,K}},
                                      false);

        paint.setColor(SK_ColorGREEN);
        canvas->drawPath(path, paint);
    }
}
