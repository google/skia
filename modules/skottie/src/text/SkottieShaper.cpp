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
#include "include/private/SkTPin.h"
#include "include/private/SkTemplates.h"
#include "modules/skshaper/include/SkShaper.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "src/core/SkTLazy.h"
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
        fFont.setBaselineSnap(false);
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
            fFirstLineAscent = std::min(fFirstLineAscent, metrics.fAscent);
        }
        fLastLineDescent = std::max(fLastLineDescent, metrics.fDescent);
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

    Shaper::Result finalize(SkSize* shaped_size) {
        if (!(fDesc.fFlags & Shaper::Flags::kFragmentGlyphs)) {
            // All glyphs are pending in a single blob.
            SkASSERT(fResult.fFragments.empty());
            fResult.fFragments.reserve(1);
            fResult.fFragments.push_back({fBuilder.make(), {fBox.x(), fBox.y()}, 0, 0, 0, false});
        }

        const auto ascent = this->ascent();

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

        // Only compute the extent box when needed.
        SkTLazy<SkRect> ebox;

        // Vertical adjustments.
        float v_offset = -fDesc.fLineShift;

        switch (fDesc.fVAlign) {
        case Shaper::VAlign::kTop:
            v_offset -= ascent;
            break;
        case Shaper::VAlign::kTopBaseline:
            // Default behavior.
            break;
        case Shaper::VAlign::kVisualTop:
            ebox.init(extent_box());
            v_offset += fBox.fTop - ebox->fTop;
            break;
        case Shaper::VAlign::kVisualCenter:
            ebox.init(extent_box());
            v_offset += fBox.centerY() - ebox->centerY();
            break;
        case Shaper::VAlign::kVisualBottom:
            ebox.init(extent_box());
            v_offset += fBox.fBottom - ebox->fBottom;
            break;
        }

        if (shaped_size) {
            if (!ebox.isValid()) {
                ebox.init(extent_box());
            }
            *shaped_size = SkSize::Make(ebox->width(), ebox->height());
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

        // In default paragraph mode (VAlign::kTop), AE clips out lines when the baseline
        // goes below the box lower edge.
        if (fDesc.fVAlign == Shaper::VAlign::kTop) {
            // fOffset is relative to the first line baseline.
            const auto max_offset = fBox.height() + this->ascent(); // NB: ascent is negative
            if (fOffset.y() > max_offset) {
                return;
            }
        }

        const auto shape_width = fDesc.fLinebreak == Shaper::LinebreakPolicy::kExplicit
                                    ? SK_ScalarMax
                                    : fBox.width();
        const auto shape_ltr   = fDesc.fDirection == Shaper::Direction::kLTR;

        fUTF8 = start;
        fShaper->shape(start, SkToSizeT(end - start), fFont, shape_ltr, shape_width, this);
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

        float ascent = 0;

        if (fDesc.fFlags & Shaper::Flags::kTrackFragmentAdvanceAscent) {
            SkFontMetrics metrics;
            rec.fFont.getMetrics(&metrics);
            ascent = metrics.fAscent;

            // Note: we use per-glyph advances for anchoring, but it's unclear whether this
            // is exactly the same as AE.  E.g. are 'acute' glyphs anchored separately for fonts
            // in which they're distinct?
            fAdvanceBuffer.resize(rec.fGlyphCount);
            fFont.getWidths(glyphs, SkToInt(rec.fGlyphCount), fAdvanceBuffer.data());
        }

        // In fragmented mode we immediately push the glyphs to fResult,
        // one fragment (blob) per glyph.  Glyph positioning is externalized
        // (positions returned in Fragment::fPos).
        for (size_t i = 0; i < rec.fGlyphCount; ++i) {
            const auto& blob_buffer = fBuilder.allocRunPos(rec.fFont, 1);
            blob_buffer.glyphs[0] = glyphs[i];
            blob_buffer.pos[0] = blob_buffer.pos[1] = 0;

            const auto advance = (fDesc.fFlags & Shaper::Flags::kTrackFragmentAdvanceAscent)
                    ? fAdvanceBuffer[SkToInt(i)]
                    : 0.0f;

            // Note: we only check the first code point in the cluster for whitespace.
            // It's unclear whether thers's a saner approach.
            fResult.fFragments.push_back({fBuilder.make(),
                                          { fBox.x() + pos[i].fX, fBox.y() + pos[i].fY },
                                          advance, ascent,
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

    SkScalar ascent() const {
        // Use the explicit ascent, when specified.
        // Note: ascent values are negative (relative to the baseline).
        return fDesc.fAscent ? fDesc.fAscent : fFirstLineAscent;
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

    SkSTArray<64, float, true>    fAdvanceBuffer;

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
                         SkSize* shaped_size = nullptr) {
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

    return blobMaker.finalize(shaped_size);
}

Shaper::Result ShapeToFit(const SkString& txt, const Shaper::TextDesc& orig_desc,
                          const SkRect& box, const sk_sp<SkFontMgr>& fontmgr) {
    Shaper::Result best_result;

    if (box.isEmpty() || orig_desc.fTextSize <= 0) {
        return best_result;
    }

    auto desc = orig_desc;

    const auto min_scale = std::max(desc.fMinTextSize / desc.fTextSize, 0.0f),
               max_scale = std::max(desc.fMaxTextSize / desc.fTextSize, min_scale);

    float in_scale = min_scale,                          // maximum scale that fits inside
         out_scale = max_scale,                          // minimum scale that doesn't fit
         try_scale = SkTPin(1.0f, min_scale, max_scale); // current probe

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
        desc.fLineShift  = try_scale * orig_desc.fLineShift;
        desc.fAscent     = try_scale * orig_desc.fAscent;

        SkSize res_size = {0, 0};
        auto res = ShapeImpl(txt, desc, box, fontmgr, &res_size);

        const auto prev_scale = try_scale;
        if (res_size.width() > box.width() || res_size.height() > box.height()) {
            out_scale = try_scale;
            try_scale = (in_scale == min_scale)
                    // initial in_scale not found yet - search exponentially
                    ? std::max(min_scale, try_scale * 0.5f)
                    // in_scale found - binary search
                    : (in_scale + out_scale) * 0.5f;
        } else {
            // It fits - so it's a candidate.
            best_result = std::move(res);

            in_scale = try_scale;
            try_scale = (out_scale == max_scale)
                    // initial out_scale not found yet - search exponentially
                    ? std::min(max_scale, try_scale * 2)
                    // out_scale found - binary search
                    : (in_scale + out_scale) * 0.5f;
        }

        if (try_scale == prev_scale) {
            // no more progress
            break;
        }
    }

    return best_result;
}


// Applies capitalization rules.
class AdjustedText {
public:
    AdjustedText(const SkString& txt, const Shaper::TextDesc& desc)
        : fText(txt) {
        switch (desc.fCapitalization) {
        case Shaper::Capitalization::kNone:
            break;
        case Shaper::Capitalization::kUpperCase:
#ifdef SK_UNICODE_AVAILABLE
            if (auto skuni = SkUnicode::Make()) {
                *fText.writable() = skuni->toUpper(*fText);
            }
#endif
            break;
        }
    }

    operator const SkString&() const { return *fText; }

private:
    SkTCopyOnFirstWrite<SkString> fText;
};

} // namespace

Shaper::Result Shaper::Shape(const SkString& orig_txt, const TextDesc& desc, const SkPoint& point,
                             const sk_sp<SkFontMgr>& fontmgr) {
    const AdjustedText txt(orig_txt, desc);

    return (desc.fResize == ResizePolicy::kScaleToFit ||
            desc.fResize == ResizePolicy::kDownscaleToFit) // makes no sense in point mode
            ? Result()
            : ShapeImpl(txt, desc, SkRect::MakeEmpty().makeOffset(point.x(), point.y()), fontmgr);
}

Shaper::Result Shaper::Shape(const SkString& orig_txt, const TextDesc& desc, const SkRect& box,
                             const sk_sp<SkFontMgr>& fontmgr) {
    const AdjustedText txt(orig_txt, desc);

    switch(desc.fResize) {
    case ResizePolicy::kNone:
        return ShapeImpl(txt, desc, box, fontmgr);
    case ResizePolicy::kScaleToFit:
        return ShapeToFit(txt, desc, box, fontmgr);
    case ResizePolicy::kDownscaleToFit: {
        SkSize size;
        auto result = ShapeImpl(txt, desc, box, fontmgr, &size);

        return (size.width() <= box.width() && size.height() <= box.height())
                ? result
                : ShapeToFit(txt, desc, box, fontmgr);
    }
    }

    SkUNREACHABLE;
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
