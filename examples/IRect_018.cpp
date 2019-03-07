// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(IRect_018, 256, 256, true, 0);
// HASH=bd8f028d9051062816c9116fea4237b2
void draw(SkCanvas* canvas) {
    SkIRect test = {0, 0, 2, 2};
    SkIRect sorted = test.makeSorted();
    SkDebugf("test %c= sorted\n", test == sorted ? '=' : '!');
}
}
