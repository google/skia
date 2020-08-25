// Copyright 2019 Google LLC.

#include "src/core/SkStringUtils.h"
#include "src/utils/SkUTF.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skshaper/src/SkUnicode.h"

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
    fTextHeightBehavior = TextHeightBehavior::kAll;
    fHintingIsOn = true;
}

bool ParagraphStyle::setEllipsis(const std::u16string& ellipsis) {

    std::unique_ptr<char[]> utf8;
    auto utf8Units = SkUnicode::convertUtf16ToUtf8((uint16_t*)ellipsis.data(), ellipsis.size(), &utf8);
    if (utf8Units < 0) {
        return false;
    }
    fEllipsis = SkString(utf8.get(), utf8Units);
    return true;
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
}  // namespace textlayout
}  // namespace skia
