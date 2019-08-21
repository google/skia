/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureContext_DEFINED
#define GrTextureContext_DEFINED

#include "src/gpu/GrSurfaceContext.h"

class GrContext;
class GrDrawingManager;
class GrSurface;
class GrTextureOpList;
class GrTextureProxy;
struct SkIPoint;
struct SkIRect;

/**
 * A helper object to orchestrate commands (currently just copies) for GrSurfaces that are
 * GrTextures and not GrRenderTargets.
 */
class GrTextureContext : public GrSurfaceContext {
public:
    SK_API ~GrTextureContext() override;

    SK_API GrSurfaceProxy* asSurfaceProxy() override { return fTextureProxy.get(); }
    SK_API const GrSurfaceProxy* asSurfaceProxy() const override { return fTextureProxy.get(); }
    SK_API sk_sp<GrSurfaceProxy> asSurfaceProxyRef() override { return fTextureProxy; }

    SK_API GrTextureProxy* asTextureProxy() override { return fTextureProxy.get(); }
    SK_API const GrTextureProxy* asTextureProxy() const override { return fTextureProxy.get(); }
    SK_API sk_sp<GrTextureProxy> asTextureProxyRef() override { return fTextureProxy; }

    SK_API GrRenderTargetProxy* asRenderTargetProxy() override;
    SK_API sk_sp<GrRenderTargetProxy> asRenderTargetProxyRef() override;

protected:
    SK_API GrTextureContext(GrRecordingContext*,
                            sk_sp<GrTextureProxy>,
                            GrColorType,
                            SkAlphaType,
                            sk_sp<SkColorSpace>);

    SkDEBUGCODE(SK_API void validate() const override;)

private:
    friend class GrDrawingManager; // for ctor

    GrOpList* getOpList() override;

    sk_sp<GrTextureProxy>  fTextureProxy;

    // In MDB-mode the GrOpList can be closed by some other renderTargetContext that has picked
    // it up. For this reason, the GrOpList should only ever be accessed via 'getOpList'.
    sk_sp<GrTextureOpList> fOpList;

    typedef GrSurfaceContext INHERITED;
};

#endif
