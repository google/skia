// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Image_dimensions, 256, 256, true, 4) {
void draw(SkCanvas* canvas) {
    SkISize dimensions = image->dimensions();
    SkIRect bounds = image->bounds();
    SkIRect dimensionsAsBounds = SkIRect::MakeSize(dimensions);
    SkDebugf("dimensionsAsBounds %c= bounds\n", dimensionsAsBounds == bounds ? '=' : '!');
}
}  // END FIDDLE
