#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=aa4781afbe3b90e7ef56a287e5b9ce1e
REG_FIDDLE(Paint_029, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    for (auto forceAutoHinting : { false, true} ) {
    paint.setAutohinted(forceAutoHinting);
    SkDebugf("paint.isAutohinted() %c="
            " !!(paint.getFlags() & SkPaint::kAutoHinting_Flag)\n",
            paint.isAutohinted() ==
            !!(paint.getFlags() & SkPaint::kAutoHinting_Flag) ? '=' : '!');
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
