/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_RenderPassDesc_DEFINED
#define skgpu_graphite_RenderPassDesc_DEFINED

#include "include/core/SkString.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/TextureFormat.h"

#include "src/gpu/Swizzle.h"

#include <array>

namespace skgpu::graphite {

class Caps;
class TextureInfo;

struct AttachmentDesc {
    TextureFormat fFormat = TextureFormat::kUnsupported;
    LoadOp fLoadOp = LoadOp::kDiscard;
    StoreOp fStoreOp = StoreOp::kDiscard;
    // NOTE: GPU-supported sample counts should always fit in a byte, and this lets AttachmentDesc
    // stay at 32-bits given the backing types of TextureFormat and Load/StoreOp.
    uint8_t fSampleCount = 1;

    bool operator==(const AttachmentDesc& other) const {
        if (fFormat == TextureFormat::kUnsupported &&
            other.fFormat == TextureFormat::kUnsupported) {
            return true;
        }

        return fFormat == other.fFormat &&
               fLoadOp == other.fLoadOp &&
               fStoreOp == other.fStoreOp &&
               fSampleCount == other.fSampleCount;
    }
    bool operator!=(const AttachmentDesc& other) const { return !(*this == other); }

    bool isCompatible(const TextureInfo&) const;

    SkString toString() const;
};
static_assert(sizeof(AttachmentDesc) == sizeof(uint32_t));

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
        return (fWriteSwizzle == other.fWriteSwizzle &&
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
    AttachmentDesc fColorResolveAttachment;
    AttachmentDesc fDepthStencilAttachment;

    // The write swizzle is applied in shader, so affects SkSL code generation, but is determined by
    // the desired SkColorType semantics and target TextureFormat combination of the render pass.
    Swizzle fWriteSwizzle;

    // The overall sample count of the render pass
    uint8_t fSampleCount;

    // The remaining fields are set on renderpasses, but don't change the structure of the pass.

    // Each renderpass determines what strategy to use for reading the dst texture. If no draws
    // within the renderpass require a dst read, this is set to be kNoneRequired. If any draw does
    // read from the dst, then each pipeline used by this RP independently determines if a dst read
    // is needed. When required, this strategy determines how to perform it.
    DstReadStrategy fDstReadStrategy;

    std::array<float, 4> fClearColor;
    float fClearDepth = 0.f;
    uint32_t fClearStencil = 0;

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
