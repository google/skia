/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTDArray.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "tools/ToolUtils.h"

#include <string.h>

static SkPath create_underline(const SkTDArray<SkScalar>& intersections,
        SkScalar last, SkScalar finalPos,
        SkScalar uPos, SkScalar uWidth, SkScalar textSize) {
    SkPath underline;
    SkScalar end = last;
    for (int index = 0; index < intersections.count(); index += 2) {
        SkScalar start = intersections[index] - uWidth;
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

namespace {

sk_sp<SkTextBlob> MakeFancyBlob(const SkPaint& paint, const SkFont& font, const char* text) {
    const size_t textLen = strlen(text);
    const int glyphCount = font.countText(text, textLen, kUTF8_SkTextEncoding);
    SkAutoTArray<SkGlyphID> glyphs(glyphCount);
    font.textToGlyphs(text, textLen, kUTF8_SkTextEncoding, glyphs.get(), glyphCount);
    SkAutoTArray<SkScalar> widths(glyphCount);
    font.getWidths(glyphs.get(), glyphCount, widths.get());

    SkTextBlobBuilder blobBuilder;
    int glyphIndex = 0;
    SkScalar advance = 0;

    // Default-positioned run.
    {
        const int defaultRunLen = glyphCount / 3;
        const SkTextBlobBuilder::RunBuffer& buf = blobBuilder.allocRun(font,
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
        const SkTextBlobBuilder::RunBuffer& buf = blobBuilder.allocRunPosH(font,
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
        const SkTextBlobBuilder::RunBuffer& buf = blobBuilder.allocRunPos(font, fullRunLen);
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
        for (SkScalar textSize = 100; textSize > 10; textSize -= 20) {
            SkFont skFont(ToolUtils::create_portable_typeface(fam[font], SkFontStyle()), textSize);
            const SkScalar uWidth = textSize / 15;
            paint.setStrokeWidth(uWidth);
            paint.setStyle(SkPaint::kFill_Style);

            sk_sp<SkTextBlob> blob = MakeFancyBlob(paint, skFont, test);
            canvas->drawTextBlob(blob, blobOffset.x(), blobOffset.y(), paint);

            const SkScalar uPos = uWidth;
            const SkScalar bounds[2] = { uPos - uWidth / 2, uPos + uWidth / 2 };
            const int interceptCount = blob->getIntercepts(bounds, nullptr, &paint);
            SkASSERT(!(interceptCount % 2));

            SkTDArray<SkScalar> intercepts;
            intercepts.setCount(interceptCount);
            blob->getIntercepts(bounds, intercepts.begin(), &paint);

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

///////////////////////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkTextBlob> make_text(const SkFont& font, const SkGlyphID glyphs[], int count) {
    return SkTextBlob::MakeFromText(glyphs, count * sizeof(SkGlyphID), font,
                                    kGlyphID_SkTextEncoding);
}

static sk_sp<SkTextBlob> make_posh(const SkFont& font, const SkGlyphID glyphs[], int count,
                                   SkScalar spacing) {
    SkAutoTArray<SkScalar> xpos(count);
    font.getXPos(glyphs, count, xpos.get());
    for (int i = 1; i < count; ++i) {
        xpos[i] += spacing * i;
    }
    return SkTextBlob::MakeFromPosTextH(glyphs, count * sizeof(SkGlyphID), xpos.get(), 0, font,
                                        kGlyphID_SkTextEncoding);
}

static sk_sp<SkTextBlob> make_pos(const SkFont& font, const SkGlyphID glyphs[], int count,
                                  SkScalar spacing) {
    SkAutoTArray<SkPoint> pos(count);
    font.getPos(glyphs, count, pos.get());
    for (int i = 1; i < count; ++i) {
        pos[i].fX += spacing * i;
    }
    return SkTextBlob::MakeFromPosText(glyphs, count * sizeof(SkGlyphID), pos.get(), font,
                                       kGlyphID_SkTextEncoding);
}

// widen the gaps with a margin (on each side of the gap), elimnating segments that go away
static int trim_with_halo(SkScalar intervals[], int count, SkScalar margin) {
    SkASSERT(count > 0 && (count & 1) == 0);

    int n = count;
    SkScalar* stop = intervals + count;
    *intervals++ -= margin;
    while (intervals < stop - 1) {
        intervals[0] += margin;
        intervals[1] -= margin;
        if (intervals[0] >= intervals[1]) { // went away
            int remaining = stop - intervals - 2;
            SkASSERT(remaining >= 0 && (remaining & 1) == 1);
            if (remaining > 0) {
                memmove(intervals, intervals + 2, remaining * sizeof(SkScalar));
            }
            stop -= 2;
            n -= 2;
        } else {
            intervals += 2;
        }
    }
    *intervals += margin;
    return n;
}

static void draw_blob_adorned(SkCanvas* canvas, sk_sp<SkTextBlob> blob) {
    SkPaint paint;

    canvas->drawTextBlob(blob.get(), 0, 0, paint);

    const SkScalar yminmax[] = { 8, 16 };
    int count = blob->getIntercepts(yminmax, nullptr);
    if (!count) {
        return;
    }

    SkAutoTArray<SkScalar> intervals(count);
    blob->getIntercepts(yminmax, intervals.get());
    count = trim_with_halo(intervals.get(), count, SkScalarHalf(yminmax[1] - yminmax[0]) * 1.5f);
    SkASSERT(count >= 2);

    const SkScalar y = SkScalarAve(yminmax[0], yminmax[1]);
    SkScalar end = 900;
    SkPath path;
    path.moveTo({0, y});
    for (int i = 0; i < count; i += 2) {
        path.lineTo(intervals[i], y).moveTo(intervals[i+1], y);
    }
    if (intervals[count - 1] < end) {
        path.lineTo(end, y);
    }

    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(yminmax[1] - yminmax[0]);
    canvas->drawPath(path, paint);
}

DEF_SIMPLE_GM(textblob_intercepts, canvas, 940, 800) {
    const char text[] = "Hyjay {worlp}.";
    const size_t length = strlen(text);
    SkFont font;
    font.setTypeface(ToolUtils::create_portable_typeface());
    font.setSize(100);
    font.setEdging(SkFont::Edging::kAntiAlias);
    const int count = font.countText(text, length, kUTF8_SkTextEncoding);
    SkAutoTArray<SkGlyphID> glyphs(count);
    font.textToGlyphs(text, length, kUTF8_SkTextEncoding, glyphs.get(), count);

    auto b0 = make_text(font, glyphs.get(), count);

    canvas->translate(20, 120);
    draw_blob_adorned(canvas, b0);
    for (SkScalar spacing = 0; spacing < 30; spacing += 20) {
        auto b1 = make_posh(font, glyphs.get(), count, spacing);
        auto b2 = make_pos( font, glyphs.get(), count, spacing);
        canvas->translate(0, 150);
        draw_blob_adorned(canvas, b1);
        canvas->translate(0, 150);
        draw_blob_adorned(canvas, b2);
    }
}
