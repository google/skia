/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSurfaceProxy_DEFINED
#define GrSurfaceProxy_DEFINED

#include "GrGpuResource.h"

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
     * @return the texture proxy associated with the surface proxy, may be NULL.
     */
    virtual GrTextureProxy* asTextureProxy() { return nullptr; }
    virtual const GrTextureProxy* asTextureProxy() const { return nullptr; }

    /**
     * @return the render target proxy associated with the surface proxy, may be NULL.
     */
    virtual GrRenderTargetProxy* asRenderTargetProxy() { return nullptr; }
    virtual const GrRenderTargetProxy* asRenderTargetProxy() const { return nullptr; }

protected:
    GrSurfaceProxy(const GrSurfaceDesc& desc, SkBackingFit fit, SkBudgeted budgeted)
        : fDesc(desc)
        , fFit(fit)
        , fBudgeted(budgeted)
        , fUniqueID(CreateUniqueID()) {
    }

    virtual ~GrSurfaceProxy() {}

    // For wrapped resources, 'fDesc' will always be filled in from the wrapped resource.
    const GrSurfaceDesc fDesc;
    const SkBackingFit  fFit;      // always exact for wrapped resources
    const SkBudgeted    fBudgeted; // set from the backing resource for wrapped resources
    const uint32_t      fUniqueID;

private:
    static uint32_t CreateUniqueID();

    // See comment in GrGpuResource.h.
    void notifyAllCntsAreZero(CntType) const { delete this; }
    bool notifyRefCountIsZero() const { return true; }

    typedef GrIORef<GrSurfaceProxy> INHERITED;

    // to access notifyAllCntsAreZero and notifyRefCntIsZero.
    friend class GrIORef<GrSurfaceProxy>;
};

#endif
