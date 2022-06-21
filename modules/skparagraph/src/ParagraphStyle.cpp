// Copyright 2019 Google LLC.

#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "src/core/SkStringUtils.h"
#include "src/utils/SkUTF.h"

namespace skia {
namespace textlayout {

StrutStyle::StrutStyle() {
    fFontStyle = SkFontStyle::Normal();
    fFontSize = 14;
    fHeight = 1;
    fLeading = -1;
    fForceHeight = false;
    fHeightOverride = false;
    fHalfLeading = false;
    fEnabled = false;
}

ParagraphStyle::ParagraphStyle() {
    fTextAlign = TextAlign::kStart;
    fTextDirection = TextDirection::kLtr;
    fLinesLimit = std::numeric_limits<size_t>::max();
    fHeight = 1;
    fTextHeightBehavior = TextHeightBehavior::kAll;
    fHintingIsOn = true;
    fReplaceTabCharacters = false;
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
