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
}

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
    typedef skiagm::GM INHERITED;
};
DEF_GM(return new TextBlobGM("hamburgefons");)

#include <vector>

enum class TextMethod {
    kGlyphs,
    kBlobs_Raw,
    kBlobs_Cached,
};

static const char* method_name(TextMethod m) {
    switch (m) {
        case TextMethod::kGlyphs:       return "glyphs";
        case TextMethod::kBlobs_Raw:    return "blobs_raw";
        case TextMethod::kBlobs_Cached: return "blobs_cached";
    }
    return "unreachable";
}

class DrawGlyphsGM : public skiagm::GM {
    SkString fText;
    TextMethod fMethod;

    struct Rec {
        SkFont                 fFont;
        std::vector<SkGlyphID> fGlyphs;
        std::vector<SkPoint>   fPositions;

        Rec(const SkFont& font, size_t count) : fFont(font) {
            fGlyphs.resize(count);
            fPositions.resize(count);
        }

        sk_sp<SkTextBlob> makeBlob() const {
            return SkTextBlob::MakeFromPosText(fGlyphs.data(), fGlyphs.size() * sizeof(SkGlyphID),
                                               fPositions.data(), fFont, SkTextEncoding::kGlyphID);
        }
    };
    std::vector<Rec> fRecs;
    std::vector<sk_sp<SkTextBlob>> fBlobs;

public:
    DrawGlyphsGM(const char* text, TextMethod method) : fText(text), fMethod(method) {}

protected:
    void onOnceBeforeDraw() override {
        auto tf = ToolUtils::create_portable_typeface("serif", SkFontStyle());

        auto make = [&](const SkFont& font, SkPoint origin) -> Rec {
            size_t N = fText.size();
            Rec r(font, N);
            font.textToGlyphs(fText.c_str(), N, SkTextEncoding::kUTF8, r.fGlyphs.data(), N);
            font.getPos(r.fGlyphs.data(), N, r.fPositions.data(), origin);
            return r;
        };

        float y = 0;
        SkFont font(tf);
        font.setEdging(SkFont::Edging::kAntiAlias);
        for (float size = 6; size < 40; size += 1) {
            font.setSize(size);
            fRecs.push_back(make(font, {0, y}));
            y += size + 2;

            if (fMethod == TextMethod::kBlobs_Cached) {
                fBlobs.push_back(fRecs.back().makeBlob());
            }
        }
    }

    SkString onShortName() override {
        SkString str;
        str.printf("drawglyphs_%s", method_name(fMethod));
        return str;
    }

    SkISize onISize() override {
        return SkISize::Make(640, 480);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(10, 10);

        SkPaint paint;
        int index = 0;
        for (const auto& r : fRecs) {
            switch (fMethod) {
                case TextMethod::kGlyphs:
                    canvas->drawGlyphs(r.fGlyphs.data(), r.fGlyphs.size(), r.fPositions.data(),
                                       r.fFont, paint);
                    break;
                case TextMethod::kBlobs_Raw:
                    canvas->drawTextBlob(r.makeBlob(), 0, 0, paint);
                    break;
                case TextMethod::kBlobs_Cached:
                    canvas->drawTextBlob(fBlobs[index], 0, 0, paint);
                    break;
            }
            index += 1;
        }
    }

    bool onAnimate(double nanos) override { return true; }
    bool runAsBench() const override { return true; }

private:
    typedef skiagm::GM INHERITED;
};

static const char* gBenchText = "H";

DEF_GM(return new DrawGlyphsGM(gBenchText, TextMethod::kBlobs_Cached);)
DEF_GM(return new DrawGlyphsGM(gBenchText, TextMethod::kBlobs_Raw);)
DEF_GM(return new DrawGlyphsGM(gBenchText, TextMethod::kGlyphs);)
