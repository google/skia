/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/DrawContext.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorType.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/base/SkAssert.h"
#include "src/base/SkEnumBitMask.h"
#include "src/core/SkColorData.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ComputePathAtlas.h"
#include "src/gpu/graphite/DrawList.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawPass.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/TextureProxyView.h"
#include "src/gpu/graphite/TextureUtils.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/task/ComputeTask.h"
#include "src/gpu/graphite/task/DrawTask.h"
#include "src/gpu/graphite/task/RenderPassTask.h"
#include "src/gpu/graphite/task/Task.h"
#include "src/gpu/graphite/task/UploadTask.h"

#include <cstdint>
#include <utility>

namespace skgpu::graphite {

class PaintParams;
class Renderer;
enum class DepthStencilFlags : int;

sk_sp<DrawContext> DrawContext::Make(const Caps* caps,
                                     sk_sp<TextureProxy> target,
                                     SkISize deviceSize,
                                     const SkColorInfo& colorInfo,
                                     const SkSurfaceProps& props) {
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
    if (!caps->areColorTypeAndTextureInfoCompatible(colorInfo.colorType(), target->textureInfo())) {
        return nullptr;
    }

    // Accept an approximate-fit texture, but make sure it's at least as large as the device's
    // logical size.
    // TODO: validate that the alpha type is compatible with the target's info
    SkASSERT(target->isFullyLazy() || (target->dimensions().width() >= deviceSize.width() &&
                                       target->dimensions().height() >= deviceSize.height()));
    SkImageInfo imageInfo = SkImageInfo::Make(deviceSize, colorInfo);
    return sk_sp<DrawContext>(new DrawContext(caps, std::move(target), imageInfo, props));
}

DrawContext::DrawContext(const Caps* caps,
                         sk_sp<TextureProxy> target,
                         const SkImageInfo& ii,
                         const SkSurfaceProps& props)
        : fTarget(std::move(target))
        , fImageInfo(ii)
        , fSurfaceProps(props)
        , fDstReadStrategy(caps->getDstReadStrategy())
        , fSupportsHardwareAdvancedBlend(caps->supportsHardwareAdvancedBlending())
        , fAdvancedBlendsRequireBarrier(caps->blendEquationSupport() ==
                                            Caps::BlendEquationSupport::kAdvancedNoncoherent)
        , fCurrentDrawTask(sk_make_sp<DrawTask>(fTarget))
        , fPendingDraws(std::make_unique<DrawList>())
        , fPendingUploads(std::make_unique<UploadList>()) {
    // Must determine a valid strategy to use should a dst texture read be required.
    SkASSERT(fDstReadStrategy != DstReadStrategy::kNoneRequired);

    if (!caps->isTexturable(fTarget->textureInfo())) {
        fReadView = {}; // Presumably this DrawContext is rendering into a swap chain
    } else {
        Swizzle swizzle = caps->getReadSwizzle(ii.colorType(), fTarget->textureInfo());
        fReadView = {fTarget, swizzle};
    }
    // TBD - Will probably want DrawLists (and its internal commands) to come from an arena
    // that the DC manages.
}

DrawContext::~DrawContext() = default;

void DrawContext::clear(const SkColor4f& clearColor) {
    this->resetForClearOrDiscard();
    fPendingDraws->reset(LoadOp::kClear, clearColor);
}

void DrawContext::discard() {
    this->resetForClearOrDiscard();
    fPendingDraws->reset(LoadOp::kDiscard);
}

void DrawContext::resetForClearOrDiscard() {
    // Non-loading operations on a fully lazy target can corrupt data beyond the DrawContext's
    // region so should be avoided.
    SkASSERT(!fTarget->isFullyLazy());

    // NOTE: Eventually the current DrawTask should be reset, once there are no longer implicit
    // dependencies on atlas tasks between DrawContexts. When that's resolved, the only tasks in the
    // current DrawTask are those that directly impact the target, which becomes irrelevant with the
    // clear op overwriting it. For now, preserve the previous tasks that might include atlas
    // uploads that are not explicitly shared between DrawContexts.
    if (fComputePathAtlas) {
        fComputePathAtlas->reset();
    }
}

bool DrawContext::readsTexture(const TextureProxy* texture) const {
    if (fPendingDraws->samplesTexture(texture)) {
        return true;
    }

    // visitProxies() before calling prepareResources() can revisit tasks in the general case
    // (e.g. processing everything in the root task list). In this case, the only tasks being
    // visited are pending tasks so their graph complexity should be minimal.
    bool notFound = fCurrentDrawTask->visitProxies(
        [texture](const TextureProxy* other) {
            // Return true to continue visiting, i.e. when we haven't found `texture` yet.
            return texture != other;
        }, /*readsOnly=*/true);

    return !notFound; // double negation means its found in a pending child task
}

void DrawContext::recordDraw(const Renderer* renderer,
                             const Transform& localToDevice,
                             const Geometry& geometry,
                             const Clip& clip,
                             DrawOrder ordering,
                             UniquePaintParamsID paintID,
                             SkEnumBitMask<DstUsage> dstUsage,
                             PipelineDataGatherer* gatherer,
                             const StrokeStyle* stroke) {
    SkASSERTF(SkIRect::MakeSize(this->imageInfo().dimensions()).contains(clip.scissor()),
              "Image %dx%d, scissor %d,%d,%d,%d",
              this->imageInfo().width(), this->imageInfo().height(),
              clip.scissor().left(), clip.scissor().top(),
              clip.scissor().right(), clip.scissor().bottom());

    // Determine whether a draw requies a barrier
    BarrierType barrierBeforeDraws = BarrierType::kNone;
    if (fDstReadStrategy == DstReadStrategy::kReadFromInput &&
        (dstUsage & DstUsage::kDstReadRequired)) {
        barrierBeforeDraws = BarrierType::kReadDstFromInput;
    }
    if ((dstUsage & DstUsage::kAdvancedBlend) &&
        fSupportsHardwareAdvancedBlend && fAdvancedBlendsRequireBarrier) {
        // A draw should only read from the dst OR use hardware for advanced blend modes.
        SkASSERT(!(dstUsage & DstUsage::kDstReadRequired));
        barrierBeforeDraws = BarrierType::kAdvancedNoncoherentBlend;
    }

    fPendingDraws->recordDraw(renderer, localToDevice, geometry, clip, ordering, paintID, dstUsage,
                              barrierBeforeDraws, gatherer, stroke);
}

bool DrawContext::recordUpload(Recorder* recorder,
                               sk_sp<TextureProxy> targetProxy,
                               const SkColorInfo& srcColorInfo,
                               const SkColorInfo& dstColorInfo,
                               const UploadSource& source,
                               const SkIRect& dstRect,
                               std::unique_ptr<ConditionalUploadContext> condContext) {
    // Our caller should have clipped to the bounds of the surface already.
    SkASSERT(targetProxy->isFullyLazy() ||
             SkIRect::MakeSize(targetProxy->dimensions()).contains(dstRect));
    SkASSERT(source.isValid());
    return fPendingUploads->recordUpload(recorder,
                                         std::move(targetProxy),
                                         srcColorInfo,
                                         dstColorInfo,
                                         source,
                                         dstRect,
                                         std::move(condContext));
}

void DrawContext::recordDependency(sk_sp<Task> task) {
    SkASSERT(task);
    // Adding `task` to the current DrawTask directly means that it will execute after any previous
    // dependent tasks and after any previous calls to flush(), but everything else that's being
    // collected on the DrawContext will execute after `task` once the next flush() is performed.
    fCurrentDrawTask->addTask(std::move(task));
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
        ComputeTask::DispatchGroupList dispatches;
        if (fComputePathAtlas->recordDispatches(recorder, &dispatches)) {
            // For now this check is valid as all coverage mask draws involve dispatches
            SkASSERT(fPendingDraws->hasCoverageMaskDraws());

            fCurrentDrawTask->addTask(ComputeTask::Make(std::move(dispatches)));
        } // else no pending compute work needed to be recorded

        fComputePathAtlas->reset();
    } // else platform doesn't support compute or atlas was never initialized.

