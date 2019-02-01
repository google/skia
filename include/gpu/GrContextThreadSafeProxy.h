/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContextThreadSafeProxy_DEFINED
#define GrContextThreadSafeProxy_DEFINED

#include "GrContextOptions.h"
#include "SkRefCnt.h"

class GrBackendFormat;
class GrCaps;
class GrContext;
class GrContext_Base;
class GrContextThreadSafeProxyPriv;
class GrSkSLFPFactoryCache;
struct SkImageInfo;
class SkSurfaceCharacterization;

/**
 * Can be used to perform actions related to the generating GrContext in a thread safe manner. The
 * proxy does not access the 3D API (e.g. OpenGL) that backs the generating GrContext.
 */
class SK_API GrContextThreadSafeProxy : public SkRefCnt {
public:
    ~GrContextThreadSafeProxy() override;

    bool matches(GrContext_Base* context) const;

    /**
     *  Create a surface characterization for a DDL that will be replayed into the GrContext
     *  that created this proxy. On failure the resulting characterization will be invalid (i.e.,
     *  "!c.isValid()").
     *
     *  @param cacheMaxResourceBytes The max resource bytes limit that will be in effect when the
     *                               DDL created with this characterization is replayed.
     *                               Note: the contract here is that the DDL will be created as
     *                               if it had a full 'cacheMaxResourceBytes' to use. If replayed
     *                               into a GrContext that already has locked GPU memory, the
     *                               replay can exceed the budget. To rephrase, all resource
     *                               allocation decisions are made at record time and at playback
     *                               time the budget limits will be ignored.
     *  @param ii                    The image info specifying properties of the SkSurface that
     *                               the DDL created with this characterization will be replayed
     *                               into.
     *                               Note: Ganesh doesn't make use of the SkImageInfo's alphaType
     *  @param backendFormat         Information about the format of the GPU surface that will
     *                               back the SkSurface upon replay
     *  @param sampleCount           The sample count of the SkSurface that the DDL created with
     *                               this characterization will be replayed into
     *  @param origin                The origin of the SkSurface that the DDL created with this
     *                               characterization will be replayed into
     *  @param surfaceProps          The surface properties of the SkSurface that the DDL created
     *                               with this characterization will be replayed into
     *  @param isMipMapped           Will the surface the DDL will be replayed into have space
     *                               allocated for mipmaps?
     *  @param willUseGLFBO0         Will the surface the DDL will be replayed into be backed by GL
     *                               FBO 0. This flag is only valid if using an GL backend.
     *  @param isTextureable         Will the surface be able to act as a texture?
     */
    SkSurfaceCharacterization createCharacterization(
                                  size_t cacheMaxResourceBytes,
                                  const SkImageInfo& ii, const GrBackendFormat& backendFormat,
                                  int sampleCount, GrSurfaceOrigin origin,
                                  const SkSurfaceProps& surfaceProps,
                                  bool isMipMapped,
                                  bool willUseGLFBO0 = false,
                                  bool isTextureable = true);

    bool operator==(const GrContextThreadSafeProxy& that) const {
        // Each GrContext should only ever have a single thread-safe proxy.
        SkASSERT((this == &that) == (fContextID == that.fContextID));
        return this == &that;
    }

    bool operator!=(const GrContextThreadSafeProxy& that) const { return !(*this == that); }

    // Provides access to functions that aren't part of the public API.
    GrContextThreadSafeProxyPriv priv();
    const GrContextThreadSafeProxyPriv priv() const;

private:
    // DDL TODO: need to add unit tests for backend & maybe options
    GrContextThreadSafeProxy(sk_sp<const GrCaps> caps,
                             uint32_t uniqueID,
                             GrBackendApi backend,
                             const GrContextOptions& options,
                             sk_sp<GrSkSLFPFactoryCache> cache);

    sk_sp<const GrCaps>         fCaps;
    const uint32_t              fContextID;
    const GrBackendApi          fBackend;
    const GrContextOptions      fOptions;
    sk_sp<GrSkSLFPFactoryCache> fFPFactoryCache;

    friend class GrDirectContext; // To construct this object
    friend class GrContextThreadSafeProxyPriv;

    typedef SkRefCnt INHERITED;
};

#endif
