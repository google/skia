/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSurfaceProxy_DEFINED
#define GrSurfaceProxy_DEFINED

#include "GrGpuResource.h"
#include "SkRect.h"

class GrTextureProxy;
class GrRenderTargetProxy;

class GrSurfaceProxy : public GrIORef<GrSurfaceProxy> {
public:
    const GrSurfaceDesc& desc() const { return fDesc; }

    GrSurfaceOrigin origin() const {
        SkASSERT(kTopLeft_GrSurfaceOrigin == fDesc.fOrigin ||
                 kBottomLeft_GrSurfaceOrigin == fDesc.fOrigin);
        return fDesc.fOrigin;
    }
    int width() const { return fDesc.fWidth; }
    int height() const { return fDesc.fWidth; }
    GrPixelConfig config() const { return fDesc.fConfig; }

    uint32_t uniqueID() const { return fUniqueID; }

    /**
     * Helper that gets the width and height of the surface as a bounding rectangle.
     */
    SkRect getBoundsRect() const { return SkRect::MakeIWH(this->width(), this->height()); }
  
    /**
     * @return the texture proxy associated with the surface proxy, may be NULL.
     */
    virtual GrTextureProxy* asTextureProxy() { return nullptr; }
    virtual const GrTextureProxy* asTextureProxy() const { return nullptr; }

    /**
     * @return the render target proxy associated with the surface proxy, may be NULL.
     */
    virtual GrRenderTargetProxy* asRenderTargetProxy() { return nullptr; }
    virtual const GrRenderTargetProxy* asRenderTargetProxy() const { return nullptr; }

    /**
     * Does the resource count against the resource budget?
     */
    SkBudgeted isBudgeted() const { return fBudgeted; }

protected:
    // Deferred version
    GrSurfaceProxy(const GrSurfaceDesc& desc, SkBackingFit fit, SkBudgeted budgeted)
        : fDesc(desc)
        , fFit(fit)
        , fBudgeted(budgeted)
        , fUniqueID(GrGpuResource::CreateUniqueID()) {
    }

    // Wrapped version
    GrSurfaceProxy(const GrSurfaceDesc& desc, SkBackingFit fit, 
                   SkBudgeted budgeted, uint32_t uniqueID)
        : fDesc(desc)
        , fFit(fit)
        , fBudgeted(budgeted)
        , fUniqueID(uniqueID) {
    }

    virtual ~GrSurfaceProxy() {}

    // For wrapped resources, 'fDesc' will always be filled in from the wrapped resource.
    const GrSurfaceDesc fDesc;
    const SkBackingFit  fFit;      // always exact for wrapped resources
    const SkBudgeted    fBudgeted; // set from the backing resource for wrapped resources
    const uint32_t      fUniqueID; // set from the backing resource for wrapped resources

private:

    // See comment in GrGpuResource.h.
    void notifyAllCntsAreZero(CntType) const { delete this; }
    bool notifyRefCountIsZero() const { return true; }

    typedef GrIORef<GrSurfaceProxy> INHERITED;

    // to access notifyAllCntsAreZero and notifyRefCntIsZero.
    friend class GrIORef<GrSurfaceProxy>;
};

#endif
