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

#include <string>

#include "DartTypes.h"
#include "TextStyle.h"
#include "include/core/SkFontStyle.h"

namespace skia {
namespace textlayout {

struct StrutStyle {
    StrutStyle();
    SkFontStyle fFontStyle;
    std::vector<std::string> fFontFamilies;
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

    StrutStyle& getStrutStyle() { return fStrutStyle; }
    void setStrutStyle(StrutStyle strutStyle) { fStrutStyle = std::move(strutStyle); }

    TextStyle& getTextStyle() { return fDefaultTextStyle; }
    void setTextStyle(const TextStyle& textStyle) { fDefaultTextStyle = textStyle; }

    inline TextDirection getTextDirection() const { return fTextDirection; }
    void setTextDirection(TextDirection direction) { fTextDirection = direction; }

    void setTextAlign(TextAlign align) { fTextAlign = align; }
    TextAlign getTextAlign() const { return fTextAlign; }

    void setMaxLines(size_t maxLines) { fLinesLimit = maxLines; }
    inline size_t getMaxLines() const { return fLinesLimit; }

    inline std::string getEllipsis() const { return fEllipsis; }
    void setEllipsis(const std::u16string& ellipsis);

    void setHeight(SkScalar height) { fHeight = height; }
    SkScalar getHeight() const { return fHeight; }

    inline bool unlimited_lines() const {
        return fLinesLimit == std::numeric_limits<size_t>::max();
    }
    inline bool ellipsized() const { return !fEllipsis.empty(); }
    TextAlign effective_align() const;
    bool hintingIsOn() const { return fHintingIsOn; }
    void turnHintingOff() { fHintingIsOn = false; }

private:
    StrutStyle fStrutStyle;
    TextStyle fDefaultTextStyle;
    TextAlign fTextAlign;
    TextDirection fTextDirection;
    size_t fLinesLimit;
    std::string fEllipsis;
    SkScalar fHeight;
    bool fHintingIsOn;
};
}  // namespace textlayout
}  // namespace skia

#endif  // ParagraphStyle_DEFINED