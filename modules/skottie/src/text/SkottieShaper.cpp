/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/text/SkottieShaper.h"

#include "include/core/SkFontMetrics.h"
#include "include/core/SkTextBlob.h"
#include "modules/skshaper/include/SkShaper.h"
#include "src/core/SkTextBlobPriv.h"
#include "src/utils/SkUTF.h"

#include <limits.h>

namespace skottie {
namespace {

SkRect ComputeBlobBounds(const sk_sp<SkTextBlob>& blob) {
    auto bounds = SkRect::MakeEmpty();

    if (!blob) {
        return bounds;
    }

    SkAutoSTArray<16, SkRect> glyphBounds;

    SkTextBlobRunIterator it(blob.get());

    for (SkTextBlobRunIterator it(blob.get()); !it.done(); it.next()) {
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

// Helper for interfacing with SkShaper: buffers shaper-fed runs and performs
// per-line position adjustments (for external line breaking, horizontal alignment, etc).
class BlobMaker final : public SkShaper::RunHandler {
public:
    BlobMaker(const Shaper::TextDesc& desc, const SkRect& box)
        : fDesc(desc)
        , fBox(box)
        , fHAlignFactor(HAlignFactor(fDesc.fHAlign))
        , fFont(fDesc.fTypeface, fDesc.fTextSize)
        , fShaper(SkShaper::Make()) {
        fFont.setHinting(SkFontHinting::kNone);
        fFont.setSubpixel(true);
        fFont.setLinearMetrics(true);
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

    void commitRunInfo() override {}

    Buffer runBuffer(const RunInfo& info) override {
        int glyphCount = SkTFitsIn<int>(info.glyphCount) ? info.glyphCount : INT_MAX;

        SkFontMetrics metrics;
        info.fFont.getMetrics(&metrics);

        const auto& blobBuffer = fBuilder.allocRunPos(info.fFont, glyphCount);

        SkVector alignmentOffset { fHAlignFactor * (fPendingLineAdvance.x() - fBox.width()), 0 };

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
        // Observe explicit line height, if specified; otherwise use line metrics.
        const auto lh = fDesc.fLineHeight > 0
                ? fDesc.fLineHeight
                : fMaxRunDescent + fMaxRunLeading - fMaxRunAscent;
        fOffset.fY += lh;
    }

    Shaper::Result makeBlob() {
        auto blob = fBuilder.make();

        SkPoint pos {fBox.x(), fBox.y()};

        // By default, first line is vertical-aligned on a baseline of 0.
        // Perform additional adjustments based on VAlign.
        switch (fDesc.fVAlign) {
        case Shaper::VAlign::kTop: {
            pos.offset(0, -ComputeBlobBounds(blob).fTop);
        } break;
        case Shaper::VAlign::kTopBaseline:
            // Default behavior.
            break;
        case Shaper::VAlign::kCenter: {
            const auto bounds = ComputeBlobBounds(blob).makeOffset(pos.x(), pos.y());
            pos.offset(0, fBox.centerY() - bounds.centerY());
        } break;
        }

        return {
            std::move(blob),
            pos
        };
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
    static float HAlignFactor(SkTextUtils::Align align) {
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
    const float               fHAlignFactor;

    SkFont                    fFont;
    SkTextBlobBuilder         fBuilder;
    std::unique_ptr<SkShaper> fShaper;

    SkScalar fMaxRunAscent;
    SkScalar fMaxRunDescent;
    SkScalar fMaxRunLeading;
    SkPoint  fCurrentPosition{ 0, 0 };
    SkPoint  fOffset{ 0, 0 };
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

    return blobMaker.makeBlob();
}

} // namespace

Shaper::Result Shaper::Shape(const SkString& txt, const TextDesc& desc, const SkPoint& point) {
    return ShapeImpl(txt, desc, SkRect::MakeEmpty().makeOffset(point.x(), point.y()));
}

Shaper::Result Shaper::Shape(const SkString& txt, const TextDesc& desc, const SkRect& box) {
    return ShapeImpl(txt, desc, box);
}

SkRect Shaper::Result::computeBounds() const {
    return ComputeBlobBounds(fBlob).makeOffset(fPos.x(), fPos.y());
}

} // namespace skottie
