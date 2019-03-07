// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(RRect_012, 256, 256, true, 0);
// HASH=c675a480b41dee157f84fa2550a2a53c
void draw(SkCanvas* canvas) {
    SkRRect unsorted = SkRRect::MakeRect({ 15, 25, 10, 5 });
    SkDebugf("unsorted width: %g\n", unsorted.width());
    SkRRect large = SkRRect::MakeRect({ -FLT_MAX, 1, FLT_MAX, 2 });
    SkDebugf("large width: %.0f\n", large.width());
}
}
