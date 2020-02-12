/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCoordTransform_DEFINED
#define GrCoordTransform_DEFINED

#include "include/core/SkMatrix.h"
#include "src/core/SkMatrixPriv.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTextureProxy.h"

class GrTexture;

/**
 * A class representing a linear transformation of local coordinates. GrFragnentProcessors
 * these transformations, and the GrGeometryProcessor implements the transformation.
 */
class GrCoordTransform {
public:
    GrCoordTransform() = default;

    GrCoordTransform(const GrCoordTransform&) = default;

    /**
     * Create a transformation that maps [0, proxy->width()] x [0, proxy->height()] to a proxy's
     * extent.
     */
    GrCoordTransform(GrSurfaceProxy* proxy, GrSurfaceOrigin origin)
            : fProxy(proxy), fOrigin(origin) {}

    /**
     * Create a transformation from a matrix. The origin implies whether a y-reversal should be
     * performed.
     */
    GrCoordTransform(const SkMatrix& m, GrSurfaceProxy* proxy, GrSurfaceOrigin origin)
            : fProxy(proxy), fOrigin(origin), fMatrix(m) {
        SkASSERT(proxy);
    }

    /**
     * Create a transformation that applies the matrix to a coord set.
     */
    GrCoordTransform(const SkMatrix& m) : fMatrix(m) {}

    GrCoordTransform& operator=(const GrCoordTransform& that) = default;

    // The textures' effect is to optionally normalize the final matrix, so a blind equality check
    // could be misleading.
    bool operator==(const GrCoordTransform& that) const = delete;
    bool operator!=(const GrCoordTransform& that) const = delete;

    bool hasSameEffectiveMatrix(const GrCoordTransform& that) const {
        // This is slightly more conservative than computing each transforms effective matrix and
        // then comparing them.
        if (!SkMatrixPriv::CheapEqual(fMatrix, that.fMatrix)) {
            return false;
        }
        if (SkToBool(fProxy) != SkToBool(that.fProxy)) {
            return false;
        }
        if (this->normalize() != that.normalize() || this->reverseY() != that.reverseY()) {
            return false;
        }
        if (this->normalize() &&
            fProxy->backingStoreDimensions() != that.fProxy->backingStoreDimensions()) {
            return false;
        }
        return true;
    }

    const SkMatrix& matrix() const { return fMatrix; }
    const GrSurfaceProxy* proxy() const { return fProxy; }
    bool normalize() const {
        return fProxy && fProxy->backendFormat().textureType() != GrTextureType::kRectangle;
    }
    bool reverseY() const { return fProxy && fOrigin == kBottomLeft_GrSurfaceOrigin; }
    bool isNoOp() const { return fMatrix.isIdentity() && !this->normalize() && !this->reverseY(); }

    // This should only ever be called at flush time after the backing texture has been
    // successfully instantiated
    GrTexture* peekTexture() const { return fProxy->peekTexture(); }

private:
    const GrSurfaceProxy* fProxy = nullptr;
    GrSurfaceOrigin fOrigin = kTopLeft_GrSurfaceOrigin;
    SkMatrix fMatrix = SkMatrix::I();
};

#endif
