// Copyright 2019 Google LLC.
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
    fForceHeight = false;
    fHeightOverride = false;
    fEnabled = false;
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
