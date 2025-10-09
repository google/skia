/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DrawContext_DEFINED
#define skgpu_graphite_DrawContext_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceProps.h"
#include "src/gpu/graphite/DrawList.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/TextureProxyView.h"

#include <array>
#include <memory>
#include <vector>

struct SkIRect;
struct SkISize;

namespace skgpu::graphite {

class Caps;
class Clip;
class ComputePathAtlas;
class ConditionalUploadContext;
class DrawOrder;
class DrawTask;
class Geometry;
class PaintParams;
class PathAtlas;
class Recorder;
class Renderer;
class StrokeStyle;
class Task;
class Transform;
class UploadList;
class UploadSource;

/**
 * DrawContext records draw commands into a specific Surface, via a general task graph
 * representing GPU work and their inter-dependencies.
 */
class DrawContext final : public SkRefCnt {
public:
    static sk_sp<DrawContext> Make(const Caps* caps,
                                   sk_sp<TextureProxy> target,
                                   SkISize deviceSize,
                                   const SkColorInfo&,
                                   const SkSurfaceProps&);

    ~DrawContext() override;

    const SkImageInfo& imageInfo() const  { return fImageInfo;             }
    const SkColorInfo& colorInfo() const  { return fImageInfo.colorInfo(); }
    TextureProxy* target()                { return fTarget.get();          }
    const TextureProxy* target()    const { return fTarget.get();          }
    sk_sp<TextureProxy> refTarget() const { return fTarget;                }

    // May be null if the target is not texturable.
    const TextureProxyView& readSurfaceView() const { return fReadView; }

    const SkSurfaceProps& surfaceProps() const { return fSurfaceProps; }

    int pendingRenderSteps() const { return fPendingDraws->renderStepCount(); }

    bool modifiesTarget() const { return fPendingDraws->modifiesTarget(); }

    bool readsTexture(const TextureProxy*) const;

    void clear(const SkColor4f& clearColor);
    void discard();

    void recordDraw(const Renderer* renderer,
                    const Transform& localToDevice,
                    const Geometry& geometry,
                    const Clip& clip,
                    DrawOrder ordering,
                    UniquePaintParamsID paintID,
                    SkEnumBitMask<DstUsage> dstUsage,
                    PipelineDataGatherer* gatherer,
                    const StrokeStyle* stroke);

    bool recordUpload(Recorder* recorder,
                      sk_sp<TextureProxy> targetProxy,
                      const SkColorInfo& srcColorInfo,
                      const SkColorInfo& dstColorInfo,
                      const UploadSource& source,
                      const SkIRect& dstRect,
                      std::unique_ptr<ConditionalUploadContext>);

    // Add a Task that will be executed *before* any of the pending draws and uploads are
    // executed as part of the next flush().
    void recordDependency(sk_sp<Task>);

    // Returns the transient path atlas that uses compute to accumulate coverage masks for atlas
    // draws recorded to this SDC. The atlas gets created lazily upon request. Returns nullptr
    // if compute path generation is not supported.
    PathAtlas* getComputePathAtlas(Recorder*);

    // Moves all accumulated pending recorded operations (draws and uploads), and any other
    // dependent tasks into the DrawTask currently being built.
    void flush(Recorder*);

    // Returns the current DrawTask to the caller, so all pending draws and uploads (if flush()
    // was not immediately called prior to this) and subsequently recorded draws and uploads will
    // go into a new DrawTask.
    sk_sp<Task> snapDrawTask();

    // Returns the dst read strategy to use when/if a paint requires a dst read
    DstReadStrategy dstReadStrategy() const { return fDstReadStrategy; }

private:
    DrawContext(const Caps*, sk_sp<TextureProxy>, const SkImageInfo&, const SkSurfaceProps&);

    void resetForClearOrDiscard();

    sk_sp<TextureProxy> fTarget;
    TextureProxyView fReadView;
    SkImageInfo fImageInfo;
    const SkSurfaceProps fSurfaceProps;

    // Does *not* reflect whether a dst read is needed by the DrawLists - simply specifies the
    // strategies to use should any encountered paint require it.
    const DstReadStrategy fDstReadStrategy;
    const bool fSupportsHardwareAdvancedBlend;
    const bool fAdvancedBlendsRequireBarrier;

    // The in-progress DrawTask that will be snapped and returned when some external requirement
    // must depend on the contents of this DrawContext's target. As higher-level Skia operations
    // are recorded, it can be necessary to flush pending draws and uploads into the task list.
    // This provides a place to reset scratch textures or buffers as their previous state will have
    // been consumed by the flushed tasks rendering to this DrawContext's target.
    sk_sp<DrawTask> fCurrentDrawTask;

    // Stores the most immediately recorded draws and uploads into the DrawContext's target. These
    // are collected outside of the DrawTask so that encoder switches can be minimized when
    // flushing.
    std::unique_ptr<DrawList> fPendingDraws;
    std::unique_ptr<UploadList> fPendingUploads;

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
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DrawContext_DEFINED
