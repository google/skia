/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkMathPriv.h"

#include "include/private/base/SkAssert.h"

#include <cstdint>

///////////////////////////////////////////////////////////////////////////////

/* www.worldserver.com/turk/computergraphics/FixedSqrt.pdf
*/
int32_t SkSqrtBits(int32_t x, int count) {
    SkASSERT(x >= 0 && count > 0 && (unsigned)count <= 30);

    uint32_t    root = 0;
    uint32_t    remHi = 0;
    uint32_t    remLo = x;

    do {
        root <<= 1;

        remHi = (remHi<<2) | (remLo>>30);
        remLo <<= 2;

        uint32_t testDiv = (root << 1) + 1;
        if (remHi >= testDiv) {
            remHi -= testDiv;
            root++;
        }
    } while (--count >= 0);

    return root;
}
