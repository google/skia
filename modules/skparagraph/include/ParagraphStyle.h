/*
 * Copyright 2019 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ParagraphStyle_DEFINED
#define ParagraphStyle_DEFINED

#include "DartTypes.h"
#include "TextStyle.h"
#include "include/core/SkFontStyle.h"

namespace skia {
namespace textlayout {

struct StrutStyle {
    StrutStyle();

    const std::vector<SkString>& getFontFamilies() const { return fFontFamilies; }
    void setFontFamilies(std::vector<SkString> families) { fFontFamilies = std::move(families); }

    SkFontStyle getFontStyle() const { return fFontStyle; }
    void setFontStyle(SkFontStyle fontStyle) { fFontStyle = fontStyle; }

    SkScalar getFontSize() const { return fFontSize; }
    void setFontSize(SkScalar size) { fFontSize = size; }

    void setHeight(SkScalar height) { fHeight = height; }
    SkScalar getHeight() const { return fHeight; }

    void setLeading(SkScalar Leading) { fLeading = Leading; }
    SkScalar getLeading() const { return fLeading; }

    bool getStrutEnabled() const { return fStrutEnabled; }
    void setStrutEnabled(bool v) { fStrutEnabled = v; }

    bool getForceStrutHeight() const { return fForceStrutHeight; }
    void setForceStrutHeight(bool v) { fForceStrutHeight = v; }

private:

    std::vector<SkString> fFontFamilies;
    SkFontStyle fFontStyle;
    SkScalar fFontSize;
    SkScalar fHeight;
    SkScalar fLeading;
    bool fForceStrutHeight;
    bool fStrutEnabled;
};

struct ParagraphStyle {
    ParagraphStyle();

    bool operator==(const ParagraphStyle& rhs) const {
        return this->fHeight == rhs.fHeight && this->fEllipsis == rhs.fEllipsis &&
               this->fTextDirection == rhs.fTextDirection && this->fTextAlign == rhs.fTextAlign &&
               this->fDefaultTextStyle == rhs.fDefaultTextStyle;
    }

    const StrutStyle& getStrutStyle() const { return fStrutStyle; }
    void setStrutStyle(StrutStyle strutStyle) { fStrutStyle = std::move(strutStyle); }

    const TextStyle& getTextStyle() const { return fDefaultTextStyle; }
    void setTextStyle(const TextStyle& textStyle) { fDefaultTextStyle = textStyle; }

    TextDirection getTextDirection() const { return fTextDirection; }
    void setTextDirection(TextDirection direction) { fTextDirection = direction; }
    
    TextAlign getTextAlign() const { return fTextAlign; }
    void setTextAlign(TextAlign align) { fTextAlign = align; }

    size_t getMaxLines() const { return fLinesLimit; }
    void setMaxLines(size_t maxLines) { fLinesLimit = maxLines; }

    const SkString& getEllipsis() const { return fEllipsis; }
    void setEllipsis(const std::u16string& ellipsis);

    SkScalar getHeight() const { return fHeight; }
    void setHeight(SkScalar height) { fHeight = height; }

    bool unlimited_lines() const {
        return fLinesLimit == std::numeric_limits<size_t>::max();
    }
    bool ellipsized() const { return fEllipsis.size() != 0; }
    TextAlign effective_align() const;
    bool hintingIsOn() const { return fHintingIsOn; }
    void turnHintingOff() { fHintingIsOn = false; }

private:
    StrutStyle fStrutStyle;
    TextStyle fDefaultTextStyle;
    TextAlign fTextAlign;
    TextDirection fTextDirection;
    size_t fLinesLimit;
    SkString fEllipsis;
    SkScalar fHeight;
    bool fHintingIsOn;
};
}  // namespace textlayout
}  // namespace skia

#endif  // ParagraphStyle_DEFINED