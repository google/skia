/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkYUVSizeInfo_DEFINED
#define SkYUVSizeInfo_DEFINED

#include "SkSize.h"

struct SkYUVSizeInfo {
    enum {
        kY          = 0,
        kU          = 1,
        kV          = 2,
    };
    SkISize fSizes[3];

    /**
     * While the widths of the Y, U, and V planes are not restricted, the
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
    size_t fWidthBytes[3];
};

#endif // SkYUVSizeInfo_DEFINED
