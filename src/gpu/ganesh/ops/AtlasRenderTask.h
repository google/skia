/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef AtlasRenderTask_DEFINED
#define AtlasRenderTask_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkNoncopyable.h"
#include "src/base/SkBlockAllocator.h"
#include "src/base/SkTBlockList.h"
#include "src/core/SkColorData.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDynamicAtlas.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/ops/GrOp.h"
#include "src/gpu/ganesh/ops/OpsTask.h"
#include "src/gpu/ganesh/tessellate/PathTessellator.h"

#include <memory>
#include <utility>

class GrArenas;
class GrOnFlushResourceProvider;
class GrOpFlushState;
class GrRecordingContext;
class GrTextureProxy;
class SkMatrix;
struct GrUserStencilSettings;
struct SkIPoint16;
struct SkIPoint;
struct SkIRect;
struct SkRect;

namespace skgpu::ganesh {

// Represents a GrRenderTask that draws paths into an atlas. This task gets added the DAG and left
// open, lays out its atlas while future tasks call addPath(), and finally adds its internal draw
// ops during onMakeClosed().
//
// The atlas texture does not get instantiated automatically. It is the creator's responsibility to
// call instantiate() at flush time.
class AtlasRenderTask final : public OpsTask {
public:
    AtlasRenderTask(GrRecordingContext*,
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
    [[nodiscard]] bool instantiate(GrOnFlushResourceProvider* onFlushRP,
                                   sk_sp<GrTexture> backingTexture = nullptr) {
        SkASSERT(this->isClosed());
        return fDynamicAtlas->instantiate(onFlushRP, std::move(backingTexture));
    }

private:
    // Adds internal ops to render the atlas before deferring to OpsTask::onMakeClosed.
    ExpectedOutcome onMakeClosed(GrRecordingContext*, SkIRect* targetUpdateBounds) override;

    void stencilAtlasRect(GrRecordingContext*, const SkRect&, const SkPMColor4f&,
                          const GrUserStencilSettings*);
    void addAtlasDrawOp(GrOp::Owner, const GrCaps&);

    // Executes the OpsTask and resolves msaa if needed.
    bool onExecute(GrOpFlushState* flushState) override;

    const std::unique_ptr<GrDynamicAtlas> fDynamicAtlas;

    // Allocate enough inline entries for 16 atlas path draws, then spill to the heap.
    using PathDrawList = PathTessellator::PathDrawList;
    using PathDrawAllocator = SkTBlockList<PathDrawList, 16>;
    PathDrawAllocator fPathDrawAllocator{64, SkBlockAllocator::GrowthPolicy::kFibonacci};

    class AtlasPathList : SkNoncopyable {
    public:
        void add(PathDrawAllocator* alloc, const SkMatrix& pathMatrix, const SkPath& path) {
            fPathDrawList = &alloc->emplace_back(pathMatrix, path, SK_PMColor4fTRANSPARENT,
                                                 fPathDrawList);
            if (path.isInverseFillType()) {
                // The atlas never has inverse paths. The inversion happens later.
                fPathDrawList->fPath.toggleInverseFillType();
            }
            fTotalCombinedPathVerbCnt += path.countVerbs();
            ++fPathCount;
        }
        const PathDrawList* pathDrawList() const { return fPathDrawList; }
        int totalCombinedPathVerbCnt() const { return fTotalCombinedPathVerbCnt; }
        int pathCount() const { return fPathCount; }

    private:
        PathDrawList* fPathDrawList = nullptr;
        int fTotalCombinedPathVerbCnt = 0;
        int fPathCount = 0;
    };

    AtlasPathList fWindingPathList;
    AtlasPathList fEvenOddPathList;
};

}  // namespace skgpu::ganesh

#endif // AtlasRenderTask_DEFINED
