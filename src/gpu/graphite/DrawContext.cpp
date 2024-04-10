/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/DrawContext.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkPixmap.h"
#include "include/private/SkColorData.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ComputePathAtlas.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/DrawList.h"
#include "src/gpu/graphite/DrawPass.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RasterPathAtlas.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/SharedContext.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/TextureProxyView.h"
#include "src/gpu/graphite/compute/DispatchGroup.h"
#include "src/gpu/graphite/geom/BoundsManager.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/task/ComputeTask.h"
#include "src/gpu/graphite/task/DrawTask.h"
#include "src/gpu/graphite/task/RenderPassTask.h"
#include "src/gpu/graphite/task/UploadTask.h"
#include "src/gpu/graphite/text/TextAtlasManager.h"

namespace skgpu::graphite {

namespace {

// Discarding content on floating point textures can leave nans as the prior color for a pixel,
// in which case hardware blending (when enabled) will fail even if the src, dst coefficients
// and coverage would produce the unmodified src value.
bool discard_op_should_use_clear(SkColorType ct) {
    switch(ct) {
        case kRGBA_F16Norm_SkColorType:
        case kRGBA_F16_SkColorType:
        case kRGBA_F32_SkColorType:
        case kA16_float_SkColorType:
        case kR16G16_float_SkColorType:
            return true;
        default:
            return false;
    }
}

} // anonymous namespace

sk_sp<DrawContext> DrawContext::Make(const Caps* caps,
                                     sk_sp<TextureProxy> target,
                                     SkISize deviceSize,
                                     const SkColorInfo& colorInfo,
                                     const SkSurfaceProps& props,
                                     bool addInitialClear) {
    if (!target) {
        return nullptr;
    }
    // We don't render to unknown or unpremul alphatypes
    if (colorInfo.alphaType() == kUnknown_SkAlphaType ||
        colorInfo.alphaType() == kUnpremul_SkAlphaType) {
        return nullptr;
    }
    if (!caps->isRenderable(target->textureInfo())) {
        return nullptr;
    }

    // Accept an approximate-fit texture, but make sure it's at least as large as the device's
    // logical size.
    // TODO: validate that the color type and alpha type are compatible with the target's info
    SkASSERT(target->isFullyLazy() || (target->dimensions().width() >= deviceSize.width() &&
                                       target->dimensions().height() >= deviceSize.height()));
    SkImageInfo imageInfo = SkImageInfo::Make(deviceSize, colorInfo);
    sk_sp<DrawContext> dc{new DrawContext(caps, std::move(target), imageInfo, props)};
    if (addInitialClear) {
        dc->clear(SkColors::kTransparent);
    }
    return dc;
}

DrawContext::DrawContext(const Caps* caps,
                         sk_sp<TextureProxy> target,
                         const SkImageInfo& ii,
                         const SkSurfaceProps& props)
        : fTarget(std::move(target))
        , fImageInfo(ii)
        , fSurfaceProps(props)
        , fCurrentDrawTask(sk_make_sp<DrawTask>(fTarget))
        , fPendingDraws(std::make_unique<DrawList>())
        , fPendingUploads(std::make_unique<UploadList>()) {
    if (!caps->isTexturable(fTarget->textureInfo())) {
        fReadView = {}; // Presumably this DrawContext is rendering into a swap chain
    } else {
        Swizzle swizzle = caps->getReadSwizzle(ii.colorType(), fTarget->textureInfo());
        fReadView = {fTarget, swizzle};
    }
    // TBD - Will probably want DrawLists (and its internal commands) to come from an arena
    // that the DC manages.

    if (fTarget->isFullyLazy()) {
        // For fully lazy targets, there can be valid content beyond the region this DrawContext
        // represents, and presumably the existing content within the region should be blended
        // against.
        fPendingLoadOp = LoadOp::kLoad;
    } else if (discard_op_should_use_clear(ii.colorType())) {
        fPendingLoadOp = LoadOp::kClear;
    }
}

DrawContext::~DrawContext() = default;

void DrawContext::clear(const SkColor4f& clearColor) {
    this->discard();

    fPendingLoadOp = LoadOp::kClear;
    SkPMColor4f pmColor = clearColor.premul();
    fPendingClearColor = pmColor.array();
}

void DrawContext::discard() {
    // Non-loading operations on a fully lazy target can corrupt data beyond the DrawContext's
    // region so should be avoided.
    SkASSERT(!fTarget->isFullyLazy());

    // A fullscreen clear or discard will overwrite anything that came before, so clear the DrawList
    // NOTE: Eventually the current DrawTask should be reset, once there are no longer implicit
    // dependencies on atlas tasks between DrawContexts. When that's resolved, the only tasks in the
    // current DrawTask are those that directly impact the target, which becomes irrelevant with the
    // clear op overwriting it. For now, preserve the previous tasks that might include atlas
    // uploads that are not explicitly shared between DrawContexts.
    if (fPendingDraws->renderStepCount() > 0) {
        fPendingDraws = std::make_unique<DrawList>();
    }
    if (fComputePathAtlas) {
        fComputePathAtlas->reset();
    }

    if (discard_op_should_use_clear(fImageInfo.colorType())) {
        // In theory the clear color shouldn't matter since a discardable state should be fully
        // overwritten by later draws, but if a previous call to clear() had injected bad data,
        // the discard should not inherit it.
        fPendingClearColor = {0.f, 0.f, 0.f, 0.f};
        fPendingLoadOp = LoadOp::kClear;
    } else {
        fPendingLoadOp = LoadOp::kDiscard;
    }
}

void DrawContext::recordDraw(const Renderer* renderer,
                             const Transform& localToDevice,
                             const Geometry& geometry,
                             const Clip& clip,
                             DrawOrder ordering,
                             const PaintParams* paint,
                             const StrokeStyle* stroke) {
    SkASSERT(SkIRect::MakeSize(this->imageInfo().dimensions()).contains(clip.scissor()));
    fPendingDraws->recordDraw(renderer, localToDevice, geometry, clip, ordering, paint, stroke);
}

bool DrawContext::recordUpload(Recorder* recorder,
                               sk_sp<TextureProxy> targetProxy,
                               const SkColorInfo& srcColorInfo,
                               const SkColorInfo& dstColorInfo,
                               const std::vector<MipLevel>& levels,
                               const SkIRect& dstRect,
                               std::unique_ptr<ConditionalUploadContext> condContext) {
    // Our caller should have clipped to the bounds of the surface already.
    SkASSERT(targetProxy->isFullyLazy() ||
             SkIRect::MakeSize(targetProxy->dimensions()).contains(dstRect));
    return fPendingUploads->recordUpload(recorder,
                                         std::move(targetProxy),
                                         srcColorInfo,
                                         dstColorInfo,
                                         levels,
                                         dstRect,
                                         std::move(condContext));
}

PathAtlas* DrawContext::getComputePathAtlas(Recorder* recorder) {
    if (!fComputePathAtlas) {
        fComputePathAtlas = recorder->priv().atlasProvider()->createComputePathAtlas(recorder);
    }
    return fComputePathAtlas.get();
}

void DrawContext::flush(Recorder* recorder) {
    if (fPendingUploads->size() > 0) {
        TRACE_EVENT_INSTANT1("skia.gpu", TRACE_FUNC, TRACE_EVENT_SCOPE_THREAD,
                             "# uploads", fPendingUploads->size());
        fCurrentDrawTask->addTask(UploadTask::Make(fPendingUploads.get()));
        // The UploadTask steals the collected upload instances, automatically resetting this list
        SkASSERT(fPendingUploads->size() == 0);
    }

    // Generate compute dispatches that render into the atlas texture used by pending draws.
    // TODO: Once compute atlas caching is implemented, DrawContext might not hold onto to this
    // at which point a recordDispatch() could be added and it stores a pending dispatches list that
    // much like how uploads are handled. In that case, Device would be responsible for triggering
    // the recording of dispatches, but that may happen naturally in AtlasProvider::recordUploads().
    if (fComputePathAtlas) {
        auto dispatchGroup = fComputePathAtlas->recordDispatches(recorder);
        fComputePathAtlas->reset();

        if (dispatchGroup) {
            // For now this check is valid as all coverage mask draws involve dispatches
            SkASSERT(fPendingDraws->hasCoverageMaskDraws());

            TRACE_EVENT_INSTANT1("skia.gpu", TRACE_FUNC, TRACE_EVENT_SCOPE_THREAD,
                                 "# dispatches", dispatchGroup->dispatches().size());
            ComputeTask::DispatchGroupList dispatches;
            dispatches.emplace_back(std::move(dispatchGroup));
            fCurrentDrawTask->addTask(ComputeTask::Make(std::move(dispatches)));
        } // else no pending compute work needed to be recorded
    } // else platform doesn't support compute or atlas was never initialized.

    if (fPendingDraws->renderStepCount() == 0 && fPendingLoadOp != LoadOp::kClear) {
        // Nothing will be rasterized to the target that warrants a RenderPassTask, but we preserve
        // any added uploads or compute tasks since those could also affect the target w/o
        // rasterizing anything directly.
        return;
    }

    // Convert the pending draws and load/store ops into a DrawPass that will be executed after
    // the collected uploads and compute dispatches.
    // TODO: At this point, there's only ever one DrawPass in a RenderPassTask to a target. When
    // subpasses are implemented, they will either be collected alongside fPendingDraws or added
    // to the RenderPassTask separately.
    std::unique_ptr<DrawPass> pass = DrawPass::Make(recorder,
                                                    std::move(fPendingDraws),
                                                    fTarget,
                                                    this->imageInfo(),
                                                    std::make_pair(fPendingLoadOp, fPendingStoreOp),
                                                    fPendingClearColor);
    fPendingDraws = std::make_unique<DrawList>();
    // Now that there is content drawn to the target, that content must be loaded on any subsequent
    // render pass.
    fPendingLoadOp = LoadOp::kLoad;
    fPendingStoreOp = StoreOp::kStore;

    if (pass) {
        SkASSERT(fTarget.get() == pass->target());

        const Caps* caps = recorder->priv().caps();
        auto [loadOp, storeOp] = pass->ops();
        auto writeSwizzle = caps->getWriteSwizzle(this->colorInfo().colorType(),
                                                  fTarget->textureInfo());

        RenderPassDesc desc = RenderPassDesc::Make(caps, fTarget->textureInfo(), loadOp, storeOp,
                                                   pass->depthStencilFlags(),
                                                   pass->clearColor(),
                                                   pass->requiresMSAA(),
                                                   writeSwizzle);

        RenderPassTask::DrawPassList passes;
        passes.emplace_back(std::move(pass));
        fCurrentDrawTask->addTask(RenderPassTask::Make(std::move(passes), desc, fTarget));
    }
    // else pass creation failed, DrawPass will have logged why. Don't discard the previously
    // accumulated tasks, however, since they may represent operations on an atlas that other
    // DrawContexts now implicitly depend on.
}

sk_sp<Task> DrawContext::snapDrawTask(Recorder* recorder) {
    // If flush() was explicitly called earlier and no new work was recorded, this call to flush()
    // is a no-op and shouldn't hurt performance.
    this->flush(recorder);

    if (!fCurrentDrawTask->hasTasks()) {
        return nullptr;
    }

    sk_sp<Task> snappedTask = std::move(fCurrentDrawTask);
    fCurrentDrawTask = sk_make_sp<DrawTask>(fTarget);
    return snappedTask;
}

RenderPassDesc RenderPassDesc::Make(const Caps* caps,
                                    const TextureInfo& targetInfo,
                                    LoadOp loadOp,
                                    StoreOp storeOp,
                                    SkEnumBitMask<DepthStencilFlags> depthStencilFlags,
                                    const std::array<float, 4>& clearColor,
                                    bool requiresMSAA,
                                    Swizzle writeSwizzle) {
    RenderPassDesc desc;
    desc.fWriteSwizzle = writeSwizzle;
    desc.fSampleCount = 1;
    // It doesn't make sense to have a storeOp for our main target not be store. Why are we doing
    // this DrawPass then
    SkASSERT(storeOp == StoreOp::kStore);
    if (requiresMSAA) {
        if (caps->msaaRenderToSingleSampledSupport()) {
            desc.fColorAttachment.fTextureInfo = targetInfo;
            desc.fColorAttachment.fLoadOp = loadOp;
            desc.fColorAttachment.fStoreOp = storeOp;
            desc.fSampleCount = caps->defaultMSAASamplesCount();
        } else {
            // TODO: If the resolve texture isn't readable, the MSAA color attachment will need to
            // be persistently associated with the framebuffer, in which case it's not discardable.
            auto msaaTextureInfo = caps->getDefaultMSAATextureInfo(targetInfo, Discardable::kYes);
            if (msaaTextureInfo.isValid()) {
                desc.fColorAttachment.fTextureInfo = msaaTextureInfo;
                if (loadOp != LoadOp::kClear) {
                    desc.fColorAttachment.fLoadOp = LoadOp::kDiscard;
                } else {
                    desc.fColorAttachment.fLoadOp = LoadOp::kClear;
                }
                desc.fColorAttachment.fStoreOp = StoreOp::kDiscard;

                desc.fColorResolveAttachment.fTextureInfo = targetInfo;
                if (loadOp != LoadOp::kLoad) {
                    desc.fColorResolveAttachment.fLoadOp = LoadOp::kDiscard;
                } else {
                    desc.fColorResolveAttachment.fLoadOp = LoadOp::kLoad;
                }
                desc.fColorResolveAttachment.fStoreOp = storeOp;

                desc.fSampleCount = msaaTextureInfo.numSamples();
            } else {
                // fall back to single sampled
                desc.fColorAttachment.fTextureInfo = targetInfo;
                desc.fColorAttachment.fLoadOp = loadOp;
                desc.fColorAttachment.fStoreOp = storeOp;
            }
        }
    } else {
        desc.fColorAttachment.fTextureInfo = targetInfo;
        desc.fColorAttachment.fLoadOp = loadOp;
        desc.fColorAttachment.fStoreOp = storeOp;
    }
    desc.fClearColor = clearColor;

    if (depthStencilFlags != DepthStencilFlags::kNone) {
        desc.fDepthStencilAttachment.fTextureInfo = caps->getDefaultDepthStencilTextureInfo(
                depthStencilFlags, desc.fSampleCount, targetInfo.isProtected());
        // Always clear the depth and stencil to 0 at the start of a DrawPass, but discard at the
        // end since their contents do not affect the next frame.
        desc.fDepthStencilAttachment.fLoadOp = LoadOp::kClear;
        desc.fClearDepth = 0.f;
        desc.fClearStencil = 0;
        desc.fDepthStencilAttachment.fStoreOp = StoreOp::kDiscard;
    }

    return desc;
}

} // namespace skgpu::graphite
