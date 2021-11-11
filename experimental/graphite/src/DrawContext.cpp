/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/DrawContext.h"

#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/DrawList.h"
#include "experimental/graphite/src/DrawPass.h"
#include "experimental/graphite/src/RenderPassTask.h"
#include "experimental/graphite/src/TextureProxy.h"
#include "experimental/graphite/src/geom/BoundsManager.h"
#include "experimental/graphite/src/geom/Shape.h"

namespace skgpu {

sk_sp<DrawContext> DrawContext::Make(sk_sp<TextureProxy> target,
                                     sk_sp<SkColorSpace> colorSpace,
                                     SkColorType colorType,
                                     SkAlphaType alphaType) {
    if (!target) {
        return nullptr;
    }

    // TODO: validate that the color type and alpha type are compatible with the target's info
    SkImageInfo imageInfo = SkImageInfo::Make(target->dimensions(),
                                              colorType,
                                              alphaType,
                                              std::move(colorSpace));
    return sk_sp<DrawContext>(new DrawContext(std::move(target), imageInfo));
}

DrawContext::DrawContext(sk_sp<TextureProxy> target, const SkImageInfo& ii)
        : fTarget(std::move(target))
        , fImageInfo(ii)
        , fPendingDraws(std::make_unique<DrawList>()) {
    // TBD - Will probably want DrawLists (and its internal commands) to come from an arena
    // that the SDC manages.
}

DrawContext::~DrawContext() {
    // If the SDC is destroyed and there are pending commands, they won't be drawn. Maybe that's ok
    // but for now consider it a bug for not calling snapDrawTask() and snapRenderPassTask()
    // TODO: determine why these asserts are firing on the GMs and re-enable
//    SkASSERT(fPendingDraws->drawCount() == 0);
//    SkASSERT(fDrawPasses.empty());
}

void DrawContext::stencilAndFillPath(const Transform& localToDevice,
                                     const Shape& shape,
                                     const Clip& clip,
                                     DrawOrder order,
                                     const PaintParams* paint)  {
    SkASSERT(SkIRect::MakeSize(fTarget->dimensions()).contains(clip.scissor()));
    fPendingDraws->stencilAndFillPath(localToDevice, shape, clip, order,paint);
}

void DrawContext::fillConvexPath(const Transform& localToDevice,
                                 const Shape& shape,
                                 const Clip& clip,
                                 DrawOrder order,
                                 const PaintParams* paint) {
    SkASSERT(SkIRect::MakeSize(fTarget->dimensions()).contains(clip.scissor()));
    fPendingDraws->fillConvexPath(localToDevice, shape, clip, order, paint);
}

void DrawContext::strokePath(const Transform& localToDevice,
                             const Shape& shape,
                             const StrokeParams& stroke,
                             const Clip& clip,
                             DrawOrder order,
                             const PaintParams* paint) {
    SkASSERT(SkIRect::MakeSize(fTarget->dimensions()).contains(clip.scissor()));
    fPendingDraws->strokePath(localToDevice, shape, stroke, clip, order, paint);
}

void DrawContext::snapDrawPass(Recorder* recorder, const BoundsManager* occlusionCuller) {
    if (fPendingDraws->drawCount() == 0) {
        return;
    }

    auto pass = DrawPass::Make(recorder, std::move(fPendingDraws), fTarget, occlusionCuller);
    fDrawPasses.push_back(std::move(pass));
    fPendingDraws = std::make_unique<DrawList>();
}

sk_sp<Task> DrawContext::snapRenderPassTask(Recorder* recorder,
                                            const BoundsManager* occlusionCuller) {
    this->snapDrawPass(recorder, occlusionCuller);
    if (fDrawPasses.empty()) {
        return nullptr;
    }

    // TODO: At this point we would determine all the targets used by the drawPasses,
    // build up the union of them and store them in the RenderPassDesc. However, for
    // the moment we should have only one drawPass.
    SkASSERT(fDrawPasses.size() == 1);
    RenderPassDesc desc;
    desc.fColorAttachment.fTextureProxy = sk_ref_sp(fDrawPasses[0]->target());
    desc.fColorAttachment.fLoadOp = LoadOp::kLoad;
    desc.fColorAttachment.fStoreOp = StoreOp::kStore;

    return RenderPassTask::Make(std::move(fDrawPasses), desc);
}

} // namespace skgpu
