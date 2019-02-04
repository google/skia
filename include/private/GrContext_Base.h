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

class SK_API GrContext_Base : public SkRefCnt {
public:
    virtual ~GrContext_Base();

    /*
     * The 3D API backing this context
     */
    GrBackendApi backend() const { return fBackend; }

    // Provides access to functions that aren't part of the public API.
    GrBaseContextPriv priv();
    const GrBaseContextPriv priv() const;

protected:
    friend class GrBaseContextPriv; // for hidden functions

    GrContext_Base(GrBackendApi backend, const GrContextOptions& options, uint32_t uniqueID);

    /**
     * An identifier for this context. The id is used by all compatible contexts. For example,
     * if SkImages are created on one thread using an image creation context, then fed into a
     * DDL Recorder on second thread (which has a recording context) and finally replayed on
     * a third thread with a direct context, then all three contexts will report the same id.
     * It is an error for an image to be used with contexts that report different ids.
     */
    uint32_t contextID() const { return fContextID; }

<<<<<<< HEAD
    /*
     * The options in effect for this context
     */
    const GrContextOptions& options() const { return fOptions; }
=======
    // Provides access to functions that aren't part of the public API.
    GrBaseContextPriv priv();
    const GrBaseContextPriv priv() const;

    const GrContextOptions& options() const { return fOptions; }

    bool matches(GrContext_Base* context) const;

    virtual sk_sp<GrContextThreadSafeProxy> threadSafeProxy(); // { return fThreadSafeProxy; }

protected:
    friend class GrBaseContextPriv; // for hidden functions

    GrContext_Base(GrBackendApi backend, const GrContextOptions& options, uint32_t uniqueID);
>>>>>>> git squash commit for proxy-inval.

    GrContext_Base* asBaseContext() { return this; }
    virtual GrImageContext* asImageContext() { return nullptr; }
    virtual GrRecordingContext* asRecordingContext() { return nullptr; }
    virtual GrContext* asDirectContext() { return nullptr; }

    // must be called after the ctor!
    bool initWeakest(sk_sp<const GrCaps> caps,
                     sk_sp<GrContextThreadSafeProxy> threadSafeProxy,
                     sk_sp<GrSkSLFPFactoryCache> cache);

    const GrCaps* caps() const { return fCaps.get(); }
    sk_sp<const GrCaps> refCaps() const { return fCaps; }

    sk_sp<GrSkSLFPFactoryCache> getFPFactoryCache();// { return fFPFactoryCache; }

    bool disableGpuYUVConversion() const { return fOptions.fDisableGpuYUVConversion; }
    bool sharpenMipmappedTextures() const { return fOptions.fSharpenMipmappedTextures; }

private:
<<<<<<< HEAD
    const GrBackendApi     fBackend;
    const GrContextOptions fOptions;
    const uint32_t         fContextID;
=======
    friend class GrContextThreadSafeProxy; // for ctor

    const GrBackendApi              fBackend;
    const GrContextOptions          fOptions;
    const uint32_t                  fUniqueID;
    sk_sp<const GrCaps>             fCaps;
    sk_sp<GrContextThreadSafeProxy> fThreadSafeProxy;
    sk_sp<GrSkSLFPFactoryCache>     fFPFactoryCache;
>>>>>>> git squash commit for proxy-inval.

    typedef SkRefCnt INHERITED;
};

#endif
