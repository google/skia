/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkReadBuffer.h"
#include "SkTextBlob.h"
#include "SkWriteBuffer.h"

#include "Sk2DPathEffect.h"

static SkPath create_underline(const SkTDArray<SkScalar>& intersections,
        SkScalar last, SkScalar finalPos,
        SkScalar uPos, SkScalar uWidth, SkScalar textSize) {
    SkPath underline;
    SkScalar end = last;
    for (int index = 0; index < intersections.count(); index += 2) {
        SkScalar start = intersections[index] - uWidth;;
        end = intersections[index + 1] + uWidth;
        if (start > last && last + textSize / 12 < start) {
            underline.moveTo(last, uPos);
            underline.lineTo(start, uPos);
        }
        last = end;
    }
    if (end < finalPos) {
        underline.moveTo(end, uPos);
        underline.lineTo(finalPos, uPos);
    }
    return underline;
}

static void find_intercepts(const char* test, size_t len, SkScalar x, SkScalar y,
        const SkPaint& paint, SkScalar uWidth, SkTDArray<SkScalar>* intersections) {
    SkScalar uPos = y + uWidth;
    SkScalar bounds[2] = { uPos - uWidth / 2, uPos + uWidth / 2 };
    int count = paint.getTextIntercepts(test, len, x, y, bounds, nullptr);
    SkASSERT(!(count % 2));
    if (count) {
        intersections->setCount(count);
        paint.getTextIntercepts(test, len, x, y, bounds, intersections->begin());
    }
}

DEF_SIMPLE_GM(fancyunderline, canvas, 900, 1350) {
    SkPaint paint;
    paint.setAntiAlias(true);
    const char* fam[] = { "sans-serif", "serif", "monospace" };
    const char test[] = "aAjJgGyY_|{-(~[,]qQ}pP}zZ";
    SkPoint textPt = { 10, 80 };
    for (size_t font = 0; font < SK_ARRAY_COUNT(fam); ++font) {
        sk_tool_utils::set_portable_typeface(&paint, fam[font]);
        for (SkScalar textSize = 100; textSize > 10; textSize -= 20) {
            paint.setTextSize(textSize);
            const SkScalar uWidth = textSize / 15;
            paint.setStrokeWidth(uWidth);
            paint.setStyle(SkPaint::kFill_Style);
            canvas->drawText(test, sizeof(test) - 1, textPt.fX, textPt.fY, paint);

            SkTDArray<SkScalar> intersections;
            find_intercepts(test, sizeof(test) - 1, textPt.fX, textPt.fY, paint, uWidth,
                &intersections);

            SkScalar start = textPt.fX;
            SkScalar end = paint.measureText(test, sizeof(test) - 1) + textPt.fX;
            SkScalar uPos = textPt.fY + uWidth;
            SkPath underline = create_underline(intersections, start, end, uPos, uWidth, textSize);
            paint.setStyle(SkPaint::kStroke_Style);
            canvas->drawPath(underline, paint);

            canvas->translate(0, textSize * 1.3f);
        }
        canvas->translate(0, 60);
    }
}

static void find_intercepts(const char* test, size_t len, const SkPoint* pos, const SkPaint& paint,
        SkScalar uWidth, SkTDArray<SkScalar>* intersections) {
    SkScalar uPos = pos[0].fY + uWidth;
    SkScalar bounds[2] = { uPos - uWidth / 2, uPos + uWidth / 2 };
    int count = paint.getPosTextIntercepts(test, len, pos, bounds, nullptr);
    SkASSERT(!(count % 2));
    if (count) {
        intersections->setCount(count);
        paint.getPosTextIntercepts(test, len, pos, bounds, intersections->begin());
    }
}

