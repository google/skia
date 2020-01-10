// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(text_only, 256, 256, true, 0) {
void draw(SkCanvas* canvas) { SkDebugf("Hello World!"); }
}  // END FIDDLE
