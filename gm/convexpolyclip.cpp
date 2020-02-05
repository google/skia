/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "src/core/SkClipOpPriv.h"
#include "src/core/SkTLList.h"
#include "tools/ToolUtils.h"

static SkBitmap make_bmp(int w, int h) {
    SkBitmap bmp;
    bmp.allocN32Pixels(w, h, true);

    SkCanvas canvas(bmp);
    SkScalar wScalar = SkIntToScalar(w);
    SkScalar hScalar = SkIntToScalar(h);

    SkPoint     pt = { wScalar / 2, hScalar / 2 };

    SkScalar    radius = 3 * std::max(wScalar, hScalar);

    SkColor colors[] = {SK_ColorDKGRAY,
                        ToolUtils::color_to_565(0xFF222255),
                        ToolUtils::color_to_565(0xFF331133),
                        ToolUtils::color_to_565(0xFF884422),
                        ToolUtils::color_to_565(0xFF000022),
                        SK_ColorWHITE,
                        ToolUtils::color_to_565(0xFFAABBCC)};

    SkScalar    pos[] = {0,
                         SK_Scalar1 / 6,
                         2 * SK_Scalar1 / 6,
                         3 * SK_Scalar1 / 6,
                         4 * SK_Scalar1 / 6,
                         5 * SK_Scalar1 / 6,
                         SK_Scalar1};

    SkPaint paint;
    SkRect rect = SkRect::MakeWH(wScalar, hScalar);
    SkMatrix mat = SkMatrix::I();
    for (int i = 0; i < 4; ++i) {
        paint.setShader(SkGradientShader::MakeRadial(
                        pt, radius,
                        colors, pos,
                        SK_ARRAY_COUNT(colors),
                        SkTileMode::kRepeat,
                        0, &mat));
        canvas.drawRect(rect, paint);
        rect.inset(wScalar / 8, hScalar / 8);
        mat.preTranslate(6 * wScalar, 6 * hScalar);
        mat.postScale(SK_Scalar1 / 3, SK_Scalar1 / 3);
    }

    SkFont font(ToolUtils::create_portable_typeface(), wScalar / 2.2f);

    paint.setShader(nullptr);
    paint.setColor(SK_ColorLTGRAY);
    constexpr char kTxt[] = "Skia";
    SkPoint texPos = { wScalar / 17, hScalar / 2 + font.getSize() / 2.5f };
    canvas.drawSimpleText(kTxt, SK_ARRAY_COUNT(kTxt)-1, SkTextEncoding::kUTF8,
                          texPos.fX, texPos.fY, font, paint);
    paint.setColor(SK_ColorBLACK);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(SK_Scalar1);
    canvas.drawSimpleText(kTxt, SK_ARRAY_COUNT(kTxt)-1, SkTextEncoding::kUTF8,
                          texPos.fX, texPos.fY, font, paint);
    return bmp;
}

namespace skiagm {
/**
 * This GM tests convex polygon clips.
 */
class ConvexPolyClip : public GM {
public:
    ConvexPolyClip() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        return SkString("convex_poly_clip");
    }

    SkISize onISize() override {
        // When benchmarking the saveLayer set of draws is skipped.
        int w = 435;
        if (kBench_Mode != this->getMode()) {
            w *= 2;
        }
        return SkISize::Make(w, 540);
    }

    void onOnceBeforeDraw() override {
        SkPath tri;
        tri.moveTo(5.f, 5.f);
        tri.lineTo(100.f, 20.f);
        tri.lineTo(15.f, 100.f);

        fClips.addToTail()->setPath(tri);

        SkPath hexagon;
        constexpr SkScalar kRadius = 45.f;
        const SkPoint center = { kRadius, kRadius };
        for (int i = 0; i < 6; ++i) {
            SkScalar angle = 2 * SK_ScalarPI * i / 6;
            SkPoint point = { SkScalarCos(angle), SkScalarSin(angle) };
            point.scale(kRadius);
            point = center + point;
            if (0 == i) {
                hexagon.moveTo(point);
            } else {
                hexagon.lineTo(point);
            }
        }
        fClips.addToTail()->setPath(hexagon);

        SkMatrix scaleM;
        scaleM.setScale(1.1f, 0.4f, kRadius, kRadius);
        hexagon.transform(scaleM);
        fClips.addToTail()->setPath(hexagon);

        fClips.addToTail()->setRect(SkRect::MakeXYWH(8.3f, 11.6f, 78.2f, 72.6f));

        SkPath rotRect;
        SkRect rect = SkRect::MakeLTRB(10.f, 12.f, 80.f, 86.f);
        rotRect.addRect(rect);
        SkMatrix rotM;
        rotM.setRotate(23.f, rect.centerX(), rect.centerY());
        rotRect.transform(rotM);
        fClips.addToTail()->setPath(rotRect);

        fBmp = make_bmp(100, 100);
    }

