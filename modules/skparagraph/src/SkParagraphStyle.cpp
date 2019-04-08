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

#include "SkParagraphStyle.h"
#include "unicode/unistr.h"

SkParagraphStyle::SkParagraphStyle() {
    fTextAlign = SkTextAlign::start;
    fTextDirection = SkTextDirection::ltr;
    fLinesLimit = std::numeric_limits<size_t>::max();
    //_ellipsis = u"\u2026";
    fLineHeight = 1;
    fHintingIsOn = true;
}

SkTextAlign SkParagraphStyle::effective_align() const {
    if (fTextAlign == SkTextAlign::start) {
        return (fTextDirection == SkTextDirection::ltr) ? SkTextAlign::left
                                                        : SkTextAlign::right;
    } else if (fTextAlign == SkTextAlign::end) {
        return (fTextDirection == SkTextDirection::ltr) ? SkTextAlign::right
                                                        : SkTextAlign::left;
    } else {
        return fTextAlign;
    }
}

void SkParagraphStyle::setEllipsis(const std::u16string& ellipsis) {
    icu::UnicodeString unicode;
    unicode.setTo((UChar*) ellipsis.data());
    unicode.toUTF8String(fEllipsis);
}
