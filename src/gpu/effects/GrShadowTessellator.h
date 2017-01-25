/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrShadowTessellator_DEFINED
#define GrShadowTessellator_DEFINED

#include "SkTDArray.h"
#include "SkPoint.h"

#include "GrColor.h"

class SkMatrix;
class SkPath;

/**
 * This class generates an ambient shadow for a path by walking the path, outsetting by the
 * radius, and setting inner and outer colors to umbraColor and penumbraColor, respectively.
 * If transparent is true, then the center of the ambient shadow will be filled in.
 */
class GrAmbientShadowTessellator {
public:
    GrAmbientShadowTessellator(const SkMatrix& viewMatrix, const SkPath& path, SkScalar radius,
                               GrColor umbraColor, GrColor penumbraColor, bool transparent);

    int      vertexCount() { return fPositions.count(); }
    SkPoint* positions() { return fPositions.begin(); }
    GrColor* colors() { return fColors.begin(); }
    int      indexCount() { return fIndices.count(); }
    uint16_t* indices() { return fIndices.begin(); }

private:
    void handleLine(const SkPoint& p);
    void handleLine(const SkMatrix& m, SkPoint p);

    void handleQuad(const SkPoint pts[3]);
    void handleQuad(const SkMatrix& m, SkPoint pts[3]);

    void handleCubic(const SkMatrix& m, SkPoint pts[4]);

    void handleConic(const SkMatrix& m, SkPoint pts[3], SkScalar w);

    void addArc(const SkVector& nextNormal);
    void finishArcAndAddEdge(const SkVector& nextPoint, const SkVector& nextNormal);
    void addEdge(const SkVector& nextPoint, const SkVector& nextNormal);

    SkScalar            fRadius;
    GrColor             fUmbraColor;
    GrColor             fPenumbraColor;
    bool                fTransparent;

    SkTDArray<SkPoint>  fPositions;
    SkTDArray<GrColor>  fColors;
    SkTDArray<uint16_t> fIndices;

    int                 fPrevInnerIndex;
    SkVector            fPrevNormal;
    int                 fFirstVertex;
    SkVector            fFirstNormal;
    SkScalar            fDirection;
    int                 fCentroidCount;

    // first three points
    SkTDArray<SkPoint>  fInitPoints;
    // temporary buffer
    SkTDArray<SkPoint>  fPointBuffer;
};

#endif
