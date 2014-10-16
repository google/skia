/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCoordTransform_DEFINED
#define GrCoordTransform_DEFINED

#include "GrProcessor.h"
#include "SkMatrix.h"
#include "GrTexture.h"
#include "GrTypes.h"

/**
 * Coordinates available to GrProcessor subclasses for requesting transformations. Transformed
 * coordinates are made available in the the portion of fragment shader emitted by the effect.
 */
enum GrCoordSet {
    /**
     * The user-space coordinates that map to the fragment being rendered. These coords account for
     * any change of coordinate system done on the CPU by GrContext before rendering, and also are
     * correct for draws that take explicit local coords rather than inferring them from the
     * primitive's positions (e.g. drawVertices). These are usually the coords a GrProcessor wants.
     */
    kLocal_GrCoordSet,

    /**
     * The actual vertex position. Note that GrContext may not draw using the original view matrix
     * specified by the caller, as it may have transformed vertices into another space. These are
     * usually not the coordinates a GrProcessor wants.
     */
    kPosition_GrCoordSet
};

/**
 * A class representing a linear transformation from one of the built-in coordinate sets (local or
 * position). GrProcessors just define these transformations, and the framework does the rest of the
 * work to make the transformed coordinates available in their fragment shader.
 */
class GrCoordTransform : SkNoncopyable {
public:
    GrCoordTransform() { SkDEBUGCODE(fInProcessor = false); }

    /**
     * Create a transformation that maps [0, 1] to a texture's boundaries.
     */
    GrCoordTransform(GrCoordSet sourceCoords, const GrTexture* texture) {
        SkDEBUGCODE(fInProcessor = false);
        this->reset(sourceCoords, texture);
    }

    /**
     * Create a transformation from a matrix. The optional texture parameter is used to infer if the
     * framework should internally do a y reversal to account for it being upside down by Skia's
     * coord convention.
     */
    GrCoordTransform(GrCoordSet sourceCoords, const SkMatrix& m, const GrTexture* texture = NULL) {
        SkDEBUGCODE(fInProcessor = false);
        this->reset(sourceCoords, m, texture);
    }

    void reset(GrCoordSet sourceCoords, const GrTexture* texture) {
        SkASSERT(!fInProcessor);
        SkASSERT(texture);
        this->reset(sourceCoords, MakeDivByTextureWHMatrix(texture), texture);
    }

    void reset(GrCoordSet sourceCoords, const SkMatrix& m, const GrTexture* texture = NULL) {
        SkASSERT(!fInProcessor);
        fSourceCoords = sourceCoords;
        fMatrix = m;
        fReverseY = texture && kBottomLeft_GrSurfaceOrigin == texture->origin();
    }

    GrCoordTransform& operator= (const GrCoordTransform& other) {
        SkASSERT(!fInProcessor);
        fSourceCoords = other.fSourceCoords;
        fMatrix = other.fMatrix;
        fReverseY = other.fReverseY;
        return *this;
    }

    /**
     * Access the matrix for editing. Note, this must be done before adding the transform to an
     * effect, since effects are immutable.
     */
    SkMatrix* accessMatrix() {
        SkASSERT(!fInProcessor);
        return &fMatrix;
    }

    bool operator== (const GrCoordTransform& that) const {
        return fSourceCoords == that.fSourceCoords &&
               fMatrix.cheapEqualTo(that.fMatrix) &&
               fReverseY == that.fReverseY;
    }

    bool operator!= (const GrCoordTransform& that) const { return !(*this == that); }

    GrCoordSet sourceCoords() const { return fSourceCoords; }
    const SkMatrix& getMatrix() const { return fMatrix; }
    bool reverseY() const { return fReverseY; }

    /** Useful for effects that want to insert a texture matrix that is implied by the texture
        dimensions */
    static inline SkMatrix MakeDivByTextureWHMatrix(const GrTexture* texture) {
        SkASSERT(texture);
        SkMatrix mat;
        mat.setIDiv(texture->width(), texture->height());
        return mat;
    }

private:
    GrCoordSet fSourceCoords;
    SkMatrix   fMatrix;
    bool       fReverseY;

    typedef SkNoncopyable INHERITED;

#ifdef SK_DEBUG
public:
    void setInProcessor() const { fInProcessor = true; }
private:
    mutable bool fInProcessor;
#endif
};

#endif
