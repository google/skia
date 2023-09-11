/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_AttachmentTypes_DEFINED
#define skgpu_graphite_AttachmentTypes_DEFINED

#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/gpu/graphite/ResourceTypes.h"

#include "src/gpu/Swizzle.h"

#include <array>

namespace skgpu::graphite {

class Caps;

/**
 * This enum is used to specify the load operation to be used when a RenderPass begins execution
 */
enum class LoadOp : uint8_t {
    kLoad,
    kClear,
    kDiscard,

    kLast = kDiscard
};
inline static constexpr int kLoadOpCount = (int)(LoadOp::kLast) + 1;

/**
 * This enum is used to specify the store operation to be used when a RenderPass ends execution.
 */
enum class StoreOp : uint8_t {
    kStore,
    kDiscard,

    kLast = kDiscard
};
inline static constexpr int kStoreOpCount = (int)(StoreOp::kLast) + 1;

struct AttachmentDesc {
    TextureInfo fTextureInfo;
    LoadOp fLoadOp;
    StoreOp fStoreOp;
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

    // TODO:
    // * bounds (TBD whether exact bounds vs. granular)
    // * input attachments
};

};  // namespace skgpu::graphite

#endif // skgpu_graphite_AttachmentTypes_DEFINED
