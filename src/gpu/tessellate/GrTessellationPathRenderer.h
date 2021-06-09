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

// This is the tie-in point for path rendering via GrPathTessellateOp. This path renderer draws
// paths using a hybrid Red Book "stencil, then cover" method. Curves get linearized by GPU
// tessellation shaders. This path renderer doesn't apply analytic AA, so it requires MSAA if AA is
// desired.
class GrTessellationPathRenderer : public GrPathRenderer, public GrOnFlushCallbackObject {
public:
    // Don't allow linearized segments to be off by more than 1/4th of a pixel from the true curve.
    constexpr static float kLinearizationPrecision = 4;

    // We send these flags to the internal path filling Ops to control how a path gets rendered.
    enum class PathFlags {
        kNone = 0,
        kStencilOnly = (1 << 0),
        kWireframe = (1 << 1)
    };

    static bool IsSupported(const GrCaps&);

    GrTessellationPathRenderer(GrRecordingContext*);
    const char* name() const final { return "GrTessellationPathRenderer"; }
    StencilSupport onGetStencilSupport(const GrStyledShape& shape) const override {
        // TODO: Single-pass (e.g., convex) paths can have full support.
        return kStencilOnly_StencilSupport;
    }
    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const override;
    bool onDrawPath(const DrawPathArgs&) override;
    void onStencilPath(const StencilPathArgs&) override;
    void preFlush(GrOnFlushResourceProvider*, SkSpan<const uint32_t> taskIDs) override;

private:
    SkPath* getAtlasUberPath(SkPathFillType fillType, bool antialias) {
        int idx = (int)antialias << 1;
        idx |= (int)fillType & 1;
        return &fAtlasUberPaths[idx];
    }
    // Allocates space in fAtlas if the path is small and simple enough, and if there is room.
    bool tryAddPathToAtlas(const GrCaps&, const SkMatrix&, const GrStyledShape&,
                           const SkRect& devBounds, GrAAType, SkIRect* devIBounds,
                           SkIPoint16* locationInAtlas, bool* transposedInAtlas);
    void renderAtlas(GrOnFlushResourceProvider*);

    GrDynamicAtlas fAtlas;
    int fMaxAtlasPathWidth = 0;
    SkPath fAtlasUberPaths[4];  // 2 fillTypes * 2 antialias modes.
};

GR_MAKE_BITFIELD_CLASS_OPS(GrTessellationPathRenderer::PathFlags);

#endif
