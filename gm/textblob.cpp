/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkColor.h"
#include "SkFontMetrics.h"
#include "SkFontStyle.h"
#include "SkPaint.h"
#include "SkPoint.h"
#include "SkRSXform.h"
#include "SkRect.h"
#include "SkRefCnt.h"
#include "SkScalar.h"
#include "SkSize.h"
#include "SkString.h"
#include "SkTDArray.h"
#include "SkTextBlob.h"
#include "SkTypeface.h"
#include "SkTypes.h"
#include "gm.h"
#include "sk_tool_utils.h"

#include <cstring>

namespace  {

enum Pos {
    kDefault_Pos = 0,
    kScalar_Pos  = 1,
    kPoint_Pos   = 2,
};

const struct BlobCfg {
    unsigned count;
    Pos      pos;
    SkScalar scale;
} blobConfigs[][3][3] = {
    {
        { { 1024, kDefault_Pos, 1 }, { 0, kDefault_Pos, 0 }, { 0, kDefault_Pos, 0 } },
        { { 1024,  kScalar_Pos, 1 }, { 0,  kScalar_Pos, 0 }, { 0,  kScalar_Pos, 0 } },
        { { 1024,   kPoint_Pos, 1 }, { 0,   kPoint_Pos, 0 }, { 0,   kPoint_Pos, 0 } },
    },
    {
        { { 4, kDefault_Pos, 1 },     { 4, kDefault_Pos, 1 },  { 4, kDefault_Pos, 1 } },
        { { 4,  kScalar_Pos, 1 },     { 4,  kScalar_Pos, 1 },  { 4,  kScalar_Pos, 1 } },
        { { 4,   kPoint_Pos, 1 },     { 4,   kPoint_Pos, 1 },  { 4,   kPoint_Pos, 1 } },
    },

    {
        { { 4, kDefault_Pos, 1 },     { 4, kDefault_Pos, 1 },  { 4,  kScalar_Pos, 1 } },
        { { 4,  kScalar_Pos, 1 },     { 4,  kScalar_Pos, 1 },  { 4,   kPoint_Pos, 1 } },
        { { 4,   kPoint_Pos, 1 },     { 4,   kPoint_Pos, 1 },  { 4, kDefault_Pos, 1 } },
    },

    {
        { { 4, kDefault_Pos, 1 },     { 4,  kScalar_Pos, 1 },  { 4,   kPoint_Pos, 1 } },
        { { 4,  kScalar_Pos, 1 },     { 4,   kPoint_Pos, 1 },  { 4, kDefault_Pos, 1 } },
        { { 4,   kPoint_Pos, 1 },     { 4, kDefault_Pos, 1 },  { 4,  kScalar_Pos, 1 } },
    },

    {
        { { 4, kDefault_Pos, .75f },     { 4, kDefault_Pos, 1 },  { 4,  kScalar_Pos, 1.25f } },
        { { 4,  kScalar_Pos, .75f },     { 4,  kScalar_Pos, 1 },  { 4,   kPoint_Pos, 1.25f } },
        { { 4,   kPoint_Pos, .75f },     { 4,   kPoint_Pos, 1 },  { 4, kDefault_Pos, 1.25f } },
    },

    {
        { { 4, kDefault_Pos, 1 },     { 4,  kScalar_Pos, .75f },  { 4,   kPoint_Pos, 1.25f } },
        { { 4,  kScalar_Pos, 1 },     { 4,   kPoint_Pos, .75f },  { 4, kDefault_Pos, 1.25f } },
        { { 4,   kPoint_Pos, 1 },     { 4, kDefault_Pos, .75f },  { 4,  kScalar_Pos, 1.25f } },
    },
};

const SkScalar kFontSize = 16;
} // namespace
static sk_sp<SkTextBlob> make_blob(unsigned blobIndex, const uint16_t* glyphs, unsigned glyphCount,
                                   const sk_sp<SkTypeface>& typeface) {
    if (blobIndex >= SK_ARRAY_COUNT(blobConfigs)) {
        return nullptr;
    }
    SkTextBlobBuilder builder;

    SkFont font(typeface);
    font.setSubpixel(true);
    font.setEdging(SkFont::Edging::kAntiAlias);

    for (unsigned l = 0; l < SK_ARRAY_COUNT(blobConfigs[blobIndex]); ++l) {
        unsigned currentGlyph = 0;

        for (unsigned c = 0; c < SK_ARRAY_COUNT(blobConfigs[blobIndex][l]); ++c) {
            const BlobCfg* cfg = &blobConfigs[blobIndex][l][c];
            unsigned count = cfg->count;

            if (count > glyphCount - currentGlyph) {
                count = glyphCount - currentGlyph;
            }
            if (0 == count) {
                break;
            }

            font.setSize(kFontSize * cfg->scale);
            const SkScalar advanceX = font.getSize() * 0.85f;
            const SkScalar advanceY = font.getSize() * 1.5f;

            SkPoint offset = SkPoint::Make(currentGlyph * advanceX + c * advanceX,
                                           advanceY * l);
            switch (cfg->pos) {
                case kDefault_Pos: {
                    const SkTextBlobBuilder::RunBuffer& buf = builder.allocRun(font, count,
                                                                               offset.x(),
                                                                               offset.y());
                    memcpy(buf.glyphs, glyphs + currentGlyph, count * sizeof(uint16_t));
                    break;
                }
                case kScalar_Pos: {
                    const SkTextBlobBuilder::RunBuffer& buf = builder.allocRunPosH(font, count,
                                                                                   offset.y());
                    SkTDArray<SkScalar> pos;
                    for (unsigned i = 0; i < count; ++i) {
                        *pos.append() = offset.x() + i * advanceX;
                    }

                    memcpy(buf.glyphs, glyphs + currentGlyph, count * sizeof(uint16_t));
                    memcpy(buf.pos, pos.begin(), count * sizeof(SkScalar));
                    break;
                }
                case kPoint_Pos: {
                    const SkTextBlobBuilder::RunBuffer& buf = builder.allocRunPos(font, count);

                    SkTDArray<SkScalar> pos;
                    for (unsigned i = 0; i < count; ++i) {
                        *pos.append() = offset.x() + i * advanceX;
                        *pos.append() = offset.y() + i * (advanceY / count);
                    }

                    memcpy(buf.glyphs, glyphs + currentGlyph, count * sizeof(uint16_t));
                    memcpy(buf.pos, pos.begin(), count * sizeof(SkScalar) * 2);
                    break;
                }
                default:
                    SK_ABORT("unhandled pos value");
            }
            currentGlyph += count;
        }
    }
    return builder.make();
}

