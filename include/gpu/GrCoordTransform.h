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
    GrCoordTransform(bool bFoo, const GrTexture* texture, GrSamplerParams::FilterMode filter) {
        SkASSERT(texture);
        SkDEBUGCODE(fInProcessor = false);
        this->reset3(bFoo, texture, filter);
    }

    /**
     * Create a transformation from a matrix. The precision is inferred from the texture size and
     * filter. The texture origin also implies whether a y-reversal should be performed.
     */
    GrCoordTransform(const SkMatrix& m, bool bFoo, const GrTexture* texture,
                     GrSamplerParams::FilterMode filter) {
        SkDEBUGCODE(fInProcessor = false);
        SkASSERT(texture);
        this->reset1(m, bFoo, texture, filter);
    }

    /**
     * Create a transformation that applies the matrix to a coord set.
     */
    GrCoordTransform(const SkMatrix& m, bool bFoo, GrSLPrecision precision = kDefault_GrSLPrecision) {
        SkDEBUGCODE(fInProcessor = false);
        this->reset2(m, bFoo, precision);
    }

    void reset3(bool bFoo, const GrTexture* texture, GrSamplerParams::FilterMode filter) {
        SkASSERT(bFoo);
        SkASSERT(!fInProcessor);
        SkASSERT(texture);
        this->reset1(SkMatrix::I(), bFoo, /*MakeDivByTextureWHMatrix(texture)*/ texture, filter);
    }

    void reset1(const SkMatrix&, bool bFoo, const GrTexture*, GrSamplerParams::FilterMode filter);
    void reset2(const SkMatrix& m, bool bFoo, GrSLPrecision precision = kDefault_GrSLPrecision);

    GrCoordTransform& operator= (const GrCoordTransform& that) {
        SkASSERT(!fInProcessor);
        fMatrix2 = that.fMatrix2;
        fFoo1 = that.fFoo1;
        fTexture = that.fTexture;
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
        return &fMatrix2;
    }

    bool operator==(const GrCoordTransform& that) const {
        return fMatrix2.cheapEqualTo(that.fMatrix2) &&
               fFoo1 == that.fFoo1 &&
               fTexture == that.fTexture &&
               fReverseY == that.fReverseY &&
               fPrecision == that.fPrecision;
    }

    bool operator!=(const GrCoordTransform& that) const { return !(*this == that); }

    const SkMatrix& getMatrix5() const { return fMatrix2; }
    bool foo() const { return fFoo1; }
    const GrTexture* texture() const { return fTexture; }
    bool reverseY1() const { return fReverseY; }
    GrSLPrecision precision() const { return fPrecision; }

private:
    SkMatrix                fMatrix2;
    bool                    fFoo1;
    const GrTexture*        fTexture;
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
