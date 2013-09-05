#ifndef SkBitmapProcState_utils_DEFINED
#define SkBitmapProcState_utils_DEFINED

// Helper to ensure that when we shift down, we do it w/o sign-extension
// so the caller doesn't have to manually mask off the top 16 bits
//
static unsigned SK_USHIFT16(unsigned x) {
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
static inline bool can_truncate_to_fixed_for_decal(SkFractionalInt frX,
                                                   SkFractionalInt frDx,
                                                   int count, unsigned max) {
    SkFixed dx = SkFractionalIntToFixed(frDx);

    // if decal_ kept SkFractionalInt precision, this would just be dx <= 0
    // I just made up the 1/256. Just don't want to perceive accumulated error
    // if we truncate frDx and lose its low bits.
    if (dx <= SK_Fixed1 / 256) {
        return false;
    }

    // We cast to unsigned so we don't have to check for negative values, which
    // will now appear as very large positive values, and thus fail our test!
    SkFixed fx = SkFractionalIntToFixed(frX);
    return (unsigned)SkFixedFloorToInt(fx) <= max &&
           (unsigned)SkFixedFloorToInt(fx + dx * (count - 1)) < max;
}

#endif /* #ifndef SkBitmapProcState_utils_DEFINED */
