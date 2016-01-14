/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Fuzz.h"

// This really is just an example Fuzz*.cpp file.
// It tests that two different ways of calculating the Paeth predictor function are equivalent.

static uint8_t paeth_std(uint8_t a, uint8_t b, uint8_t c) {
    int p = a+b-c;

    int pa = abs(p-a),
        pb = abs(p-b),
        pc = abs(p-c);

    if (pb < pa) { pa = pb; a = b; }
    if (pc < pa) {          a = c; }
    return a;
}

static uint8_t paeth_alt(uint8_t a, uint8_t b, uint8_t c) {
    int min = SkTMin(a,b),
        max = SkTMax(a,b);
    int delta = (max-min)/3;

    if (c <= min+delta) return max;
    if (c >= max-delta) return min;
    return c;
}

DEF_FUZZ(Paeth, fuzz) {
    auto a = fuzz->nextB(),
         b = fuzz->nextB(),
         c = fuzz->nextB();
    ASSERT(paeth_alt(a,b,c) == paeth_std(a,b,c));
}
