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
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/DrawList.h"
#include "src/gpu/graphite/DrawPass.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RenderPassTask.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/SharedContext.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/UploadTask.h"
#include "src/gpu/graphite/geom/BoundsManager.h"
#include "src/gpu/graphite/geom/Geometry.h"

#ifdef SK_ENABLE_PIET_GPU
#include "src/gpu/graphite/PietRenderTask.h"
#endif

namespace skgpu::graphite {

sk_sp<DrawContext> DrawContext::Make(sk_sp<TextureProxy> target,
                                     const SkColorInfo& colorInfo,
                                     const SkSurfaceProps& props) {
    if (!target) {
        return nullptr;
    }

    // TODO: validate that the color type and alpha type are compatible with the target's info
    SkImageInfo imageInfo = SkImageInfo::Make(target->dimensions(), colorInfo);
    return sk_sp<DrawContext>(new DrawContext(std::move(target), imageInfo, props));
}

DrawContext::DrawContext(sk_sp<TextureProxy> target,
                         const SkImageInfo& ii,
                         const SkSurfaceProps& props)
        : fTarget(std::move(target))
        , fImageInfo(ii)
        , fSurfaceProps(props)
        , fPendingDraws(std::make_unique<DrawList>())
        , fPendingUploads(std::make_unique<UploadList>()) {
    // TBD - Will probably want DrawLists (and its internal commands) to come from an arena
    // that the DC manages.
}

DrawContext::~DrawContext() {
    // If the DC is destroyed and there are pending commands, they won't be drawn.
    fPendingDraws.reset();
    fDrawPasses.clear();
}

void DrawContext::clear(const SkColor4f& clearColor) {
    fPendingLoadOp = LoadOp::kClear;
    SkPMColor4f pmColor = clearColor.premul();
    fPendingClearColor = pmColor.array();

    // a fullscreen clear will overwrite anything that came before, so start a new DrawList
    // and clear any drawpasses that haven't been snapped yet
    fPendingDraws = std::make_unique<DrawList>();
    fDrawPasses.clear();
}

void DrawContext::recordDraw(const Renderer& renderer,
                             const Transform& localToDevice,
                             const Geometry& geometry,
                             const Clip& clip,
                             DrawOrder ordering,
                             const PaintParams* paint,
                             const StrokeStyle* stroke) {
    SkASSERT(SkIRect::MakeSize(fTarget->dimensions()).contains(clip.scissor()));
    fPendingDraws->recordDraw(renderer, localToDevice, geometry, clip, ordering, paint, stroke);
}

bool DrawContext::recordUpload(Recorder* recorder,
                               sk_sp<TextureProxy> targetProxy,
                               SkColorType colorType,
                               const std::vector<MipLevel>& levels,
                               const SkIRect& dstRect) {
    // Our caller should have clipped to the bounds of the surface already.
    SkASSERT(SkIRect::MakeSize(targetProxy->dimensions()).contains(dstRect));
    return fPendingUploads->recordUpload(recorder,
                                         std::move(targetProxy),
                                         colorType,
                                         levels,
                                         dstRect);
}

#ifdef SK_ENABLE_PIET_GPU
bool DrawContext::recordPietSceneRender(Recorder*,
                                        sk_sp<TextureProxy> targetProxy,
                                        sk_sp<const skgpu::piet::Scene> scene) {
    fPendingPietRenders.push_back(PietRenderInstance(std::move(scene), std::move(targetProxy)));
    return true;
}
#endif

void DrawContext::snapDrawPass(Recorder* recorder) {
    if (fPendingDraws->drawCount() == 0) {
        return;
    }

    auto pass = DrawPass::Make(recorder,
                               std::move(fPendingDraws),
                               fTarget,
                               std::make_pair(fPendingLoadOp, fPendingStoreOp),
                               fPendingClearColor);
    fDrawPasses.push_back(std::move(pass));
    fPendingDraws = std::make_unique<DrawList>();
    fPendingLoadOp = LoadOp::kLoad;
    fPendingStoreOp = StoreOp::kStore;
}

sk_sp<Task> DrawContext::snapRenderPassTask(Recorder* recorder) {
    this->snapDrawPass(recorder);
    if (fDrawPasses.empty()) {
        return nullptr;
    }

    const Caps* caps = recorder->priv().caps();

    // TODO: At this point we would determine all the targets used by the drawPasses,
    // build up the union of them and store them in the RenderPassDesc. However, for
    // the moment we should have only one drawPass.
    SkASSERT(fDrawPasses.size() == 1);
    RenderPassDesc desc;
    auto& drawPass = fDrawPasses[0];
    const TextureInfo& targetInfo = drawPass->target()->textureInfo();
    auto [loadOp, storeOp] = drawPass->ops();
    // It doesn't make sense to have a storeOp for our main target not be store. Why are we doing
    // this DrawPass then
    SkASSERT(storeOp == StoreOp::kStore);
    if (drawPass->requiresMSAA()) {
        desc.fColorAttachment.fTextureInfo = caps->getDefaultMSAATextureInfo(targetInfo);
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
    } else {
        desc.fColorAttachment.fTextureInfo = targetInfo;
        desc.fColorAttachment.fLoadOp = loadOp;
        desc.fColorAttachment.fStoreOp = storeOp;
    }
    desc.fClearColor = drawPass->clearColor();

    if (drawPass->depthStencilFlags() != DepthStencilFlags::kNone) {
        desc.fDepthStencilAttachment.fTextureInfo = caps->getDefaultDepthStencilTextureInfo(
                drawPass->depthStencilFlags(),
                desc.fColorAttachment.fTextureInfo.numSamples(),
                Protected::kNo);
        // Always clear the depth and stencil to 0 at the start of a DrawPass, but discard at the
        // end since their contents do not affect the next frame.
        desc.fDepthStencilAttachment.fLoadOp = LoadOp::kClear;
        desc.fClearDepth = 0.f;
        desc.fClearStencil = 0;
        desc.fDepthStencilAttachment.fStoreOp = StoreOp::kDiscard;
    }

    sk_sp<TextureProxy> targetProxy = sk_ref_sp(fDrawPasses[0]->target());
    return RenderPassTask::Make(std::move(fDrawPasses), desc, std::move(targetProxy));
}

sk_sp<Task> DrawContext::snapUploadTask(Recorder* recorder) {
    if (!fPendingUploads || fPendingUploads->size() == 0) {
        return nullptr;
    }

    sk_sp<Task> uploadTask = UploadTask::Make(fPendingUploads.get());

    fPendingUploads = std::make_unique<UploadList>();

    return uploadTask;
}

#ifdef SK_ENABLE_PIET_GPU
sk_sp<Task> DrawContext::snapPietRenderTask(Recorder* recorder) {
    if (fPendingPietRenders.empty()) {
        return nullptr;
    }
    return sk_sp<Task>(new PietRenderTask(std::move(fPendingPietRenders)));
}
#endif

} // namespace skgpu::graphite