    if (!fPendingDraws->modifiesTarget()) {
        // Nothing will be rasterized to the target that warrants a RenderPassTask, but we preserve
        // any added uploads or compute tasks since those could also affect the target w/o
        // rasterizing anything directly.
        return;
    }

    // Extract certain properties from DrawList relevant for DrawTask construction before
    // relinquishing the pending draw list to the DrawPass constructor.
    SkIRect dstReadPixelBounds = fPendingDraws->dstReadBounds().makeRoundOut().asSkIRect();
    const bool drawsRequireMSAA = fPendingDraws->drawsRequireMSAA();
    const SkEnumBitMask<DepthStencilFlags> dsFlags = fPendingDraws->depthStencilFlags();
    // Determine the optimal dst read strategy for the drawpass given pending draw characteristics
    const DstReadStrategy drawPassDstReadStrategy = fPendingDraws->drawsReadDst()
                                                            ? this->dstReadStrategy()
                                                            : DstReadStrategy::kNoneRequired;

    // Convert the pending draws and load/store ops into a DrawPass that will be executed after
    // the collected uploads and compute dispatches.
    // TODO: At this point, there's only ever one DrawPass in a RenderPassTask to a target. When
    // subpasses are implemented, they will either be collected alongside fPendingDraws or added
    // to the RenderPassTask separately.
    std::unique_ptr<DrawPass> pass = fPendingDraws->snapDrawPass(recorder,
                                                                 fTarget,
                                                                 this->imageInfo(),
                                                                 drawPassDstReadStrategy);
    SkASSERT(!fPendingDraws->modifiesTarget()); // Should be drained into `pass`.

