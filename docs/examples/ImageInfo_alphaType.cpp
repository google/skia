// Copyright 2019 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"

#include <array>

REG_FIDDLE(ImageInfo_alphaType, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    static constexpr auto alphas = std::to_array<const char*>({
            "Unknown",
            "Opaque",
            "Premul",
            "Unpremul",
    });
    SkImageInfo info = SkImageInfo::MakeA8(16, 32);
    SkDebugf("alpha type: k" "%s" "_SkAlphaType\n", alphas[info.alphaType()]);
}
}  // END FIDDLE
