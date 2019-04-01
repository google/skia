/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottieShaper.h"

#include "SkFontMetrics.h"
#include "SkShaper.h"
#include "SkTextBlob.h"
#include "SkTextBlobPriv.h"
#include "SkUTF.h"

namespace skottie {
namespace {

// Helper for interfacing with SkShaper: buffers shaper-fed runs and performs
// per-line position adjustments (for external line breaking, horizontal alignment, etc).
class BlobMaker final : public SkShaper::RunHandler {
public:
    BlobMaker(const Shaper::TextDesc& desc, const SkRect& box)
        : fDesc(desc)
        , fBox(box)
        , fAlignFactor(AlignFactor(fDesc.fAlign))
        , fFont(fDesc.fTypeface, fDesc.fTextSize)
        , fShaper(SkShaper::Make())
        , fOffset({fBox.x(), fBox.y()}) {
        fFont.setHinting(kNo_SkFontHinting);
        fFont.setSubpixel(true);
        fFont.setEdging(SkFont::Edging::kAntiAlias);
    }

    void beginLine() override {
        fCurrentPosition = fOffset;
        fPendingLineAdvance  = { 0, 0 };
        fMaxRunAscent = 0;
        fMaxRunDescent = 0;
        fMaxRunLeading = 0;
    }

    void runInfo(const RunInfo& info) override {
        fPendingLineAdvance += info.fAdvance;
        SkFontMetrics metrics;
        info.fFont.getMetrics(&metrics);
        fMaxRunAscent = SkTMin(fMaxRunAscent, metrics.fAscent);
        fMaxRunDescent = SkTMax(fMaxRunDescent, metrics.fDescent);
        fMaxRunLeading = SkTMax(fMaxRunLeading, metrics.fLeading);
    }

    void commitRunInfo() override {
        fCurrentPosition.fY -= fMaxRunAscent;
    }

    Buffer runBuffer(const RunInfo& info) override {
        int glyphCount = SkTFitsIn<int>(info.glyphCount) ? info.glyphCount : INT_MAX;

        SkFontMetrics metrics;
        info.fFont.getMetrics(&metrics);

        const auto& blobBuffer = fBuilder.allocRunPos(info.fFont, glyphCount);

        SkVector alignmentOffset {
            fAlignFactor * (fPendingLineAdvance.x() - fBox.width()),

            // When in point mode, the given position represents the baseline
            //   => we adjust for SkShaper which treats it as (baseline - ascent).
            fBox.isEmpty() ? metrics.fAscent : 0
        };

        return {
            blobBuffer.glyphs,
            blobBuffer.points(),
            nullptr,
            nullptr,
            fCurrentPosition + alignmentOffset
        };
    }

    void commitRunBuffer(const RunInfo& info) override {
        fCurrentPosition += info.fAdvance;
    }

    void commitLine() override {
        fOffset += { 0, fMaxRunDescent + fMaxRunLeading - fMaxRunAscent };
    }

    sk_sp<SkTextBlob> makeBlob() {
        return fBuilder.make();
    }

    void shapeLine(const char* start, const char* end) {
        if (!fShaper) {
            return;
        }

        // When no text box is present, text is laid out on a single infinite line
        // (modulo explicit line breaks).
        const auto shape_width = fBox.isEmpty() ? SK_ScalarMax
                                                : fBox.width();

        fShaper->shape(start, SkToSizeT(end - start), fFont, true, shape_width, this);
    }

private:
    static float AlignFactor(SkTextUtils::Align align) {
        switch (align) {
        case SkTextUtils::kLeft_Align:   return  0.0f;
        case SkTextUtils::kCenter_Align: return -0.5f;
        case SkTextUtils::kRight_Align:  return -1.0f;
        }
        return 0.0f; // go home, msvc...
    }

    struct Run {
        SkFont                          fFont;
        SkShaper::RunHandler::RunInfo   fInfo;
        SkSTArray<128, SkGlyphID, true> fGlyphs;
        SkSTArray<128, SkPoint  , true> fPositions;

        Run(const SkFont& font, const SkShaper::RunHandler::RunInfo& info, int count)
            : fFont(font)
            , fInfo(info)
            , fGlyphs   (count)
            , fPositions(count) {
            fGlyphs   .push_back_n(count);
            fPositions.push_back_n(count);
        }

        size_t size() const {
            SkASSERT(fGlyphs.size() == fPositions.size());
            return fGlyphs.size();
        }
    };

    const Shaper::TextDesc&   fDesc;
    const SkRect&             fBox;
    const float               fAlignFactor;

    SkFont                    fFont;
    SkTextBlobBuilder         fBuilder;
    std::unique_ptr<SkShaper> fShaper;

    SkScalar fMaxRunAscent;
    SkScalar fMaxRunDescent;
    SkScalar fMaxRunLeading;
    SkPoint fCurrentPosition{0,0};
    SkPoint fOffset;
    SkVector fPendingLineAdvance{ 0, 0 };
};

Shaper::Result ShapeImpl(const SkString& txt, const Shaper::TextDesc& desc, const SkRect& box) {
    const auto& is_line_break = [](SkUnichar uch) {
        // TODO: other explicit breaks?
        return uch == '\r';
    };

    const char* ptr        = txt.c_str();
    const char* line_start = ptr;
    const char* end        = ptr + txt.size();

    BlobMaker blobMaker(desc, box);
    while (ptr < end) {
        if (is_line_break(SkUTF::NextUTF8(&ptr, end))) {
            blobMaker.shapeLine(line_start, ptr - 1);
            line_start = ptr;
        }
    }
    blobMaker.shapeLine(line_start, ptr);

    return {
        blobMaker.makeBlob(),
        { 0, 0 }               // the box offset is currently/temporarily baked into the blob.
    };
}

} // namespace

Shaper::Result Shaper::Shape(const SkString& txt, const TextDesc& desc, const SkPoint& point) {
    return ShapeImpl(txt, desc, SkRect::MakeEmpty().makeOffset(point.x(), point.y()));
}

Shaper::Result Shaper::Shape(const SkString& txt, const TextDesc& desc, const SkRect& box) {
    return ShapeImpl(txt, desc, box);
}

SkRect Shaper::Result::computeBounds() const {
    auto bounds = SkRect::MakeEmpty();

    if (!fBlob) {
        return bounds;
    }

    SkAutoSTArray<16, SkRect> glyphBounds;

    SkTextBlobRunIterator it(fBlob.get());

    for (SkTextBlobRunIterator it(fBlob.get()); !it.done(); it.next()) {
        glyphBounds.reset(SkToInt(it.glyphCount()));
        it.font().getBounds(it.glyphs(), it.glyphCount(), glyphBounds.get(), nullptr);

        SkASSERT(it.positioning() == SkTextBlobRunIterator::kFull_Positioning);
        for (uint32_t i = 0; i < it.glyphCount(); ++i) {
            bounds.join(glyphBounds[i].makeOffset(it.pos()[i * 2    ],
                                                  it.pos()[i * 2 + 1]));
        }
    }

    return bounds;
}

} // namespace skottie
