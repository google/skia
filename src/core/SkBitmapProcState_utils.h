/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapProcState_utils_DEFINED
#define SkBitmapProcState_utils_DEFINED

// Helper to ensure that when we shift down, we do it w/o sign-extension
// so the caller doesn't have to manually mask off the top 16 bits
//
static inline unsigned SK_USHIFT16(unsigned x) {
    return x >> 16;
}

/*
 *  The decal_ functions require that
 *  1. dx > 0
 *  2. [fx, fx+dx, fx+2dx, fx+3dx, ... fx+(count-1)dx] are all <= maxX
 *
 *  In addition, we use SkFractionalInt to keep more fractional precision than
 *  just SkFixed, so we will abort the decal_ call if dx is very small, since
 *  the decal_ function just operates on SkFixed. If that were changed, we could
 *  skip the very_small test here.
 */
static inline bool can_truncate_to_fixed_for_decal(SkFixed fx,
                                                   SkFixed dx,
                                                   int count, unsigned max) {
    SkASSERT(count > 0);

    // if decal_ kept SkFractionalInt precision, this would just be dx <= 0
    // I just made up the 1/256. Just don't want to perceive accumulated error
    // if we truncate frDx and lose its low bits.
    if (dx <= SK_Fixed1 / 256) {
        return false;
    }

    // Note: it seems the test should be (fx <= max && lastFx <= max); but
    // historically it's been a strict inequality check, and changing produces
    // unexpected diffs.  Further investigation is needed.

    // We cast to unsigned so we don't have to check for negative values, which
    // will now appear as very large positive values, and thus fail our test!
    if ((unsigned)SkFixedFloorToInt(fx) >= max) {
        return false;
    }

    // Promote to 64bit (48.16) to avoid overflow.
    const uint64_t lastFx = fx + sk_64_mul(dx, count - 1);

    return sk_64_isS32(lastFx) && (unsigned)SkFixedFloorToInt(sk_64_asS32(lastFx)) < max;
}

#endif /* #ifndef SkBitmapProcState_utils_DEFINED */
