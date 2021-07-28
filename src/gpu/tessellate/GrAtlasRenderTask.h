/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGrAtlasRenderTask_DEFINED
#define GrGrAtlasRenderTask_DEFINED

#include "include/core/SkPath.h"
#include "src/gpu/GrDynamicAtlas.h"
#include "src/gpu/GrOpsTask.h"
#include "src/gpu/GrTBlockList.h"
#include "src/gpu/tessellate/GrPathTessellator.h"

struct SkIPoint16;

// Represents a GrRenderTask that draws paths into an atlas. This task gets added the DAG and left
// open, lays out its atlas while future tasks call addPath(), and finally adds its internal draw
// ops during onMakeClosed().
//
// The atlas texture does not get instantiated automatically. It is the creator's responsibility to
// call instantiate() at flush time.
class GrAtlasRenderTask : public GrOpsTask {
public:
    GrAtlasRenderTask(GrRecordingContext*,
                      sk_sp<GrArenas>,
                      std::unique_ptr<GrDynamicAtlas>);

    const GrTextureProxy* atlasProxy() const { return fDynamicAtlas->textureProxy(); }
    GrSurfaceProxyView readView(const GrCaps& caps) const { return fDynamicAtlas->readView(caps); }

    // Allocates a rectangle for, and stages the given path to be rendered into the atlas. Returns
    // false if there was not room in the atlas. On success, writes out the location of the path's
    // upper-left corner to 'locationInAtlas'.
    bool addPath(const SkMatrix&, const SkPath&, SkIPoint pathDevTopLeft, int widthInAtlas,
                 int heightInAtlas, bool transposedInAtlas, SkIPoint16* locationInAtlas);

    // Must be called at flush time. The texture proxy is instantiated with 'backingTexture', if
    // provided. See GrDynamicAtlas.
    void instantiate(GrOnFlushResourceProvider* onFlushRP,
                     sk_sp<GrTexture> backingTexture = nullptr) {
        SkASSERT(this->isClosed());
        fDynamicAtlas->instantiate(onFlushRP, std::move(backingTexture));
    }

private:
    // Adds internal ops to render the atlas before deferring to GrOpsTask::onMakeClosed.
    ExpectedOutcome onMakeClosed(GrRecordingContext*, SkIRect* targetUpdateBounds) override;

    void stencilAtlasRect(GrRecordingContext*, const SkRect&, const SkPMColor4f&,
                          const GrUserStencilSettings*);
    void addAtlasDrawOp(GrOp::Owner, const GrCaps&);

    // Executes the GrOpsTask and resolves msaa if needed.
    bool onExecute(GrOpFlushState* flushState) override;

    const std::unique_ptr<GrDynamicAtlas> fDynamicAtlas;

    // Allocate enough inline entries for 16 atlas path draws, then spill to the heap.
    using PathDrawAllocator = GrTBlockList<GrPathTessellator::PathDrawList, 16>;
    PathDrawAllocator fPathDrawAllocator{64, GrBlockAllocator::GrowthPolicy::kFibonacci};

    class AtlasPathList : SkNoncopyable {
    public:
        void add(PathDrawAllocator* alloc, const SkMatrix& pathMatrix, const SkPath& path) {
            fPathDrawList = &alloc->emplace_back(pathMatrix, path, fPathDrawList);
            if (path.isInverseFillType()) {
                // The atlas never has inverse paths. The inversion happens later.
                fPathDrawList->fPath.toggleInverseFillType();
            }
            fTotalCombinedPathVerbCnt += path.countVerbs();
            ++fPathCount;
        }
        const GrPathTessellator::PathDrawList* pathDrawList() const { return fPathDrawList; }
        int totalCombinedPathVerbCnt() const { return fTotalCombinedPathVerbCnt; }
        int pathCount() const { return fPathCount; }

    private:
        GrPathTessellator::PathDrawList* fPathDrawList = nullptr;
        int fTotalCombinedPathVerbCnt = 0;
        int fPathCount = 0;
    };

    AtlasPathList fWindingPathList;
    AtlasPathList fEvenOddPathList;
};

#endif
