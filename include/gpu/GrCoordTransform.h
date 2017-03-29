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
#include "GrTypes.h"
#include "GrShaderVar.h"

class GrTextureProxy;

/**
 * A class representing a linear transformation of local coordinates. GrFragnentProcessors
 * these transformations, and the GrGeometryProcessor implements the transformation.
 */
class GrCoordTransform : SkNoncopyable {
public:
    GrCoordTransform()
        : fProxy(nullptr)
        , fNormalize(false)
        , fReverseY(false)
        , fPrecision(kDefault_GrSLPrecision) {
        SkDEBUGCODE(fInProcessor = false);
    }

    /**
     * Create a transformation that maps [0, 1] to a texture's boundaries. The precision is inferred
     * from the texture size and filter. The texture origin also implies whether a y-reversal should
     * be performed.
     */
    GrCoordTransform(GrResourceProvider* resourceProvider, GrTextureProxy* proxy,
                     GrSamplerParams::FilterMode filter) {
        SkASSERT(proxy);
        SkDEBUGCODE(fInProcessor = false);
        this->reset(resourceProvider, SkMatrix::I(), proxy, filter);
    }

    /**
     * Create a transformation from a matrix. The precision is inferred from the texture size and
     * filter. The texture origin also implies whether a y-reversal should be performed.
     */
    GrCoordTransform(GrResourceProvider* resourceProvider, const SkMatrix& m,
                     GrTextureProxy* proxy, GrSamplerParams::FilterMode filter) {
        SkASSERT(proxy);
        SkDEBUGCODE(fInProcessor = false);
        this->reset(resourceProvider, m, proxy, filter);
    }

    /**
     * Create a transformation that applies the matrix to a coord set.
     */
    GrCoordTransform(const SkMatrix& m, GrSLPrecision precision = kDefault_GrSLPrecision) {
        SkDEBUGCODE(fInProcessor = false);
        this->reset(m, precision);
    }

    void reset(GrResourceProvider*, const SkMatrix&, GrTextureProxy*,
               GrSamplerParams::FilterMode filter, bool normalize = true);

    void reset(const SkMatrix& m, GrSLPrecision precision = kDefault_GrSLPrecision) {
        SkASSERT(!fInProcessor);
        fMatrix = m;
        fProxy = nullptr;
        fNormalize = false;
        fReverseY = false;
        fPrecision = precision;
    }

    GrCoordTransform& operator= (const GrCoordTransform& that) {
        SkASSERT(!fInProcessor);
        fMatrix = that.fMatrix;
        fProxy = that.fProxy;
        fNormalize = that.fNormalize;
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

    bool hasSameEffectAs(const GrCoordTransform& that) const {
        if (fNormalize != that.fNormalize ||
            fReverseY != that.fReverseY ||
            fPrecision != that.fPrecision ||
            !fMatrix.cheapEqualTo(that.fMatrix)) {
            return false;
        }

        if (fNormalize) {
//            SkASSERT(fTexture && that.fTexture);
//            return fTexture->width() == that.fTexture->width() &&
//                   fTexture->height() == that.fTexture->height();
        }

        return true;
    }

    const SkMatrix& getMatrix() const { return fMatrix; }
    const GrTextureProxy* proxy() const { return fProxy; }
    bool normalize() const { return fNormalize; }
    bool reverseY() const { return fReverseY; }
    GrSLPrecision precision() const { return fPrecision; }

private:
    // The textures' effect is to optionally normalize the final matrix, so a blind
    // equality check could be misleading
    bool operator==(const GrCoordTransform& that) const;
    bool operator!=(const GrCoordTransform& that) const;

    SkMatrix                fMatrix;
    const GrTextureProxy*   fProxy;
    bool                    fNormalize;
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
