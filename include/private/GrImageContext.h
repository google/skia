/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrImageContext_DEFINED
#define GrImageContext_DEFINED

#include "include/private/GrContext_Base.h"
#include "include/private/GrSingleOwner.h"

class GrImageContextPriv;
class GrProxyProvider;

class SK_API GrImageContext : public GrContext_Base {
public:
    ~GrImageContext() override;

    // Provides access to functions that aren't part of the public API.
    GrImageContextPriv priv();
    const GrImageContextPriv priv() const;

protected:
    friend class GrImageContextPriv; // for hidden functions

    GrImageContext(GrBackendApi, const GrContextOptions&, uint32_t contextID);

    virtual void abandonContext();
    bool abandoned() const;

    GrProxyProvider* proxyProvider() { return fProxyProvider.get(); }
    const GrProxyProvider* proxyProvider() const { return fProxyProvider.get(); }

    /** This is only useful for debug purposes */
    GrSingleOwner* singleOwner() const { return &fSingleOwner; }

    GrImageContext* asImageContext() override { return this; }

private:
    std::unique_ptr<GrProxyProvider> fProxyProvider;
    bool                             fAbandoned = false;

    // In debug builds we guard against improper thread handling
    // This guard is passed to the GrDrawingManager and, from there to all the
    // GrRenderTargetContexts.  It is also passed to the GrResourceProvider and SkGpuDevice.
    mutable GrSingleOwner            fSingleOwner;

    typedef GrContext_Base INHERITED;
};

#endif
