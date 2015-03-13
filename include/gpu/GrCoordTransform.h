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
#include "GrShaderVar.h"

/**
 * Coordinates available to GrProcessor subclasses for requesting transformations. Transformed
 * coordinates are made available in the the portion of fragment shader emitted by the effect.
 *
 * The precision of the shader var that interpolates the transformed coordinates can be specified.
 */
enum GrCoordSet {
    /**
     * The user-space coordinates that map to the fragment being rendered. This is the space in
     * which SkShader operates. It is usually the space in which geometry passed to SkCanvas is
     * specified (before the view matrix is applied). However, some draw calls take explicit local
     * coords that map onto the geometry (e.g. drawVertices, drawBitmapRectToRect).
     */
    kLocal_GrCoordSet,

    /**
     * The device space position of the fragment being shaded.
     */
    kDevice_GrCoordSet,
};

/**
 * A class representing a linear transformation from one of the built-in coordinate sets (local or
 * position). GrProcessors just define these transformations, and the framework does the rest of the
 * work to make the transformed coordinates available in their fragment shader.
 */
class GrCoordTransform : SkNoncopyable {
public:
    GrCoordTransform() : fSourceCoords(kLocal_GrCoordSet) { SkDEBUGCODE(fInProcessor = false); }

    /**
     * Create a transformation that maps [0, 1] to a texture's boundaries. The precision is inferred
     * from the texture size and filter. The texture origin also implies whether a y-reversal should
     * be performed.
     */
    GrCoordTransform(GrCoordSet sourceCoords,
                     const GrTexture* texture,
                     GrTextureParams::FilterMode filter) {
        SkASSERT(texture);
        SkDEBUGCODE(fInProcessor = false);
        this->reset(sourceCoords, texture, filter);
    }

    /**
     * Create a transformation from a matrix. The precision is inferred from the texture size and
     * filter. The texture origin also implies whether a y-reversal should be performed.
     */
    GrCoordTransform(GrCoordSet sourceCoords, const SkMatrix& m,
                     const GrTexture* texture, GrTextureParams::FilterMode filter) {
        SkDEBUGCODE(fInProcessor = false);
        SkASSERT(texture);
        this->reset(sourceCoords, m, texture, filter);
    }

    /**
     * Create a transformation that applies the matrix to a coord set.
     */
    GrCoordTransform(GrCoordSet sourceCoords, const SkMatrix& m,
                     GrSLPrecision precision = kDefault_GrSLPrecision) {
        SkDEBUGCODE(fInProcessor = false);
        this->reset(sourceCoords, m, precision);
    }

    void reset(GrCoordSet sourceCoords, const GrTexture* texture,
               GrTextureParams::FilterMode filter) {
        SkASSERT(!fInProcessor);
        SkASSERT(texture);
        this->reset(sourceCoords, MakeDivByTextureWHMatrix(texture), texture, filter);
    }

    void reset(GrCoordSet, const SkMatrix&, const GrTexture*, GrTextureParams::FilterMode filter);
    void reset(GrCoordSet sourceCoords, const SkMatrix& m,
               GrSLPrecision precision = kDefault_GrSLPrecision);

    GrCoordTransform& operator= (const GrCoordTransform& that) {
        SkASSERT(!fInProcessor);
        fSourceCoords = that.fSourceCoords;
        fMatrix = that.fMatrix;
        fReverseY = that.fReverseY;
        fPrecision = that.fPrecision;
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

    bool operator==(const GrCoordTransform& that) const {
        return fSourceCoords == that.fSourceCoords &&
               fMatrix.cheapEqualTo(that.fMatrix) &&
               fReverseY == that.fReverseY &&
               fPrecision == that.fPrecision;
    }

    bool operator!=(const GrCoordTransform& that) const { return !(*this == that); }

    GrCoordSet sourceCoords() const { return fSourceCoords; }
    const SkMatrix& getMatrix() const { return fMatrix; }
    bool reverseY() const { return fReverseY; }
    GrSLPrecision precision() const { return fPrecision; }

    /** Useful for effects that want to insert a texture matrix that is implied by the texture
        dimensions */
    static inline SkMatrix MakeDivByTextureWHMatrix(const GrTexture* texture) {
        SkASSERT(texture);
        SkMatrix mat;
        (void)mat.setIDiv(texture->width(), texture->height());
        return mat;
    }

private:
    GrCoordSet              fSourceCoords;
    SkMatrix                fMatrix;
    bool                    fReverseY;
    GrSLPrecision           fPrecision;
    typedef SkNoncopyable INHERITED;

#ifdef SK_DEBUG
public:
    void setInProcessor() const { fInProcessor = true; }
private:
    mutable bool fInProcessor;
#endif
};

#endif
