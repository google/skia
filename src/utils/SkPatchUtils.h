/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPatchUtils_DEFINED
#define SkPatchUtils_DEFINED

#include "SkColorPriv.h"
#include "SkMatrix.h"

class SK_API SkPatchUtils {

public:
    /**
     * Structure that holds the vertex data related to the tessellation of a patch. It is passed
     * as a parameter to the function getVertexData which sets the points, colors and texture
     * coordinates of the vertices and the indices for them to be drawn as triangles.
     */
    struct VertexData {
        int fVertexCount, fIndexCount;
        SkPoint* fPoints;
        SkPoint* fTexCoords;
        uint32_t* fColors;
        uint16_t* fIndices;

        VertexData()
        : fVertexCount(0)
        , fIndexCount(0)
        , fPoints(nullptr)
        , fTexCoords(nullptr)
        , fColors(nullptr)
        , fIndices(nullptr) { }

        ~VertexData() {
            delete[] fPoints;
            delete[] fTexCoords;
            delete[] fColors;
            delete[] fIndices;
        }
    };

    // Enums for control points based on the order specified in the constructor (clockwise).
    enum CubicCtrlPts {
        kTopP0_CubicCtrlPts = 0,
        kTopP1_CubicCtrlPts = 1,
        kTopP2_CubicCtrlPts = 2,
        kTopP3_CubicCtrlPts = 3,

        kRightP0_CubicCtrlPts = 3,
        kRightP1_CubicCtrlPts = 4,
        kRightP2_CubicCtrlPts = 5,
        kRightP3_CubicCtrlPts = 6,

        kBottomP0_CubicCtrlPts = 9,
        kBottomP1_CubicCtrlPts = 8,
        kBottomP2_CubicCtrlPts = 7,
        kBottomP3_CubicCtrlPts = 6,

        kLeftP0_CubicCtrlPts = 0,
        kLeftP1_CubicCtrlPts = 11,
        kLeftP2_CubicCtrlPts = 10,
        kLeftP3_CubicCtrlPts = 9,
    };

    // Enum for corner also clockwise.
    enum Corner {
        kTopLeft_Corner = 0,
        kTopRight_Corner,
        kBottomRight_Corner,
        kBottomLeft_Corner
    };

    enum {
        kNumCtrlPts = 12,
        kNumCorners = 4,
        kNumPtsCubic = 4
    };

    /**
     * Method that calculates a level of detail (number of subdivisions) for a patch in both axis.
     */
    static SkISize GetLevelOfDetail(const SkPoint cubics[12], const SkMatrix* matrix);

    /**
     * Get the points corresponding to the top cubic of cubics.
     */
    static void getTopCubic(const SkPoint cubics[12], SkPoint points[4]);

    /**
     * Get the points corresponding to the bottom cubic of cubics.
     */
    static void getBottomCubic(const SkPoint cubics[12], SkPoint points[4]);

    /**
     * Get the points corresponding to the left cubic of cubics.
     */
    static void getLeftCubic(const SkPoint cubics[12], SkPoint points[4]);

    /**
     * Get the points corresponding to the right cubic of cubics.
     */
    static void getRightCubic(const SkPoint cubics[12], SkPoint points[4]);

    /**
     * Function that evaluates the coons patch interpolation.
     * data refers to the pointer of the PatchData struct in which the tessellation data is set.
     * cubics refers to the points of the cubics.
     * lod refers the level of detail for each axis.
     * colors refers to the corner colors that will be bilerp across the patch (optional parameter)
     * texCoords refers to the corner texture coordinates that will be bilerp across the patch
        (optional parameter)
     */
    static bool getVertexData(SkPatchUtils::VertexData* data, const SkPoint cubics[12],
                              const SkColor colors[4], const SkPoint texCoords[4],
                              int lodX, int lodY);
};

#endif
