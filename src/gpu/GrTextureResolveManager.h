/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureResolveManager_DEFINED
#define GrTextureResolveManager_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrTypes.h"

class GrCaps;
class GrDrawingManager;
class GrRenderTask;
class GrTextureProxy;

/*
 * This class is a shallow view of the drawing manager. It is passed to render tasks when setting up
 * the dependency DAG, and gives them limited access to functionality for making new tasks that
 * regenerate mipmaps and/or resolve MSAA.
 */
class GrTextureResolveManager {
public:
    explicit GrTextureResolveManager(GrDrawingManager* drawingManager)
            : fDrawingManager(drawingManager) {}

   enum class ResolveFlags : bool {
        kNone = 0,
        kMipMaps = 1 << 0,
        // TODO: kMSAA = 1 << 1
    };

    GrRenderTask* newTextureResolveRenderTask(
            sk_sp<GrTextureProxy>, ResolveFlags, const GrCaps&) const;

private:
    GrDrawingManager* fDrawingManager;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrTextureResolveManager::ResolveFlags);

#endif
