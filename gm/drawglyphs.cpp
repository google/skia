/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkSpan.h"
#include "include/private/base/SkTDArray.h"
#include "src/base/SkZip.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

static const char gText[] = "Call me Ishmael. Some years ago—never mind how long precisely";

class DrawGlyphsGM : public skiagm::GM {
public:
    void onOnceBeforeDraw() override {
        fTypeface = ToolUtils::CreatePortableTypeface("serif", SkFontStyle());
        fFont = SkFont(fTypeface);
        fFont.setSubpixel(true);
        fFont.setSize(18);
        const size_t txtLen = strlen(gText);
        fGlyphCount = fFont.countText(gText, txtLen, SkTextEncoding::kUTF8);

        fGlyphs.append(fGlyphCount);
        fFont.textToGlyphs(gText, txtLen, SkTextEncoding::kUTF8, fGlyphs.begin(), fGlyphCount);

        fPositions.append(fGlyphCount);
        fFont.getPos(fGlyphs.begin(), fGlyphCount, fPositions.begin());
        auto positions = SkSpan(fPositions.begin(), fGlyphCount);

        fLength = positions.back().x() - positions.front().x();
        fRadius = fLength / SK_FloatPI;
        fXforms.append(fGlyphCount);

        for (auto [xform, pos] : SkMakeZip(fXforms.begin(), positions)) {
            const SkScalar lengthToGlyph = pos.x() - positions.front().x();
            const SkScalar angle = SK_FloatPI * (fLength - lengthToGlyph) / fLength;
            const SkScalar cos = std::cos(angle);
            const SkScalar sin = std::sin(angle);
            xform = SkRSXform::Make(sin, cos, fRadius*cos, -fRadius*sin);
        }
    }

    SkString getName() const override { return SkString("drawglyphs"); }

    SkISize getISize() override { return SkISize::Make(640, 480); }

    void onDraw(SkCanvas* canvas) override {
        canvas->drawGlyphs(fGlyphCount, fGlyphs.begin(), fPositions.begin(), {50, 100}, fFont,
                           SkPaint{});

        canvas->drawGlyphs(fGlyphCount, fGlyphs.begin(), fPositions.begin(), {50, 120}, fFont,
                           SkPaint{});

        // Check bounding box calculation.
        for (auto& pos : fPositions) {
            pos += {0, -500};
        }
        canvas->drawGlyphs(fGlyphCount, fGlyphs.begin(), fPositions.begin(), {50, 640}, fFont,
                           SkPaint{});

        canvas->drawGlyphs(fGlyphCount, fGlyphs.begin(), fXforms.begin(),
                           {50 + fLength / 2, 160 + fRadius}, fFont, SkPaint{});

        // TODO: add tests for cluster versions of drawGlyphs.
    }

private:
    sk_sp<SkTypeface>   fTypeface;
    SkFont fFont;
    SkTDArray<SkGlyphID> fGlyphs;
    SkTDArray<SkPoint>   fPositions;
    SkTDArray<SkRSXform> fXforms;
    int fGlyphCount;
    SkScalar fRadius;
    SkScalar fLength;
};

DEF_GM(return new DrawGlyphsGM{};)