    if (pass) {
        SkASSERT(fTarget.get() == pass->target());

        // If any paint used within the DrawPass reads from the dst texture (indicated by nonempty
        // dstReadPixelBounds) and the dstReadStrategy is kTextureCopy, then add a CopyTask.
        sk_sp<TextureProxy> dstCopy;
        if (!dstReadPixelBounds.isEmpty() &&
            drawPassDstReadStrategy == DstReadStrategy::kTextureCopy) {
            TRACE_EVENT_INSTANT0("skia.gpu", "DrawPass requires dst copy",
                                 TRACE_EVENT_SCOPE_THREAD);
            sk_sp<Image> imageCopy = Image::Copy(
                    recorder,
                    this,
                    fReadView,
                    fImageInfo.colorInfo(),
                    dstReadPixelBounds,
                    Budgeted::kYes,
                    Mipmapped::kNo,
                    SkBackingFit::kApprox,
                    "DstCopy");
            if (!imageCopy) {
                SKGPU_LOG_W("DrawContext::flush Image::Copy failed, draw pass dropped!");
                return;
            }
            dstCopy = imageCopy->textureProxyView().refProxy();
            SkASSERT(dstCopy);
        }

        const Caps* caps = recorder->priv().caps();
        auto [loadOp, storeOp] = pass->ops();
        auto writeSwizzle = caps->getWriteSwizzle(this->colorInfo().colorType(),
                                                  fTarget->textureInfo());

        RenderPassDesc desc = RenderPassDesc::Make(caps, fTarget->textureInfo(), loadOp, storeOp,
                                                   dsFlags,
                                                   pass->clearColor(),
                                                   drawsRequireMSAA,
                                                   writeSwizzle,
                                                   drawPassDstReadStrategy);

        RenderPassTask::DrawPassList passes;
        passes.emplace_back(std::move(pass));
        fCurrentDrawTask->addTask(RenderPassTask::Make(std::move(passes), desc, fTarget,
                                                       std::move(dstCopy), dstReadPixelBounds));
        if (fTarget->mipmapped() == Mipmapped::kYes) {
            if (!GenerateMipmaps(recorder, this, fTarget, fImageInfo.colorInfo())) {
                SKGPU_LOG_W("DrawContext::flush GenerateMipmaps failed, draw pass dropped!");
                return;
            }
        }
    }
    // else pass creation failed, DrawPass will have logged why. Don't discard the previously
    // accumulated tasks, however, since they may represent operations on an atlas that other
    // DrawContexts now implicitly depend on.
}

sk_sp<Task> DrawContext::snapDrawTask() {
    if (!fCurrentDrawTask->hasTasks()) {
        return nullptr;
    }

    sk_sp<Task> snappedTask = std::move(fCurrentDrawTask);
    fCurrentDrawTask = sk_make_sp<DrawTask>(fTarget);
    return snappedTask;
}

} // namespace skgpu::graphite
