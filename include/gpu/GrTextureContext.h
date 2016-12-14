/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureContext_DEFINED
#define GrTextureContext_DEFINED

#include "GrSurfaceContext.h"
#include "../private/GrTextureProxy.h"

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
class SK_API GrTextureContext : public GrSurfaceContext {
public:
    ~GrTextureContext() override;

    bool copySurface(GrSurface* src, const SkIRect& srcRect, const SkIPoint& dstPoint) override;

protected:
    GrTextureContext(GrContext*, GrDrawingManager*, sk_sp<GrTextureProxy>, GrAuditTrail*,
                     GrSingleOwner*);

    GrDrawingManager* drawingManager() { return fDrawingManager; }

    SkDEBUGCODE(void validate() const;)

private:
    friend class GrDrawingManager; // for ctor

    GrTextureOpList* getOpList();

    GrDrawingManager*            fDrawingManager;
    sk_sp<GrTextureProxy>        fTextureProxy;

    // In MDB-mode the GrOpList can be closed by some other renderTargetContext that has picked
    // it up. For this reason, the GrOpList should only ever be accessed via 'getOpList'.
    GrTextureOpList*             fOpList;
};

#endif
