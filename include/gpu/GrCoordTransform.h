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
 * A class representing a linear transformation of local coordinates. GrFragnentProcessors
 * these transformations, and the GrGeometryProcessor implements the transformation.
 */
class GrCoordTransform : SkNoncopyable {
public:
    GrCoordTransform() { SkDEBUGCODE(fInProcessor = false); }

    /**
     * Create a transformation that maps [0, 1] to a texture's boundaries. The precision is inferred
     * from the texture size and filter. The texture origin also implies whether a y-reversal should
     * be performed.
     */
    GrCoordTransform(const GrTexture* texture, GrSamplerParams::FilterMode filter) {
        SkASSERT(texture);
        SkDEBUGCODE(fInProcessor = false);
        this->reset(texture, filter);
    }

    /**
     * Create a transformation from a matrix. The precision is inferred from the texture size and
     * filter. The texture origin also implies whether a y-reversal should be performed.
     */
    GrCoordTransform(const SkMatrix& m, const GrTexture* texture,
                     GrSamplerParams::FilterMode filter) {
        SkDEBUGCODE(fInProcessor = false);
        SkASSERT(texture);
        this->reset(m, texture, filter);
    }

    /**
     * Create a transformation that applies the matrix to a coord set.
     */
    GrCoordTransform(const SkMatrix& m, GrSLPrecision precision = kDefault_GrSLPrecision) {
        SkDEBUGCODE(fInProcessor = false);
        this->reset(m, precision);
    }

    void reset(const GrTexture* texture, GrSamplerParams::FilterMode filter) {
        SkASSERT(!fInProcessor);
        SkASSERT(texture);
        this->reset(MakeDivByTextureWHMatrix(texture), texture, filter);
    }

    void reset(const SkMatrix&, const GrTexture*, GrSamplerParams::FilterMode filter);
    void reset(const SkMatrix& m, GrSLPrecision precision = kDefault_GrSLPrecision);

    GrCoordTransform& operator= (const GrCoordTransform& that) {
        SkASSERT(!fInProcessor);
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
        return fMatrix.cheapEqualTo(that.fMatrix) &&
               fReverseY == that.fReverseY &&
               fPrecision == that.fPrecision;
    }

    bool operator!=(const GrCoordTransform& that) const { return !(*this == that); }

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
