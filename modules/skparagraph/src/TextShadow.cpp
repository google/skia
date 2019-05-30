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

#include "TextShadow.h"
#include "include/core/SkColor.h"

namespace skia {
namespace textlayout {

TextShadow::TextShadow() = default;
TextShadow::TextShadow(SkColor color, SkPoint offset, double blurRadius)
        : fColor(color), fOffset(offset), fBlurRadius(blurRadius) {}

bool TextShadow::operator==(const TextShadow& other) const {
    if (fColor != other.fColor) return false;
    if (fOffset != other.fOffset) return false;
    if (fBlurRadius != other.fBlurRadius) return false;

    return true;
}

bool TextShadow::operator!=(const TextShadow& other) const { return !(*this == other); }

bool TextShadow::hasShadow() const {
    if (!fOffset.isZero()) return true;
    if (fBlurRadius != 0.0) return true;

    return false;
}

}  // namespace textlayout
}  // namespace skia
