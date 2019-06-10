/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieShaper_DEFINED
#define SkottieShaper_DEFINED

#include "include/core/SkPoint.h"
#include "include/utils/SkTextUtils.h"

#include <vector>

class SkTextBlob;

namespace skottie {

// Helper implementing After Effects text shaping semantics on top of SkShaper.

class Shaper final {
public:
    struct Fragment {
        sk_sp<SkTextBlob> fBlob;
        SkPoint           fPos;

        // Only valid for kFragmentGlyphs
        uint32_t          fLineIndex;    // 0-based index for the line this fragment belongs to.
        bool              fIsWhitespace; // True if the first code point in the corresponding
                                         // cluster is whitespace.
    };

    struct Result {
        std::vector<Fragment> fFragments;

        SkRect computeBounds() const;
    };

    enum class VAlign : uint8_t {
        // Align the first line visual top with the text box top.
        kTop,
        // Align the first line baseline with the text box top.
        kTopBaseline,
        // Align the center of the shaped text bounds with the center of the text box.
        kCenter,
        // Align the last line visual bottom with the text box bottom.
        kBottom,
        // Resize the text such that it fits (snuggly) in the given box.
        kResizeToFit,
    };

    enum Flags : uint32_t {
        kNone           = 0x00,

        // Split out individual glyphs into separate Fragments
        // (useful when the caller intends to manipulate glyphs independently).
        kFragmentGlyphs = 0x01,
    };

    struct TextDesc {
        const sk_sp<SkTypeface>&  fTypeface;
        SkScalar                  fTextSize,
                                  fLineHeight;
        SkTextUtils::Align        fHAlign;
        VAlign                    fVAlign;
        uint32_t                  fFlags;
    };

    // Performs text layout along an infinite horizontal line, starting at |textPoint|.
    // Only explicit line breaks (\r) are observed.
    static Result Shape(const SkString& text, const TextDesc& desc, const SkPoint& textPoint);

    // Performs text layout within |textBox|, injecting line breaks as needed to ensure
    // horizontal fitting.  The result is *not* guaranteed to fit vertically (it may extend
    // below the box bottom).
    static Result Shape(const SkString& text, const TextDesc& desc, const SkRect& textBox);

private:
    Shaper() = delete;
};

} // namespace skottie

#endif // SkottieShaper_DEFINED
