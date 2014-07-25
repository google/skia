/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPatch_DEFINED
#define SkPatch_DEFINED

#include "SkColor.h"
#include "SkPreConfig.h"
#include "SkPoint.h"

/**
 * Class that represents a coons patch.
 */
class SK_API SkPatch {

public:
    /**
     * Structure that holds the vertex data related to the tessellation of a SkPatch. It is passed 
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
        , fPoints(NULL)
        , fTexCoords(NULL)
        , fColors(NULL)
        , fIndices(NULL) { }
        
        ~VertexData() {
            SkDELETE_ARRAY(fPoints);
            SkDELETE_ARRAY(fTexCoords);
            SkDELETE_ARRAY(fColors);
            SkDELETE_ARRAY(fIndices);
        }
    };
    
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
    
    /**
     * Points are in the following order:
     *              (top curve)
     *                0 1 2 3
     * (left curve)  11     4   (right curve)
     *               10     5
     *                9 8 7 6
     *              (bottom curve)
     * Used pointer to an array to guarantee that this method receives an array of 4 SkColors
     */
    SkPatch(SkPoint points[12], SkColor colors[4]);

    /**
     * Function that evaluates the coons patch interpolation.
     * data refers to the pointer of the PatchData struct in which the tessellation data is set.
     * divisions defines the number of steps in which the SkPatch is going to be subdivided per
     * axis.
     */
    bool getVertexData(SkPatch::VertexData* data, int divisions);

    void getTopPoints(SkPoint points[4]) {
        points[0] = fCtrlPoints[kTopP0_CubicCtrlPts];
        points[1] = fCtrlPoints[kTopP1_CubicCtrlPts];
        points[2] = fCtrlPoints[kTopP2_CubicCtrlPts];
        points[3] = fCtrlPoints[kTopP3_CubicCtrlPts];
    }
    
    void getBottomPoints(SkPoint points[4]) {
        points[0] = fCtrlPoints[kBottomP0_CubicCtrlPts];
        points[1] = fCtrlPoints[kBottomP1_CubicCtrlPts];
        points[2] = fCtrlPoints[kBottomP2_CubicCtrlPts];
        points[3] = fCtrlPoints[kBottomP3_CubicCtrlPts];
    }

    void getLeftPoints(SkPoint points[4]) {
        points[0] = fCtrlPoints[kLeftP0_CubicCtrlPts];
        points[1] = fCtrlPoints[kLeftP1_CubicCtrlPts];
        points[2] = fCtrlPoints[kLeftP2_CubicCtrlPts];
        points[3] = fCtrlPoints[kLeftP3_CubicCtrlPts];
    }

    void getRightPoints(SkPoint points[4]) {
        points[0] = fCtrlPoints[kRightP0_CubicCtrlPts];
        points[1] = fCtrlPoints[kRightP1_CubicCtrlPts];
        points[2] = fCtrlPoints[kRightP2_CubicCtrlPts];
        points[3] = fCtrlPoints[kRightP3_CubicCtrlPts];
    }
    
private:
    SkPoint fCtrlPoints[12];
    SkPMColor fCornerColors[4];
};

#endif
