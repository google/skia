/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/text/SkottieShaper.h"

#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkTextBlob.h"
#include "include/private/SkTemplates.h"
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
    BlobMaker(const Shaper::TextDesc& desc, const SkRect& box, const sk_sp<SkFontMgr>& fontmgr)
        : fDesc(desc)
        , fBox(box)
        , fHAlignFactor(HAlignFactor(fDesc.fHAlign))
        , fFont(fDesc.fTypeface, fDesc.fTextSize)
        , fShaper(SkShaper::Make(fontmgr)) {
        fFont.setHinting(SkFontHinting::kNone);
        fFont.setSubpixel(true);
        fFont.setLinearMetrics(true);
        fFont.setEdging(SkFont::Edging::kAntiAlias);
    }

    void beginLine() override {
        fLineGlyphs.reset(0);
        fLinePos.reset(0);
        fLineClusters.reset(0);
        fLineRuns.reset();
        fLineGlyphCount = 0;

        fCurrentPosition = fOffset;
        fPendingLineAdvance  = { 0, 0 };

        fLastLineDescent = 0;
    }

    void runInfo(const RunInfo& info) override {
        fPendingLineAdvance += info.fAdvance;

        SkFontMetrics metrics;
        info.fFont.getMetrics(&metrics);
        if (!fLineCount) {
            fFirstLineAscent = SkTMin(fFirstLineAscent, metrics.fAscent);
        }
        fLastLineDescent = SkTMax(fLastLineDescent, metrics.fDescent);
    }

    void commitRunInfo() override {}

    Buffer runBuffer(const RunInfo& info) override {
        const auto run_start_index = fLineGlyphCount;
        fLineGlyphCount += info.glyphCount;

        fLineGlyphs.realloc(fLineGlyphCount);
        fLinePos.realloc(fLineGlyphCount);
        fLineClusters.realloc(fLineGlyphCount);
        fLineRuns.push_back({info.fFont, info.glyphCount});

        SkVector alignmentOffset { fHAlignFactor * (fPendingLineAdvance.x() - fBox.width()), 0 };

        return {
            fLineGlyphs.get()   + run_start_index,
            fLinePos.get()      + run_start_index,
            nullptr,
            fLineClusters.get() + run_start_index,
            fCurrentPosition + alignmentOffset
        };
    }

    void commitRunBuffer(const RunInfo& info) override {
        fCurrentPosition += info.fAdvance;
    }

    void commitLine() override {
        fOffset.fY += fDesc.fLineHeight;

        // TODO: justification adjustments

        const auto commit_proc = (fDesc.fFlags & Shaper::Flags::kFragmentGlyphs)
            ? &BlobMaker::commitFragementedRun
            : &BlobMaker::commitConsolidatedRun;

        size_t run_offset = 0;
        for (const auto& rec : fLineRuns) {
            SkASSERT(run_offset < fLineGlyphCount);
            (this->*commit_proc)(rec,
                        fLineGlyphs.get()   + run_offset,
                        fLinePos.get()      + run_offset,
                        fLineClusters.get() + run_offset,
                        fLineCount);
            run_offset += rec.fGlyphCount;
        }

        fLineCount++;
    }

    Shaper::Result finalize(float* shaped_height) {
        if (!(fDesc.fFlags & Shaper::Flags::kFragmentGlyphs)) {
            // All glyphs are pending in a single blob.
            SkASSERT(fResult.fFragments.empty());
            fResult.fFragments.reserve(1);
            fResult.fFragments.push_back({fBuilder.make(), {fBox.x(), fBox.y()}, 0, false});
        }

        // Use the explicit ascent, when specified.
        // Note: ascent values are negative (relative to the baseline).
        const auto ascent = fDesc.fAscent ? fDesc.fAscent : fFirstLineAscent;

        // For visual VAlign modes, we use a hybrid extent box computed as the union of
        // actual visual bounds and the vertical typographical extent.
        //
        // This ensures that
        //
        //   a) text doesn't visually overflow the alignment boundaries
        //
        //   b) leading/trailing empty lines are still taken into account for alignment purposes

        auto extent_box = [&]() {
            auto box = fResult.computeVisualBounds();

            // By default, first line is vertically-aligned on a baseline of 0.
            // The typographical height considered for vertical alignment is the distance between
            // the first line top (ascent) to the last line bottom (descent).
            const auto typographical_top    = fBox.fTop + ascent,
                       typographical_bottom = fBox.fTop + fLastLineDescent + fDesc.fLineHeight *
                                                           (fLineCount > 0 ? fLineCount - 1 : 0ul);

            box.fTop    = std::min(box.fTop,    typographical_top);
            box.fBottom = std::max(box.fBottom, typographical_bottom);

            return box;
        };

        SkASSERT(!shaped_height || fDesc.fVAlign == Shaper::VAlign::kVisualCenter);

        // Perform additional adjustments based on VAlign.
        float v_offset = 0;
        switch (fDesc.fVAlign) {
        case Shaper::VAlign::kTop:
            v_offset = -ascent;
            break;
        case Shaper::VAlign::kTopBaseline:
            // Default behavior.
            break;
        case Shaper::VAlign::kVisualTop:
            v_offset = fBox.fTop - extent_box().fTop;
            break;
        case Shaper::VAlign::kVisualCenter: {
            const auto ebox = extent_box();
            v_offset = fBox.centerY() - ebox.centerY();
            if (shaped_height) {
                *shaped_height = ebox.height();
            }
        } break;
        case Shaper::VAlign::kVisualBottom:
            v_offset = fBox.fBottom - extent_box().fBottom;
            break;
        case Shaper::VAlign::kVisualResizeToFit:
        case Shaper::VAlign::kVisualDownscaleToFit:
            SkASSERT(false);
            break;
        }

        if (v_offset) {
            for (auto& fragment : fResult.fFragments) {
                fragment.fPos.fY += v_offset;
            }
        }

        return std::move(fResult);
    }

    void shapeLine(const char* start, const char* end) {
        if (!fShaper) {
            return;
        }

        SkASSERT(start <= end);
        if (start == end) {
            // SkShaper doesn't care for empty lines.
            this->beginLine();
            this->commitLine();
            return;
        }

        // When no text box is present, text is laid out on a single infinite line
        // (modulo explicit line breaks).
        const auto shape_width = fBox.isEmpty() ? SK_ScalarMax
                                                : fBox.width();

        fUTF8 = start;
        fShaper->shape(start, SkToSizeT(end - start), fFont, true, shape_width, this);
        fUTF8 = nullptr;
    }

