/*
 * Copyright 2015 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGifInterlaceIter.h"

static const uint8_t kStartingInterlaceYValues[] = { 0, 4, 2, 1 };
static const uint8_t kDeltaInterlaceYValues[] = { 8, 8, 4, 2 };

SkGifInterlaceIter::SkGifInterlaceIter(int height) : fHeight(height) {
    fStartYPtr = kStartingInterlaceYValues;
    fDeltaYPtr = kDeltaInterlaceYValues;

    fCurrY = *fStartYPtr++;
    fDeltaY = *fDeltaYPtr++;
}

void SkGifInterlaceIter::prepareY() {
    int32_t y = fCurrY + fDeltaY;

    // Iterate through fStartYPtr until a valid row is found.
    // This ensures that we do not move past the height of the small images.
    while (y >= fHeight) {
        if (kStartingInterlaceYValues +
                SK_ARRAY_COUNT(kStartingInterlaceYValues) == fStartYPtr) {
            // Now we have iterated over the entire image.  Forbid any
            // subsequent calls to nextY().
            SkDEBUGCODE(fStartYPtr = NULL;)
            SkDEBUGCODE(fDeltaYPtr = NULL;)
            y = 0;
        } else {
            y = *fStartYPtr++;
            fDeltaY = *fDeltaYPtr++;
        }
    }
    fCurrY = y;
}

int32_t SkGifInterlaceIter::nextY() {
    SkASSERT(fStartYPtr);
    SkASSERT(fDeltaYPtr);
    int32_t y = fCurrY;
    prepareY();
    return y;
}
