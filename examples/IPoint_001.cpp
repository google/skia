// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(IPoint_001, 256, 256, true, 0);
// HASH=eed4185294f8a8216fc354e6ee6b2e3a
void draw(SkCanvas* canvas) {
    SkIPoint pt1 = {45, 66};
    SkDebugf("pt1.fX %c= pt1.x()\n", pt1.fX == pt1.x() ? '=' : '!');
}
}
