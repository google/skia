/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkHalf.h"

#include "include/private/base/SkFloatingPoint.h"
#include "src/base/SkVx.h"

// NOTE: These are defined within the CPP compilation unit so that they are not inlined everywhere
// and increase code size. Performance critical code is likely already using skvx directly, and
// they will be inlined where performance vs. code size is worth it.
SkHalf SkFloatToHalf(float f) {
    if (std::isnan(f)) {
        return SK_HalfNaN;
    } else {
        return to_half(skvx::Vec<1,float>(f))[0];
    }
}

float SkHalfToFloat(SkHalf h) {
    return from_half(skvx::Vec<1,uint16_t>(h))[0];
}
