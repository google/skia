/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/text/SkottieShaper.h"

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
    }

    void runInfo(const RunInfo& info) override {
        fPendingLineAdvance += info.fAdvance;
    }

    void commitRunInfo() override {}

    Buffer runBuffer(const RunInfo& info) override {
        int glyphCount = SkTFitsIn<int>(info.glyphCount) ? info.glyphCount : INT_MAX;

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
        fOffset.fY += fDesc.fLineHeight;
    }

    Shaper::Result makeBlob() {
        auto blob = fBuilder.make();

        SkPoint pos {fBox.x(), fBox.y()};

        // By default, first line is vertical-aligned on a baseline of 0.
        // Perform additional adjustments based on VAlign.
        switch (fDesc.fVAlign) {
        case Shaper::VAlign::kTop:
            pos.fY -= ComputeBlobBounds(blob).fTop;
            break;
        case Shaper::VAlign::kTopBaseline:
            // Default behavior.
            break;
        case Shaper::VAlign::kCenter: {
            const auto bounds = ComputeBlobBounds(blob).makeOffset(pos.x(), pos.y());
            pos.fY += fBox.centerY() - bounds.centerY();
        } break;
        case Shaper::VAlign::kBottom:
            pos.fY += fBox.height() - ComputeBlobBounds(blob).fBottom;
            break;
        case Shaper::VAlign::kResizeToFit:
            SkASSERT(false);
            break;
        }

        // single blob for now
        return { std::vector<Shaper::Fragment>(1ul, { std::move(blob), pos })};
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

    SkPoint  fCurrentPosition{ 0, 0 };
    SkPoint  fOffset{ 0, 0 };
    SkVector fPendingLineAdvance{ 0, 0 };
};

Shaper::Result ShapeImpl(const SkString& txt, const Shaper::TextDesc& desc, const SkRect& box) {
    SkASSERT(desc.fVAlign != Shaper::VAlign::kResizeToFit);

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

Shaper::Result ShapeToFit(const SkString& txt, const Shaper::TextDesc& orig_desc,
                          const SkRect& box) {
    SkASSERT(orig_desc.fVAlign == Shaper::VAlign::kResizeToFit);

    Shaper::Result best_result;

    if (box.isEmpty() || orig_desc.fTextSize <= 0) {
        return best_result;
    }

    auto desc = orig_desc;
    desc.fVAlign = Shaper::VAlign::kCenter;

    float in_scale = 0,                                 // maximum scale that fits inside
         out_scale = std::numeric_limits<float>::max(), // minimum scale that doesn't fit
         try_scale = 1;                                 // current probe

    // Perform a binary search for the best vertical fit (SkShaper already handles
    // horizontal fitting), starting with the specified text size.
    //
    // This hybrid loop handles both the binary search (when in/out extremes are known), and an
    // exponential search for the extremes.
    static constexpr size_t kMaxIter = 16;
    for (size_t i = 0; i < kMaxIter; ++i) {
        SkASSERT(try_scale >= in_scale && try_scale <= out_scale);
        desc.fTextSize   = try_scale * orig_desc.fTextSize;
        desc.fLineHeight = try_scale * orig_desc.fLineHeight;

        auto res = ShapeImpl(txt, desc, box);
        auto res_height = res.computeBounds().height();

        if (res_height > box.height()) {
            out_scale = try_scale;
            try_scale = (in_scale == 0)
                    ? try_scale * 0.5f // initial in_scale not found yet - search exponentially
                    : (in_scale + out_scale) * 0.5f; // in_scale found - binary search
        } else {
            // It fits - so it's a candidate.
            best_result = std::move(res);
            static constexpr float kTolerance = 1;
            if (box.height() - res_height <= kTolerance) {
                // Jackpot.
                break;
            }

            in_scale = try_scale;
            try_scale = (out_scale == std::numeric_limits<float>::max())
                    ? try_scale * 2 // initial out_scale not found yet - search exponentially
                    : (in_scale + out_scale) * 0.5f; // out_scale found - binary search
        }
    }

    return best_result;
}

} // namespace

Shaper::Result Shaper::Shape(const SkString& txt, const TextDesc& desc, const SkPoint& point) {
    return (desc.fVAlign == VAlign::kResizeToFit) // makes no sense in point mode
            ? Result()
            : ShapeImpl(txt, desc, SkRect::MakeEmpty().makeOffset(point.x(), point.y()));
}

Shaper::Result Shaper::Shape(const SkString& txt, const TextDesc& desc, const SkRect& box) {
    return (desc.fVAlign == VAlign::kResizeToFit)
            ? ShapeToFit(txt, desc, box)
            : ShapeImpl(txt, desc, box);
}

SkRect Shaper::Result::computeBounds() const {
    auto bounds = SkRect::MakeEmpty();

    for (const auto& fragment : fFragments) {
        bounds.join(ComputeBlobBounds(fragment.fBlob).makeOffset(fragment.fPos.x(),
                                                                 fragment.fPos.y()));
    }

    return bounds;
}

} // namespace skottie
