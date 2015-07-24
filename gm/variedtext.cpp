/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkTypeface.h"
#include "SkRandom.h"

/**
 * Draws text with random parameters. The text draws each get their own clip rect. It is also
 * used as a bench to measure how well the GPU backend batches text draws.
 */

class VariedTextGM : public skiagm::GM {
public:
    VariedTextGM(bool effectiveClip, bool lcd)
        : fEffectiveClip(effectiveClip)
        , fLCD(lcd) {
        memset(fTypefacesToUnref, 0, sizeof(fTypefacesToUnref));
    }

    ~VariedTextGM() {
        for (size_t i = 0; i < SK_ARRAY_COUNT(fTypefacesToUnref); ++i) {
            SkSafeUnref(fTypefacesToUnref[i]);
        }
    }

protected:
    SkString onShortName() override {
        SkString name("varied_text");
        if (fEffectiveClip) {
            name.append("_clipped");
        } else {
            name.append("_ignorable_clip");
        }
        if (fLCD) {
            name.append("_lcd");
        } else {
            name.append("_no_lcd");
        }
        return name;
    }

    SkISize onISize() override {
        return SkISize::Make(640, 480);
    }

    void onOnceBeforeDraw() override {
        fPaint.setAntiAlias(true);
        fPaint.setLCDRenderText(fLCD);

        SkISize size = this->getISize();
        SkScalar w = SkIntToScalar(size.fWidth);
        SkScalar h = SkIntToScalar(size.fHeight);

        SK_COMPILE_ASSERT(4 == SK_ARRAY_COUNT(fTypefacesToUnref), typeface_cnt);
        fTypefacesToUnref[0] = sk_tool_utils::create_portable_typeface("sans-serif", SkTypeface::kNormal);
        fTypefacesToUnref[1] = sk_tool_utils::create_portable_typeface("sans-serif", SkTypeface::kBold);
        fTypefacesToUnref[2] = sk_tool_utils::create_portable_typeface("serif", SkTypeface::kNormal);
        fTypefacesToUnref[3] = sk_tool_utils::create_portable_typeface("serif", SkTypeface::kBold);

        SkRandom random;
        for (int i = 0; i < kCnt; ++i) {
            int length = random.nextRangeU(kMinLength, kMaxLength);
            char text[kMaxLength];
            for (int j = 0; j < length; ++j) {
                text[j] = (char)random.nextRangeU('!', 'z');
            }
            fStrings[i].set(text, length);

            fColors[i] = random.nextU();
            fColors[i] |= 0xFF000000;
            fColors[i] = sk_tool_utils::color_to_565(fColors[i]);

            static const SkScalar kMinPtSize = 8.f;
            static const SkScalar kMaxPtSize = 32.f;

            fPtSizes[i] = random.nextRangeScalar(kMinPtSize, kMaxPtSize);

            fTypefaces[i] = fTypefacesToUnref[
                random.nextULessThan(SK_ARRAY_COUNT(fTypefacesToUnref))];

            SkRect r;
            fPaint.setColor(fColors[i]);
            fPaint.setTypeface(fTypefaces[i]);
            fPaint.setTextSize(fPtSizes[i]);

            fPaint.measureText(fStrings[i].c_str(), fStrings[i].size(), &r);
            // safeRect is set of x,y positions where we can draw the string without hitting
            // the GM's border.
            SkRect safeRect = SkRect::MakeLTRB(-r.fLeft, -r.fTop, w - r.fRight, h - r.fBottom);
            if (safeRect.isEmpty()) {
                // If we don't fit then just don't worry about how we get cliped to the device
                // border.
                safeRect = SkRect::MakeWH(w, h);
            }
            fPositions[i].fX = random.nextRangeScalar(safeRect.fLeft, safeRect.fRight);
            fPositions[i].fY = random.nextRangeScalar(safeRect.fTop, safeRect.fBottom);

            fClipRects[i] = r;
            fClipRects[i].offset(fPositions[i].fX, fPositions[i].fY);
            fClipRects[i].outset(2.f, 2.f);

            if (fEffectiveClip) {
                fClipRects[i].fRight -= 0.25f * fClipRects[i].width();
            }
        }
    }

    void onDraw(SkCanvas* canvas) override {
        for (int i = 0; i < kCnt; ++i) {
            fPaint.setColor(fColors[i]);
            fPaint.setTextSize(fPtSizes[i]);
            fPaint.setTypeface(fTypefaces[i]);

            canvas->save();
                canvas->clipRect(fClipRects[i]);
                canvas->translate(fPositions[i].fX, fPositions[i].fY);
                canvas->drawText(fStrings[i].c_str(), fStrings[i].size(), 0, 0, fPaint);
            canvas->restore();
        }

        // Visualize the clips, but not in bench mode.
        if (kBench_Mode != this->getMode()) {
            SkPaint wirePaint;
            wirePaint.setAntiAlias(true);
            wirePaint.setStrokeWidth(0);
            wirePaint.setStyle(SkPaint::kStroke_Style);
            for (int i = 0; i < kCnt; ++i) {
                canvas->drawRect(fClipRects[i], wirePaint);
            }
        }
    }

    bool runAsBench() const override { return true; }

private:
    static const int kCnt = 30;
    static const int kMinLength = 15;
    static const int kMaxLength = 40;

    bool        fEffectiveClip;
    bool        fLCD;
    SkTypeface* fTypefacesToUnref[4];
    SkPaint     fPaint;

    // precomputed for each text draw
    SkString        fStrings[kCnt];
    SkColor         fColors[kCnt];
    SkScalar        fPtSizes[kCnt];
    SkTypeface*     fTypefaces[kCnt];
    SkPoint         fPositions[kCnt];
    SkRect          fClipRects[kCnt];

    typedef skiagm::GM INHERITED;
};

DEF_GM( return SkNEW(VariedTextGM(false, false)); )
DEF_GM( return SkNEW(VariedTextGM(true, false)); )
DEF_GM( return SkNEW(VariedTextGM(false, true)); )
DEF_GM( return SkNEW(VariedTextGM(true, true)); )
