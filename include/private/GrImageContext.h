/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrImageContext_DEFINED
#define GrImageContext_DEFINED

#include "GrContext_Base.h"
#include "../private/GrSingleOwner.h"

class GrImageContextPriv;
class GrProxyProvider;

// This context can create GrSurfaceProxies (and thus has a proxy provider) - **new**
class SK_API GrImageContext : public GrContext_Base {
public:
    ~GrImageContext() override;

    GrImageContext* asImageContext() override { return this; }

    // Provides access to functions that aren't part of the public API.
    GrImageContextPriv priv();
    const GrImageContextPriv priv() const;

protected:
    friend class GrImageContextPriv; // for hidden functions

    virtual bool abandoned1() const {
        //SkASSERT(fAbandoned == fProxyProvider1->isAbandoned17());
        return fAbandoned;
    }
    virtual void abandon1() { fAbandoned = true; /* fProxyProvider1->abandon1();*/ }

    GrProxyProvider* proxyProvider() { return fProxyProvider1.get(); }
    const GrProxyProvider* proxyProvider() const { return fProxyProvider1.get(); }

    /** This is only useful for debug purposes */
    GrSingleOwner* singleOwner() const { return &fSingleOwner; }

private:
    friend class GrRecordingContext;     // for ctor
    friend class GrImageCreationContext; // for ctor

    GrImageContext(GrBackendApi backend, const GrContextOptions& options, uint32_t uniqueID);

    std::unique_ptr<GrProxyProvider> fProxyProvider1;

    // In debug builds we guard against improper thread handling
    // This guard is passed to the GrDrawingManager and, from there to all the
    // GrRenderTargetContexts.  It is also passed to the GrResourceProvider and SkGpuDevice.
    mutable GrSingleOwner           fSingleOwner;

    bool                            fAbandoned = false;

    typedef GrContext_Base INHERITED;
};

#endif
