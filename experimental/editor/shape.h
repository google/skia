// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "include/core/SkFont.h"
#include "include/core/SkTextBlob.h"

#include <cstddef>
#include <vector>

namespace editor {

struct ShapeResult {
    sk_sp<SkTextBlob> blob;
    std::vector<std::size_t> lineBreakOffsets;
    std::vector<SkRect> glyphBounds;
    std::vector<bool> wordBreaks;
    int verticalAdvance;
};

ShapeResult Shape(const char* ut8text,
                  size_t textByteLen,
                  const SkFont& font,
                  const char* locale,
                  float width);

}