    void onDraw(SkCanvas* canvas) override {
        SkScalar y = 0;
        constexpr SkScalar kMargin = 10.f;

        SkPaint bgPaint;
        bgPaint.setAlpha(0x15);
        SkISize size = canvas->getBaseLayerSize();
        canvas->drawBitmapRect(fBmp, SkRect::MakeIWH(size.fWidth, size.fHeight), &bgPaint);

        constexpr char kTxt[] = "Clip Me!";
        SkFont         font(ToolUtils::create_portable_typeface(), 23);
        SkScalar textW = font.measureText(kTxt, SK_ARRAY_COUNT(kTxt)-1, SkTextEncoding::kUTF8);
        SkPaint txtPaint;
        txtPaint.setColor(SK_ColorDKGRAY);

        SkScalar startX = 0;
        int testLayers = kBench_Mode != this->getMode();
        for (int doLayer = 0; doLayer <= testLayers; ++doLayer) {
            for (ClipList::Iter iter(fClips, ClipList::Iter::kHead_IterStart);
                 iter.get();
                 iter.next()) {
                const Clip* clip = iter.get();
                SkScalar x = startX;
                for (int aa = 0; aa < 2; ++aa) {
                    if (doLayer) {
                        SkRect bounds;
                        clip->getBounds(&bounds);
                        bounds.outset(2, 2);
                        bounds.offset(x, y);
                        canvas->saveLayer(&bounds, nullptr);
                    } else {
                        canvas->save();
                    }
                    canvas->translate(x, y);
                    clip->setOnCanvas(canvas, kIntersect_SkClipOp, SkToBool(aa));
                    canvas->drawBitmap(fBmp, 0, 0);
                    canvas->restore();
                    x += fBmp.width() + kMargin;
                }
                for (int aa = 0; aa < 2; ++aa) {

                    SkPaint clipOutlinePaint;
                    clipOutlinePaint.setAntiAlias(true);
                    clipOutlinePaint.setColor(0x50505050);
                    clipOutlinePaint.setStyle(SkPaint::kStroke_Style);
                    clipOutlinePaint.setStrokeWidth(0);

                    if (doLayer) {
                        SkRect bounds;
                        clip->getBounds(&bounds);
                        bounds.outset(2, 2);
                        bounds.offset(x, y);
                        canvas->saveLayer(&bounds, nullptr);
                    } else {
                        canvas->save();
                    }
                    canvas->translate(x, y);
                    SkPath closedClipPath;
                    clip->asClosedPath(&closedClipPath);
                    canvas->drawPath(closedClipPath, clipOutlinePaint);
                    clip->setOnCanvas(canvas, kIntersect_SkClipOp, SkToBool(aa));
                    canvas->scale(1.f, 1.8f);
                    canvas->drawSimpleText(kTxt, SK_ARRAY_COUNT(kTxt)-1, SkTextEncoding::kUTF8,
                                     0, 1.5f * font.getSize(), font, txtPaint);
                    canvas->restore();
                    x += textW + 2 * kMargin;
                }
                y += fBmp.height() + kMargin;
            }
            y = 0;
            startX += 2 * fBmp.width() + SkScalarCeilToInt(2 * textW) + 6 * kMargin;
        }
    }

    bool runAsBench() const override { return true; }

private:
    class Clip {
    public:
        enum ClipType {
            kNone_ClipType,
            kPath_ClipType,
            kRect_ClipType
        };

        Clip () : fClipType(kNone_ClipType) {}

        void setOnCanvas(SkCanvas* canvas, SkClipOp op, bool aa) const {
            switch (fClipType) {
                case kPath_ClipType:
                    canvas->clipPath(fPath, op, aa);
                    break;
                case kRect_ClipType:
                    canvas->clipRect(fRect, op, aa);
                    break;
                case kNone_ClipType:
                    SkDEBUGFAIL("Uninitialized Clip.");
                    break;
            }
        }

        void asClosedPath(SkPath* path) const {
            switch (fClipType) {
                case kPath_ClipType:
                    *path = fPath;
                    path->close();
                    break;
                case kRect_ClipType:
                    path->reset();
                    path->addRect(fRect);
                    break;
                case kNone_ClipType:
                    SkDEBUGFAIL("Uninitialized Clip.");
                    break;
            }
        }

        void setPath(const SkPath& path) {
            fClipType = kPath_ClipType;
            fPath = path;
        }

        void setRect(const SkRect& rect) {
            fClipType = kRect_ClipType;
            fRect = rect;
            fPath.reset();
        }

        ClipType getType() const { return fClipType; }

        void getBounds(SkRect* bounds) const {
            switch (fClipType) {
                case kPath_ClipType:
                    *bounds = fPath.getBounds();
                    break;
                case kRect_ClipType:
                    *bounds = fRect;
                    break;
                case kNone_ClipType:
                    SkDEBUGFAIL("Uninitialized Clip.");
                    break;
            }
        }

    private:
        ClipType fClipType;
        SkPath fPath;
        SkRect fRect;
    };

    typedef SkTLList<Clip, 1> ClipList;
    ClipList         fClips;
    SkBitmap         fBmp;

    typedef GM INHERITED;
};

DEF_GM(return new ConvexPolyClip;)
}
