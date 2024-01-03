/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DescriptorTypes_DEFINED
#define skgpu_graphite_DescriptorTypes_DEFINED

#include "src/base/SkEnumBitMask.h"

namespace skgpu::graphite {

/**
 * Types of descriptors supported within graphite
*/
enum class DescriptorType : uint8_t {
    kUniformBuffer = 0,
    kTextureSampler,
    kTexture,
    kCombinedTextureSampler,
    kStorageBuffer,
    kInputAttachment,

    kLast = kInputAttachment,
};
static constexpr int kDescriptorTypeCount = (int)(DescriptorType::kLast) + 1;

enum class PipelineStageFlags : uint8_t {
    kVertexShader = 0b001,
    kFragmentShader = 0b010,
    kCompute = 0b100,
};
SK_MAKE_BITMASK_OPS(PipelineStageFlags);

struct DescriptorData {
    DescriptorData(DescriptorType descType,
                   uint32_t descCount,
                   int bindingIdx,
                   SkEnumBitMask<PipelineStageFlags> stageFlags)
            : type (descType)
            , count (descCount)
            , bindingIndex (bindingIdx)
            , pipelineStageFlags(stageFlags) {}

    DescriptorType type;
    uint32_t count;
    int bindingIndex;
    SkEnumBitMask<PipelineStageFlags> pipelineStageFlags;
};

};  // namespace skgpu::graphite

#endif // skgpu_graphite_DescriptorTypes_DEFINED
