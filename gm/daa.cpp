/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkFont.h"
#include "SkPath.h"

#include "../../blend2d/src/blend2d.h"

// This GM shows off a flaw in delta-based rasterizers (DAA, CCPR, etc.).
// See also the bottom of dashing4 and skia:6886.

static const int K = 49;

static void draw_with_Blend2D(BLPath path, SkCanvas* canvas) {
    BLImage img(K,K, BL_FORMAT_PRGB32);
    BLContext ctx(img);

    ctx.setFillStyle(BLRgba32{0x00000000});
    ctx.setCompOp(BL_COMP_OP_SRC_COPY);
    ctx.fillAll();

    ctx.setFillStyle(BLRgba32{0xffffff00});
    ctx.fillPath(path);
    ctx.end();

    BLImageData peek;
    img.getData(&peek);

    SkBitmap bm;
    bm.installPixels(SkImageInfo::Make(K,K, kBGRA_8888_SkColorType, kPremul_SkAlphaType),
            peek.pixelData, peek.stride);
    canvas->drawBitmap(bm, 0,0);
}

DEF_SIMPLE_GM(daa, canvas, K+350, 8*K) {
    SkPaint paint;
    paint.setAntiAlias(true);

    {
        paint.setColor(SK_ColorBLACK);
        canvas->drawString("(b2d) yellow square with no red showing through.",
                           K*1.5f, K*0.5f, SkFont(), paint);

        paint.setColor(SK_ColorRED);
        canvas->drawRect({0,0,K,K}, paint);

        BLPoint tri1[] = {{0,0},{K,K},{0,K},{0,0}};
        BLPoint tri2[] = {{0,0},{K,K},{K,0},{0,0}};

        BLPath path;
        path.moveTo(0,0);
        path.polyTo(tri1, SK_ARRAY_COUNT(tri1));
        path.polyTo(tri2, SK_ARRAY_COUNT(tri2));

        draw_with_Blend2D(path, canvas);
    }

    canvas->translate(0,K);
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
        canvas->drawString("(b2d) adj. rects wound opposite.  all yellow?",
                           K*1.5f, K*0.5f, SkFont(), paint);

        paint.setColor(SK_ColorRED);
        canvas->drawRect({0,0,K,K}, paint);

        BLPoint rect1[] = {{0,0},{0,K},{K*0.5f,K},{K*0.5f,0}};
        BLPoint rect2[] = {{K*0.5f,0},{K,0},{K,K},{K*0.5f,K}};

        BLPath path;
        path.moveTo(0,0);
        path.polyTo(rect1, SK_ARRAY_COUNT(rect1));
        path.moveTo(rect2[0]);
        path.polyTo(rect2, SK_ARRAY_COUNT(rect2));

        draw_with_Blend2D(path, canvas);
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
        canvas->drawString("(b2d) one poly, wound opposite, all yellow?",
                           K*1.5f, K*0.5f, SkFont(), paint);

        paint.setColor(SK_ColorRED);
        canvas->drawRect({0,0,K,K}, paint);

        BLPoint poly[] = {{K*0.5f,0},{0,0},{0,K},{K*0.5f,K},{K*0.5f,0},{K,0},{K,K},{K*0.5f,K}};

        BLPath path;
        path.moveTo(poly[0]);
        path.polyTo(poly, SK_ARRAY_COUNT(poly));

        draw_with_Blend2D(path, canvas);
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
