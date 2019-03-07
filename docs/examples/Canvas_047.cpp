// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
REG_FIDDLE(Canvas_047, 256, 256, true, 0) {
// HASH=c0d5fa544759704768f47cac91ae3832
void draw(SkCanvas* canvas) {
    SkDebugf("isIdentity %s\n", canvas->getTotalMatrix().isIdentity() ? "true" : "false");
}
}
