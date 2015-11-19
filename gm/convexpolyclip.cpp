
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkBitmap.h"
#include "SkGradientShader.h"
#include "SkPath.h"
#include "SkTLList.h"

static SkBitmap make_bmp(int w, int h) {
    SkBitmap bmp;
    bmp.allocN32Pixels(w, h, true);

    SkCanvas canvas(bmp);
    SkScalar wScalar = SkIntToScalar(w);
    SkScalar hScalar = SkIntToScalar(h);

    SkPoint     pt = { wScalar / 2, hScalar / 2 };

    SkScalar    radius = 3 * SkMaxScalar(wScalar, hScalar);

    SkColor     colors[] = { sk_tool_utils::color_to_565(SK_ColorDKGRAY),
                             sk_tool_utils::color_to_565(0xFF222255),
                             sk_tool_utils::color_to_565(0xFF331133),
                             sk_tool_utils::color_to_565(0xFF884422),
                             sk_tool_utils::color_to_565(0xFF000022), SK_ColorWHITE,
                             sk_tool_utils::color_to_565(0xFFAABBCC) };

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
        paint.setShader(SkGradientShader::CreateRadial(
                        pt, radius,
                        colors, pos,
                        SK_ARRAY_COUNT(colors),
                        SkShader::kRepeat_TileMode,
                        0, &mat))->unref();
        canvas.drawRect(rect, paint);
        rect.inset(wScalar / 8, hScalar / 8);
        mat.preTranslate(6 * wScalar, 6 * hScalar);
        mat.postScale(SK_Scalar1 / 3, SK_Scalar1 / 3);
    }

    paint.setAntiAlias(true);
    sk_tool_utils::set_portable_typeface(&paint);
    paint.setTextSize(wScalar / 2.2f);
    paint.setShader(0);
    paint.setColor(sk_tool_utils::color_to_565(SK_ColorLTGRAY));
    static const char kTxt[] = "Skia";
    SkPoint texPos = { wScalar / 17, hScalar / 2 + paint.getTextSize() / 2.5f };
    canvas.drawText(kTxt, SK_ARRAY_COUNT(kTxt)-1, texPos.fX, texPos.fY, paint);
    paint.setColor(SK_ColorBLACK);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(SK_Scalar1);
    canvas.drawText(kTxt, SK_ARRAY_COUNT(kTxt)-1, texPos.fX, texPos.fY, paint);
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
        static const SkScalar kRadius = 45.f;
        const SkPoint center = { kRadius, kRadius };
        for (int i = 0; i < 6; ++i) {
            SkScalar angle = 2 * SK_ScalarPI * i / 6;
            SkPoint point;
            point.fY = SkScalarSinCos(angle, &point.fX);
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
        static const SkScalar kMargin = 10.f;

        SkPaint bgPaint;
        bgPaint.setAlpha(0x15);
        SkISize size = canvas->getDeviceSize();
        canvas->drawBitmapRect(fBmp, SkRect::MakeIWH(size.fWidth, size.fHeight), &bgPaint);

        static const char kTxt[] = "Clip Me!";
        SkPaint txtPaint;
        txtPaint.setTextSize(23.f);
        txtPaint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&txtPaint);
        txtPaint.setColor(sk_tool_utils::color_to_565(SK_ColorDKGRAY));
        SkScalar textW = txtPaint.measureText(kTxt, SK_ARRAY_COUNT(kTxt)-1);

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
                    clip->setOnCanvas(canvas, SkRegion::kIntersect_Op, SkToBool(aa));
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
                    clip->setOnCanvas(canvas, SkRegion::kIntersect_Op, SkToBool(aa));
                    canvas->scale(1.f, 1.8f);
                    canvas->drawText(kTxt, SK_ARRAY_COUNT(kTxt)-1,
                                     0, 1.5f * txtPaint.getTextSize(),
                                     txtPaint);
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

        void setOnCanvas(SkCanvas* canvas, SkRegion::Op op, bool aa) const {
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
