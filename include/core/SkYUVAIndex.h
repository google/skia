/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkYUVAIndex_DEFINED
#define SkYUVAIndex_DEFINED

#include "SkTypes.h"

/** \enum SkColorChannel
    Describes different color channels one can manipulate
*/
enum class SkColorChannel {
    kR,  // the red channel
    kG,  // the green channel
    kB,  // the blue channel
    kA,  // the alpha channel

    kLastEnum = kA,
};

/** \struct SkYUVAIndex
    Describes from which image source and which channel to read each individual YUVA plane.

    SkYUVAIndex contains a index for which image source to read from and a enum for which channel
    to read from.
*/
struct SK_API SkYUVAIndex {
    // Index in the array of SkYUVAIndex
    enum Index {
        kY_Index = 0,
        kU_Index = 1,
        kV_Index = 2,
        kA_Index = 3
    };

    /** The index is a number between -1..3 which definies which image source to read from, where -1
     * means the image source doesn't exist. The assumption is we will always have image sources for
     * each of YUV planes, but optionally have image source for A plane. */
    int            fIndex;
    /** The channel describes from which channel to read the info from. Currently we only deal with
     * YUV and NV12 and channel info is ignored. */
    SkColorChannel fChannel;
};

#endif
