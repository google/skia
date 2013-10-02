/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCoordTransform_DEFINED
#define GrCoordTransform_DEFINED

#include "GrEffect.h"
#include "SkMatrix.h"
#include "GrTexture.h"
#include "GrTypes.h"

/**
 * Coordinates available to GrEffect subclasses for requesting transformations. Transformed
 * coordinates are made available in the the portion of fragment shader emitted by the effect.
 */
enum GrCoordSet {
    /**
     * The user-space coordinates that map to the fragment being rendered. These coords account for
     * any change of coordinate system done on the CPU by GrContext before rendering, and also are
     * correct for draws that take explicit local coords rather than inferring them from the
     * primitive's positions (e.g. drawVertices). These are usually the coords a GrEffect wants.
     */
    kLocal_GrCoordSet,

    /**
     * The actual vertex position. Note that GrContext may not draw using the original view matrix
     * specified by the caller, as it may have transformed vertices into another space. These are
     * usually not the coordinates a GrEffect wants.
     */
    kPosition_GrCoordSet
};

/**
 * A class representing a linear transformation from one of the built-in coordinate sets (local or
 * position). GrEffects just define these transformations, and the framework does the rest of the
 * work to make the transformed coordinates available in their fragment shader.
 */
class GrCoordTransform : public SkNoncopyable {
public:
    GrCoordTransform() {}

    /**
     * Create a transformation that maps [0, 1] to a texture's boundaries.
     */
    GrCoordTransform(GrCoordSet sourceCoords, const GrTexture* texture) {
        this->reset(sourceCoords, texture);
    }

    /**
     * Create a transformation from a matrix. The optional texture parameter is used to infer if the
     * framework should internally do a y reversal to account for it being upside down by Skia's
     * coord convention.
     */
    GrCoordTransform(GrCoordSet sourceCoords, const SkMatrix& m, const GrTexture* texture = NULL) {
        this->reset(sourceCoords, m, texture);
    }

    void reset(GrCoordSet sourceCoords, const GrTexture* texture) {
        SkASSERT(NULL != texture);
        this->reset(sourceCoords, GrEffect::MakeDivByTextureWHMatrix(texture), texture);
    }

    void reset(GrCoordSet sourceCoords, const SkMatrix& m, const GrTexture* texture = NULL) {
        fSourceCoords = sourceCoords;
        fMatrix = m;
        fReverseY = NULL != texture && kBottomLeft_GrSurfaceOrigin == texture->origin();
    }

    bool operator== (const GrCoordTransform& other) const {
        return fSourceCoords == other.fSourceCoords &&
               fMatrix.cheapEqualTo(other.fMatrix) &&
               fReverseY == other.fReverseY;
    }

    GrCoordSet sourceCoords() const { return fSourceCoords; }
    const SkMatrix& getMatrix() const { return fMatrix; }
    bool reverseY() const { return fReverseY; }

private:
    GrCoordSet fSourceCoords;
    SkMatrix   fMatrix;
    bool       fReverseY;

    typedef SkNoncopyable INHERITED;
};

#endif
