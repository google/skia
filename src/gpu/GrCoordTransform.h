/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCoordTransform_DEFINED
#define GrCoordTransform_DEFINED

#include "SkMatrix.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTextureProxy.h"

class GrTexture;

/**
 * A class representing a linear transformation of local coordinates. GrFragnentProcessors
 * these transformations, and the GrGeometryProcessor implements the transformation.
 */
class GrCoordTransform {
public:
    GrCoordTransform()
            : fProxy(nullptr)
            , fNormalize(false)
            , fReverseY(false) {
        SkDEBUGCODE(fInProcessor = false);
    }

    GrCoordTransform(const GrCoordTransform&) = default;

    /**
     * Create a transformation that maps [0, 1] to a proxy's boundaries. The proxy origin also
     * implies whether a y-reversal should be performed.
     */
    GrCoordTransform(GrTextureProxy* proxy) {
        SkASSERT(proxy);
        SkDEBUGCODE(fInProcessor = false);
        this->reset(SkMatrix::I(), proxy);
    }

    /**
     * Create a transformation from a matrix. The proxy origin also implies whether a y-reversal
     * should be performed.
     */
    GrCoordTransform(const SkMatrix& m, GrTextureProxy* proxy) {
        SkASSERT(proxy);
        SkDEBUGCODE(fInProcessor = false);
        this->reset(m, proxy);
    }

    /**
     * Create a transformation that applies the matrix to a coord set.
     */
    GrCoordTransform(const SkMatrix& m) {
        SkDEBUGCODE(fInProcessor = false);
        this->reset(m);
    }

    GrCoordTransform& operator= (const GrCoordTransform& that) {
        SkASSERT(!fInProcessor);
        fMatrix = that.fMatrix;
        fProxy = that.fProxy;
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
            if (fProxy->underlyingUniqueID() != that.fProxy->underlyingUniqueID()) {
                return false;
            }
        }

        return true;
    }

    const SkMatrix& getMatrix() const { return fMatrix; }
    const GrTextureProxy* proxy() const { return fProxy; }
    bool normalize() const { return fNormalize; }
    bool reverseY() const { return fReverseY; }

    // This should only ever be called at flush time after the backing texture has been
    // successfully instantiated
    GrTexture* peekTexture() const { return fProxy->peekTexture(); }

private:
    void reset(const SkMatrix& m, GrTextureProxy* proxy = nullptr) {
        SkASSERT(!fInProcessor);

        fMatrix = m;
        fProxy = proxy;
        fNormalize = proxy && proxy->textureType() != GrTextureType::kRectangle;
        fReverseY = proxy && kBottomLeft_GrSurfaceOrigin == proxy->origin();
    }

    // The textures' effect is to optionally normalize the final matrix, so a blind
    // equality check could be misleading
    bool operator==(const GrCoordTransform& that) const;
    bool operator!=(const GrCoordTransform& that) const;

    SkMatrix                fMatrix;
    const GrTextureProxy*   fProxy;
    bool                    fNormalize;
    bool                    fReverseY;

#ifdef SK_DEBUG
public:
    void setInProcessor() const { fInProcessor = true; }
private:
    mutable bool fInProcessor;
#endif
};

#endif
