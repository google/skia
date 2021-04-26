// Copyright 2019 Google LLC.
#include "include/core/SkColor.h"
#include "modules/skparagraph/include/TextShadow.h"

namespace skia {
namespace textlayout {

TextShadow::TextShadow() = default;
TextShadow::TextShadow(SkColor color, SkPoint offset, double blurSigma)
        : fColor(color), fOffset(offset), fBlurSigma(blurSigma) {}

bool TextShadow::operator==(const TextShadow& other) const {
    if (fColor != other.fColor) return false;
    if (fOffset != other.fOffset) return false;
    if (fBlurSigma != other.fBlurSigma) return false;

    return true;
}

bool TextShadow::operator!=(const TextShadow& other) const { return !(*this == other); }

bool TextShadow::hasShadow() const {
    if (!fOffset.isZero()) return true;
    if (fBlurSigma != 0.0) return true;

    return false;
}

}  // namespace textlayout
}  // namespace skia
