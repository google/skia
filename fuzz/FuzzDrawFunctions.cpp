/*
 * Copyright 2016 Mozilla Foundation
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Fuzz.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkImage.h"
#include "SkPath.h"
#include "SkSurface.h"
#include "SkTypeface.h"
#include "SkClipOpPriv.h"

static const int kBmpSize = 24;
static const int kMaxX = 250;
static const int kMaxY = 250;
static const int kPtsLen = 10;
static const int kTxtLen = 5;

static void init_string(Fuzz* fuzz, char* str, size_t bufSize) {
    for (size_t i = 0; i < bufSize-1; ++i) {
        fuzz->nextRange(&str[i], 0x20, 0x7E); // printable ASCII
    }
    str[bufSize-1] = '\0';
}

// make_paint mostly borrowed from FilterFuzz.cpp
static void init_paint(Fuzz* fuzz, SkPaint* p) {
    bool b;
    fuzz->next(&b);
    p->setAntiAlias(b);

    uint8_t tmp_u8;
    fuzz->nextRange(&tmp_u8, 0, (int)SkBlendMode::kLastMode);
    p->setBlendMode(static_cast<SkBlendMode>(tmp_u8));

    SkColor co;
    fuzz->next(&co);
    p->setColor(co);

    fuzz->next(&b);
    p->setDither(b);

    fuzz->nextRange(&tmp_u8, 0, (int)kHigh_SkFilterQuality);
    p->setFilterQuality(static_cast<SkFilterQuality>(tmp_u8));

    fuzz->nextRange(&tmp_u8, 0, (int)SkPaint::kFull_Hinting);
    p->setHinting(static_cast<SkPaint::Hinting>(tmp_u8));

    fuzz->nextRange(&tmp_u8, 0, (int)SkPaint::kLast_Cap);
    p->setStrokeCap(static_cast<SkPaint::Cap>(tmp_u8));

    fuzz->nextRange(&tmp_u8, 0, (int)SkPaint::kLast_Join);
    p->setStrokeJoin(static_cast<SkPaint::Join>(tmp_u8));

    SkScalar sc;
    fuzz->next(&sc);
    p->setStrokeMiter(sc);

    fuzz->next(&sc);
    p->setStrokeWidth(sc);

    fuzz->nextRange(&tmp_u8, 0, (int)SkPaint::kStrokeAndFill_Style);
    p->setStyle(static_cast<SkPaint::Style>(tmp_u8));
}

static void init_bitmap(Fuzz* fuzz, SkBitmap* bmp) {
    uint8_t colorType;
    fuzz->nextRange(&colorType, 0, (int)kLastEnum_SkColorType);
    // ColorType needs to match what the system configuration is.
    if (colorType == kRGBA_8888_SkColorType || colorType == kBGRA_8888_SkColorType) {
        colorType = kN32_SkColorType;
    }
    bool b;
    fuzz->next(&b);
    SkImageInfo info = SkImageInfo::Make(kBmpSize,
                                         kBmpSize,
                                         (SkColorType)colorType,
                                         b ? kOpaque_SkAlphaType : kPremul_SkAlphaType);
    if (!bmp->tryAllocPixels(info)) {
        SkDebugf("Bitmap not allocated\n");
    }
    SkColor c;
    fuzz->next(&c);
    bmp->eraseColor(c);

    fuzz->next(&b);
    SkPaint p;
    if (b) {
        init_paint(fuzz, &p);
    }
    else {
        fuzz->next(&c);
        p.setColor(c);
    }
}

static void init_surface(Fuzz* fuzz, sk_sp<SkSurface>* s) {
    uint8_t x, y;
    fuzz->nextRange(&x, 1, kMaxX);
    fuzz->nextRange(&y, 1, kMaxY);
    *s = SkSurface::MakeRasterN32Premul(x, y);
}


static void fuzz_drawText(Fuzz* fuzz, sk_sp<SkTypeface> font) {
    SkPaint p;
    init_paint(fuzz, &p);
    sk_sp<SkSurface> surface;
    init_surface(fuzz, &surface);

    char text[kTxtLen];
    init_string(fuzz, text, kTxtLen);

    SkScalar x, y;
    fuzz->next(&x, &y);
    // populate pts array
    SkPoint pts[kPtsLen];
    for (uint8_t i = 0; i < kPtsLen; ++i) {
        pts[i].set(x, y);
        x += p.getTextSize();
    }

    p.setTypeface(font);
    // set text related attributes
    bool b;
    fuzz->next(&b);
    p.setAutohinted(b);
    fuzz->next(&b);
    p.setDevKernText(b);
    fuzz->next(&b);
    p.setEmbeddedBitmapText(b);
    fuzz->next(&b);
    p.setFakeBoldText(b);
    fuzz->next(&b);
    p.setLCDRenderText(b);
    fuzz->next(&b);
    p.setLinearText(b);
    fuzz->next(&b);
    p.setSubpixelText(b);
    fuzz->next(&x);
    p.setTextScaleX(x);
    fuzz->next(&x);
    p.setTextSkewX(x);
    fuzz->next(&x);
    p.setTextSize(x);
    fuzz->next(&b);
    p.setVerticalText(b);

    SkCanvas* cnv = surface->getCanvas();
    cnv->drawPosText(text, (kTxtLen-1), pts, p);

    fuzz->next(&x);
    fuzz->next(&y);
    cnv->drawText(text, (kTxtLen-1), x, y, p);
}

static void fuzz_drawCircle(Fuzz* fuzz) {
    SkPaint p;
    init_paint(fuzz, &p);
    sk_sp<SkSurface> surface;
    init_surface(fuzz, &surface);

    SkScalar a, b, c;
    fuzz->next(&a, &b, &c);
    surface->getCanvas()->drawCircle(a, b, c, p);
}

static void fuzz_drawLine(Fuzz* fuzz) {
    SkPaint p;
    init_paint(fuzz, &p);
    sk_sp<SkSurface> surface;
    init_surface(fuzz, &surface);

    SkScalar a, b, c, d;
    fuzz->next(&a, &b, &c, &d);
    surface->getCanvas()->drawLine(a, b, c, d, p);
}

static void fuzz_drawRect(Fuzz* fuzz) {
    SkPaint p;
    init_paint(fuzz, &p);
    sk_sp<SkSurface> surface;
    init_surface(fuzz, &surface);

    SkScalar a, b, c, d;
    fuzz->next(&a, &b, &c, &d);
    SkRect r;
    r = SkRect::MakeXYWH(a, b, c, d);

    SkCanvas* cnv = surface->getCanvas();
    cnv->drawRect(r, p);

    bool bl;
    fuzz->next(&bl);
    fuzz->next(&a, &b, &c, &d);
    r = SkRect::MakeXYWH(a, b, c, d);
    cnv->clipRect(r, kIntersect_SkClipOp, bl);
}

static void fuzz_drawPath(Fuzz* fuzz) {
    SkPaint p;
    init_paint(fuzz, &p);
    sk_sp<SkSurface> surface;
    init_surface(fuzz, &surface);

    // TODO(kjlubick): put the ability to fuzz a path in shared file, with
    // other common things (e.g. rects, lines)
    uint8_t i, j;
    fuzz->nextRange(&i, 0, 10); // set i to number of operations to perform
    SkPath path;
    SkScalar a, b, c, d, e, f;
    for (int k = 0; k < i; ++k) {
        fuzz->nextRange(&j, 0, 5); // set j to choose operation to perform
        switch (j) {
            case 0:
                fuzz->next(&a, &b);
                path.moveTo(a, b);
                break;
            case 1:
                fuzz->next(&a, &b);
                path.lineTo(a, b);
                break;
            case 2:
                fuzz->next(&a, &b, &c, &d);
                path.quadTo(a, b, c, d);
                break;
            case 3:
                fuzz->next(&a, &b, &c, &d, &e);
                path.conicTo(a, b, c, d, e);
                break;
            case 4:
                fuzz->next(&a, &b, &c, &d, &e, &f);
                path.cubicTo(a, b, c, d, e, f);
                break;
            case 5:
                fuzz->next(&a, &b, &c, &d, &e);
                path.arcTo(a, b, c, d, e);
                break;
        }
    }
    path.close();

    SkCanvas* cnv = surface->getCanvas();
    cnv->drawPath(path, p);

    bool bl;
    fuzz->next(&bl);
    cnv->clipPath(path, kIntersect_SkClipOp, bl);
}

static void fuzz_drawBitmap(Fuzz* fuzz) {
    SkPaint p;
    init_paint(fuzz, &p);
    sk_sp<SkSurface> surface;
    init_surface(fuzz, &surface);
    SkBitmap bmp;
    init_bitmap(fuzz, &bmp);

    SkScalar a, b;
    fuzz->next(&a, &b);
    surface->getCanvas()->drawBitmap(bmp, a, b, &p);
}

static void fuzz_drawImage(Fuzz* fuzz) {
    SkPaint p;
    init_paint(fuzz, &p);
    sk_sp<SkSurface> surface;
    init_surface(fuzz, &surface);
    SkBitmap bmp;
    init_bitmap(fuzz, &bmp);

    sk_sp<SkImage> image(SkImage::MakeFromBitmap(bmp));

    bool bl;
    fuzz->next(&bl);
    SkScalar a, b;
    fuzz->next(&a, &b);
    if (bl) {
        surface->getCanvas()->drawImage(image, a, b, &p);
    }
    else {
        SkRect dst = SkRect::MakeWH(a, b);
        fuzz->next(&a, &b);
        SkRect src = SkRect::MakeWH(a, b);
        uint8_t x;
        fuzz->nextRange(&x, 0, 1);
        SkCanvas::SrcRectConstraint cst = (SkCanvas::SrcRectConstraint)x;
        surface->getCanvas()->drawImageRect(image, src, dst, &p, cst);
    }
}

static void fuzz_drawPaint(Fuzz* fuzz) {
    SkPaint l, p;
    init_paint(fuzz, &p);
    sk_sp<SkSurface> surface;
    init_surface(fuzz, &surface);

    surface->getCanvas()->drawPaint(p);
}

DEF_FUZZ(DrawFunctions, fuzz) {
    uint8_t i;
    fuzz->next(&i);

    switch(i) {
        case 0: {
            sk_sp<SkTypeface> f = SkTypeface::MakeDefault();
            if (f == nullptr) {
              SkDebugf("Could not initialize font.\n");
              fuzz->signalBug();
            }
            SkDebugf("Fuzz DrawText\n");
            fuzz_drawText(fuzz, f);
            return;
        }
        case 1:
            SkDebugf("Fuzz DrawRect\n");
            fuzz_drawRect(fuzz);
            return;
        case 2:
            SkDebugf("Fuzz DrawCircle\n");
            fuzz_drawCircle(fuzz);
            return;
        case 3:
            SkDebugf("Fuzz DrawLine\n");
            fuzz_drawLine(fuzz);
            return;
        case 4:
            SkDebugf("Fuzz DrawPath\n");
            fuzz_drawPath(fuzz);
            return;
        case 5:
            SkDebugf("Fuzz DrawImage/DrawImageRect\n");
            fuzz_drawImage(fuzz);
            return;
        case 6:
            SkDebugf("Fuzz DrawBitmap\n");
            fuzz_drawBitmap(fuzz);
            return;
        case 7:
            SkDebugf("Fuzz DrawPaint\n");
            fuzz_drawPaint(fuzz);
            return;
    }
}
