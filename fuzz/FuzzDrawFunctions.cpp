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
#include "SkLayerRasterizer.h"
#include "SkPath.h"
#include "SkSurface.h"
#include "SkTypeface.h"

const int BMP_SIZE = 24;
const int MAX_X = 250;
const int MAX_Y = 250;
const int PTS_LEN = 10;
const int TXT_LEN = 5;
const char* TEST_FONT = "DroidSans-Regular.ttf";

static void initBitmap(Fuzz* fuzz, SkBitmap* bmp) {
    uint8_t colorType;
    fuzz->nextRange(&colorType, 0, (int)kLastEnum_SkColorType);
    SkImageInfo info = SkImageInfo::Make(BMP_SIZE,
                                         BMP_SIZE,
                                         (SkColorType)colorType,
                                         kPremul_SkAlphaType);
    if (!bmp->tryAllocPixels(info)) {
        SkDebugf("Bitmap not allocated\n");
    }
    bool b;
    fuzz->next(&b);
    if (b) { // initialize
        SkCanvas canvas(*bmp);
        canvas.clear(0);
        SkColor c;
        fuzz->next(&c);
        SkPaint p; // TODO: maybe initPaint?
        p.setColor(c);
        canvas.drawRect(SkRect::MakeXYWH(0, 0, BMP_SIZE, BMP_SIZE), p);
    }
}

static void initString(Fuzz* fuzz, char* str, size_t bufSize, bool printable) {
    for (size_t i = 0; i < bufSize-1; ++i) {
        fuzz->nextRange(&str[i], 0x20, 0x7E); // printable ASCII
    }
    str[bufSize-1] = '\0';
}

// make_paint mostly borrowed from FilterFuzz.cpp
static void initPaint(Fuzz* fuzz, SkPaint* p) {
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


static void initSurface(Fuzz* fuzz, sk_sp<SkSurface> *s) {
    uint8_t x, y;
    fuzz->nextRange(&x, 1, MAX_X);
    fuzz->nextRange(&y, 1, MAX_Y);
    *s = SkSurface::MakeRasterN32Premul(x, y);
}


void fuzzDrawText(Fuzz* fuzz, sk_sp<SkTypeface> font) {
    SkPaint p;
    initPaint(fuzz, &p);
    sk_sp<SkSurface> surface;
    initSurface(fuzz, &surface);

    bool b;
    fuzz->next(&b);
    char text[TXT_LEN];
    initString(fuzz, text, TXT_LEN, b);

    SkScalar x, y;
    fuzz->next(&x, &y);
    // populate pts array
    SkPoint pts[PTS_LEN];
    for (uint8_t i = 0; i < PTS_LEN; ++i) {
        pts[i].set(x, y);
        x += p.getTextSize();
    }

    p.setTypeface(font);
    // set text related attributes
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
    p.setStrikeThruText(b);
    fuzz->next(&b);
    p.setSubpixelText(b);
    fuzz->next(&x);
    p.setTextScaleX(x);
    fuzz->next(&x);
    p.setTextSkewX(x);
    fuzz->next(&x);
    p.setTextSize(x);
    fuzz->next(&b);
    p.setUnderlineText(b);
    fuzz->next(&b);
    p.setVerticalText(b);

    SkCanvas* cnv = surface->getCanvas();
    cnv->drawPosText(text, (TXT_LEN-1), pts, p);

    fuzz->next(&x);
    fuzz->next(&y);
    cnv->drawText(text, (TXT_LEN-1), x, y, p);

    return;
}

void fuzzDrawCircle(Fuzz* fuzz) {
    SkPaint p;
    initPaint(fuzz, &p);
    sk_sp<SkSurface> surface;
    initSurface(fuzz, &surface);

    SkScalar a, b, c;
    fuzz->next(&a, &b, &c);
    surface->getCanvas()->drawCircle(a, b, c, p);
}

void fuzzDrawLine(Fuzz* fuzz) {
    SkPaint p;
    initPaint(fuzz, &p);
    sk_sp<SkSurface> surface;
    initSurface(fuzz, &surface);

    SkScalar a, b, c, d;
    fuzz->next(&a, &b, &c, &d);
    surface->getCanvas()->drawLine(a, b, c, d, p);
}

void fuzzDrawRect(Fuzz* fuzz) {
    SkPaint p;
    initPaint(fuzz, &p);
    sk_sp<SkSurface> surface;
    initSurface(fuzz, &surface);

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

void fuzzDrawPath(Fuzz* fuzz) {
    SkPaint p;
    initPaint(fuzz, &p);
    sk_sp<SkSurface> surface;
    initSurface(fuzz, &surface);

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
    // see skia/include/core/SkClipOp.h
    cnv->clipPath(path, kIntersect_SkClipOp, bl);
}

void fuzzDrawBitmap(Fuzz* fuzz) {
    SkPaint p;
    initPaint(fuzz, &p);
    sk_sp<SkSurface> surface;
    initSurface(fuzz, &surface);
    SkBitmap bmp;
    initBitmap(fuzz, &bmp);

    SkScalar a, b;
    fuzz->next(&a, &b);
    surface->getCanvas()->drawBitmap(bmp, a, b, &p);
}

void fuzzDrawImage(Fuzz* fuzz) {
    SkPaint p;
    initPaint(fuzz, &p);
    sk_sp<SkSurface> surface;
    initSurface(fuzz, &surface);
    SkBitmap bmp;
    initBitmap(fuzz, &bmp);

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

void fuzzDrawPaint(Fuzz* fuzz) {
    SkPaint l, p;
    initPaint(fuzz, &p);
    sk_sp<SkSurface> surface;
    initSurface(fuzz, &surface);

    // add layers
    uint8_t x;
    fuzz->nextRange(&x, 1, 3); // max 3 layers
    SkLayerRasterizer::Builder builder;
    for (int i = 0; i < x; i++) {
        initPaint(fuzz, &l);
        builder.addLayer(l);
    }

    sk_sp<SkLayerRasterizer> raster(builder.detach());
    p.setRasterizer(raster);

    surface->getCanvas()->drawPaint(p);
}

DEF_FUZZ(DrawFunctions, fuzz) {
    uint8_t i;
    fuzz->next(&i);

    switch(i % 8) {
        case 0: {
            sk_sp<SkTypeface> f = SkTypeface::MakeFromFile(TEST_FONT);
            if (f == nullptr) {
              SkDebugf("Missing file: %s\n", TEST_FONT);
              fuzz->signalBug();
            }
            SkDebugf("Fuzz DrawText\n");
            fuzzDrawText(fuzz, f);
            return;
        }
        case 1:
            SkDebugf("Fuzz DrawRect\n");
            fuzzDrawRect(fuzz);
            return;
        case 2:
            SkDebugf("Fuzz DrawCircle\n");
            fuzzDrawCircle(fuzz);
            return;
        case 3:
            SkDebugf("Fuzz DrawLine\n");
            fuzzDrawLine(fuzz);
            return;
        case 4:
            SkDebugf("Fuzz DrawPath\n");
            fuzzDrawPath(fuzz);
            return;
        case 5:
            SkDebugf("Fuzz DrawImage/DrawImageRect\n");
            fuzzDrawImage(fuzz);
            return;
        case 6:
            SkDebugf("Fuzz DrawBitmap\n");
            fuzzDrawBitmap(fuzz);
            return;
        case 7:
            SkDebugf("Fuzz DrawPaint\n");
            fuzzDrawPaint(fuzz);
            return;
    }
    return;
}
