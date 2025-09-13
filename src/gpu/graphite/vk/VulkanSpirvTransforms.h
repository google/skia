/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanSpirvTransforms_DEFINED
#define skgpu_graphite_VulkanSpirvTransforms_DEFINED

#include "src/sksl/codegen/SkSLNativeShader.h"

namespace skgpu::graphite {

/*
 * The SPIR-V transformer is capable of applying multiple transformations in one pass. However,
 * there is currently only one transformation needed. The SPIRVTransformOptions controls which
 * transformations to apply, and is provided for extensibility.
 */
struct SPIRVTransformOptions {
    // Adjust the SPIR-V to support loading from input attachments when multisampled.
    bool fMultisampleInputLoad = false;
};

SkSL::NativeShader TransformSPIRV(const SkSL::NativeShader& spirv,
                                  const SPIRVTransformOptions& options);

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_VulkanSpirvTransforms_DEFINED
