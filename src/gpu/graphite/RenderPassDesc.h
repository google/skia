/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_RenderPassDesc_DEFINED
#define skgpu_graphite_RenderPassDesc_DEFINED

#include "include/core/SkString.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/gpu/graphite/ResourceTypes.h"

#include "src/gpu/Swizzle.h"

#include <array>

namespace skgpu::graphite {

class Caps;

struct AttachmentDesc {
    TextureInfo fTextureInfo;
    LoadOp fLoadOp;
    StoreOp fStoreOp;

    SkString toString() const;
};

struct RenderPassDesc {
    static RenderPassDesc Make(const Caps* caps,
                               const TextureInfo& targetInfo,
                               LoadOp loadOp,
                               StoreOp storeOp,
                               SkEnumBitMask<DepthStencilFlags> depthStencilFlags,
                               const std::array<float, 4>& clearColor,
                               bool requiresMSAA,
                               Swizzle writeSwizzle);

    AttachmentDesc fColorAttachment;
    std::array<float, 4> fClearColor;
    AttachmentDesc fColorResolveAttachment;

    AttachmentDesc fDepthStencilAttachment;
    float fClearDepth;
    uint32_t fClearStencil;

    Swizzle fWriteSwizzle;

    // This samples count usually matches fColorAttachment & fDepthStencilAttachment's samples
    // count. The only exceptional case is when multisampled render to single sampled is used. In
    // that case, the fColorAttachment's samples count will be 1 and fSampleCount will be > 1.
    uint32_t fSampleCount;

    SkString toString() const;
    // Only includes fixed state relevant to pipeline creation
    SkString toPipelineLabel() const;

    // TODO:
    // * bounds (TBD whether exact bounds vs. granular)
    // * input attachments
    // * subpass makeup information
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_RenderPassDesc_DEFINED
