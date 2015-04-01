/*
 * Copyright 2015 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

/*
 * Helper class to determine the destination y-values for interlaced gifs
 */
class SkGifInterlaceIter : SkNoncopyable {
public:

    explicit SkGifInterlaceIter(int32_t height);

    /*
     * Get the next destination y-value
     */
    int32_t nextY();

private:

    /*
     * Updates the iterator to prepare the next y-value
     */
    void prepareY();

    const int32_t fHeight;
    int32_t        fCurrY;
    int32_t        fDeltaY;
    const uint8_t* fStartYPtr;
    const uint8_t* fDeltaYPtr;
};
