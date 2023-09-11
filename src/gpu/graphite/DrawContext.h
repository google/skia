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
#include "include/private/base/SkTArray.h"

#include "src/gpu/graphite/AttachmentTypes.h"
#include "src/gpu/graphite/DrawList.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/UploadTask.h"

#include <vector>

class SkPixmap;

namespace skgpu::graphite {

class Geometry;
class Recorder;
class Transform;

class Caps;
class ComputePathAtlas;
class DispatchGroup;
class DrawPass;
class PathAtlas;
class SoftwarePathAtlas;
class Task;
class TextAtlasManager;
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

    bool recordTextUploads(TextAtlasManager*);
    bool recordUpload(Recorder* recorder,
                      sk_sp<TextureProxy> targetProxy,
                      const SkColorInfo& srcColorInfo,
                      const SkColorInfo& dstColorInfo,
                      const std::vector<MipLevel>& levels,
                      const SkIRect& dstRect,
                      std::unique_ptr<ConditionalUploadContext>);

    // Returns the transient path atlas that accumulates coverage masks for atlas draws recorded to
    // this SDC. The atlas gets created lazily upon request. Returns nullptr if atlas draws are not
    // supported.
    //
    // TODO: Should this be explicit about how the atlas gets drawn (i.e. GPU compute vs CPU)?
    // Currently this is assumed to use GPU compute atlas. Maybe the PathAtlas class should report
    // its rendering algorithm to aid the renderer selection in `chooseRenderer`?
    PathAtlas* getOrCreatePathAtlas(Recorder*);

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

    // Moves all accummulated DispatchGroups into a ComputeTask and returns it. A DispatchGroup may
    // be recorded internally as a dependency of a DrawPass (which may happen during a call to
    // `snapDrawPass()`) or directly by the caller (e.g. as part of compute-based atlas render).
    //
    // The returned Task encapsulates all recorded dispatches and the caller is responsible for
    // ensuring that the Task gets executed ahead of draws.
    //
    // Returns null if there are no pending dispatches to move into a task.
    //
    // TODO: implement DispatchGroup recording as part of snapDrawPass for geometry processing
    // TBD: The current broad design requires that compute tasks are executed before draws. The
    // current thinking around image filters that may operate on the result of a draw involves
    // maintaining this order by adding a post-draw compute pass to a subsequent DrawContext. This
    // design needs to get hashed out.
    sk_sp<Task> snapComputeTask(Recorder*);

private:
    DrawContext(sk_sp<TextureProxy>, const SkImageInfo&, const SkSurfaceProps&);

    // If a compute atlas was initialized, schedule its accummulated paths to be rendered.
    void snapPathAtlasDispatches(Recorder*);

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

    // Accumulates atlas coverage masks generated by compute dispatches that are required by one or
    // more entries in `fPendingDraws`. When pending draws are snapped into a new DrawPass, a
    // compute dispatch group gets recorded which schedules the accumulated masks to get drawn into
    // an atlas texture. The accumulated masks are then cleared which frees up the atlas for
    // future draws.
    //
    // TODO: Currently every PathAtlas contains a single texture. If multiple snapped draw
    // passes resulted in multiple ComputePathAtlas dispatch groups, the later dispatches would
    // overwrite the atlas texture since all compute tasks are scheduled before render tasks. This
    // is currently not an issue since there is only one DrawPass per flush but we may want to
    // either support one atlas texture per DrawPass or record the dispatches once per
    // RenderPassTask rather than DrawPass.
    std::unique_ptr<ComputePathAtlas> fComputePathAtlas;

    // Accumulates atlas coverage masks generated by software rendering that are required by one or
    // more entries in `fPendingDraws`. During the snapUploadTask step, prior to pending draws
    // being snapped into a new DrawPass, any necessary uploads into an atlas texture are recorded
    // for the accumulated masks. The accumulated masks are then cleared which frees up the atlas
    // for future draws.
    //
    // TODO: We should not clear all accumulated masks but cache masks over more than one frame.
    //
    // TODO: We may need a method to generate software masks in separate threads prior to upload.
    std::unique_ptr<SoftwarePathAtlas> fSoftwarePathAtlas;

    // Stores previously snapped DrawPasses of this DC, or inlined child DCs whose content
    // couldn't have been copied directly to fPendingDraws. While each DrawPass is immutable, the
    // list of DrawPasses is not final until there is an external dependency on the SDC's content
    // that requires it to be resolved as its own render pass (vs. inlining the SDC's passes into a
    // parent's render pass).
    // TODO: It will be easier to debug/understand the DrawPass structure of a context if
    // consecutive DrawPasses to the same target are stored in a DrawPassChain. A DrawContext with
    // multiple DrawPassChains is then clearly accumulating subpasses across multiple targets.
    skia_private::TArray<std::unique_ptr<DrawPass>> fDrawPasses;

    // Stores the most immediately recorded uploads into Textures. This list is mutable and
    // can be appended to, or have its commands rewritten if they are inlined into a parent DC.
    std::unique_ptr<UploadList> fPendingUploads;

    // Stores all compute dispatches that have been recorded as a dependency of a draw.
    skia_private::TArray<std::unique_ptr<DispatchGroup>> fDispatchGroups;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DrawContext_DEFINED
