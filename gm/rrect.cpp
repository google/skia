/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradient.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

typedef void (*InsetProc)(const SkRRect&, SkScalar dx, SkScalar dy, SkRRect*);

static void inset0(const SkRRect& src, SkScalar dx, SkScalar dy, SkRRect* dst) {
    SkRect r = src.rect();

    r.inset(dx, dy);
    if (r.isEmpty()) {
        dst->setEmpty();
        return;
    }

    SkVector radii[4];
    for (int i = 0; i < 4; ++i) {
        radii[i] = src.radii((SkRRect::Corner)i);
    }
    for (int i = 0; i < 4; ++i) {
        radii[i].fX -= dx;
        radii[i].fY -= dy;
    }
    dst->setRectRadii(r, radii);
}

static void inset1(const SkRRect& src, SkScalar dx, SkScalar dy, SkRRect* dst) {
    SkRect r = src.rect();

    r.inset(dx, dy);
    if (r.isEmpty()) {
        dst->setEmpty();
        return;
    }

    SkVector radii[4];
    for (int i = 0; i < 4; ++i) {
        radii[i] = src.radii((SkRRect::Corner)i);
    }
    dst->setRectRadii(r, radii);
}

static void inset2(const SkRRect& src, SkScalar dx, SkScalar dy, SkRRect* dst) {
    SkRect r = src.rect();

    r.inset(dx, dy);
    if (r.isEmpty()) {
        dst->setEmpty();
        return;
    }

    SkVector radii[4];
    for (int i = 0; i < 4; ++i) {
        radii[i] = src.radii((SkRRect::Corner)i);
    }
    for (int i = 0; i < 4; ++i) {
        if (radii[i].fX) {
            radii[i].fX -= dx;
        }
        if (radii[i].fY) {
            radii[i].fY -= dy;
        }
    }
    dst->setRectRadii(r, radii);
}

static SkScalar prop(SkScalar radius, SkScalar newSize, SkScalar oldSize) {
    return newSize * radius / oldSize;
}

static void inset3(const SkRRect& src, SkScalar dx, SkScalar dy, SkRRect* dst) {
    SkRect r = src.rect();

    r.inset(dx, dy);
    if (r.isEmpty()) {
        dst->setEmpty();
        return;
    }

    SkVector radii[4];
    for (int i = 0; i < 4; ++i) {
        radii[i] = src.radii((SkRRect::Corner)i);
    }
    for (int i = 0; i < 4; ++i) {
        radii[i].fX = prop(radii[i].fX, r.width(), src.rect().width());
        radii[i].fY = prop(radii[i].fY, r.height(), src.rect().height());
    }
    dst->setRectRadii(r, radii);
}

static void draw_rrect_color(SkCanvas* canvas, const SkRRect& rrect) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);

    if (rrect.isRect()) {
        paint.setColor(SK_ColorRED);
    } else if (rrect.isOval()) {
        paint.setColor(ToolUtils::color_to_565(0xFF008800));
    } else if (rrect.isSimple()) {
        paint.setColor(SK_ColorBLUE);
    } else {
        paint.setColor(SK_ColorBLACK);
    }
    canvas->drawRRect(rrect, paint);
}

static void drawrr(SkCanvas* canvas, const SkRRect& rrect, InsetProc proc) {
    SkRRect rr;
    for (SkScalar d = -30; d <= 30; d += 5) {
        proc(rrect, d, d, &rr);
        draw_rrect_color(canvas, rr);
    }
}

class RRectGM : public skiagm::GM {
public:
    RRectGM() {}

protected:
    SkString getName() const override { return SkString("rrect"); }

    SkISize getISize() override { return SkISize::Make(820, 710); }

    void onDraw(SkCanvas* canvas) override {
        constexpr InsetProc insetProcs[] = {
            inset0, inset1, inset2, inset3
        };

        SkRRect rrect[4];
        SkRect r = { 0, 0, 120, 100 };
        SkVector radii[4] = {
            { 0, 0 }, { 30, 1 }, { 10, 40 }, { 40, 40 }
        };

        rrect[0].setRect(r);
        rrect[1].setOval(r);
        rrect[2].setRectXY(r, 20, 20);
        rrect[3].setRectRadii(r, radii);

        canvas->translate(50.5f, 50.5f);
        for (size_t j = 0; j < std::size(insetProcs); ++j) {
            canvas->save();
            for (size_t i = 0; i < std::size(rrect); ++i) {
                drawrr(canvas, rrect[i], insetProcs[j]);
                canvas->translate(200, 0);
            }
            canvas->restore();
            canvas->translate(0, 170);
        }
    }

private:
    using INHERITED = GM;
};

DEF_GM( return new RRectGM; )

class RRectBlurGM : public skiagm::GM {
public:
    RRectBlurGM() {}

protected:
    SkString getName() const override { return SkString("rrect_blurs"); }

    static constexpr int kWidth = 300;
    static constexpr int kHeight = 400;
    // how much to exagerate the diffs
    static constexpr int kDiffMaginification = 16;
    static constexpr bool kPrintDiffMetrics = false;

    SkISize getISize() override { return SkISize::Make(kWidth, kHeight); }

