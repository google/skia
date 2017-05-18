/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"

#include "SkCanvas.h"
#include "SkPoint.h"
#include "SkTextBlob.h"
#include "SkTDArray.h"

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
}

class TextBlobGM : public skiagm::GM {
public:
    TextBlobGM(const char* txt)
        : fText(txt) {
    }

protected:
    void onOnceBeforeDraw() override {
        fTypeface = sk_tool_utils::create_portable_typeface("serif", SkFontStyle());
        SkPaint p;
        p.setTypeface(fTypeface);
        size_t txtLen = strlen(fText);
        int glyphCount = p.textToGlyphs(fText, txtLen, nullptr);

        fGlyphs.append(glyphCount);
        p.textToGlyphs(fText, txtLen, fGlyphs.begin());
    }

    SkString onShortName() override {
        return SkString("textblob");
    }

    SkISize onISize() override {
        return SkISize::Make(640, 480);
    }

    void onDraw(SkCanvas* canvas) override {
        for (unsigned b = 0; b < SK_ARRAY_COUNT(blobConfigs); ++b) {
            sk_sp<SkTextBlob> blob(this->makeBlob(b));

            SkPaint p;
            SkPoint offset = SkPoint::Make(SkIntToScalar(10 + 300 * (b % 2)),
                                           SkIntToScalar(20 + 150 * (b / 2)));

            canvas->drawTextBlob(blob, offset.x(), offset.y(), p);

            p.setColor(SK_ColorBLUE);
            p.setStyle(SkPaint::kStroke_Style);
            SkRect box = blob->bounds();
            box.offset(offset);
            canvas->drawRect(box, p);

        }
    }

private:
    sk_sp<SkTextBlob> makeBlob(unsigned blobIndex) {
        SkTextBlobBuilder builder;

        SkPaint font;
        font.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
        font.setAntiAlias(true);
        font.setSubpixelText(true);
        font.setTypeface(fTypeface);

        for (unsigned l = 0; l < SK_ARRAY_COUNT(blobConfigs[blobIndex]); ++l) {
            unsigned currentGlyph = 0;

            for (unsigned c = 0; c < SK_ARRAY_COUNT(blobConfigs[blobIndex][l]); ++c) {
                const BlobCfg* cfg = &blobConfigs[blobIndex][l][c];
                unsigned count = cfg->count;

                if (count > fGlyphs.count() - currentGlyph) {
                    count = fGlyphs.count() - currentGlyph;
                }
                if (0 == count) {
                    break;
                }

                font.setTextSize(kFontSize * cfg->scale);
                const SkScalar advanceX = font.getTextSize() * 0.85f;
                const SkScalar advanceY = font.getTextSize() * 1.5f;

                SkPoint offset = SkPoint::Make(currentGlyph * advanceX + c * advanceX,
                                               advanceY * l);
                switch (cfg->pos) {
                case kDefault_Pos: {
                    const SkTextBlobBuilder::RunBuffer& buf = builder.allocRun(font, count,
                                                                               offset.x(),
                                                                               offset.y());
                    memcpy(buf.glyphs, fGlyphs.begin() + currentGlyph, count * sizeof(uint16_t));
                } break;
                case kScalar_Pos: {
                    const SkTextBlobBuilder::RunBuffer& buf = builder.allocRunPosH(font, count,
                                                                                   offset.y());
                    SkTDArray<SkScalar> pos;
                    for (unsigned i = 0; i < count; ++i) {
                        *pos.append() = offset.x() + i * advanceX;
                    }

                    memcpy(buf.glyphs, fGlyphs.begin() + currentGlyph, count * sizeof(uint16_t));
                    memcpy(buf.pos, pos.begin(), count * sizeof(SkScalar));
                } break;
                case kPoint_Pos: {
                    const SkTextBlobBuilder::RunBuffer& buf = builder.allocRunPos(font, count);

                    SkTDArray<SkScalar> pos;
                    for (unsigned i = 0; i < count; ++i) {
                        *pos.append() = offset.x() + i * advanceX;
                        *pos.append() = offset.y() + i * (advanceY / count);
                    }

                    memcpy(buf.glyphs, fGlyphs.begin() + currentGlyph, count * sizeof(uint16_t));
                    memcpy(buf.pos, pos.begin(), count * sizeof(SkScalar) * 2);
                } break;
                default:
                    SK_ABORT("unhandled pos value");
                }

                currentGlyph += count;
            }
        }

        return builder.make();
    }

    SkTDArray<uint16_t> fGlyphs;
    sk_sp<SkTypeface>   fTypeface;
    const char*         fText;
    typedef skiagm::GM INHERITED;
};

DEF_GM(return new TextBlobGM("hamburgefons");)