private:
    struct RunRec {
        SkFont fFont;
        size_t fGlyphCount;
    };

    void commitFragementedRun(const RunRec& rec,
                              const SkGlyphID* glyphs,
                              const SkPoint* pos,
                              const uint32_t* clusters,
                              uint32_t line_index) {

        static const auto is_whitespace = [](char c) {
            return c == ' ' || c == '\t' || c == '\r' || c == '\n';
        };

        // In fragmented mode we immediately push the glyphs to fResult,
        // one fragment (blob) per glyph.  Glyph positioning is externalized
        // (positions returned in Fragment::fPos).
        for (size_t i = 0; i < rec.fGlyphCount; ++i) {
            const auto& blob_buffer = fBuilder.allocRunPos(rec.fFont, 1);
            blob_buffer.glyphs[0] = glyphs[i];
            blob_buffer.pos[0] = blob_buffer.pos[1] = 0;

            // Note: we only check the first code point in the cluster for whitespace.
            // It's unclear whether thers's a saner approach.
            fResult.fFragments.push_back({fBuilder.make(),
                                          { fBox.x() + pos[i].fX, fBox.y() + pos[i].fY },
                                          line_index, is_whitespace(fUTF8[clusters[i]])
                                         });
            fResult.fMissingGlyphCount += (glyphs[i] == kMissingGlyphID);
        }
    }

    void commitConsolidatedRun(const RunRec& rec,
                               const SkGlyphID* glyphs,
                               const SkPoint* pos,
                               const uint32_t*,
                               uint32_t) {
        // In consolidated mode we just accumulate glyphs to the blob builder, then push
        // to fResult as a single blob in finalize().  Glyph positions are baked in the
        // blob (Fragment::fPos only reflects the box origin).
        const auto& blob_buffer = fBuilder.allocRunPos(rec.fFont, rec.fGlyphCount);
        for (size_t i = 0; i < rec.fGlyphCount; ++i) {
            blob_buffer.glyphs[i] = glyphs[i];
            fResult.fMissingGlyphCount += (glyphs[i] == kMissingGlyphID);
        }
        sk_careful_memcpy(blob_buffer.pos   , pos   , rec.fGlyphCount * sizeof(SkPoint));
    }

    static float HAlignFactor(SkTextUtils::Align align) {
        switch (align) {
        case SkTextUtils::kLeft_Align:   return  0.0f;
        case SkTextUtils::kCenter_Align: return -0.5f;
        case SkTextUtils::kRight_Align:  return -1.0f;
        }
        return 0.0f; // go home, msvc...
    }

    static constexpr SkGlyphID kMissingGlyphID = 0;

    const Shaper::TextDesc&   fDesc;
    const SkRect&             fBox;
    const float               fHAlignFactor;

    SkFont                    fFont;
    SkTextBlobBuilder         fBuilder;
    std::unique_ptr<SkShaper> fShaper;

    SkAutoSTMalloc<64, SkGlyphID> fLineGlyphs;
    SkAutoSTMalloc<64, SkPoint>   fLinePos;
    SkAutoSTMalloc<64, uint32_t>  fLineClusters;
    SkSTArray<16, RunRec>         fLineRuns;
    size_t                        fLineGlyphCount = 0;

    SkPoint  fCurrentPosition{ 0, 0 };
    SkPoint  fOffset{ 0, 0 };
    SkVector fPendingLineAdvance{ 0, 0 };
    uint32_t fLineCount = 0;
    float    fFirstLineAscent = 0,
             fLastLineDescent = 0;

    const char* fUTF8 = nullptr; // only valid during shapeLine() calls

    Shaper::Result fResult;
};

