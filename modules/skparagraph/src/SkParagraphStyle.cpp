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

#include "modules/skparagraph/include/SkParagraphStyle.h"
#include "unicode/unistr.h"

SkStrutStyle::SkStrutStyle() {
    fFontStyle = SkFontStyle::Normal();
    fFontSize = 14;
    fHeight = 1;
    fLeading = -1;
    fForceStrutHeight = false;
    fStrutEnabled = false;
}

SkParagraphStyle::SkParagraphStyle() {
    fTextAlign = SkTextAlign::kStart;
    fTextDirection = SkTextDirection::kLtr;
    fLinesLimit = std::numeric_limits<size_t>::max();
    fHeight = 1;
    fHintingIsOn = true;
}

SkTextAlign SkParagraphStyle::effective_align() const {
    if (fTextAlign == SkTextAlign::kStart) {
        return (fTextDirection == SkTextDirection::kLtr) ? SkTextAlign::kLeft : SkTextAlign::kRight;
    } else if (fTextAlign == SkTextAlign::kEnd) {
        return (fTextDirection == SkTextDirection::kLtr) ? SkTextAlign::kRight : SkTextAlign::kLeft;
    } else {
        return fTextAlign;
    }
}

void SkParagraphStyle::setEllipsis(const std::u16string& ellipsis) {
    icu::UnicodeString unicode;
    unicode.setTo((UChar*)ellipsis.data());
    unicode.toUTF8String(fEllipsis);
}
