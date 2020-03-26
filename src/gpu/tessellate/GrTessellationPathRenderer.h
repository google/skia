/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTessellationPathRenderer_DEFINED
#define GrTessellationPathRenderer_DEFINED

#include "src/gpu/GrDynamicAtlas.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrPathRenderer.h"
#include <map>

// This is the tie-in point for path rendering via GrTessellatePathOp. This path renderer draws
// paths using a hybrid Red Book "stencil, then cover" method. Curves get linearized by GPU
// tessellation shaders. This path renderer doesn't apply analytic AA, so it requires a render
// target that supports either MSAA or mixed samples if AA is desired.
class GrTessellationPathRenderer : public GrPathRenderer, public GrOnFlushCallbackObject {
public:
    GrTessellationPathRenderer(const GrCaps&);
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
    SkPath* getAtlasUberPath(SkPathFillType fillType, bool antialias) {
        int idx = (int)antialias << 1;
        idx |= (int)fillType & 1;
        return &fAtlasUberPaths[idx];
    }
    // Allocates space in fAtlas if the path is small and simple enough, and if there is room.
    bool tryAddPathToAtlas(const GrCaps&, const SkMatrix&, const SkPath&, GrAAType,
                           SkIRect* devIBounds, SkIVector* devToAtlasOffset);
    void renderAtlas(GrOnFlushResourceProvider*);

    GrDynamicAtlas fAtlas;
    SkPath fAtlasUberPaths[4];  // 2 fillTypes * 2 antialias modes.
};

#endif
