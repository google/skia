/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkString.h"
#include "SkTypeface.h"
#include "SkTypes.h"

static void getGlyphPositions(const SkPaint& paint, const uint16_t glyphs[],
                             int count, SkScalar x, SkScalar y, SkPoint pos[]) {
    SkASSERT(SkPaint::kGlyphID_TextEncoding == paint.getTextEncoding());

    SkAutoSTMalloc<128, SkScalar> widthStorage(count);
    SkScalar* widths = widthStorage.get();
    paint.getTextWidths(glyphs, count * sizeof(uint16_t), widths);

    for (int i = 0; i < count; ++i) {
        pos[i].set(x, y);
        x += widths[i];
    }
}

static void applyKerning(SkPoint pos[], const int32_t adjustments[], int count,
                         const SkPaint& paint) {
    SkScalar scale = paint.getTextSize() / paint.getTypeface()->getUnitsPerEm();

    SkScalar globalAdj = 0;
    for (int i = 0; i < count - 1; ++i) {
        globalAdj += adjustments[i] * scale;
        pos[i + 1].fX += globalAdj;
    }
}

static void drawKernText(SkCanvas* canvas, const void* text, size_t len,
                         SkScalar x, SkScalar y, const SkPaint& paint) {
    SkTypeface* face = paint.getTypeface();
    if (!face) {
        canvas->drawText(text, len, x, y, paint);
        return;
    }

    SkAutoSTMalloc<128, uint16_t> glyphStorage(len);
    uint16_t* glyphs = glyphStorage.get();
    int glyphCount = paint.textToGlyphs(text, len, glyphs);
    if (glyphCount < 1) {
        return;
    }

    SkAutoSTMalloc<128, int32_t> adjustmentStorage(glyphCount - 1);
    int32_t* adjustments = adjustmentStorage.get();
    if (!face->getKerningPairAdjustments(glyphs, glyphCount, adjustments)) {
        canvas->drawText(text, len, x, y, paint);
        return;
    }

    SkPaint glyphPaint(paint);
    glyphPaint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);

    SkAutoSTMalloc<128, SkPoint> posStorage(glyphCount);
    SkPoint* pos = posStorage.get();
    getGlyphPositions(glyphPaint, glyphs, glyphCount, x, y, pos);

    applyKerning(pos, adjustments, glyphCount, glyphPaint);
    canvas->drawPosText(glyphs, glyphCount * sizeof(uint16_t), pos, glyphPaint);
}

static const struct {
    const char* fName;
    SkTypeface::Style   fStyle;
} gFaceStyles[] = {
    { "sans-serif", SkTypeface::kNormal },
    { "sans-serif", SkTypeface::kBold },
    { "sans-serif", SkTypeface::kItalic },
    { "sans-serif", SkTypeface::kBoldItalic },
    { "serif", SkTypeface::kNormal },
    { "serif", SkTypeface::kBold },
    { "serif", SkTypeface::kItalic },
    { "serif", SkTypeface::kBoldItalic },
    { "monospace", SkTypeface::kNormal },
    { "monospace", SkTypeface::kBold },
    { "monospace", SkTypeface::kItalic },
    { "monospace", SkTypeface::kBoldItalic },
};

static const int gFaceStylesCount = SK_ARRAY_COUNT(gFaceStyles);

class TypefaceStylesGM : public skiagm::GM {
    sk_sp<SkTypeface> fFaces[gFaceStylesCount];
    bool fApplyKerning;

public:
    TypefaceStylesGM(bool applyKerning)
        : fApplyKerning(applyKerning) {
        memset(fFaces, 0, sizeof(fFaces));
    }

protected:
    void onOnceBeforeDraw() override {
        for (int i = 0; i < gFaceStylesCount; i++) {
            fFaces[i] = SkTypeface::MakeFromName(
                    sk_tool_utils::platform_font_name(
                        gFaceStyles[i].fName), SkFontStyle::FromOldStyle(gFaceStyles[i].fStyle));
        }
    }

    SkString onShortName() override {
        SkString name("typefacestyles");
        if (fApplyKerning) {
            name.append("_kerning");
        }
        name.append(sk_tool_utils::major_platform_os_name());
        return name;
    }

    SkISize onISize() override {
        return SkISize::Make(640, 480);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTextSize(SkIntToScalar(30));

        const char* text = fApplyKerning ? "Type AWAY" : "Hamburgefons";
        const size_t textLen = strlen(text);

        SkScalar x = SkIntToScalar(10);
        SkScalar dy = paint.getFontMetrics(nullptr);
        SkScalar y = dy;

        if (fApplyKerning) {
            paint.setSubpixelText(true);
        } else {
            paint.setLinearText(true);
        }
        for (int i = 0; i < gFaceStylesCount; i++) {
            paint.setTypeface(fFaces[i]);
            canvas->drawText(text, textLen, x, y, paint);
            if (fApplyKerning) {
                drawKernText(canvas, text, textLen, x + 240, y, paint);
            }
            y += dy;
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new TypefaceStylesGM(false); )
DEF_GM( return new TypefaceStylesGM(true); )
