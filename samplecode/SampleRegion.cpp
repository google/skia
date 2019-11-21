/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkPath.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/effects/SkGradientShader.h"
#include "samplecode/Sample.h"
#include "src/utils/SkUTF.h"

#include <math.h>

static void test_strokerect(SkCanvas* canvas) {
    int width = 100;
    int height = 100;

    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::MakeA8(width*2, height*2));
    bitmap.eraseColor(SK_ColorTRANSPARENT);

    SkScalar dx = 20;
    SkScalar dy = 20;

    SkPath path;
    path.addRect(0.0f, 0.0f,
                 SkIntToScalar(width), SkIntToScalar(height),
                 SkPathDirection::kCW);
    SkRect r = SkRect::MakeWH(SkIntToScalar(width), SkIntToScalar(height));

    SkCanvas c(bitmap);
    c.translate(dx, dy);

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(1);

    // use the rect
    c.clear(SK_ColorTRANSPARENT);
    c.drawRect(r, paint);
    canvas->drawBitmap(bitmap, 0, 0, nullptr);

    // use the path
    c.clear(SK_ColorTRANSPARENT);
    c.drawPath(path, paint);
    canvas->drawBitmap(bitmap, SkIntToScalar(2*width), 0, nullptr);
}

static void drawFadingText(SkCanvas* canvas,
                           const char* text, size_t len, SkScalar x, SkScalar y,
                           const SkFont& font, const SkPaint& paint) {
    // Need a bounds for the text
    SkRect bounds;
    SkFontMetrics fm;

    font.getMetrics(&fm);
    bounds.setLTRB(x, y + fm.fTop,
                   x + font.measureText(text, len, SkTextEncoding::kUTF8), y + fm.fBottom);

    // may need to outset bounds a little, to account for hinting and/or
    // antialiasing
    bounds.inset(-SkIntToScalar(2), -SkIntToScalar(2));

    canvas->saveLayer(&bounds, nullptr);
    canvas->drawSimpleText(text, len, SkTextEncoding::kUTF8, x, y, font, paint);

    const SkPoint pts[] = {
        { bounds.fLeft, y },
        { bounds.fRight, y }
    };
    const SkColor colors[] = { SK_ColorBLACK, SK_ColorBLACK, 0 };

    // pos[1] value is where we start to fade, relative to the width
    // of our pts[] array.
    const SkScalar pos[] = { 0, 0.9f, SK_Scalar1 };

    SkPaint p;
    p.setShader(SkGradientShader::MakeLinear(pts, colors, pos, 3, SkTileMode::kClamp));
    p.setBlendMode(SkBlendMode::kDstIn);
    canvas->drawRect(bounds, p);

    canvas->restore();
}

static void test_text(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);

    SkFont font;
    font.setSize(20);

    const char* str = "Hamburgefons";
    size_t len = strlen(str);
    SkScalar x = 20;
    SkScalar y = 20;

    canvas->drawSimpleText(str, len, SkTextEncoding::kUTF8, x, y, font, paint);

    y += 20;

    const SkPoint pts[] = { { x                                                    , y },
                            { x + font.measureText(str, len, SkTextEncoding::kUTF8), y } };
    const SkColor colors[] = { SK_ColorBLACK, SK_ColorBLACK, 0 };
    const SkScalar pos[] = { 0, 0.9f, 1 };
    paint.setShader(SkGradientShader::MakeLinear(pts, colors, pos,
                                                 SK_ARRAY_COUNT(colors),
                                                 SkTileMode::kClamp));
    canvas->drawSimpleText(str, len, SkTextEncoding::kUTF8, x, y, font, paint);

    y += 20;
    paint.setShader(nullptr);
    drawFadingText(canvas, str, len, x, y, font, paint);
}

static void scale_rect(SkIRect* dst, const SkIRect& src, float scale) {
    dst->fLeft = (int)::roundf(src.fLeft * scale);
    dst->fTop = (int)::roundf(src.fTop * scale);
    dst->fRight = (int)::roundf(src.fRight * scale);
    dst->fBottom = (int)::roundf(src.fBottom * scale);
}

static void scale_rgn(SkRegion* dst, const SkRegion& src, float scale) {
    SkRegion tmp;
    SkRegion::Iterator iter(src);

    for (; !iter.done(); iter.next()) {
        SkIRect r;
        scale_rect(&r, iter.rect(), scale);
        tmp.op(r, SkRegion::kUnion_Op);
    }
    dst->swap(tmp);
}

static void paint_rgn(SkCanvas* canvas, const SkRegion& rgn,
                      const SkPaint& paint) {
    SkRegion scaled;
    scale_rgn(&scaled, rgn, 0.5f);

    SkRegion::Iterator  iter(rgn);

    for (; !iter.done(); iter.next())
    {
        SkRect    r;
        r.set(iter.rect());
        canvas->drawRect(r, paint);
    }
}

class RegionView : public Sample {
public:
    RegionView() {
        fBase.setLTRB(100, 100, 150, 150);
        fRect = fBase;
        fRect.inset(5, 5);
        fRect.offset(25, 25);
        this->setBGColor(0xFFDDDDDD);
    }

    void build_base_rgn(SkRegion* rgn) {
        rgn->setRect(fBase);
        SkIRect r = fBase;
        r.offset(75, 20);
        rgn->op(r, SkRegion::kUnion_Op);
    }

    void build_rgn(SkRegion* rgn, SkRegion::Op op) {
        build_base_rgn(rgn);
        rgn->op(fRect, op);
    }


protected:
    SkString name() override { return SkString("Regions"); }

