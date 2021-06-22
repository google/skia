/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGrAtlasRenderTask_DEFINED
#define GrGrAtlasRenderTask_DEFINED

#include "include/core/SkPath.h"
#include "src/gpu/GrOpsTask.h"
#include "src/gpu/GrDynamicAtlas.h"

struct SkIPoint16;

// Represents a GrRenderTask that renders an atlas of path masks. This task gets inserted into the
// DAG and closed upfront, lays out its atlas as dependent tasks get built and reference it, but
// does not create its ops to render itself until flush time.
class GrAtlasRenderTask : public GrOpsTask {
public:
    GrAtlasRenderTask(GrRecordingContext*, GrAuditTrail*, sk_sp<GrArenas>,
                      std::unique_ptr<GrDynamicAtlas>);

    const GrDynamicAtlas* dynamicAtlas() const { return fDynamicAtlas.get(); }
    const GrTextureProxy* atlasProxy() const { return fDynamicAtlas->textureProxy(); }
    GrSurfaceProxyView readView(const GrCaps& caps) const { return fDynamicAtlas->readView(caps); }

    // Allocates a rectangle for, and renders the given path in the atlas. Returns false if there
    // was not room in the atlas. On success, writes out the location of the path mask's upper-left
    // corner to 'locationInAtlas'.
    bool addPath(const SkMatrix&, const SkPath&, bool antialias, SkIPoint pathDevTopLeft,
                 int widthInAtlas, int heightInAtlas, bool transposedInAtlas,
                 SkIPoint16* locationInAtlas);

    // Returns whether two atlas tasks can render to the same underlying GrTexture. This will be the
    // case if they are the same size and if they do not have shared direct dependents. (The DAG
    // should otherwise guarantee that render tasks never use more than 1 atlas in cases other than
    // shared direct dependents.)
    bool canShareAtlasTexture(GrAtlasRenderTask*);

    // Instantiates the atlas's lazy texture proxy and creates internal ops to render the atlas.
    // The texture proxy is instantiated with 'backingTexture', if provided. See GrDynamicAtlas.
    void instantiate(GrOnFlushResourceProvider*, sk_sp<GrTexture> backingTexture = nullptr);

private:
    void stencilAtlasRect(GrRecordingContext*, const SkRect&, const SkPMColor4f&,
                          const GrUserStencilSettings*);
    void addAtlasDrawOp(GrOp::Owner, bool usesMSAA, const GrCaps&);

    bool onExecute(GrOpFlushState* flushState) override;

    SkPath* getUberPath(SkPathFillType fillType, bool antialias) {
        int idx = (int)antialias << 1;
        idx |= (int)fillType & 1;
        return &fUberPaths[idx];
    }

    const std::unique_ptr<GrDynamicAtlas> fDynamicAtlas;
    SkPath fUberPaths[4];  // 2 fillTypes * 2 antialias modes.
};

#endif
