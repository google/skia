/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_DrawContext_DEFINED
#define skgpu_DrawContext_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"

#include "experimental/graphite/include/GraphiteTypes.h"

#include <vector>

class SkPath;
class SkM44;

namespace skgpu {

class BoundsManager;
class DrawList;
class DrawPass;
class Task;

struct PaintParams;
struct StrokeParams;

/**
 * DrawContext records draw commands into a specific Surface, via a general task graph
 * representing GPU work and their inter-dependencies.
 */
class DrawContext final : public SkRefCnt {
public:
    static sk_sp<DrawContext> Make(const SkImageInfo&);

    ~DrawContext() override;

    const SkImageInfo& imageInfo() { return fImageInfo; }

    // TODO: need color/depth clearing functions (so DCL will probably need those too)

    void stencilAndFillPath(const SkM44& localToDevice,
                            const SkPath& path,
                            const SkIRect& scissor,
                            CompressedPaintersOrder colorDepthOrder,
                            CompressedPaintersOrder stencilOrder,
                            uint16_t depth,
                            const PaintParams* paint);

    void fillConvexPath(const SkM44& localToDevice,
                        const SkPath& path,
                        const SkIRect& scissor,
                        CompressedPaintersOrder colorDepthOrder,
                        uint16_t depth,
                        const PaintParams* paint);

    void strokePath(const SkM44& localToDevice,
                    const SkPath& path,
                    const StrokeParams& stroke,
                    const SkIRect& scissor,
                    CompressedPaintersOrder colorDepthOrder,
                    uint16_t depth,
                    const PaintParams* paint);

    // Ends the current DrawList being accumulated by the SDC, converting it into an optimized and
    // immutable DrawPass. The DrawPass will be ordered after any other snapped DrawPasses or
    // appended DrawPasses from a child SDC. A new DrawList is started to record subsequent drawing
    // operations.
    //
    // If 'occlusionCuller' is null, then culling is skipped when converting the DrawList into a
    // DrawPass.
    // TBD - should this also return the task so the caller can point to it with its own
    // dependencies? Or will that be mostly automatic based on draws and proxy refs?
    void snapDrawPass(const BoundsManager* occlusionCuller);

    // TBD: snapRenderPassTask() might not need to be public, and could be spec'ed to require that
    // snapDrawPass() must have been called first. A lot of it will depend on how the task graph is
    // managed.

    // Ends the current DrawList if needed, as in 'snapDrawPass', and moves the new DrawPass and all
    // prior accumulated DrawPasses into a RenderPassTask that can be drawn and depended on. The
    // returned task will automatically depend on any previous snapped task of the SDC.
    //
    // Returns null if there are no pending commands or draw passes to move into a task.
    sk_sp<Task> snapRenderPassTask(const BoundsManager* occlusionCuller);

private:
    DrawContext(const SkImageInfo&);

    SkImageInfo fImageInfo;

    // Stores the most immediately recorded draws into the SDC's surface. This list is mutable and
    // can be appended to, or have its commands rewritten if they are inlined into a parent SDC.
    std::unique_ptr<DrawList> fPendingDraws;

    // Stores previously snapped DrawPasses of this SDC, or inlined child SDCs whose content
    // couldn't have been copied directly to fPendingDraws. While each DrawPass is immutable, the
    // list of DrawPasses is not final until there is an external dependency on the SDC's content
    // that requires it to be resolved as its own render pass (vs. inlining the SDC's passes into a
    // parent's render pass).
    std::vector<std::unique_ptr<DrawPass>> fDrawPasses;

    // TBD - Does the SDC even need to hold on to its tail task? Or when it finalizes its list of
    // passes into a RenderPassTask it can send that back to the Recorder as part of a larger task
    // graph? The only question then would be how to track the dependencies of that RenderPassTask
    // since it would depend on the prior RenderPassTask and the SDC's of the DrawPasses.
    sk_sp<Task> fTail;
};

} // namespace skgpu

#endif // skgpu_DrawContext_DEFINED