Shaper::Result ShapeImpl(const SkString& txt, const Shaper::TextDesc& desc,
                         const SkRect& box, const sk_sp<SkFontMgr>& fontmgr,
                         float* shaped_height = nullptr) {
    SkASSERT(desc.fVAlign != Shaper::VAlign::kVisualResizeToFit);

    const auto& is_line_break = [](SkUnichar uch) {
        // TODO: other explicit breaks?
        return uch == '\r';
    };

    const char* ptr        = txt.c_str();
    const char* line_start = ptr;
    const char* end        = ptr + txt.size();

    BlobMaker blobMaker(desc, box, fontmgr);
    while (ptr < end) {
        if (is_line_break(SkUTF::NextUTF8(&ptr, end))) {
            blobMaker.shapeLine(line_start, ptr - 1);
            line_start = ptr;
        }
    }
    blobMaker.shapeLine(line_start, ptr);

    return blobMaker.finalize(shaped_height);
}

Shaper::Result ShapeToFit(const SkString& txt, const Shaper::TextDesc& orig_desc,
                          const SkRect& box, const sk_sp<SkFontMgr>& fontmgr) {
    SkASSERT(orig_desc.fVAlign == Shaper::VAlign::kVisualResizeToFit);

    Shaper::Result best_result;

    if (box.isEmpty() || orig_desc.fTextSize <= 0) {
        return best_result;
    }

    auto desc = orig_desc;
    desc.fVAlign = Shaper::VAlign::kVisualCenter;

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
        desc.fAscent     = try_scale * orig_desc.fAscent;

        float res_height = 0;
        auto res = ShapeImpl(txt, desc, box, fontmgr, &res_height);

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

Shaper::Result Shaper::Shape(const SkString& txt, const TextDesc& desc, const SkPoint& point,
                             const sk_sp<SkFontMgr>& fontmgr) {
    return (desc.fVAlign == VAlign::kVisualResizeToFit ||
            desc.fVAlign == VAlign::kVisualDownscaleToFit) // makes no sense in point mode
            ? Result()
            : ShapeImpl(txt, desc, SkRect::MakeEmpty().makeOffset(point.x(), point.y()), fontmgr);
}

Shaper::Result Shaper::Shape(const SkString& txt, const TextDesc& desc, const SkRect& box,
                             const sk_sp<SkFontMgr>& fontmgr) {
    if (desc.fVAlign == VAlign::kVisualResizeToFit) {
        return ShapeToFit(txt, desc, box, fontmgr);
    }

    if (desc.fVAlign == VAlign::kVisualDownscaleToFit) {
        auto adjusted_desc = desc;
        adjusted_desc.fVAlign = VAlign::kVisualCenter;

        float height;
        auto result = ShapeImpl(txt, adjusted_desc, box, fontmgr, &height);

        if (height <= box.height()) {
            return result;
        }

        adjusted_desc.fVAlign = VAlign::kVisualResizeToFit;

        return ShapeToFit(txt, adjusted_desc, box, fontmgr);
    }

    return ShapeImpl(txt, desc, box, fontmgr);
}

SkRect Shaper::Result::computeVisualBounds() const {
    auto bounds = SkRect::MakeEmpty();

    for (const auto& fragment : fFragments) {
        bounds.join(ComputeBlobBounds(fragment.fBlob).makeOffset(fragment.fPos.x(),
                                                                 fragment.fPos.y()));
    }

    return bounds;
}

} // namespace skottie