DEF_SIMPLE_GM(fancyposunderline, canvas, 900, 1350) {
    SkPaint paint;
    paint.setAntiAlias(true);
    const char* fam[] = { "sans-serif", "serif", "monospace" };
    const char test[] = "aAjJgGyY_|{-(~[,]qQ}pP}zZ";
    SkPoint textPt = { 10, 80 };
    for (size_t font = 0; font < SK_ARRAY_COUNT(fam); ++font) {
        sk_tool_utils::set_portable_typeface(&paint, fam[font]);
        for (SkScalar textSize = 100; textSize > 10; textSize -= 20) {
            paint.setTextSize(textSize);
            const SkScalar uWidth = textSize / 15;
            paint.setStrokeWidth(uWidth);
            paint.setStyle(SkPaint::kFill_Style);
            int widthCount = paint.getTextWidths(test, sizeof(test) - 1, nullptr);
            SkTDArray<SkScalar> widths;
            widths.setCount(widthCount);
            (void) paint.getTextWidths(test, sizeof(test) - 1, widths.begin());
            SkTDArray<SkPoint> pos;
            pos.setCount(widthCount);
            SkScalar posX = textPt.fX;
            for (int index = 0; index < widthCount; ++index) {
                pos[index].fX = posX;
                posX += widths[index];
                pos[index].fY = textPt.fY + (textSize / 25) * (index % 4);
            }
            canvas->drawPosText(test, sizeof(test) - 1, pos.begin(), paint);

            SkTDArray<SkScalar> intersections;
            find_intercepts(test, sizeof(test) - 1, pos.begin(), paint, uWidth, &intersections);

            SkScalar start = textPt.fX;
            SkScalar end = posX;
            SkScalar uPos = textPt.fY + uWidth;
            SkPath underline = create_underline(intersections, start, end, uPos, uWidth, textSize);
            paint.setStyle(SkPaint::kStroke_Style);
            canvas->drawPath(underline, paint);

            canvas->translate(0, textSize * 1.3f);
        }
        canvas->translate(0, 60);
    }
}

namespace {

sk_sp<SkTextBlob> MakeFancyBlob(const SkPaint& paint, const char* text) {
    SkPaint blobPaint(paint);

    const size_t textLen = strlen(text);
    const int glyphCount = blobPaint.textToGlyphs(text, textLen, nullptr);
    SkAutoTArray<SkGlyphID> glyphs(glyphCount);
    blobPaint.textToGlyphs(text, textLen, glyphs.get());

    blobPaint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    const size_t glyphTextBytes = SkTo<uint32_t>(glyphCount) * sizeof(SkGlyphID);
    const int widthCount = blobPaint.getTextWidths(glyphs.get(), glyphTextBytes, nullptr);
    SkAssertResult(widthCount == glyphCount);

    SkAutoTArray<SkScalar> widths(glyphCount);
    blobPaint.getTextWidths(glyphs.get(), glyphTextBytes, widths.get());

    SkTextBlobBuilder blobBuilder;
    int glyphIndex = 0;
    SkScalar advance = 0;

    // Default-positioned run.
    {
        const int defaultRunLen = glyphCount / 3;
        const SkTextBlobBuilder::RunBuffer& buf = blobBuilder.allocRun(blobPaint,
                                                                       defaultRunLen,
                                                                       advance, 0);
        memcpy(buf.glyphs, glyphs.get(), SkTo<uint32_t>(defaultRunLen) * sizeof(SkGlyphID));

        for (int i = 0; i < defaultRunLen; ++i) {
            advance += widths[glyphIndex++];
        }
    }

    // Horizontal-positioned run.
    {
        const int horizontalRunLen = glyphCount / 3;
        const SkTextBlobBuilder::RunBuffer& buf = blobBuilder.allocRunPosH(blobPaint,
                                                                           horizontalRunLen,
                                                                           0);
        memcpy(buf.glyphs, glyphs.get() + glyphIndex,
               SkTo<uint32_t>(horizontalRunLen) * sizeof(SkGlyphID));
        for (int i = 0; i < horizontalRunLen; ++i) {
            buf.pos[i] = advance;
            advance += widths[glyphIndex++];
        }
    }

    // Full-positioned run.
    {
        const int fullRunLen = glyphCount - glyphIndex;
        const SkTextBlobBuilder::RunBuffer& buf = blobBuilder.allocRunPos(blobPaint, fullRunLen);
        memcpy(buf.glyphs, glyphs.get() + glyphIndex,
               SkTo<uint32_t>(fullRunLen) * sizeof(SkGlyphID));
        for (int i = 0; i < fullRunLen; ++i) {
            buf.pos[i * 2 + 0] = advance; // x offset
            buf.pos[i * 2 + 1] = 0;       // y offset
            advance += widths[glyphIndex++];
        }
    }

    return blobBuilder.make();
}

} // anonymous ns

