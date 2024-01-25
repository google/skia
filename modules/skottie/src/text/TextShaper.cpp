/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/include/TextShaper.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTPin.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "modules/skshaper/include/SkShaper.h"
#include "src/base/SkTLazy.h"
#include "src/base/SkUTF.h"
#include "src/core/SkFontPriv.h"

#ifdef SK_UNICODE_AVAILABLE
#include "modules/skunicode/include/SkUnicode.h"
#endif

#include <algorithm>
#include <limits.h>
#include <numeric>

using namespace skia_private;

namespace skottie {
namespace {

static bool is_whitespace(char c) {
    // TODO: we've been getting away with this simple heuristic,
    // but ideally we should use SkUicode::isWhiteSpace().
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

// Helper for interfacing with SkShaper: buffers shaper-fed runs and performs
// per-line position adjustments (for external line breaking, horizontal alignment, etc).
class ResultBuilder final : public SkShaper::RunHandler {
public:
    ResultBuilder(const Shaper::TextDesc& desc, const SkRect& box, const sk_sp<SkFontMgr>& fontmgr)
        : fDesc(desc)
        , fBox(box)
        , fHAlignFactor(HAlignFactor(fDesc.fHAlign))
        , fFont(fDesc.fTypeface, fDesc.fTextSize)
        , fFontMgr(fontmgr)
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
        fLineRuns.clear();
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

        // Observed AE handling of whitespace, for alignment purposes:
        //
        //   - leading whitespace contributes to alignment
        //   - trailing whitespace is ignored
        //   - auto line breaking retains all separating whitespace on the first line (no artificial
        //     leading WS is created).
        auto adjust_trailing_whitespace = [this]() {
            // For left-alignment, trailing WS doesn't make any difference.
            if (fLineRuns.empty() || fDesc.fHAlign == SkTextUtils::Align::kLeft_Align) {
                return;
            }

            // Technically, trailing whitespace could span multiple runs, but realistically,
            // SkShaper has no reason to split it.  Hence we're only checking the last run.
            size_t ws_count = 0;
            for (size_t i = 0; i < fLineRuns.back().fSize; ++i) {
                if (is_whitespace(fUTF8[fLineClusters[SkToInt(fLineGlyphCount - i - 1)]])) {
                    ++ws_count;
                } else {
                    break;
                }
            }

            // No trailing whitespace.
            if (!ws_count) {
                return;
            }

            // Compute the cumulative whitespace advance.
            fAdvanceBuffer.resize(ws_count);
            fLineRuns.back().fFont.getWidths(fLineGlyphs.data() + fLineGlyphCount - ws_count,
                                             SkToInt(ws_count), fAdvanceBuffer.data(), nullptr);

            const auto ws_advance = std::accumulate(fAdvanceBuffer.begin(),
                                                    fAdvanceBuffer.end(),
                                                    0.0f);

            // Offset needed to compensate for whitespace.
            const auto offset = ws_advance*-fHAlignFactor;

            // Shift the whole line horizontally by the computed offset.
            std::transform(fLinePos.data(),
                           fLinePos.data() + fLineGlyphCount,
                           fLinePos.data(),
                           [&offset](SkPoint pos) { return SkPoint{pos.fX + offset, pos.fY}; });
        };

        adjust_trailing_whitespace();

        const auto commit_proc = (fDesc.fFlags & Shaper::Flags::kFragmentGlyphs)
            ? &ResultBuilder::commitFragementedRun
            : &ResultBuilder::commitConsolidatedRun;

        size_t run_offset = 0;
        for (const auto& rec : fLineRuns) {
            SkASSERT(run_offset < fLineGlyphCount);
            (this->*commit_proc)(rec,
                        fLineGlyphs.get()   + run_offset,
                        fLinePos.get()      + run_offset,
                        fLineClusters.get() + run_offset,
                        fLineCount);
            run_offset += rec.fSize;
        }

        fLineCount++;
    }

    Shaper::Result finalize(SkSize* shaped_size) {
        if (!(fDesc.fFlags & Shaper::Flags::kFragmentGlyphs)) {
            // All glyphs (if any) are pending in a single fragment.
            SkASSERT(fResult.fFragments.size() <= 1);
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

        auto extent_box = [&](bool include_typographical_extent) {
            auto box = fResult.computeVisualBounds();

            if (include_typographical_extent) {
                // Hybrid visual alignment mode, based on typographical extent.

                // By default, first line is vertically-aligned on a baseline of 0.
                // The typographical height considered for vertical alignment is the distance
                // between the first line top (ascent) to the last line bottom (descent).
                const auto typographical_top    = fBox.fTop + ascent,
                           typographical_bottom = fBox.fTop + fLastLineDescent +
                                          fDesc.fLineHeight*(fLineCount > 0 ? fLineCount - 1 : 0ul);

                box.fTop    = std::min(box.fTop,    typographical_top);
                box.fBottom = std::max(box.fBottom, typographical_bottom);
            }

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
        case Shaper::VAlign::kHybridTop:
        case Shaper::VAlign::kVisualTop:
            ebox.init(extent_box(fDesc.fVAlign == Shaper::VAlign::kHybridTop));
            v_offset += fBox.fTop - ebox->fTop;
            break;
        case Shaper::VAlign::kHybridCenter:
        case Shaper::VAlign::kVisualCenter:
            ebox.init(extent_box(fDesc.fVAlign == Shaper::VAlign::kHybridCenter));
            v_offset += fBox.centerY() - ebox->centerY();
            break;
        case Shaper::VAlign::kHybridBottom:
        case Shaper::VAlign::kVisualBottom:
            ebox.init(extent_box(fDesc.fVAlign == Shaper::VAlign::kHybridBottom));
            v_offset += fBox.fBottom - ebox->fBottom;
            break;
        }

        if (shaped_size) {
            if (!ebox.isValid()) {
                ebox.init(extent_box(true));
            }
            *shaped_size = SkSize::Make(ebox->width(), ebox->height());
        }

        if (v_offset) {
            for (auto& fragment : fResult.fFragments) {
                fragment.fOrigin.fY += v_offset;
            }
        }

        return std::move(fResult);
    }

    void shapeLine(const char* start, const char* end, size_t utf8_offset) {
        if (!fShaper) {
            return;
        }

        SkASSERT(start <= end);
        if (start == end) {
            // SkShaper doesn't care for empty lines.
            this->beginLine();
            this->commitLine();

            // The calls above perform bookkeeping, but they do not add any fragments (since there
            // are no runs to commit).
            //
            // Certain Skottie features (line-based range selectors) do require accurate indexing
            // information even for empty lines though -- so we inject empty fragments solely for
            // line index tracking.
            //
            // Note: we don't add empty fragments in consolidated mode because 1) consolidated mode
            // assumes there is a single result fragment and 2) kFragmentGlyphs is always enabled
            // for cases where line index tracking is relevant.
            //
            // TODO(fmalita): investigate whether it makes sense to move this special case down
            // to commitFragmentedRun().
            if (fDesc.fFlags & Shaper::Flags::kFragmentGlyphs) {
                fResult.fFragments.push_back({
                    Shaper::ShapedGlyphs(),
                    {fBox.x(),fBox.y()},
                    0, 0,
                    fLineCount - 1,
                    false
                });
            }

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

        const auto shape_width  = fDesc.fLinebreak == Shaper::LinebreakPolicy::kExplicit
                                    ? SK_ScalarMax
                                    : fBox.width();
        const auto shape_ltr    = fDesc.fDirection == Shaper::Direction::kLTR;
        const size_t utf8_bytes = SkToSizeT(end - start);

        static constexpr uint8_t kBidiLevelLTR = 0,
                                 kBidiLevelRTL = 1;
        const auto lang_iter = fDesc.fLocale
                ? std::make_unique<SkShaper::TrivialLanguageRunIterator>(fDesc.fLocale, utf8_bytes)
                : SkShaper::MakeStdLanguageRunIterator(start, utf8_bytes);
#if defined(SKOTTIE_TRIVIAL_FONTRUN_ITER)
        // Chrome Linux/CrOS does not have a fallback-capable fontmgr, and crashes if fallback is
        // triggered.  Using a TrivialFontRunIterator avoids the issue (https://crbug.com/1520148).
        const auto font_iter = std::make_unique<SkShaper::TrivialFontRunIterator>(fFont,
                                                                                  utf8_bytes);
#else
        const auto font_iter = SkShaper::MakeFontMgrRunIterator(
                                    start, utf8_bytes, fFont,
                                    fFontMgr ? fFontMgr : SkFontMgr::RefEmpty(), // used as fallback
                                    fDesc.fFontFamily,
                                    fFont.getTypeface()->fontStyle(),
                                    lang_iter.get());
#endif
        const auto bidi_iter = SkShaper::MakeBiDiRunIterator(start, utf8_bytes,
                                    shape_ltr ? kBidiLevelLTR : kBidiLevelRTL);
        const auto scpt_iter = SkShaper::MakeScriptRunIterator(start, utf8_bytes,
                                    SkSetFourByteTag('Z', 'z', 'z', 'z'));

        if (!font_iter || !bidi_iter || !scpt_iter || !lang_iter) {
            return;
        }

        fUTF8 = start;
        fUTF8Offset = utf8_offset;
        fShaper->shape(start, utf8_bytes,
                       *font_iter,
                       *bidi_iter,
                       *scpt_iter,
                       *lang_iter,
                       shape_width, this);
        fUTF8 = nullptr;
    }

private:
    void commitFragementedRun(const skottie::Shaper::RunRec& run,
                              const SkGlyphID* glyphs,
                              const SkPoint* pos,
                              const uint32_t* clusters,
                              uint32_t line_index) {
        float ascent = 0;

        if (fDesc.fFlags & Shaper::Flags::kTrackFragmentAdvanceAscent) {
            SkFontMetrics metrics;
            run.fFont.getMetrics(&metrics);
            ascent = metrics.fAscent;

            // Note: we use per-glyph advances for anchoring, but it's unclear whether this
            // is exactly the same as AE.  E.g. are 'acute' glyphs anchored separately for fonts
            // in which they're distinct?
            fAdvanceBuffer.resize(run.fSize);
            fFont.getWidths(glyphs, SkToInt(run.fSize), fAdvanceBuffer.data());
        }

        // In fragmented mode we immediately push the glyphs to fResult,
        // one fragment per glyph.  Glyph positioning is externalized
        // (positions returned in Fragment::fPos).
        for (size_t i = 0; i < run.fSize; ++i) {
            const auto advance = (fDesc.fFlags & Shaper::Flags::kTrackFragmentAdvanceAscent)
                    ? fAdvanceBuffer[SkToInt(i)]
                    : 0.0f;

            fResult.fFragments.push_back({
                {
                    { {run.fFont, 1} },
                    { glyphs[i] },
                    { {0,0} },
                    fDesc.fFlags & Shaper::kClusters
                        ? std::vector<size_t>{ fUTF8Offset + clusters[i] }
                        : std::vector<size_t>({}),
                },
                { fBox.x() + pos[i].fX, fBox.y() + pos[i].fY },
                advance, ascent,
                line_index, is_whitespace(fUTF8[clusters[i]])
            });

            // Note: we only check the first code point in the cluster for whitespace.
            // It's unclear whether thers's a saner approach.
            fResult.fMissingGlyphCount += (glyphs[i] == kMissingGlyphID);
        }
    }

    void commitConsolidatedRun(const skottie::Shaper::RunRec& run,
                               const SkGlyphID* glyphs,
                               const SkPoint* pos,
                               const uint32_t* clusters,
                               uint32_t) {
        // In consolidated mode we just accumulate glyphs to a single fragment in ResultBuilder.
        // Glyph positions are baked in the fragment runs (Fragment::fPos only reflects the
        // box origin).

        if (fResult.fFragments.empty()) {
            fResult.fFragments.push_back({{{}, {}, {}, {}}, {fBox.x(), fBox.y()}, 0, 0, 0, false});
        }

        auto& current_glyphs = fResult.fFragments.back().fGlyphs;
        current_glyphs.fRuns.push_back(run);
        current_glyphs.fGlyphIDs.insert(current_glyphs.fGlyphIDs.end(), glyphs, glyphs + run.fSize);
        current_glyphs.fGlyphPos.insert(current_glyphs.fGlyphPos.end(), pos   , pos    + run.fSize);

        for (size_t i = 0; i < run.fSize; ++i) {
            fResult.fMissingGlyphCount += (glyphs[i] == kMissingGlyphID);
        }

        if (fDesc.fFlags & Shaper::kClusters) {
            current_glyphs.fClusters.reserve(current_glyphs.fClusters.size() + run.fSize);
            for (size_t i = 0; i < run.fSize; ++i) {
                current_glyphs.fClusters.push_back(fUTF8Offset + clusters[i]);
            }
        }
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

    inline static constexpr SkGlyphID kMissingGlyphID = 0;

    const Shaper::TextDesc&   fDesc;
    const SkRect&             fBox;
    const float               fHAlignFactor;

    SkFont                          fFont;
    const sk_sp<SkFontMgr>          fFontMgr;
    const std::unique_ptr<SkShaper> fShaper;

    AutoSTMalloc<64, SkGlyphID>          fLineGlyphs;
    AutoSTMalloc<64, SkPoint>            fLinePos;
    AutoSTMalloc<64, uint32_t>           fLineClusters;
    STArray<16, skottie::Shaper::RunRec> fLineRuns;
    size_t                                 fLineGlyphCount = 0;

    STArray<64, float, true> fAdvanceBuffer;

    SkPoint  fCurrentPosition{ 0, 0 };
    SkPoint  fOffset{ 0, 0 };
    SkVector fPendingLineAdvance{ 0, 0 };
    uint32_t fLineCount = 0;
    float    fFirstLineAscent = 0,
             fLastLineDescent = 0;

    const char* fUTF8       = nullptr; // only valid during shapeLine() calls
    size_t      fUTF8Offset = 0;       // current line offset within the original string

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
    const char* begin      = ptr;
    const char* end        = ptr + txt.size();

    ResultBuilder rbuilder(desc, box, fontmgr);
    while (ptr < end) {
        if (is_line_break(SkUTF::NextUTF8(&ptr, end))) {
            rbuilder.shapeLine(line_start, ptr - 1, SkToSizeT(line_start - begin));
            line_start = ptr;
        }
    }
    rbuilder.shapeLine(line_start, ptr, SkToSizeT(line_start - begin));

    return rbuilder.finalize(shaped_size);
}

bool result_fits(const Shaper::Result& res, const SkSize& res_size,
                 const SkRect& box, const Shaper::TextDesc& desc) {
    // optional max line count constraint
    if (desc.fMaxLines) {
        const auto line_count = res.fFragments.empty()
                ? 0
                : res.fFragments.back().fLineIndex + 1;
        if (line_count > desc.fMaxLines) {
            return false;
        }
    }

    // geometric constraint
    return res_size.width() <= box.width() && res_size.height() <= box.height();
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
        if (!result_fits(res, res_size, box, desc)) {
            out_scale = try_scale;
            try_scale = (in_scale == min_scale)
                    // initial in_scale not found yet - search exponentially
                    ? std::max(min_scale, try_scale * 0.5f)
                    // in_scale found - binary search
                    : (in_scale + out_scale) * 0.5f;
        } else {
            // It fits - so it's a candidate.
            best_result = std::move(res);
            best_result.fScale = try_scale;

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

Shaper::Result Shaper::Shape(const SkString& text, const TextDesc& desc, const SkPoint& point,
                             const sk_sp<SkFontMgr>& fontmgr) {
    const AdjustedText adjText(text, desc);

    return (desc.fResize == ResizePolicy::kScaleToFit ||
            desc.fResize == ResizePolicy::kDownscaleToFit) // makes no sense in point mode
            ? Result()
            : ShapeImpl(adjText, desc, SkRect::MakeEmpty().makeOffset(point.x(), point.y()),
                        fontmgr);
}

Shaper::Result Shaper::Shape(const SkString& text, const TextDesc& desc, const SkRect& box,
                             const sk_sp<SkFontMgr>& fontmgr) {
    const AdjustedText adjText(text, desc);

    switch(desc.fResize) {
    case ResizePolicy::kNone:
        return ShapeImpl(adjText, desc, box, fontmgr);
    case ResizePolicy::kScaleToFit:
        return ShapeToFit(adjText, desc, box, fontmgr);
    case ResizePolicy::kDownscaleToFit: {
        SkSize size;
        auto result = ShapeImpl(adjText, desc, box, fontmgr, &size);

        return result_fits(result, size, box, desc)
                ? result
                : ShapeToFit(adjText, desc, box, fontmgr);
    }
    }

    SkUNREACHABLE;
}

SkRect Shaper::ShapedGlyphs::computeBounds(BoundsType btype) const {
    auto bounds = SkRect::MakeEmpty();

    AutoSTArray<16, SkRect> glyphBounds;

    size_t offset = 0;
    for (const auto& run : fRuns) {
        SkRect font_bounds;
        if (btype == BoundsType::kConservative) {
            font_bounds = SkFontPriv::GetFontBounds(run.fFont);

            // Empty font bounds is likely a font bug -- fall back to tight bounds.
            if (font_bounds.isEmpty()) {
                btype = BoundsType::kTight;
            }
        }

        switch (btype) {
        case BoundsType::kConservative: {
            SkRect run_bounds;
            run_bounds.setBounds(fGlyphPos.data() + offset, SkToInt(run.fSize));
            run_bounds.fLeft   += font_bounds.left();
            run_bounds.fTop    += font_bounds.top();
            run_bounds.fRight  += font_bounds.right();
            run_bounds.fBottom += font_bounds.bottom();

            bounds.join(run_bounds);
        } break;
        case BoundsType::kTight: {
            glyphBounds.reset(SkToInt(run.fSize));
            run.fFont.getBounds(fGlyphIDs.data() + offset,
                                SkToInt(run.fSize), glyphBounds.data(), nullptr);

            for (size_t i = 0; i < run.fSize; ++i) {
                bounds.join(glyphBounds[SkToInt(i)].makeOffset(fGlyphPos[offset + i]));
            }
        } break;
        }

        offset += run.fSize;
    }

    return bounds;
}

void Shaper::ShapedGlyphs::draw(SkCanvas* canvas,
                                const SkPoint& origin,
                                const SkPaint& paint) const {
    size_t offset = 0;
    for (const auto& run : fRuns) {
        canvas->drawGlyphs(SkToInt(run.fSize),
                           fGlyphIDs.data() + offset,
                           fGlyphPos.data() + offset,
                           origin,
                           run.fFont,
                           paint);
        offset += run.fSize;
    }
}

SkRect Shaper::Result::computeVisualBounds() const {
    auto bounds = SkRect::MakeEmpty();

    for (const auto& frag: fFragments) {
        bounds.join(frag.fGlyphs.computeBounds(ShapedGlyphs::BoundsType::kTight)
                                .makeOffset(frag.fOrigin));
    }

    return bounds;
}

} // namespace skottie
