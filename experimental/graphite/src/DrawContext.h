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

#include "experimental/graphite/src/DrawList.h"
#include "experimental/graphite/src/DrawOrder.h"
#include "experimental/graphite/src/DrawTypes.h"

#include <vector>

namespace skgpu {

class BoundsManager;
class Recorder;
class Shape;
class Transform;

class DrawPass;
class Task;
class TextureProxy;

/**
 * DrawContext records draw commands into a specific Surface, via a general task graph
 * representing GPU work and their inter-dependencies.
 */
class DrawContext final : public SkRefCnt {
public:
    static sk_sp<DrawContext> Make(sk_sp<TextureProxy> target,
                                   sk_sp<SkColorSpace> colorSpace,
                                   SkColorType colorType,
                                   SkAlphaType alphaType);

    ~DrawContext() override;

    const SkImageInfo&  imageInfo() const { return fImageInfo;    }
    TextureProxy* target()                { return fTarget.get(); }
    const TextureProxy* target()    const { return fTarget.get(); }

    int pendingDrawCount() const { return fPendingDraws->drawCount(); }

    // TODO: need color/depth clearing functions (so DCL will probably need those too)

    void clear(const SkColor4f& clearColor);

    void stencilAndFillPath(const Transform& localToDevice,
                            const Shape& shape,
                            const Clip& clip,
                            DrawOrder order,
                            const PaintParams* paint);

    void fillConvexPath(const Transform& localToDevice,
                        const Shape& shape,
                        const Clip& clip,
                        DrawOrder order,
                        const PaintParams* paint);

    void strokePath(const Transform& localToDevice,
                    const Shape& shape,
                    const StrokeParams& stroke,
                    const Clip& clip,
                    DrawOrder order,
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
    void snapDrawPass(Recorder*, const BoundsManager* occlusionCuller);

    // TBD: snapRenderPassTask() might not need to be public, and could be spec'ed to require that
    // snapDrawPass() must have been called first. A lot of it will depend on how the task graph is
    // managed.

    // Ends the current DrawList if needed, as in 'snapDrawPass', and moves the new DrawPass and all
    // prior accumulated DrawPasses into a RenderPassTask that can be drawn and depended on. The
    // caller is responsible for configuring the returned Tasks's dependencies.
    //
    // Returns null if there are no pending commands or draw passes to move into a task.
    sk_sp<Task> snapRenderPassTask(Recorder*, const BoundsManager* occlusionCuller);

private:
    DrawContext(sk_sp<TextureProxy>, const SkImageInfo&);

    sk_sp<TextureProxy> fTarget;
    SkImageInfo fImageInfo;

    // Stores the most immediately recorded draws into the SDC's surface. This list is mutable and
    // can be appended to, or have its commands rewritten if they are inlined into a parent SDC.
    std::unique_ptr<DrawList> fPendingDraws;
    // Load and store information for the current pending draws.
    LoadOp fPendingLoadOp = LoadOp::kLoad;
    StoreOp fPendingStoreOp = StoreOp::kStore;
    std::array<float, 4> fPendingClearColor = { 0, 0, 0, 0 };

    // Stores previously snapped DrawPasses of this SDC, or inlined child SDCs whose content
    // couldn't have been copied directly to fPendingDraws. While each DrawPass is immutable, the
    // list of DrawPasses is not final until there is an external dependency on the SDC's content
    // that requires it to be resolved as its own render pass (vs. inlining the SDC's passes into a
    // parent's render pass).
    // TODO: It will be easier to debug/understand the DrawPass structure of a context if
    // consecutive DrawPasses to the same target are stored in a DrawPassChain. A DrawContext with
    // multiple DrawPassChains is then clearly accumulating subpasses across multiple targets.
    std::vector<std::unique_ptr<DrawPass>> fDrawPasses;
};

} // namespace skgpu

#endif // skgpu_DrawContext_DEFINED
