/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShadowTessellator_DEFINED
#define SkShadowTessellator_DEFINED

#include "SkTDArray.h"
#include "SkPoint.h"

#include "SkColor.h"

class SkMatrix;
class SkPath;

// TODO: derive these two classes from a base class containing common elements

/**
 * This class generates an ambient shadow for a path by walking the path, outsetting by the
 * radius, and setting inner and outer colors to umbraColor and penumbraColor, respectively.
 * If transparent is true, then the center of the ambient shadow will be filled in.
 */
class SkAmbientShadowTessellator {
public:
    SkAmbientShadowTessellator(const SkPath& path, SkScalar radius, SkColor umbraColor,
                               SkColor penumbraColor, bool transparent);

    int      vertexCount() { return fPositions.count(); }
    SkPoint* positions() { return fPositions.begin(); }
    SkColor* colors() { return fColors.begin(); }
    int      indexCount() { return fIndices.count(); }
    uint16_t* indices() { return fIndices.begin(); }

private:
    void handleLine(const SkPoint& p);

    void handleQuad(const SkPoint pts[3]);

    void handleCubic(SkPoint pts[4]);

    void handleConic(SkPoint pts[3], SkScalar w);

    void addArc(const SkVector& nextNormal);
    void finishArcAndAddEdge(const SkVector& nextPoint, const SkVector& nextNormal);
    void addEdge(const SkVector& nextPoint, const SkVector& nextNormal);

    SkScalar            fRadius;
    SkColor             fUmbraColor;
    SkColor             fPenumbraColor;
    bool                fTransparent;

    SkTDArray<SkPoint>  fPositions;
    SkTDArray<SkColor>  fColors;
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

/**
 * This class generates an spot shadow for a path by walking the transformed path, further 
 * transforming by the scale and translation, and outsetting and insetting by a radius.
 * The center will be clipped against the original path unless transparent is true.
 */
class SkSpotShadowTessellator {
public:
    SkSpotShadowTessellator(const SkPath& path, SkScalar scale, const SkVector& translate,
                            SkScalar radius, SkColor umbraColor, SkColor penumbraColor,
                            bool transparent);

    int      vertexCount() { return fPositions.count(); }
    SkPoint* positions() { return fPositions.begin(); }
    SkColor* colors() { return fColors.begin(); }
    int      indexCount() { return fIndices.count(); }
    uint16_t* indices() { return fIndices.begin(); }

private:
    void computeClipBounds(const SkPath& path);

    void handleLine(const SkPoint& p);
    void handleLine(SkScalar scale, const SkVector& xlate, SkPoint p);

    void handleQuad(const SkPoint pts[3]);
    void handleQuad(SkScalar scale, const SkVector& xlate, SkPoint pts[3]);

    void handleCubic(SkScalar scale, const SkVector& xlate, SkPoint pts[4]);

    void handleConic(SkScalar scale, const SkVector& xlate, SkPoint pts[3], SkScalar w);

    void mapPoints(SkScalar scale, const SkVector& xlate, SkPoint* pts, int count);
    void addInnerPoint(const SkPoint& pathPoint, SkColor umbraColor, SkScalar radiusSqd);
    void addArc(const SkVector& nextNormal);
    void finishArcAndAddEdge(const SkVector& nextPoint, const SkVector& nextNormal);
    void addEdge(const SkVector& nextPoint, const SkVector& nextNormal);

    SkScalar            fRadius;
    SkColor             fUmbraColor;
    SkColor             fPenumbraColor;

    SkTDArray<SkPoint>  fPositions;
    SkTDArray<SkColor>  fColors;
    SkTDArray<uint16_t> fIndices;

    int                 fPrevInnerIndex;
    SkPoint             fPrevPoint;
    SkVector            fPrevNormal;
    int                 fFirstVertex;
    SkPoint             fFirstPoint;
    SkVector            fFirstNormal;
    SkScalar            fDirection;

    SkPoint             fCentroid;
    SkTDArray<SkPoint>  fClipPolygon;

    // first three points
    SkTDArray<SkPoint>  fInitPoints;
    // temporary buffer
    SkTDArray<SkPoint>  fPointBuffer;
};

#endif
