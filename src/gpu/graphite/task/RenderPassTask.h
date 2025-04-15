/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_task_RenderPassTask_DEFINED
#define skgpu_graphite_task_RenderPassTask_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/DrawPass.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/task/Task.h"

#include <memory>

namespace skgpu::graphite {

class TextureProxy;
class CommandBuffer;
class Context;
class ResourceProvider;
class RuntimeEffectDictionary;
class ScratchResourceManager;

/**
 * RenderPassTask handles preparing and recording DrawLists into a single render pass within a
 * command buffer. If the backend supports subpasses, and the DrawLists/surfaces are compatible, a
 * RenderPassTask can execute multiple DrawLists across different surfaces as subpasses nested
 * within a single render pass. If there is no such support, a RenderPassTask is one-to-one with a
 * "render pass" to specific surface.
 */
class RenderPassTask final : public Task {
public:
    using DrawPassList = skia_private::STArray<1, std::unique_ptr<DrawPass>>;

    // dstCopy should only be provided if the draw passes require a texture copy
    // for dst reads and must cover the union of all `DrawPass::dstReadBounds()` values in the
    // render pass. It is assumed that the copy's (0,0) texel matches the top-left corner of the
    // pass's dst copy bounds. The copy can be larger than the required bounds.
    static sk_sp<RenderPassTask> Make(DrawPassList,
                                      const RenderPassDesc&,
                                      sk_sp<TextureProxy> target,
                                      sk_sp<TextureProxy> dstCopy,
                                      SkIRect dstReadBounds);

    ~RenderPassTask() override;

    Status prepareResources(ResourceProvider*,
                            ScratchResourceManager*,
                            const RuntimeEffectDictionary*) override;

    Status addCommands(Context*, CommandBuffer*, ReplayTargetData) override;

private:
    RenderPassTask(DrawPassList,
                   const RenderPassDesc&,
                   sk_sp<TextureProxy> target,
                   sk_sp<TextureProxy> dstCopy,
                   SkIRect dstReadBounds);

    DrawPassList fDrawPasses;
    RenderPassDesc fRenderPassDesc;
    sk_sp<TextureProxy> fTarget;

    sk_sp<TextureProxy> fDstCopy;
    SkIRect fDstReadBounds;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_task_RenderPassTask_DEFINED