DEF_SIMPLE_GM(fancyblobunderline, canvas, 1480, 1380) {
    SkPaint paint;
    paint.setAntiAlias(true);
    const char* fam[] = { "sans-serif", "serif", "monospace" };
    const char test[] = "aAjJgGyY_|{-(~[,]qQ}pP}zZ";
    const SkPoint blobOffset = { 10, 80 };

    for (size_t font = 0; font < SK_ARRAY_COUNT(fam); ++font) {
        sk_tool_utils::set_portable_typeface(&paint, fam[font]);
        for (SkScalar textSize = 100; textSize > 10; textSize -= 20) {
            paint.setTextSize(textSize);
            const SkScalar uWidth = textSize / 15;
            paint.setStrokeWidth(uWidth);
            paint.setStyle(SkPaint::kFill_Style);

            sk_sp<SkTextBlob> blob = MakeFancyBlob(paint, test);
            canvas->drawTextBlob(blob, blobOffset.x(), blobOffset.y(), paint);

            const SkScalar uPos = uWidth;
            const SkScalar bounds[2] = { uPos - uWidth / 2, uPos + uWidth / 2 };
            const int interceptCount = paint.getTextBlobIntercepts(blob.get(), bounds, nullptr);
            SkASSERT(!(interceptCount % 2));

            SkTDArray<SkScalar> intercepts;
            intercepts.setCount(interceptCount);
            paint.getTextBlobIntercepts(blob.get(), bounds, intercepts.begin());

            const SkScalar start = blob->bounds().left();
            const SkScalar end = blob->bounds().right();
            SkPath underline = create_underline(intercepts, start, end, uPos, uWidth, textSize);
            underline.offset(blobOffset.x(), blobOffset.y());
            paint.setStyle(SkPaint::kStroke_Style);
            canvas->drawPath(underline, paint);

            canvas->translate(0, textSize * 1.3f);
        }

        canvas->translate(0, 60);
    }
}

DEF_SIMPLE_GM(fancyunderlinebars, canvas, 1500, 460) {
    SkPaint paint;
    paint.setAntiAlias(true);
    const char test[] = " .}]_ .}]_ .}]_ .}]_ .}]_ .}]_ .}]_ .}]_ .}]_ .}]_ .}]_ .}]_ .}]_";
    SkPoint textPt = { 10, 80 };
    sk_tool_utils::set_portable_typeface(&paint, "serif");
    for (SkScalar textSize = 100; textSize > 10; textSize -= 20) {
        paint.setTextSize(textSize);
        SkScalar uWidth = textSize / 15;
        paint.setStrokeWidth(uWidth);
        paint.setStyle(SkPaint::kFill_Style);
        int widthCount = paint.getTextWidths(test, sizeof(test) - 1, nullptr);
        SkTDArray<SkScalar> widths;
        widths.setCount(widthCount);
        (void) paint.getTextWidths(test, sizeof(test) - 1, widths.begin());
        SkTDArray<SkPoint> pos;
        pos.setCount(widthCount);
        SkScalar posX = textPt.fX;
        pos[0] = textPt;
        posX += widths[0];
        for (int index = 1; index < widthCount; ++index) {
            pos[index].fX = posX;
            posX += widths[index];
            pos[index].fY = textPt.fY - (textSize / 50) * (index / 5) + textSize / 50 * 4;
        }
        canvas->drawPosText(test, sizeof(test) - 1, pos.begin(), paint);

        SkTDArray<SkScalar> intersections;
        find_intercepts(test, sizeof(test) - 1, pos.begin(), paint, uWidth, &intersections);

        SkScalar start = textPt.fX;
        SkScalar end = posX;
        SkScalar uPos = pos[0].fY + uWidth;
        SkPath underline = create_underline(intersections, start, end, uPos, uWidth, textSize);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawPath(underline, paint);
        canvas->translate(0, textSize * 1.3f);
    }
}
