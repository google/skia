/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkYUVASizeInfo_DEFINED
#define SkYUVASizeInfo_DEFINED

#include "SkSize.h"

struct SkYUVASizeInfo {
    enum YUVAIndex {
        kY          = 0,
        kU          = 1,
        kV          = 2,
        kA          = 3
    };
    SkISize fSizes[4];

    /**
     * While the widths of the Y, U, V and A planes are not restricted, the
     * implementation often requires that the width of the memory allocated
     * for each plane be a multiple of 8.
     *
     * This struct allows us to inform the client how many "widthBytes"
     * that we need.  Note that we use the new idea of "widthBytes"
     * because this idea is distinct from "rowBytes" (used elsewhere in
     * Skia).  "rowBytes" allow the last row of the allocation to not
     * include any extra padding, while, in this case, every single row of
     * the allocation must be at least "widthBytes".
     */
    size_t fWidthBytes[4];
};

#endif // SkYUVASizeInfo_DEFINED
