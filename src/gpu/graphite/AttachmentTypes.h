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
                               bool requiresMSAA);

    AttachmentDesc fColorAttachment;
    std::array<float, 4> fClearColor;
    AttachmentDesc fColorResolveAttachment;

    AttachmentDesc fDepthStencilAttachment;
    float fClearDepth;
    uint32_t fClearStencil;

    // TODO:
    // * bounds (TBD whether exact bounds vs. granular)
    // * input attachments
};

};  // namespace skgpu::graphite

#endif // skgpu_graphite_AttachmentTypes_DEFINED
