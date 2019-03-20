/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkTypeface.h"
#include "ToolUtils.h"
#include "gm.h"

/**
 * Draws text with random parameters. The text draws each get their own clip rect. It is also
 * used as a bench to measure how well the GPU backend combines draw ops for text draws.
 */

class VariedTextGM : public skiagm::GM {
public:
    VariedTextGM(bool effectiveClip, bool lcd)
        : fEffectiveClip(effectiveClip)
        , fLCD(lcd) {
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
        fFont.setEdging(fLCD ? SkFont::Edging::kSubpixelAntiAlias : SkFont::Edging::kAntiAlias);

        SkISize size = this->getISize();
        SkScalar w = SkIntToScalar(size.fWidth);
        SkScalar h = SkIntToScalar(size.fHeight);

        static_assert(4 == SK_ARRAY_COUNT(fTypefaces), "typeface_cnt");
        fTypefaces[0] = ToolUtils::create_portable_typeface("sans-serif", SkFontStyle());
        fTypefaces[1] = ToolUtils::create_portable_typeface("sans-serif", SkFontStyle::Bold());
        fTypefaces[2] = ToolUtils::create_portable_typeface("serif", SkFontStyle());
        fTypefaces[3] = ToolUtils::create_portable_typeface("serif", SkFontStyle::Bold());

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
            fColors[i] = ToolUtils::color_to_565(fColors[i]);

            constexpr SkScalar kMinPtSize = 8.f;
            constexpr SkScalar kMaxPtSize = 32.f;

            fPtSizes[i] = random.nextRangeScalar(kMinPtSize, kMaxPtSize);

            fTypefaceIndices[i] = random.nextULessThan(SK_ARRAY_COUNT(fTypefaces));

            SkRect r;
            fPaint.setColor(fColors[i]);
            fFont.setTypeface(fTypefaces[fTypefaceIndices[i]]);
            fFont.setSize(fPtSizes[i]);

            fFont.measureText(fStrings[i].c_str(), fStrings[i].size(), kUTF8_SkTextEncoding, &r);
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
            fFont.setSize(fPtSizes[i]);
            fFont.setTypeface(fTypefaces[fTypefaceIndices[i]]);

            canvas->save();
                canvas->clipRect(fClipRects[i]);
                canvas->translate(fPositions[i].fX, fPositions[i].fY);
                canvas->drawSimpleText(fStrings[i].c_str(), fStrings[i].size(), kUTF8_SkTextEncoding,
                                       0, 0, fFont, fPaint);
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
    static constexpr int kCnt = 30;
    static constexpr int kMinLength = 15;
    static constexpr int kMaxLength = 40;

    bool        fEffectiveClip;
    bool        fLCD;
    sk_sp<SkTypeface> fTypefaces[4];
    SkPaint     fPaint;
    SkFont      fFont;

    // precomputed for each text draw
    SkString        fStrings[kCnt];
    SkColor         fColors[kCnt];
    SkScalar        fPtSizes[kCnt];
    int             fTypefaceIndices[kCnt];
    SkPoint         fPositions[kCnt];
    SkRect          fClipRects[kCnt];

    typedef skiagm::GM INHERITED;
};

DEF_GM(return new VariedTextGM(false, false);)
DEF_GM(return new VariedTextGM(true, false);)
DEF_GM(return new VariedTextGM(false, true);)
DEF_GM(return new VariedTextGM(true, true);)
