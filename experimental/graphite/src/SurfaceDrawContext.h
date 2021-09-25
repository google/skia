/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_SurfaceDrawContext_DEFINED
#define skgpu_SurfaceDrawContext_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"

class SkPath;
class SkM44;

namespace skgpu {

class BoundsManager;
class DrawList;
class Task;

struct PaintParams;
struct StrokeParams;

/**
 * SurfaceDrawContext records draw commands into a specific Surface, via a general task graph
 * representing GPU work and their inter-dependencies.
 */
class SurfaceDrawContext final : public SkRefCnt {
public:
    static sk_sp<SurfaceDrawContext> Make(const SkImageInfo&);

    ~SurfaceDrawContext() override;

    const SkImageInfo& imageInfo() { return fImageInfo; }

    // TODO: need color/depth clearing functions (so DCL will probably need those too)

    void fillPath(const SkM44& localToDevice,
                  const SkPath& path,
                  const SkIRect& scissor,
                  uint16_t sortZ,
                  uint16_t testZ,
                  const PaintParams* paint);

    void strokePath(const SkM44& localToDevice,
                    const SkPath& path,
                    const StrokeParams& stroke,
                    const SkIRect& scissor,
                    uint16_t sortZ,
                    uint16_t testZ,
                    const PaintParams* paint);

    // Ends the current DrawList being accumulated by the SDC, converting it into an optimized and
    // immutable DrawPass. The DrawPass will be ordered after any other snapped DrawPasses or
    // appended DrawPasses from a child SDC. A new DrawList is started to record subsequent drawing
    // operations.

    // If 'occlusionCuller' is null, then culling is skipped when converting the DrawList into a
    // DrawPass.
    // TBD - should this also return the task so the caller can point to it with its own
    // dependencies? Or will that be mostly automatic based on draws and proxy refs?
    void snapDrawPass(const BoundsManager* occlusionCuller);

private:
    SurfaceDrawContext(const SkImageInfo&);

    SkImageInfo fImageInfo;

    // TODO: After discussing the possibility of sub renderpasses and memoryless saved layers,
    // an SDC ought to hold on to a list of {DrawList, Proxy} that represent the pending subpasses.
    // A saveLayer can be then be drawn by (1) a regular render pass, followed by a regular texture
    // sample in the parent's SDC, (2) a sub pass where the layer's draw list targets a
    // "memory-less" 2nd surface w/in a regular render pass for the parent, or (3) fully eliding the
    // layer by appending it's draw list directly to the parent's draw list. This will need to be
    // represented at this high API level because it requires decisions to be made by Device when
    // creating the SkDevice for the new layer, and during the restore.
    std::unique_ptr<DrawList> fPendingDraws;

    // TBD - Does the SDC even need to hold on to its tail task? Or when it finalizes its current
    // DCL into a DrawTask it can send that back to the Recorder as part of a larger task graph?
    // The only question then would be how to track the dependencies of that DrawTask since it would
    // depend on the prior DrawTask and the SDC's surface view.
    sk_sp<Task> fTail;
};

} // namespace skgpu

#endif // skgpu_SurfaceDrawContext_DEFINED
