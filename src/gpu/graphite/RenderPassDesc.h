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

    bool operator==(const AttachmentDesc& other) const {
        if (!fTextureInfo.isValid() && !other.fTextureInfo.isValid()) {
            return true;
        }

        return fTextureInfo == other.fTextureInfo &&
               fLoadOp == other.fLoadOp &&
               fStoreOp == other.fStoreOp;
    }

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
                               Swizzle writeSwizzle,
                               const DstReadStrategy);

    bool operator==(const RenderPassDesc& other) const {
        return (fSampleCount == other.fSampleCount &&
                fWriteSwizzle == other.fWriteSwizzle &&
                fClearDepth == other.fClearDepth &&
                fClearColor == other.fClearColor &&
                fColorAttachment == other.fColorAttachment &&
                fColorResolveAttachment == other.fColorResolveAttachment &&
                fDepthStencilAttachment == other.fDepthStencilAttachment &&
                fDstReadStrategy == other.fDstReadStrategy);
    }

    bool operator!=(const RenderPassDesc& other) const {
        return !(*this == other);
    }

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

    // Each renderpass determines what strategy to use for reading the dst texture. If no draws
    // within the renderpass require a dst read, this is set to be kNoneRequired. If any draw does
    // read from the dst, then each pipeline used by this RP independently determines if a dst read
    // is needed. When required, this strategy determines how to perform it.
    DstReadStrategy fDstReadStrategy;

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