static SkTDArray<uint16_t> to_glyphs(const sk_sp<SkTypeface>& typeface, const char* text) {
    SkTDArray<uint16_t> glyphs;
    SkFont font(typeface);
    size_t txtLen = strlen(text);
    int glyphCount = font.countText(text, txtLen, kUTF8_SkTextEncoding);
    glyphs.append(glyphCount);
    font.textToGlyphs(text, txtLen, kUTF8_SkTextEncoding, glyphs.begin(), glyphCount);
    return glyphs;
}


class TextBlobGM : public skiagm::GM {
public:
    TextBlobGM(const char* txt) : fText(txt) {}
protected:
    void onOnceBeforeDraw() override {
        fTypeface = sk_tool_utils::create_portable_typeface("serif", SkFontStyle());
        fGlyphs = to_glyphs(fTypeface, fText);
    }

    SkString onShortName() override { return SkString("textblob"); }

    SkISize onISize() override { return SkISize::Make(640, 480); }

    void onDraw(SkCanvas* canvas) override {
        for (unsigned b = 0; b < SK_ARRAY_COUNT(blobConfigs); ++b) {
            sk_sp<SkTextBlob> blob(
                    make_blob(b, fGlyphs.begin(), (unsigned)fGlyphs.size(), fTypeface));
            SkPaint p;
            p.setAntiAlias(true);
            SkPoint offset = SkPoint::Make(SkIntToScalar(10 + 300 * (b % 2)),
                                           SkIntToScalar(20 + 150 * (b / 2)));

            canvas->drawTextBlob(blob, offset.x(), offset.y(), p);

            p.setColor(SK_ColorBLUE);
            p.setStyle(SkPaint::kStroke_Style);
            SkRect box = blob->bounds();
            box.offset(offset);
            p.setAntiAlias(false);
            canvas->drawRect(box, p);

        }
    }
    SkTDArray<uint16_t> fGlyphs;
    sk_sp<SkTypeface>   fTypeface;
    const char*         fText;
    typedef skiagm::GM INHERITED;
};

DEF_GM(return new TextBlobGM("hamburgefons");)

