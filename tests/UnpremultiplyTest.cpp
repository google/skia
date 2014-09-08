/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorPriv.h"
#include "SkUnPreMultiply.h"
#include "Test.h"

DEF_TEST(Unpremultiply, reporter) {
    // Here we test that unpremultiplication is injective:
    // no two distinct premul colors map to the same unpremul color.

    // DM exploits this fact to safely hash .pngs instead of the original bitmaps.

    // It is sufficient to test red.  Green and blue follow the same rules.
    // This means we have at most 256*256 possible colors to deal with.
    int hits[256*256];
    for (size_t i = 0; i < SK_ARRAY_COUNT(hits); i++) {
        hits[i] = 0;
    }

    for (int a = 0; a < 256; a++) {
        for (int r = 0; r <= a; r++) {
            SkPMColor pm = SkPackARGB32(a, r, 0, 0);
            SkColor upm = SkUnPreMultiply::PMColorToColor(pm);

            // ARGB -> AR
            hits[upm >> 16]++;
        }
    }

    for (size_t i = 0; i < SK_ARRAY_COUNT(hits); i++) {
        REPORTER_ASSERT(reporter, hits[i] < 2);
    }
}
