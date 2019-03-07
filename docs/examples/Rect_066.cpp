// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
REG_FIDDLE(Rect_066, 256, 256, true, 0) {
// HASH=e1ea5f949d80276f3637931eae93a07c
void draw(SkCanvas* canvas) {
    SkRect rect = {7, 11, 13, 17};
 SkDebugf("rect.asScalars() %c= &rect.fLeft\n", rect.asScalars() == &rect.fLeft? '=' : '!');
}
}
