/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkFont.h"
#include "include/core/SkPath.h"

// This GM shows off a flaw in delta-based rasterizers (DAA, CCPR, etc.).
// See also the bottom of dashing4 and skia:6886.

static const int K = 49;

DEF_SIMPLE_GM(daa, canvas, K+350, 5*K) {
    SkPaint paint;
    paint.setAntiAlias(true);

    {
        paint.setColor(SK_ColorBLACK);
        canvas->drawString("Should be a green square with no red showing through.",
                           K*1.5f, K*0.5f, SkFont(), paint);

        paint.setColor(SK_ColorRED);
        canvas->drawRect({0,0,K,K}, paint);

        SkPath path;
        SkPoint tri1[] = {{0,0},{K,K},{0,K},{0,0}};
        SkPoint tri2[] = {{0,0},{K,K},{K,0},{0,0}};
        path.addPoly(tri1, SK_ARRAY_COUNT(tri1), false);
        path.addPoly(tri2, SK_ARRAY_COUNT(tri2), false);

        paint.setColor(SK_ColorGREEN);
        canvas->drawPath(path, paint);
    }

    canvas->translate(0,K);
    {
        paint.setColor(SK_ColorBLACK);
        canvas->drawString("Adjacent rects, two draws.  Blue then green, no red?",
                           K*1.5f, K*0.5f, SkFont(), paint);

        paint.setColor(SK_ColorRED);
        canvas->drawRect({0,0,K,K}, paint);

        {
            SkPath path;
            SkPoint rect1[] = {{0,0},{0,K},{K*0.5f,K},{K*0.5f,0}};
            path.addPoly(rect1, SK_ARRAY_COUNT(rect1), false);

            paint.setColor(SK_ColorBLUE);
            canvas->drawPath(path, paint);
        }

        {
            SkPath path;
            SkPoint rect2[] = {{K*0.5f,0},{K*0.5f,K},{K,K},{K,0}};
            path.addPoly(rect2, SK_ARRAY_COUNT(rect2), false);

            paint.setColor(SK_ColorGREEN);
            canvas->drawPath(path, paint);
        }
    }

    canvas->translate(0,K);
    {
        paint.setColor(SK_ColorBLACK);
        canvas->drawString("Adjacent rects, wound together.  All green?",
                           K*1.5f, K*0.5f, SkFont(), paint);

        paint.setColor(SK_ColorRED);
        canvas->drawRect({0,0,K,K}, paint);

        {
            SkPath path;
            SkPoint rect1[] = {{0,0},{0,K},{K*0.5f,K},{K*0.5f,0}};
            SkPoint rect2[] = {{K*0.5f,0},{K*0.5f,K},{K,K},{K,0}};

            path.addPoly(rect1, SK_ARRAY_COUNT(rect1), false);
            path.addPoly(rect2, SK_ARRAY_COUNT(rect2), false);

            paint.setColor(SK_ColorGREEN);
            canvas->drawPath(path, paint);
        }
    }

    canvas->translate(0,K);
    {
        paint.setColor(SK_ColorBLACK);
        canvas->drawString("Adjacent rects, wound opposite.  All green?",
                           K*1.5f, K*0.5f, SkFont(), paint);

        paint.setColor(SK_ColorRED);
        canvas->drawRect({0,0,K,K}, paint);

        {
            SkPath path;
            SkPoint rect1[] = {{0,0},{0,K},{K*0.5f,K},{K*0.5f,0}};
            SkPoint rect2[] = {{K*0.5f,0},{K,0},{K,K},{K*0.5f,K}};

            path.addPoly(rect1, SK_ARRAY_COUNT(rect1), false);
            path.addPoly(rect2, SK_ARRAY_COUNT(rect2), false);

            paint.setColor(SK_ColorGREEN);
            canvas->drawPath(path, paint);
        }
    }

    canvas->translate(0,K);
    {
        paint.setColor(SK_ColorBLACK);
        canvas->drawString("One poly, wound opposite.  All green?",
                           K*1.5f, K*0.5f, SkFont(), paint);

        paint.setColor(SK_ColorRED);
        canvas->drawRect({0,0,K,K}, paint);

        {
            SkPath path;
            SkPoint poly[] = {{K*0.5f,0},{0,0},{0,K},{K*0.5f,K},{K*0.5f,0},{K,0},{K,K},{K*0.5f,K}};

            path.addPoly(poly, SK_ARRAY_COUNT(poly), false);

            paint.setColor(SK_ColorGREEN);
            canvas->drawPath(path, paint);
        }
    }
}