SkPaint stroke_paint(SkColor c, bool antiAlias) {
    SkPaint p;
    p.setStyle(SkPaint::kStroke_Style);
    p.setAntiAlias(antiAlias);
    p.setColor(c);
    p.setStrokeWidth(0);
    return p;
}

SkRect get_bounds(const SkTextBlob::Iter::Run& run, const SkPaint* paint) {
    SkRect runBounds = {0, 0, 0, 0};
    std::unique_ptr<SkRect[]> bounds(new SkRect[run.fGlyphCount]);
    run.fFont->getBounds(run.fGlyphs, (int)run.fGlyphCount, bounds.get(), paint);
    for (unsigned i = 0; i < run.fGlyphCount; ++i) {
        SkMatrix mat;
        mat.setRSXform(run.fPositions[i]);
        runBounds.join(mat.mapRect(bounds[i]));
    }
    return runBounds;
}

static void concat(SkCanvas* canvas, const SkRSXform& rsxForm) {
    SkMatrix mat;
    mat.setRSXform(rsxForm);
    canvas->concat(mat);
}

DEF_SIMPLE_GM(textblob_tightbounds, canvas, 640, 480) {
    sk_sp<SkTypeface> typeface = sk_tool_utils::create_portable_typeface("serif", SkFontStyle());
    const char text[] = "hamburgefons";
    SkTDArray<uint16_t> glyphs = to_glyphs(typeface, text);
    SkPaint black;
    black.setAntiAlias(true);
    SkPaint underline   = stroke_paint(SkColorSetARGB(0xFF, 0x00, 0x00, 0x00), true);
    SkPaint strike      = stroke_paint(SkColorSetARGB(0xFF, 0x00, 0x00, 0x00), true);
    SkPaint redStroke   = stroke_paint(SkColorSetARGB(0xFF, 0xFF, 0x80, 0x80), false);
    SkPaint greenStroke = stroke_paint(SkColorSetARGB(0xFF, 0x80, 0xFF, 0x80), false);
    SkPaint blueStroke  = stroke_paint(SkColorSetARGB(0xFF, 0x80, 0x80, 0xFF), false);
    for (unsigned b = 0; b < SK_ARRAY_COUNT(blobConfigs); ++b) {
        sk_sp<SkTextBlob> blob(
                make_blob(b, glyphs.begin(), (unsigned)glyphs.size(), typeface));
        SkPoint offset = SkPoint{10.0f + 300.0f * (b % 2), 20.0f + 150.0f * (b / 2)};
        SkRect box = blob->bounds();
        box.offset(offset);
        canvas->drawRect(box, blueStroke);
        for (const SkTextBlob::Iter::Run run : *blob) {
            SkRect runBounds = get_bounds(run, &black);
            runBounds.offset(offset);
            canvas->drawRect(runBounds, redStroke);
        }
        for (const SkTextBlob::Iter::Run run : *blob) {
            SkASSERT(run.fFont);
            SkASSERT(run.fGlyphs);
            SkASSERT(run.fGlyphCount);
            SkASSERT(run.fPositions);

            float underlineThickness = 1, underlinePosition = 1.5f;
            float strikeoutThickness = 1, strikeoutPosition = -4;
            SkFontMetrics metrics;
            (void)run.fFont->getMetrics(&metrics);
            (void)metrics.hasUnderlineThickness(&underlineThickness);
            (void)metrics.hasUnderlinePosition(&underlinePosition);
            (void)metrics.hasStrikeoutThickness(&strikeoutThickness);
            (void)metrics.hasStrikeoutPosition(&strikeoutPosition);
            underline.setStrokeWidth(underlineThickness);
            strike.setStrokeWidth(strikeoutThickness);

            std::unique_ptr<SkRect[]> bounds(new SkRect[run.fGlyphCount]);
            run.fFont->getBounds(run.fGlyphs, (int)run.fGlyphCount, bounds.get(), &black);

            for (unsigned i = 0; i < run.fGlyphCount; ++i) {
                SkAutoCanvasRestore autoCanvasRestore(canvas, true);
                canvas->translate(offset.x(), offset.y());
                concat(canvas, run.fPositions[i]);
                canvas->drawRect(bounds[i], greenStroke);

                canvas->drawLine(bounds[i].left(), underlinePosition,
                                 bounds[i].right(), underlinePosition, underline);
                canvas->drawLine(bounds[i].left(), strikeoutPosition,
                                 bounds[i].right(), strikeoutPosition, strike);
            }
        }
        canvas->drawTextBlob(blob, offset.x(), offset.y(), black);
    }
}

