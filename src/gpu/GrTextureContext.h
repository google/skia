/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureContext_DEFINED
#define GrTextureContext_DEFINED

#include "src/gpu/GrSurfaceContext.h"
#include "src/gpu/GrTextureProxy.h"

class GrContext;
class GrDrawingManager;
class GrSurface;
class GrTextureProxy;
struct SkIPoint;
struct SkIRect;

/**
 * A helper object to orchestrate commands (currently just copies) for GrSurfaces that are
 * GrTextures and not GrRenderTargets.
 */
class GrTextureContext : public GrSurfaceContext {
public:
    ~GrTextureContext() override;

    GrSurfaceProxy* asSurfaceProxy() override { return fTextureProxy.get(); }
    const GrSurfaceProxy* asSurfaceProxy() const override { return fTextureProxy.get(); }
    sk_sp<GrSurfaceProxy> asSurfaceProxyRef() override { return fTextureProxy; }

    GrTextureProxy* asTextureProxy() override { return fTextureProxy.get(); }
    const GrTextureProxy* asTextureProxy() const override { return fTextureProxy.get(); }
    sk_sp<GrTextureProxy> asTextureProxyRef() override { return fTextureProxy; }

    GrRenderTargetProxy* asRenderTargetProxy() override;
    sk_sp<GrRenderTargetProxy> asRenderTargetProxyRef() override;

protected:
    GrTextureContext(GrRecordingContext*,
                     sk_sp<GrTextureProxy>,
                     GrColorType,
                     SkAlphaType,
                     sk_sp<SkColorSpace>);

    SkDEBUGCODE(void validate() const override;)

private:
    friend class GrDrawingManager; // for ctor

    sk_sp<GrTextureProxy>  fTextureProxy;

    typedef GrSurfaceContext INHERITED;
};

#endif