    static void draw_blurry_rrect(
            SkCanvas* canvas, int cellY, sk_sp<SkMaskFilter> mf, SkColor color, const SkRRect& rr) {
        const int kCellSize = 100;
        SkPaint rrectPaint;
        rrectPaint.setColor(color);
        rrectPaint.setMaskFilter(mf);

        const int paddingX = (kCellSize - rr.width()) / 2;
        const int paddingY = (kCellSize - rr.height()) / 2;
        const SkRRect left = rr.makeOffset(paddingX, paddingY + cellY);
        canvas->drawRRect(left, rrectPaint);

        const SkRRect right = rr.makeOffset(2 * kCellSize + paddingX, paddingY + cellY);
        canvas->drawPath(SkPath::RRect(right), rrectPaint);

        // In an ideal world, there would be no diffs at all between the two drawing
        // methods. The point of this gm is to show those differences and allow us to
        // measure the differences.
        SkBitmap leftBitmap;
        leftBitmap.allocPixels(SkImageInfo::MakeN32Premul(kCellSize, kCellSize));
        SkImageInfo infoLeft = leftBitmap.info();
        if (!canvas->readPixels(infoLeft,
                                leftBitmap.pixmap().writable_addr(),
                                infoLeft.minRowBytes(),
                                0,
                                cellY)) {
            return;
        }

        SkBitmap rightBitmap;
        rightBitmap.allocPixels(SkImageInfo::MakeN32Premul(kCellSize, kCellSize));
        SkImageInfo infoRight = rightBitmap.info();
        if (!canvas->readPixels(infoRight,
                                rightBitmap.pixmap().writable_addr(),
                                infoRight.minRowBytes(),
                                2 * kCellSize,
                                cellY)) {
            return;
        }

        int diffPixels = 0;
        SkBitmap diffBitmap;
        diffBitmap.allocPixels(SkImageInfo::MakeN32Premul(kCellSize, kCellSize));
        for (int y = 0; y < kCellSize; ++y) {
            for (int x = 0; x < kCellSize; ++x) {
                SkColor leftColor = leftBitmap.getColor(x, y);
                SkColor rightColor = rightBitmap.getColor(x, y);
                // Add up the diffs in the 4 channels, then treat that as how bright
                // to draw the diff
                int diff = abs((int)(SkColorGetA(leftColor) - SkColorGetA(rightColor))) +
                           abs((int)(SkColorGetR(leftColor) - SkColorGetR(rightColor))) +
                           abs((int)(SkColorGetG(leftColor) - SkColorGetG(rightColor))) +
                           abs((int)(SkColorGetB(leftColor) - SkColorGetB(rightColor)));
                SkASSERT(diff >= 0);
                const U8CPU grey = std::min(diff * kDiffMaginification, 255);
                if (grey > 0) {
                    diffPixels++;
                }
                *diffBitmap.pixmap().writable_addr32(x, y) = SkColorSetARGB(0xFF, grey, grey, grey);
            }
        }
        if (kPrintDiffMetrics) {
            SkDebugf("%d pixels diff\n", diffPixels);
        }

        canvas->writePixels(diffBitmap, kCellSize, cellY);
    }

    void onDraw(SkCanvas* canvas) override {
        // Because of the read/write pixels, this doesn't draw right if viewer zooms in.
        canvas->resetMatrix();
        canvas->clear(SK_ColorDKGRAY);

        draw_blurry_rrect(canvas, 0,
                          SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 1.0f, false /*=respectCTM*/),
                          SK_ColorWHITE,
                          SkRRect::MakeRectXY(SkRect::MakeWH(50, 50), 10, 15));

        draw_blurry_rrect(canvas, 100,
                          SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 0.5f, false /*=respectCTM*/),
                          SK_ColorYELLOW,
                          SkRRect::MakeRectXY(SkRect::MakeWH(60, 80), 3.1f, 1.5f));

        SkRRect rr;
        rr.setNinePatch(SkRect::MakeWH(70, 80),
                        5,   // left
                        10,  // top
                        13,  // right
                        7);  // bottom
        draw_blurry_rrect(canvas, 200,
                          SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 2.5f, false /*=respectCTM*/),
                          SkColorSetARGB(255, 200, 100, 30),
                          rr);

        SkVector radii[4] = {{0, 0}, {20, 1}, {10, 30}, {30, 30}};
        rr.setRectRadii(SkRect::MakeWH(90, 90), radii);
        draw_blurry_rrect(canvas, 300,
                          SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 1.1f, false /*=respectCTM*/),
                          SkColorSetARGB(255, 35, 120, 220),
                          rr);

        // labels after to avoid contaminating the diffs
        SkPaint labelPaint;
        labelPaint.setColor(SK_ColorWHITE);
        labelPaint.setAntiAlias(true);
        SkFont font = ToolUtils::DefaultPortableFont();
        canvas->drawString("drawRRect", 15, 15, font, labelPaint);
        canvas->drawString("diff", 140, 15, font, labelPaint);
        canvas->drawString("drawPath", 220, 15, font, labelPaint);
        canvas->drawLine(100, 0, 100, kHeight, labelPaint);
        canvas->drawLine(200, 0, 200, kHeight, labelPaint);
        canvas->drawLine(0, 100, kWidth, 100, labelPaint);
        canvas->drawLine(0, 200, kWidth, 200, labelPaint);
        canvas->drawLine(0, 300, kWidth, 300, labelPaint);
    }
};

DEF_GM(return new RRectBlurGM;)
