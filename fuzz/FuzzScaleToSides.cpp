/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Reminder of how to run:
//  $ env CC=afl-clang CXX=afl-clang++ ./gyp_skia
//  $ ninja -C out/Debug fuzz
//  $ afl-fuzz -i fuzz-in -o fuzz-out out/Debug/fuzz -n ScaleToSides -b @@
// where you seed fuzz-in/ with one or more small files.

#include "Fuzz.h"
#include "SkScaleToSides.h"
#include <cmath>

DEF_FUZZ(ScaleToSides, fuzz) {
    float radius1 = fuzz->nextF(),
          radius2 = fuzz->nextF(),
          width   = fuzz->nextF();

    if (!std::isfinite(radius1) ||
        !std::isfinite(radius2) ||
        !std::isfinite(width)   ||
        radius1 <= 0.0f         ||
        radius2 <= 0.0f         ||
        width <= 0.0f)
    {
        fuzz->signalBoring();
    }

    double scale = (double)width / ((double)radius1 + (double)radius2);
    if (scale >= 1.0 || scale <= 0.0) {
        fuzz->signalBoring();
    }
    SkDebugf("%g %g %g %g\n", radius1, radius2, width, scale);
    SkScaleToSides::AdjustRadii(width, scale, &radius1, &radius2);

    // TODO(mtklein): add fuzz->keepResult()
    volatile float junk = 0.0f;
    junk *= radius1;
    junk *= radius2;
}
