/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkTextBlobPriv.h"
#include "src/core/SkGlyphRun.h"
#include "tools/ToolUtils.h"

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
}  // namespace

class TextBlobGM : public skiagm::GM {
public:
    TextBlobGM(const char* txt)
        : fText(txt) {
    }

protected:
    void onOnceBeforeDraw() override {
        fTypeface = ToolUtils::create_portable_typeface("serif", SkFontStyle());
        SkFont font(fTypeface);
        size_t txtLen = strlen(fText);
        int glyphCount = font.countText(fText, txtLen, SkTextEncoding::kUTF8);

        fGlyphs.append(glyphCount);
        font.textToGlyphs(fText, txtLen, SkTextEncoding::kUTF8, fGlyphs.begin(), glyphCount);
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

private:
    sk_sp<SkTextBlob> makeBlob(unsigned blobIndex) {
        SkTextBlobBuilder builder;

        SkFont font;
        font.setSubpixel(true);
        font.setEdging(SkFont::Edging::kAntiAlias);
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
    using INHERITED = skiagm::GM;
};

DEF_GM(return new TextBlobGM("hamburgefons");)

static const char text[] = "Call me Ishmael. Some years ago—never mind how long precisely";
extern bool gForceSDF;
extern bool gOverridePaint;
class TextStylesGM : public skiagm::GM {
protected:
    void onOnceBeforeDraw() override {
        fTypeface = SkTypeface::MakeFromName("Palatino", SkFontStyle());
        SkString familyName;
        fTypeface->getFamilyName(&familyName);
        SkDebugf("typeface name: %s\n", familyName.c_str());
    }

    SkString onShortName() override {
        return SkString("textstyle");
    }

    SkISize onISize() override {
        return SkISize::Make(640, 480);
    }

    void onDraw(SkCanvas* canvas) override {

        auto tag = [&](const char* tag, SkScalar y) {
            auto face = SkTypeface::MakeFromName("Helvetica", SkFontStyle());
            SkFont font{face};
            font.setSize(12);
            canvas->drawString(tag, 475, y, font, SkPaint{});
        };

        {
            SkFont font{fTypeface};
            font.setSize(14);
            font.setEdging(SkFont::Edging::kAlias);
            auto blob = SkTextBlob::MakeFromText(text, strlen(text), font);
            canvas->drawTextBlob(blob, 40.75, 100, SkPaint{});
            tag("Black and white", 100);
        }

        {
            SkFont font{fTypeface};
            font.setSize(14);
            font.setEdging(SkFont::Edging::kAntiAlias);
            auto blob = SkTextBlob::MakeFromText(text, strlen(text), font);
            canvas->drawTextBlob(blob, 40.50, 120, SkPaint{});
            tag("Anti aliased", 120);
        }

        {
            SkFont font{fTypeface};
            font.setSize(14);
            font.setEdging(SkFont::Edging::kAntiAlias);
            font.setSubpixel(true);
            auto blob = SkTextBlob::MakeFromText(text, strlen(text), font);
            canvas->drawTextBlob(blob, 40.50, 140, SkPaint{});
            tag("Anti aliased & sub pixel", 140);
        }

        {
            SkFont font{fTypeface};
            font.setSize(14);
            font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
            font.setSubpixel(true);
            auto blob = SkTextBlob::MakeFromText(text, strlen(text), font);
            canvas->drawTextBlob(blob, 40.50, 160, SkPaint{});
            tag("Anti aliased & sub pixel & lcd", 160);
        }

        {
            SkFont font{fTypeface};
            font.setSize(14);
            font.setEdging(SkFont::Edging::kAntiAlias);
            font.setSubpixel(true);
            auto blob = SkTextBlob::MakeFromText(text, strlen(text), font);
            SkGlyphRunBuilder builder;
            auto glyphRunList = builder.blobToGlyphRunList(*blob, {40.50, 180});

            for (auto& run : glyphRunList) {
                for (auto [glyphID, pos] : run.source()) {
                    SkPath path;
                    font.getPath(glyphID, &path);
                    SkMatrix translate = SkMatrix::Translate(pos + glyphRunList.origin());
                    path.transform(translate);
                    SkPaint paint;
                    paint.setAntiAlias(true);
                    canvas->drawPath(path, paint);
                }
            }
            tag("Path", 180);
        }

        {
            SkFont font{fTypeface};
            font.setSize(14);
            font.setEdging(SkFont::Edging::kAntiAlias);
            font.setSubpixel(true);
            auto blob = SkTextBlob::MakeFromText(text, strlen(text), font);
            gForceSDF = true;
            canvas->drawTextBlob(blob, 40.50, 200, SkPaint{});
            gForceSDF = false;
            tag("SDF", 200);
        }

        {
            SkFont font{fTypeface};
            font.setSize(14);
            font.setEdging(SkFont::Edging::kAntiAlias);
            font.setSubpixel(true);
            auto blob = SkTextBlob::MakeFromText(text, strlen(text), font);
            SkPaint background;
            background.setColor(SK_ColorGREEN);
            canvas->drawRect(blob->bounds().makeOffset(40.50, 240), background);
            SkPaint foreground;
            foreground.setColor(SK_ColorWHITE);
            canvas->drawTextBlob(blob, 40.50, 240, foreground);
            tag("Color compensated", 240);
        }

        {
            SkFont font{fTypeface};
            font.setSize(14);
            font.setEdging(SkFont::Edging::kAntiAlias);
            font.setSubpixel(true);
            auto blob = SkTextBlob::MakeFromText(text, strlen(text), font);
            SkPaint background;
            background.setColor(SK_ColorGREEN);
            canvas->drawRect(blob->bounds().makeOffset(40.50, 260), background);
            SkPaint foreground;
            foreground.setColor(SK_ColorWHITE);
            gOverridePaint = true;
            canvas->drawTextBlob(blob, 40.50, 260, foreground);
            gOverridePaint = false;
            tag("Black mask", 260);
        }
    }

private:
    sk_sp<SkTypeface>   fTypeface;
    using INHERITED = skiagm::GM;
};

DEF_GM(return new TextStylesGM{};)
