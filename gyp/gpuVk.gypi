#
# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#

# Include this gypi to include all 'gpu' files
# The parent gyp/gypi file must define
#       'skia_src_path'     e.g. skia/trunk/src
#       'skia_include_path' e.g. skia/trunk/include
#
# The skia build defines these in common_variables.gypi
#
{
  'variables': {
    'skgpu_vk_sources': [
      '../include/gpu/vk/GrVkInterface.h',
      '../src/gpu/vk/GrVkBuffer.cpp',
      '../src/gpu/vk/GrVkBuffer.h',
      '../src/gpu/vk/GrVkCaps.cpp',
      '../src/gpu/vk/GrVkCaps.h',
      '../src/gpu/vk/GrVkCommandBuffer.cpp',
      '../src/gpu/vk/GrVkCommandBuffer.h',
      '../src/gpu/vk/GrVkDescriptorPool.cpp',
      '../src/gpu/vk/GrVkDescriptorPool.h',
      '../src/gpu/vk/GrVkFramebuffer.cpp',
      '../src/gpu/vk/GrVkFramebuffer.h',
      '../src/gpu/vk/GrVkGpu.cpp',
      '../src/gpu/vk/GrVkGpu.h',
      '../src/gpu/vk/GrVkImage.cpp',
      '../src/gpu/vk/GrVkImage.h',
      '../src/gpu/vk/GrVkImageView.cpp',
      '../src/gpu/vk/GrVkImageView.h',
      '../src/gpu/vk/GrVkIndexBuffer.cpp',
      '../src/gpu/vk/GrVkIndexBuffer.h',
      '../src/gpu/vk/GrVkInterface.cpp',
      '../src/gpu/vk/GrVkMemory.cpp',
      '../src/gpu/vk/GrVkMemory.h',
      '../src/gpu/vk/GrVkPipeline.cpp',
      '../src/gpu/vk/GrVkPipeline.h',
      '../src/gpu/vk/GrVkProgram.cpp',
      '../src/gpu/vk/GrVkProgram.h',
      '../src/gpu/vk/GrVkProgramBuilder.cpp',
      '../src/gpu/vk/GrVkProgramBuilder.h',
      '../src/gpu/vk/GrVkProgramDataManager.cpp',
      '../src/gpu/vk/GrVkProgramDataManager.h',
      '../src/gpu/vk/GrVkProgramDesc.cpp',
      '../src/gpu/vk/GrVkProgramDesc.h',
      '../src/gpu/vk/GrVkRenderPass.cpp',
      '../src/gpu/vk/GrVkRenderPass.h',
      '../src/gpu/vk/GrVkRenderTarget.cpp',
      '../src/gpu/vk/GrVkRenderTarget.h',
      '../src/gpu/vk/GrVkResource.h',
      '../src/gpu/vk/GrVkResourceProvider.cpp',
      '../src/gpu/vk/GrVkResourceProvider.h',
      '../src/gpu/vk/GrVkSampler.cpp',
      '../src/gpu/vk/GrVkSampler.h',
      '../src/gpu/vk/GrVkStencilAttachment.cpp',
      '../src/gpu/vk/GrVkStencilAttachment.h',
      '../src/gpu/vk/GrVkTexture.cpp',
      '../src/gpu/vk/GrVkTexture.h',
      '../src/gpu/vk/GrVkTextureRenderTarget.cpp',
      '../src/gpu/vk/GrVkTextureRenderTarget.h',
      '../src/gpu/vk/GrVkTransferBuffer.cpp',
      '../src/gpu/vk/GrVkTransferBuffer.h',
      '../src/gpu/vk/GrVkUniformBuffer.cpp',
      '../src/gpu/vk/GrVkUniformBuffer.h',
      '../src/gpu/vk/GrVkUniformHandler.cpp',
      '../src/gpu/vk/GrVkUniformHandler.h',
      '../src/gpu/vk/GrVkUtil.cpp',
      '../src/gpu/vk/GrVkUtil.h',
      '../src/gpu/vk/GrVkVaryingHandler.cpp',
      '../src/gpu/vk/GrVkVaryingHandler.h',
      '../src/gpu/vk/GrVkVertexBuffer.cpp',
      '../src/gpu/vk/GrVkVertexBuffer.h',

#      '../testfiles/vktest.cpp',
    ],
  },
}
