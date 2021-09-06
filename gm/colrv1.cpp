/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include <string.h>
#include <initializer_list>

namespace skiagm {

class ColrV1GM : public GM {
public:

  // TODO(drott): Consolidate test fonts.
  enum ColrV1TestType {
    kSkiaSampleFont,
    kColorFontsRepoGradients,
    kColorFontsRepoScaling,
    kColorFontsRepoExtendMode,
    kColorFontsRepoRotate,
    kColorFontsRepoSkew,
    kColorFontsRepoTransform,
    kColorFontsRepoClipBox,
    kColorFontsRepoComposite
  };

  ColrV1GM(ColrV1TestType testType, SkScalar skewX, SkScalar rotateDeg)
          : fSkewX(skewX), fRotateDeg(rotateDeg), fTestType(testType) {}

protected:
    static SkString testTypeToString(ColrV1TestType testType) {
        switch (testType) {
            case kSkiaSampleFont:
                return SkString("skia");
            case kColorFontsRepoGradients:
                return SkString("gradients");
            case kColorFontsRepoScaling:
                return SkString("scaling");
            case kColorFontsRepoExtendMode:
                return SkString("extend_mode");
            case kColorFontsRepoRotate:
                return SkString("rotate");
            case kColorFontsRepoSkew:
                return SkString("skew");
            case kColorFontsRepoTransform:
                return SkString("transform");
            case kColorFontsRepoClipBox:
                return SkString("clipbox");
            case kColorFontsRepoComposite:
                return SkString("composite");
        }
        SkASSERT(false); /* not reached */
        return SkString();
    }

    struct EmojiFont {
        sk_sp<SkTypeface> fTypeface;
        std::vector<uint16_t> fGlyphs;
        size_t bytesize() { return fGlyphs.size() * sizeof(uint16_t); }
    } fEmojiFont;

    void onOnceBeforeDraw() override {
        if (fTestType == kSkiaSampleFont) {
            fEmojiFont.fTypeface = MakeResourceAsTypeface("fonts/colrv1_samples.ttf");
            fEmojiFont.fGlyphs = {19, 33, 34, 35, 20, 21, 22, 23, 24, 25};
            return;
        }

        fEmojiFont.fTypeface = MakeResourceAsTypeface("fonts/more_samples-glyf_colr_1.ttf");

        switch (fTestType) {
            case kSkiaSampleFont:
                SkASSERT(false);
                break;
            case kColorFontsRepoGradients:
                fEmojiFont.fGlyphs = {2, 5, 6, 7, 8};
                break;
            case kColorFontsRepoScaling:
                fEmojiFont.fGlyphs = {9, 10, 11, 12, 13, 14};
                break;
            case kColorFontsRepoExtendMode:
                fEmojiFont.fGlyphs = {15, 16, 17, 18, 19, 20};
                break;
            case kColorFontsRepoRotate:
                fEmojiFont.fGlyphs = {21, 22, 23, 24};
                break;
            case kColorFontsRepoSkew:
                fEmojiFont.fGlyphs = {25, 26, 27, 28, 29, 30};
                break;
            case kColorFontsRepoTransform:
                fEmojiFont.fGlyphs = {31, 32, 33, 34};
                break;
            case kColorFontsRepoClipBox:
                fEmojiFont.fGlyphs = {35, 36, 37, 38, 39};
                break;
            case kColorFontsRepoComposite:
                fEmojiFont.fGlyphs = {40, 41, 42, 43, 44, 45, 46};
                break;
        }
    }

    SkString onShortName() override {
        SkString gm_name = SkStringPrintf("colrv1_%s_samples",
                                          testTypeToString(fTestType).c_str());
        if (fSkewX) {
            gm_name.append("_skew");
        }

        if (fRotateDeg) {
            gm_name.append("_rotate");
        }
        return gm_name;
    }

    SkISize onISize() override { return SkISize::Make(1400, 600); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        canvas->drawColor(SK_ColorWHITE);
        SkPaint paint;

        canvas->translate(200, 20);

        if (!fEmojiFont.fTypeface) {
          *errorMsg = "Did not recognize COLR v1 font format.";
          return DrawResult::kSkip;
        }

        canvas->rotate(fRotateDeg);
        canvas->skew(fSkewX, 0);

        SkFont font(fEmojiFont.fTypeface);

        SkFontMetrics metrics;
        SkScalar y = 0;
        for (SkScalar textSize : { 12, 18, 30, 120 }) {
            font.setSize(textSize);
            font.getMetrics(&metrics);
            y += -metrics.fAscent;
            canvas->drawSimpleText(fEmojiFont.fGlyphs.data(),
                                   fEmojiFont.bytesize(),
                                   SkTextEncoding::kGlyphID,
                                   10, y, font, paint);
            y += metrics.fDescent + metrics.fLeading;
        }
        return DrawResult::kOk;
    }

private:
    using INHERITED = GM;
    SkScalar fSkewX;
    SkScalar fRotateDeg;
    ColrV1TestType fTestType;
};

DEF_GM(return new ColrV1GM(ColrV1GM::kSkiaSampleFont, 0.f, 0.f);)
DEF_GM(return new ColrV1GM(ColrV1GM::kSkiaSampleFont, -0.5f, 0.f);)
DEF_GM(return new ColrV1GM(ColrV1GM::kSkiaSampleFont, 0.f, 20.f);)
DEF_GM(return new ColrV1GM(ColrV1GM::kSkiaSampleFont, -0.5f, 20.f);)
DEF_GM(return new ColrV1GM(ColrV1GM::kColorFontsRepoGradients, 0.f, 0.f);)
DEF_GM(return new ColrV1GM(ColrV1GM::kColorFontsRepoScaling, 0.f, 0.f);)
DEF_GM(return new ColrV1GM(ColrV1GM::kColorFontsRepoExtendMode, 0.f, 0.f);)
DEF_GM(return new ColrV1GM(ColrV1GM::kColorFontsRepoRotate, 0.f, 0.f);)
DEF_GM(return new ColrV1GM(ColrV1GM::kColorFontsRepoSkew, 0.f, 0.f);)
DEF_GM(return new ColrV1GM(ColrV1GM::kColorFontsRepoTransform, 0.f, 0.f);)
DEF_GM(return new ColrV1GM(ColrV1GM::kColorFontsRepoClipBox, 0.f, 0.f);)
DEF_GM(return new ColrV1GM(ColrV1GM::kColorFontsRepoClipBox, -0.5f, 20.f);)
DEF_GM(return new ColrV1GM(ColrV1GM::kColorFontsRepoComposite, 0.f, 0.f);)

}  // namespace skiagm