    static void drawstr(SkCanvas* canvas, const char text[], const SkPoint& loc,
                        bool hilite) {
        SkPaint paint;
        paint.setColor(hilite ? SK_ColorRED : 0x40FF0000);
        SkFont font;
        font.setSize(SkIntToScalar(20));
        canvas->drawSimpleText(text, strlen(text), SkTextEncoding::kUTF8, loc.fX, loc.fY, font, paint);
    }

    void drawPredicates(SkCanvas* canvas, const SkPoint pts[]) {
        SkRegion rgn;
        build_base_rgn(&rgn);

        drawstr(canvas, "Intersects", pts[0], rgn.intersects(fRect));
        drawstr(canvas, "Contains", pts[1], rgn.contains(fRect));
    }

    void drawOrig(SkCanvas* canvas, bool bg) {
        SkRect      r;
        SkPaint     paint;

        paint.setStyle(SkPaint::kStroke_Style);
        if (bg)
            paint.setColor(0xFFBBBBBB);

        SkRegion rgn;
        build_base_rgn(&rgn);
        paint_rgn(canvas, rgn, paint);

        r.set(fRect);
        canvas->drawRect(r, paint);
    }

    void drawRgnOped(SkCanvas* canvas, SkRegion::Op op, SkColor color) {
        SkRegion    rgn;

        this->build_rgn(&rgn, op);

        {
            SkRegion tmp, tmp2(rgn);

            tmp = tmp2;
            tmp.translate(5, -3);

            {
                char    buffer[1000];
                SkDEBUGCODE(size_t  size = ) tmp.writeToMemory(nullptr);
                SkASSERT(size <= sizeof(buffer));
                SkDEBUGCODE(size_t  size2 = ) tmp.writeToMemory(buffer);
                SkASSERT(size == size2);

                SkRegion    tmp3;
                SkDEBUGCODE(size2 = ) tmp3.readFromMemory(buffer, 1000);
                SkASSERT(size == size2);

                SkASSERT(tmp3 == tmp);
            }

            rgn.translate(20, 30, &tmp);
            SkASSERT(rgn.isEmpty() || tmp != rgn);
            tmp.translate(-20, -30);
            SkASSERT(tmp == rgn);
        }

        this->drawOrig(canvas, true);

        SkPaint paint;
        paint.setColor((color & ~(0xFF << 24)) | (0x44 << 24));
        paint_rgn(canvas, rgn, paint);

        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(color);
        paint_rgn(canvas, rgn, paint);
    }

    void drawPathOped(SkCanvas* canvas, SkRegion::Op op, SkColor color) {
        SkRegion    rgn;
        SkPath      path;

        this->build_rgn(&rgn, op);
        rgn.getBoundaryPath(&path);

        this->drawOrig(canvas, true);

        SkPaint paint;

        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor((color & ~(0xFF << 24)) | (0x44 << 24));
        canvas->drawPath(path, paint);
        paint.setColor(color);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawPath(path, paint);
    }

    void onDrawContent(SkCanvas* canvas) override {
        if (false) { // avoid bit rot, suppress warning
            test_strokerect(canvas);
            return;
        }
        if (false) { // avoid bit rot, suppress warning
            test_text(canvas);
            return;
        }

        const SkPoint origins[] = {
            { 30*SK_Scalar1, 50*SK_Scalar1 },
            { 150*SK_Scalar1, 50*SK_Scalar1 },
        };
        this->drawPredicates(canvas, origins);

        static const struct {
            SkColor         fColor;
            const char*     fName;
            SkRegion::Op    fOp;
        } gOps[] = {
            { SK_ColorBLACK,    "Difference",   SkRegion::kDifference_Op    },
            { SK_ColorRED,      "Intersect",    SkRegion::kIntersect_Op     },
            { 0xFF008800,       "Union",        SkRegion::kUnion_Op         },
            { SK_ColorBLUE,     "XOR",          SkRegion::kXOR_Op           }
        };

        SkFont font;
        font.setSize(SK_Scalar1*24);

        this->drawOrig(canvas, false);
        canvas->save();
            canvas->translate(SkIntToScalar(200), 0);
            this->drawRgnOped(canvas, SkRegion::kUnion_Op, SK_ColorBLACK);
        canvas->restore();

        canvas->translate(0, SkIntToScalar(200));

        for (size_t op = 0; op < SK_ARRAY_COUNT(gOps); op++) {
            canvas->drawSimpleText(gOps[op].fName, strlen(gOps[op].fName), SkTextEncoding::kUTF8,
                                   SkIntToScalar(75), SkIntToScalar(50), font, SkPaint());

            this->drawRgnOped(canvas, gOps[op].fOp, gOps[op].fColor);

            canvas->save();
            canvas->translate(0, SkIntToScalar(200));
            this->drawPathOped(canvas, gOps[op].fOp, gOps[op].fColor);
            canvas->restore();

            canvas->translate(SkIntToScalar(200), 0);
        }
    }

    virtual Sample::Click* onFindClickHandler(SkScalar x, SkScalar y,
                                              skui::ModifierKey modi) override {
        return fRect.contains(SkScalarRoundToInt(x),
                              SkScalarRoundToInt(y)) ? new Click() : nullptr;
    }

    bool onClick(Click* click) override {
        fRect.offset(click->fCurr.fX - click->fPrev.fX,
                     click->fCurr.fY - click->fPrev.fY);
        return true;
    }

private:
    SkIRect    fBase, fRect;

    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new RegionView(); )
