/*
 * Copyright 2019 Google, Inc.
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

#include <string>
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "unicode/unistr.h"

namespace skia {
namespace textlayout {

StrutStyle::StrutStyle() {
    fFontStyle = SkFontStyle::Normal();
    fFontSize = 14;
    fHeight = 1;
    fLeading = -1;
    fForceStrutHeight = false;
    fStrutEnabled = false;
}

ParagraphStyle::ParagraphStyle() {
    fTextAlign = TextAlign::kStart;
    fTextDirection = TextDirection::kLtr;
    fLinesLimit = std::numeric_limits<size_t>::max();
    fHeight = 1;
    fHintingIsOn = true;
}

TextAlign ParagraphStyle::effective_align() const {
    if (fTextAlign == TextAlign::kStart) {
        return (fTextDirection == TextDirection::kLtr) ? TextAlign::kLeft : TextAlign::kRight;
    } else if (fTextAlign == TextAlign::kEnd) {
        return (fTextDirection == TextDirection::kLtr) ? TextAlign::kRight : TextAlign::kLeft;
    } else {
        return fTextAlign;
    }
}

void ParagraphStyle::setEllipsis(const std::u16string& ellipsis) {
    icu::UnicodeString unicode;
    unicode.setTo((UChar*)ellipsis.data());
    std::string str;
    unicode.toUTF8String(str);
    fEllipsis = SkString(str.c_str());
}
}  // namespace textlayout
}  // namespace skia
