/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGpuTessellationPathRenderer_DEFINED
#define GrGpuTessellationPathRenderer_DEFINED

#include "src/gpu/GrDynamicAtlas.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrPathRenderer.h"
#include <map>

// This is the tie-in point for path rendering via GrTessellatePathOp.
class GrGpuTessellationPathRenderer : public GrPathRenderer, public GrOnFlushCallbackObject {
public:
    GrGpuTessellationPathRenderer(const GrCaps&);
    StencilSupport onGetStencilSupport(const GrShape& shape) const override {
        // TODO: Single-pass (e.g., convex) paths can have full support.
        return kStencilOnly_StencilSupport;
    }
    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const override;
    bool onDrawPath(const DrawPathArgs&) override;
    void onStencilPath(const StencilPathArgs&) override;
    void preFlush(GrOnFlushResourceProvider*, const uint32_t* opsTaskIDs,
                  int numOpsTaskIDs) override;

private:
    SkPath* getAtlasUberPath(SkPathFillType fillType) {
        return &fAtlasUberPaths[(int)fillType & 1];
    }
    void resetAtlasUberPaths();
    bool tryAddPathToAtlas(const SkMatrix&, const SkPath&, SkIRect* devIBounds,
                           SkIVector* devToAtlasOffset);
    void renderAtlas(GrOnFlushResourceProvider*);

    GrDynamicAtlas fAtlas;
    SkPath fAtlasUberPaths[2];
};

#endif
