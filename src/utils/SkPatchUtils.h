/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPatchUtils_DEFINED
#define SkPatchUtils_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkVertices.h"
#include "include/private/SkColorData.h"

class SkColorSpace;

class SkPatchUtils {

public:
    // Enums for control points based on the order specified in the constructor (clockwise).
    enum {
        kNumCtrlPts = 12,
        kNumCorners = 4,
        kNumPtsCubic = 4
    };

    /**
     * Get the points corresponding to the top cubic of cubics.
     */
    static void GetTopCubic(const SkPoint cubics[12], SkPoint points[4]);

    /**
     * Get the points corresponding to the bottom cubic of cubics.
     */
    static void GetBottomCubic(const SkPoint cubics[12], SkPoint points[4]);

    /**
     * Get the points corresponding to the left cubic of cubics.
     */
    static void GetLeftCubic(const SkPoint cubics[12], SkPoint points[4]);

    /**
     * Get the points corresponding to the right cubic of cubics.
     */
    static void GetRightCubic(const SkPoint cubics[12], SkPoint points[4]);

    /**
     * Method that calculates a level of detail (number of subdivisions) for a patch in both axis.
     */
    static SkISize GetLevelOfDetail(const SkPoint cubics[12], const SkMatrix* matrix);

    static sk_sp<SkVertices> MakeVertices(const SkPoint cubics[12], const SkColor colors[4],
                                          const SkPoint texCoords[4], int lodX, int lodY,
                                          SkColorSpace* colorSpace = nullptr);
};

#endif
