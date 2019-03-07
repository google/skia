// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
REG_FIDDLE(Path_041, 256, 256, true, 0) {
// HASH=bb761cd858e6d0ca05627262cd22ff5e
void draw(SkCanvas* canvas) {
    double times[2] = { 0, 0 };
    for (int i = 0; i < 10000; ++i) {
      SkPath path;
      for (int j = 1; j < 100; ++ j) {
        path.addCircle(50 + j, 45 + j, 25 + j);
      }
      if (1 & i) {
        path.updateBoundsCache();
      }
      double start = SkTime::GetNSecs();
      (void) path.getBounds();
      times[1 & i] += SkTime::GetNSecs() - start;
    }
    SkDebugf("uncached avg: %g ms\n", times[0] * 1e-6);
    SkDebugf("cached avg: %g ms\n", times[1] * 1e-6);
}
}
