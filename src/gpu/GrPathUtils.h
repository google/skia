
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrPathUtils_DEFINED
#define GrPathUtils_DEFINED

#include "GrMatrix.h"
#include "GrPath.h"

/**
 *  Utilities for evaluating paths.
 */
namespace GrPathUtils {
    GrScalar scaleToleranceToSrc(GrScalar devTol,
                                 const GrMatrix& viewM,
                                 const GrRect& pathBounds);

    /// Since we divide by tol if we're computing exact worst-case bounds,
    /// very small tolerances will be increased to gMinCurveTol.
    int worstCasePointCount(const GrPath&,
                            int* subpaths,
                            GrScalar tol);
    /// Since we divide by tol if we're computing exact worst-case bounds,
    /// very small tolerances will be increased to gMinCurveTol.
    uint32_t quadraticPointCount(const GrPoint points[], GrScalar tol);
    uint32_t generateQuadraticPoints(const GrPoint& p0,
                                     const GrPoint& p1,
                                     const GrPoint& p2,
                                     GrScalar tolSqd,
                                     GrPoint** points,
                                     uint32_t pointsLeft);
    /// Since we divide by tol if we're computing exact worst-case bounds,
    /// very small tolerances will be increased to gMinCurveTol.
    uint32_t cubicPointCount(const GrPoint points[], GrScalar tol);
    uint32_t generateCubicPoints(const GrPoint& p0,
                                 const GrPoint& p1,
                                 const GrPoint& p2,
                                 const GrPoint& p3,
                                 GrScalar tolSqd,
                                 GrPoint** points,
                                 uint32_t pointsLeft);

};
#endif
