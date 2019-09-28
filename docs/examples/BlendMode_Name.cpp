// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=3996f4994bf4e90b4cd86524c1f9f1a6
REG_FIDDLE(BlendMode_Name, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkDebugf("default blend: SkBlendMode::k%s\n", SkBlendMode_Name(SkPaint().getBlendMode()));
}
}  // END FIDDLE
