/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DrawContext_DEFINED
#define skgpu_graphite_DrawContext_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceProps.h"

#include "src/gpu/graphite/AttachmentTypes.h"
#include "src/gpu/graphite/DrawList.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/UploadTask.h"

#include <vector>

#ifdef SK_ENABLE_PIET_GPU
#include "src/gpu/graphite/PietRenderTask.h"
namespace skgpu::piet {
class Scene;
}
#endif

class SkPixmap;

namespace skgpu::graphite {

class Geometry;
class Recorder;
class Transform;

class AtlasManager;
class Caps;
class DrawPass;
class Task;
class TextureProxy;
class TextureProxyView;

/**
 * DrawContext records draw commands into a specific Surface, via a general task graph
 * representing GPU work and their inter-dependencies.
 */
class DrawContext final : public SkRefCnt {
public:
    static sk_sp<DrawContext> Make(sk_sp<TextureProxy> target,
                                   SkISize deviceSize,
                                   const SkColorInfo&,
                                   const SkSurfaceProps&);

    ~DrawContext() override;

    const SkImageInfo& imageInfo() const { return fImageInfo;    }
    const SkColorInfo& colorInfo() const { return fImageInfo.colorInfo(); }
    TextureProxy* target()                { return fTarget.get(); }
    const TextureProxy* target()    const { return fTarget.get(); }

    TextureProxyView readSurfaceView(const Caps*);

    const SkSurfaceProps& surfaceProps() const { return fSurfaceProps; }

    int pendingDrawCount() const { return fPendingDraws->drawCount(); }

    void clear(const SkColor4f& clearColor);

    void recordDraw(const Renderer* renderer,
                    const Transform& localToDevice,
                    const Geometry& geometry,
                    const Clip& clip,
                    DrawOrder ordering,
                    const PaintParams* paint,
                    const StrokeStyle* stroke);

    bool recordTextUploads(AtlasManager*);
    bool recordUpload(Recorder* recorder,
                      sk_sp<TextureProxy> targetProxy,
                      const SkColorInfo& srcColorInfo,
                      const SkColorInfo& dstColorInfo,
                      const std::vector<MipLevel>& levels,
                      const SkIRect& dstRect,
                      std::unique_ptr<ConditionalUploadContext>);

#ifdef SK_ENABLE_PIET_GPU
    bool recordPietSceneRender(Recorder* recorder,
                               sk_sp<TextureProxy> targetProxy,
                               sk_sp<const skgpu::piet::Scene> pietScene);
#endif

    // Ends the current DrawList being accumulated by the SDC, converting it into an optimized and
    // immutable DrawPass. The DrawPass will be ordered after any other snapped DrawPasses or
    // appended DrawPasses from a child SDC. A new DrawList is started to record subsequent drawing
    // operations.
    //
    // TBD - Should this take a special occluder list to filter the DrawList?
    // TBD - should this also return the task so the caller can point to it with its own
    // dependencies? Or will that be mostly automatic based on draws and proxy refs?
    void snapDrawPass(Recorder*);

    // TBD: snapRenderPassTask() might not need to be public, and could be spec'ed to require that
    // snapDrawPass() must have been called first. A lot of it will depend on how the task graph is
    // managed.

    // Ends the current DrawList if needed, as in 'snapDrawPass', and moves the new DrawPass and all
    // prior accumulated DrawPasses into a RenderPassTask that can be drawn and depended on. The
    // caller is responsible for configuring the returned Tasks's dependencies.
    //
    // Returns null if there are no pending commands or draw passes to move into a task.
    sk_sp<Task> snapRenderPassTask(Recorder*);

    // Ends the current UploadList if needed, and moves the accumulated Uploads into an UploadTask
    // that can be drawn and depended on. The caller is responsible for configuring the returned
    // Tasks's dependencies.
    //
    // Returns null if there are no pending uploads to move into a task.
    //
    // TODO: see if we can merge transfers into this
    sk_sp<Task> snapUploadTask(Recorder*);

#ifdef SK_ENABLE_PIET_GPU
    sk_sp<Task> snapPietRenderTask(Recorder*);
#endif

private:
    DrawContext(sk_sp<TextureProxy>, const SkImageInfo&, const SkSurfaceProps&);

    sk_sp<TextureProxy> fTarget;
    SkImageInfo fImageInfo;
    const SkSurfaceProps fSurfaceProps;

    // Stores the most immediately recorded draws into the SDC's surface. This list is mutable and
    // can be appended to, or have its commands rewritten if they are inlined into a parent SDC.
    std::unique_ptr<DrawList> fPendingDraws;
    // Load and store information for the current pending draws.
    LoadOp fPendingLoadOp = LoadOp::kLoad;
    StoreOp fPendingStoreOp = StoreOp::kStore;
    std::array<float, 4> fPendingClearColor = { 0, 0, 0, 0 };

    // Stores previously snapped DrawPasses of this DC, or inlined child DCs whose content
    // couldn't have been copied directly to fPendingDraws. While each DrawPass is immutable, the
    // list of DrawPasses is not final until there is an external dependency on the SDC's content
    // that requires it to be resolved as its own render pass (vs. inlining the SDC's passes into a
    // parent's render pass).
    // TODO: It will be easier to debug/understand the DrawPass structure of a context if
    // consecutive DrawPasses to the same target are stored in a DrawPassChain. A DrawContext with
    // multiple DrawPassChains is then clearly accumulating subpasses across multiple targets.
    std::vector<std::unique_ptr<DrawPass>> fDrawPasses;

    // Stores the most immediately recorded uploads into Textures. This list is mutable and
    // can be appended to, or have its commands rewritten if they are inlined into a parent DC.
    std::unique_ptr<UploadList> fPendingUploads;

#ifdef SK_ENABLE_PIET_GPU
    std::vector<PietRenderInstance> fPendingPietRenders;
#endif
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DrawContext_DEFINED
