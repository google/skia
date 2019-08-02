/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureResolveManager_DEFINED
#define GrTextureResolveManager_DEFINED

#include "include/gpu/GrTypes.h"
#include "include/core/SkRefCnt.h"

class GrCaps;
class GrDrawingManager;
class GrRenderTask;
class GrTextureProxy;

class GrTextureResolveManager {
public:
    explicit GrTextureResolveManager(GrDrawingManager* drawingManager)
            : fDrawingManager(drawingManager) {}

   enum class ResolveFlags : bool {
        kNone = 0,
        kMipMaps = 1
    };

    GrRenderTask* newTextureResolveRenderTask(
            sk_sp<GrTextureProxy>, ResolveFlags, const GrCaps&) const;

private:
    GrDrawingManager* fDrawingManager;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrTextureResolveManager::ResolveFlags);

#endif
