// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(DCIToXYZD50, 512, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkColorSpacePrimaries p;
    // DCI-P3 Primaries from https://en.wikipedia.org/wiki/DCI-P3
    p.fRX = 0.680f; p.fRY = 0.320f;
    p.fGX = 0.265f; p.fGY = 0.690f;
    p.fBX = 0.150f; p.fBY = 0.060f;

    // DCI-P3 D65
    p.fWX = 0.3127f; p.fWY = 0.3290f;

    // DCI-P3 Theater
    // p.fWX = 0.314; p.fWY = 0.351;

    skcms_Matrix3x3 toXYZ;
    p.toXYZD50(&toXYZ);

    for (int i = 0; i < 3; ++i) {
        SkDebugf("%f %f %f\n", toXYZ.vals[i][0], toXYZ.vals[i][1], toXYZ.vals[i][2]);
    }
}
}  // END FIDDLE
