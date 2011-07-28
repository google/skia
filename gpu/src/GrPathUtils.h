
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrPathUtils_DEFINED
#define GrPathUtils_DEFINED

#include "GrNoncopyable.h"
#include "GrPoint.h"
#include "GrPath.h"

/**
 *  Utilities for evaluating paths.
 */
class GrPathUtils : public GrNoncopyable {
public:
    /// Since we divide by tol if we're computing exact worst-case bounds,
    /// very small tolerances will be increased to gMinCurveTol.
    static int worstCasePointCount(const GrPath&,
                                   int* subpaths,
                                   GrScalar tol);
    /// Since we divide by tol if we're computing exact worst-case bounds,
    /// very small tolerances will be increased to gMinCurveTol.
    static uint32_t quadraticPointCount(const GrPoint points[], GrScalar tol);
    static uint32_t generateQuadraticPoints(const GrPoint& p0,
                                            const GrPoint& p1,
                                            const GrPoint& p2,
                                            GrScalar tolSqd,
                                            GrPoint** points,
                                            uint32_t pointsLeft);
    /// Since we divide by tol if we're computing exact worst-case bounds,
    /// very small tolerances will be increased to gMinCurveTol.
    static uint32_t cubicPointCount(const GrPoint points[], GrScalar tol);
    static uint32_t generateCubicPoints(const GrPoint& p0,
                                        const GrPoint& p1,
                                        const GrPoint& p2,
                                        const GrPoint& p3,
                                        GrScalar tolSqd,
                                        GrPoint** points,
                                        uint32_t pointsLeft);

private:
    static const GrScalar gMinCurveTol;
};
#endif
