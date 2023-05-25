/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DescriptorTypes_DEFINED
#define skgpu_graphite_DescriptorTypes_DEFINED

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

struct DescTypeAndCount {
    DescTypeAndCount(DescriptorType descType, uint32_t descCount)
            : type (descType), count (descCount) {}

    DescriptorType type;
    uint32_t count;
};

};  // namespace skgpu::graphite

#endif // skgpu_graphite_DescriptorTypes_DEFINED
