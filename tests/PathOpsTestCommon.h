/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef PathOpsTestCommon_DEFINED
#define PathOpsTestCommon_DEFINED

#include "include/private/SkTArray.h"
#include "src/pathops/SkPathOpsQuad.h"

struct SkPathOpsBounds;

struct QuadPts {
    static const int kPointCount = 3;
    SkDPoint fPts[kPointCount];
};

struct ConicPts {
    QuadPts fPts;
    SkScalar fWeight;
};

struct CubicPts {
    static const int kPointCount = 4;
    SkDPoint fPts[kPointCount];
};

void CubicPathToQuads(const SkPath& cubicPath, SkPath* quadPath);
void CubicPathToSimple(const SkPath& cubicPath, SkPath* simplePath);
void CubicToQuads(const SkDCubic& cubic, double precision, SkTArray<SkDQuad, true>& quads);
bool ValidBounds(const SkPathOpsBounds& );
bool ValidConic(const SkDConic& cubic);
bool ValidCubic(const SkDCubic& cubic);
bool ValidLine(const SkDLine& line);
bool ValidPoint(const SkDPoint& pt);
bool ValidPoints(const SkPoint* pts, int count);
bool ValidQuad(const SkDQuad& quad);
bool ValidVector(const SkDVector& v);

#endif
