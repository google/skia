/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/vk/precompile/VulkanPrecompileShader.h"

#include "include/gpu/vk/VulkanTypes.h"
#include "src/gpu/graphite/precompile/PrecompileImageShader.h"
#include "src/gpu/graphite/vk/VulkanYcbcrConversion.h"

namespace skgpu::graphite {

sk_sp<PrecompileShader> PrecompileShaders::VulkanYCbCrImage(
        skgpu::VulkanYcbcrConversionInfo& YCbCrConversionInfo,
        ImageShaderFlags shaderFlags,
        SkSpan<const SkColorInfo> colorInfos,
        SkSpan<const SkTileMode> tileModes) {
    if (!YCbCrConversionInfo.isValid()) {
        return nullptr;
    }

    sk_sp<PrecompileImageShader> shader(new PrecompileImageShader(shaderFlags,
                                                                  colorInfos,
                                                                  tileModes,
                                                                  /* raw= */false));

    shader->setImmutableSamplerInfo(
            VulkanYcbcrConversion::ToImmutableSamplerInfo(YCbCrConversionInfo));
    return PrecompileShaders::LocalMatrix({ std::move(shader) });
}


} // namespace skgpu::graphite
