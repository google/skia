/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCoordTransform_DEFINED
#define GrCoordTransform_DEFINED

#include "SkMatrix.h"
#include "GrTexture.h"

class GrResourceProvider;
class GrTextureProxy;

/**
 * A class representing a linear transformation of local coordinates. GrFragnentProcessors
 * these transformations, and the GrGeometryProcessor implements the transformation.
 */
class GrCoordTransform : SkNoncopyable {
public:
    GrCoordTransform()
        : fTexture(nullptr)
        , fNormalize(false)
        , fReverseY(false) {
        SkDEBUGCODE(fInProcessor = false);
    }

    /**
     * Create a transformation that maps [0, 1] to a proxy's boundaries. The proxy origin also
     * implies whether a y-reversal should be performed.
     */
    GrCoordTransform(GrResourceProvider* resourceProvider, GrTextureProxy* proxy) {
        SkASSERT(proxy);
        SkDEBUGCODE(fInProcessor = false);
        this->reset(resourceProvider, SkMatrix::I(), proxy);
    }

    /**
     * Create a transformation from a matrix. The proxy origin also implies whether a y-reversal
     * should be performed.
     */
    GrCoordTransform(GrResourceProvider* resourceProvider, const SkMatrix& m,
                     GrTextureProxy* proxy) {
        SkASSERT(proxy);
        SkDEBUGCODE(fInProcessor = false);
        this->reset(resourceProvider, m, proxy);
    }

    /**
     * Create a transformation that applies the matrix to a coord set.
     */
    GrCoordTransform(const SkMatrix& m) {
        SkDEBUGCODE(fInProcessor = false);
        this->reset(m);
    }

    void reset(GrResourceProvider*, const SkMatrix&, GrTextureProxy*, bool normalize = true);

    void reset(const SkMatrix& m) {
        SkASSERT(!fInProcessor);
        fMatrix = m;
        fTexture = nullptr;
        fNormalize = false;
        fReverseY = false;
    }

    GrCoordTransform& operator= (const GrCoordTransform& that) {
        SkASSERT(!fInProcessor);
        fMatrix = that.fMatrix;
        fTexture = that.fTexture;
        fNormalize = that.fNormalize;
        fReverseY = that.fReverseY;
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
            !fMatrix.cheapEqualTo(that.fMatrix)) {
            return false;
        }

        if (fNormalize) {
            SkASSERT(fTexture && that.fTexture);
            return fTexture->width() == that.fTexture->width() &&
                   fTexture->height() == that.fTexture->height();
        }

        return true;
    }

    const SkMatrix& getMatrix() const { return fMatrix; }
    const GrTexture* texture() const { return fTexture; }
    bool normalize() const { return fNormalize; }
    bool reverseY() const { return fReverseY; }

private:
    // The textures' effect is to optionally normalize the final matrix, so a blind
    // equality check could be misleading
    bool operator==(const GrCoordTransform& that) const;
    bool operator!=(const GrCoordTransform& that) const;

    SkMatrix                fMatrix;
    const GrTexture*        fTexture;
    bool                    fNormalize;
    bool                    fReverseY;
    typedef SkNoncopyable INHERITED;

#ifdef SK_DEBUG
public:
    void setInProcessor() const { fInProcessor = true; }
private:
    mutable bool fInProcessor;
#endif
};

#endif
