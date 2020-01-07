/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCenterWedgePatchGen_DEFINED
#define GrCenterWedgePatchGen_DEFINED

#include "include/core/SkPoint.h"
#include "include/private/SkTArray.h"
#include "src/gpu/tessellate/GrTessellateWedgeShader.h"
#include <array>

class SkPath;

// Generates an array of cubic "wedges" from an SkPath, converting any lines and quadratics to
// cubics. These wedges can then be fed into GrTessellateWedgeShader to stencil the path. A wedge is
// a 5-point tessellation patch consisting of 4 cubic control points, plus an anchor point fanning
// from the center of the curve's resident contour.
// TODO: Eventually we want to use rational cubic wedges in order to support perspective and conics.
class GrCenterWedgePatchGen {
public:
    GrCenterWedgePatchGen(const SkPath& path) : fPath(path) {}

    // Iterates over the path and returns the number of wedges.
    //
    // If vertexData is null, then contourMidpoints must be empty and the center point of each
    // contour, in order, is appended at the end.
    //
    // If vertexData is non-null, then wedges are written out to vertexData using the in-order
    // values from contourMidpoints as the fan points for each contour.
    int walkPath(std::array<SkPoint, 5>* vertexData, SkTArray<SkPoint, true>* contourMidpoints);

private:
    SkPoint lineTo(const SkPoint& p1);
    SkPoint quadraticTo(const SkPoint& p1, const SkPoint& p2);
    SkPoint cubicTo(const SkPoint& p1, const SkPoint& p2, const SkPoint& p3);
    SkPoint close(const SkPoint& startPt);

    const SkPath& fPath;

    // Stateful data for path iteration.
    std::array<SkPoint, 5>* fWedgeData;
    SkTArray<SkPoint, true>* fContourMidpoints;
    SkPoint fLastPt;
    int fCurrContourInitialPatchCount;
    union {
        SkPoint* fCurrContourMidpoint;  // When fPatchData != null
        SkPoint fCurrContourFanPtsSum;  // When fPatchData == null
    };
    int fNumWedges;
};

#endif
