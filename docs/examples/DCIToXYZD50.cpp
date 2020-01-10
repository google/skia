// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(DCIToXYZD50, 512, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkColorSpacePrimaries p;
    // DCI-P3 Primaries from https://en.wikipedia.org/wiki/DCI-P3
    p.fRX = 0.680; p.fRY = 0.320;
    p.fGX = 0.265; p.fGY = 0.690;
    p.fBX = 0.150; p.fBY = 0.060;

    // DCI-P3 D65
    p.fWX = 0.3127; p.fWY = 0.3290;

    // DCI-P3 Theater
    // p.fWX = 0.314; p.fWY = 0.351;

    skcms_Matrix3x3 toXYZ;
    p.toXYZD50(&toXYZ);

    for (int i = 0; i < 3; ++i) {
      SkDebugf("%f %f %f\n",
        toXYZ.vals[i][0], toXYZ.vals[i][1], toXYZ.vals[i][2]);
   }
}
}  // END FIDDLE
