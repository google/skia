/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContext_Base_DEFINED
#define GrContext_Base_DEFINED

#include "SkRefCnt.h"
#include "GrContextOptions.h"
#include "GrTypes.h"

class GrBaseContextPriv;
class GrCaps;
class GrContext;
class GrContextThreadSafeProxy;
class GrImageContext;
class GrRecordingContext;
class GrSkSLFPFactoryCache;

// This context only has the caps and the unique ID - thread-safe context
class SK_API GrContext_Base : public SkRefCnt {
public:
    virtual ~GrContext_Base();

    // Provides access to functions that aren't part of the public API.
    GrBaseContextPriv priv();
    const GrBaseContextPriv priv() const;

    const GrContextOptions& options() const { return fOptions; }

    /**
     * An ID associated with this context, guaranteed to be unique.
     */
    uint32_t uniqueID() const { return fUniqueID; }

    bool matches(GrContext_Base* context) const;

    virtual sk_sp<GrContextThreadSafeProxy> threadSafeProxy() { return fThreadSafeProxy; }

    GrContext_Base* weakest() { return this; }
    virtual GrImageContext* asImageContext() { return nullptr; }
    virtual GrRecordingContext* asRecordingContext() { return nullptr; }
    virtual GrContext* asDirectContext() { return nullptr; }

protected:
    friend class GrBaseContextPriv; // for hidden functions

    // must be called after the ctor!
    bool initWeakest(sk_sp<const GrCaps> caps,
                     sk_sp<GrContextThreadSafeProxy> threadSafeProxy,
                     sk_sp<GrSkSLFPFactoryCache> cache);

    /*
     * The 3D API backing this context
     */
    GrBackendApi backend() const { return fBackend; }

    const GrCaps* caps() const { return fCaps.get(); }
    sk_sp<const GrCaps> refCaps() const { return fCaps; }

    sk_sp<GrSkSLFPFactoryCache> getFPFactoryCache();// { return fFPFactoryCache; }

    bool disableGpuYUVConversion() const { return fOptions.fDisableGpuYUVConversion; }
    bool sharpenMipmappedTextures() const { return fOptions.fSharpenMipmappedTextures; }

private:
    friend class GrImageContext;           // for ctor
    friend class GrContextThreadSafeProxy; // for ctor

    GrContext_Base(GrBackendApi backend, const GrContextOptions& options, uint32_t uniqueID);

    const GrBackendApi              fBackend;
    const GrContextOptions          fOptions;
    const uint32_t                  fUniqueID;
    sk_sp<const GrCaps>             fCaps;
    sk_sp<GrContextThreadSafeProxy> fThreadSafeProxy;
    sk_sp<GrSkSLFPFactoryCache>     fFPFactoryCache;

    typedef SkRefCnt INHERITED;
};

#endif
