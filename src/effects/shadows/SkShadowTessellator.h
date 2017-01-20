/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShadowTessellator_DEFINED
#define SkShadowTessellator_DEFINED

#include "GrColor.h"
#include "SkPaint.h"
#include "SkPath.h"

class SkAmbientShadowTessellator {
public:
    SkAmbientShadowTessellator(const SkMatrix& viewMatrix, const SkPath& path,
                               SkScalar radius, GrColor umbraColor, bool transparent);

    int      vertexCount() { return fPositions.count(); }
    SkPoint* positions() { return fPositions.begin(); }
    GrColor* colors() { return fColors.begin(); }
    int      indexCount() { return fIndices.count(); }
    uint16_t* indices() { return fIndices.begin(); }

private:
    void lineTo(const SkPoint& p);
    void lineTo(const SkMatrix& m, SkPoint p);

    void quadTo(const SkPoint pts[3]);
    void quadTo(const SkMatrix& m, SkPoint pts[3]);

    void cubicTo(const SkMatrix& m, SkPoint pts[4]);

    void conicTo(const SkMatrix& m, SkPoint pts[3], SkScalar w);

    SkScalar            fRadius;
    GrColor             fUmbraColor;
    bool                fTransparent;

    SkTDArray<SkPoint>  fPositions;
    SkTDArray<GrColor>  fColors;
    SkTDArray<uint16_t> fIndices;

    int                 fPrevInnerIndex;
    int                 fPrevOuterIndex;
    SkVector            fPrevNormal;
    int                 fFirstVertex;
    SkVector            fFirstNormal;
    SkScalar            fDirection;

    // first three points
    SkTDArray<SkPoint>  fInitPoints;
    // temporary buffer
    SkTDArray<SkPoint>  fPointBuffer;
};

#endif
