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

/** Class that exposes methods on GrImageContext that are only intended for use internal to Skia.
    This class is purely a privileged window into GrImageContext. It should never have
    additional data members or virtual methods. */
class GrImageContextPriv {
public:
    // from GrContext_Base
    GrBackendApi backend() const { return fContext->backend(); }

    const GrCaps* caps() const { return fContext->caps(); }
    sk_sp<const GrCaps> refCaps() const { return fContext->refCaps(); }

    sk_sp<GrSkSLFPFactoryCache> getFPFactoryCache() { fContext->getFPFactoryCache(); }

    bool disableGpuYUVConversion() const { return fContext->disableGpuYUVConversion(); }
    bool sharpenMipmappedTextures() const { return fContext->sharpenMipmappedTextures(); }

    // from GrImageContext
    bool abandoned1() const { return fContext->abandoned1(); }
    void abandon1() { return fContext->abandon1(); }

    GrProxyProvider* proxyProvider() { return fContext->proxyProvider(); }
    const GrProxyProvider* proxyProvider() const { return fContext->proxyProvider(); }

    GrSingleOwner* singleOwner() const { return fContext->singleOwner(); }

private:
    explicit GrImageContextPriv(GrImageContext* context) : fContext(context) {}
    GrImageContextPriv(const GrImageContextPriv&); // unimpl
    GrImageContextPriv& operator=(const GrImageContextPriv&); // unimpl

    // No taking addresses of this type.
    const GrImageContextPriv* operator&() const;
    GrImageContextPriv* operator&();

    GrImageContext* fContext;

    friend class GrImageContext; // to construct/copy this type.
};

inline GrImageContextPriv GrImageContext::priv() { return GrImageContextPriv(this); }

inline const GrImageContextPriv GrImageContext::priv () const {
    return GrImageContextPriv(const_cast<GrImageContext*>(this));
}

#endif
