/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieTextShaper_DEFINED
#define SkottieTextShaper_DEFINED

#include "include/core/SkFont.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkTypeTraits.h"
#include "include/utils/SkTextUtils.h"

#include <vector>

class SkCanvas;
class SkFontMgr;
class SkTypeface;
class SkString;

struct SkRect;

namespace skottie {

// Helper implementing After Effects text shaping semantics on top of SkShaper.

class Shaper final {
public:
    struct RunRec {
        SkFont fFont;
        size_t fSize;

        static_assert(::sk_is_trivially_relocatable<decltype(fFont)>::value);

        using sk_is_trivially_relocatable = std::true_type;
    };

    struct ShapedGlyphs {
        std::vector<RunRec>    fRuns;

        // Consolidated storage for all runs.
        std::vector<SkGlyphID> fGlyphIDs;
        std::vector<SkPoint>   fGlyphPos;

        // fClusters[i] is an input string index, pointing to the start of the UTF sequence
        // associated with fGlyphs[i].  The number of entries matches the number of glyphs.
        // Only available with Flags::kClusters.
        std::vector<size_t>    fClusters;

        enum class BoundsType { kConservative, kTight };
        SkRect computeBounds(BoundsType) const;

        void draw(SkCanvas*, const SkPoint& origin, const SkPaint&) const;
    };

    struct Fragment {
        ShapedGlyphs fGlyphs;
        SkPoint      fOrigin;

        // Only valid for kFragmentGlyphs
        float        fAdvance,
                     fAscent;
        uint32_t     fLineIndex;    // 0-based index for the line this fragment belongs to.
        bool         fIsWhitespace; // True if the first code point in the corresponding
                                    // cluster is whitespace.
    };

    struct Result {
        std::vector<Fragment> fFragments;
        size_t                fMissingGlyphCount = 0;
        // Relative text size scale, when using an auto-scaling ResizePolicy
        // (otherwise 1.0).  This is informative of the final text size, and is
        // not required to render the Result.
        float                 fScale = 1.0f;

        SkRect computeVisualBounds() const;
    };

    enum class VAlign : uint8_t {
        // Align the first line typographical top with the text box top (AE box text).
        kTop,
        // Align the first line typographical baseline with the text box top (AE point text).
        kTopBaseline,

        // Skottie vertical alignment extensions

        // These are based on a hybrid extent box defined (in Y) as
        //
        //   ------------------------------------------------------
        //   MIN(visual_top_extent   , typographical_top_extent   )
        //
        //                         ...
        //
        //   MAX(visual_bottom_extent, typographical_bottom_extent)
        //   ------------------------------------------------------
        kHybridTop,     // extent box top    -> text box top
        kHybridCenter,  // extent box center -> text box center
        kHybridBottom,  // extent box bottom -> text box bottom

        // Visual alignement modes -- these are using tight visual bounds for the paragraph.
        kVisualTop,     // visual top    -> text box top
        kVisualCenter,  // visual center -> text box center
        kVisualBottom,  // visual bottom -> text box bottom
    };

    enum class ResizePolicy : uint8_t {
        // Use the specified text size.
        kNone,
        // Resize the text such that the extent box fits (snuggly) in the text box,
        // both horizontally and vertically.
        kScaleToFit,
        // Same kScaleToFit if the text doesn't fit at the specified font size.
        // Otherwise, same as kNone.
        kDownscaleToFit,
    };

    enum class LinebreakPolicy : uint8_t {
        // Break lines such that they fit in a non-empty paragraph box, horizontally.
        kParagraph,
        // Only break lines when requested explicitly (\r), regardless of paragraph box dimensions.
        kExplicit,
    };

    // Initial text direction.
    enum class Direction : uint8_t { kLTR, kRTL };

    enum class Capitalization {
        kNone,
        kUpperCase,
    };

    enum Flags : uint32_t {
        kNone                       = 0x00,

        // Split out individual glyphs into separate Fragments
        // (useful when the caller intends to manipulate glyphs independently).
        kFragmentGlyphs             = 0x01,

        // Compute the advance and ascent for each fragment.
        kTrackFragmentAdvanceAscent = 0x02,

        // Return cluster information.
        kClusters                   = 0x04,
    };

    struct TextDesc {
        const sk_sp<SkTypeface>&  fTypeface;
        const char*               fLocale         = nullptr;
        SkScalar                  fTextSize       = 0,
                                  fMinTextSize    = 0,  // when auto-sizing
                                  fMaxTextSize    = 0,  // when auto-sizing
                                  fLineHeight     = 0,
                                  fLineShift      = 0,
                                  fAscent         = 0;
        SkTextUtils::Align        fHAlign         = SkTextUtils::kLeft_Align;
        VAlign                    fVAlign         = Shaper::VAlign::kTop;
        ResizePolicy              fResize         = Shaper::ResizePolicy::kNone;
        LinebreakPolicy           fLinebreak      = Shaper::LinebreakPolicy::kExplicit;
        Direction                 fDirection      = Shaper::Direction::kLTR ;
        Capitalization            fCapitalization = Shaper::Capitalization::kNone;
        size_t                    fMaxLines       = 0;  // when auto-sizing, 0 -> no max
        uint32_t                  fFlags          = 0;
    };

    // Performs text layout along an infinite horizontal line, starting at |point|.
    // Only explicit line breaks (\r) are observed.
    static Result Shape(const SkString& text, const TextDesc& desc, const SkPoint& point,
                        const sk_sp<SkFontMgr>&);

    // Performs text layout within |box|, injecting line breaks as needed to ensure
    // horizontal fitting.  The result is *not* guaranteed to fit vertically (it may extend
    // below the box bottom).
    static Result Shape(const SkString& text, const TextDesc& desc, const SkRect& box,
                        const sk_sp<SkFontMgr>&);

private:
    Shaper() = delete;
};

} // namespace skottie

#endif // SkottieTextShaper_DEFINED
