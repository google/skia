/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/src/ContextPriv.h"

#include "experimental/graphite/include/mtl/MtlTypes.h"
#include "experimental/graphite/src/Buffer.h"
#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/Gpu.h"
#include "experimental/graphite/src/RenderPipeline.h"
#include "experimental/graphite/src/ResourceProvider.h"
#include "experimental/graphite/src/Texture.h"

#if GRAPHITE_TEST_UTILS
// set to 1 if you want to do GPU capture of the commandBuffer
#define CAPTURE_COMMANDBUFFER 0
#endif

using namespace skgpu;

/*
 * This is to test the various pieces of the CommandBuffer interface.
 */
DEF_GRAPHITE_TEST_FOR_CONTEXTS(CommandBufferTest, reporter, context) {
    constexpr int kTextureWidth = 1024;
    constexpr int kTextureHeight = 768;

    auto gpu = context->priv().gpu();
    REPORTER_ASSERT(reporter, gpu);

#if GRAPHITE_TEST_UTILS && CAPTURE_COMMANDBUFFER
    gpu->testingOnly_startCapture();
#endif
    auto commandBuffer = gpu->resourceProvider()->createCommandBuffer();

    SkISize textureSize = { kTextureWidth, kTextureHeight };
#ifdef SK_METAL
    skgpu::mtl::TextureInfo mtlTextureInfo = {
        1,
        1,
        70,     // MTLPixelFormatRGBA8Unorm
        0x0005, // MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead
        2,      // MTLStorageModePrivate
    };
    TextureInfo textureInfo(mtlTextureInfo);
#else
    TextureInfo textureInfo;
#endif

    sk_sp<Texture> texture = gpu->resourceProvider()->findOrCreateTexture(textureSize,
                                                                          textureInfo);
    REPORTER_ASSERT(reporter, texture);

    RenderPassDesc renderPassDesc = {};
    renderPassDesc.fColorAttachment.fTexture = texture;
    renderPassDesc.fColorAttachment.fLoadOp = LoadOp::kClear;
    renderPassDesc.fColorAttachment.fStoreOp = StoreOp::kStore;
    renderPassDesc.fClearColor = { 1, 0, 0, 1 }; // red

    commandBuffer->beginRenderPass(renderPassDesc);

    RenderPipelineDesc pipelineDesc;
    pipelineDesc.setTestingOnlyShaderIndex(1);
    auto renderPipeline = gpu->resourceProvider()->findOrCreateRenderPipeline(pipelineDesc);
    commandBuffer->bindRenderPipeline(std::move(renderPipeline));
    struct UniformData {
        float fPosXform[4];
        float fColor[4];
    };
    sk_sp<Buffer> uniformBuffer = gpu->resourceProvider()->findOrCreateBuffer(
            sizeof(UniformData), BufferType::kUniform, PrioritizeGpuReads::kNo);
    UniformData* uniforms = (UniformData*)uniformBuffer->map();
    uniforms->fPosXform[0] = 2;
    uniforms->fPosXform[1] = 2;
    uniforms->fPosXform[2] = -1;
    uniforms->fPosXform[3] = -1;
    uniforms->fColor[0] = 1;
    uniforms->fColor[1] = 1;
    uniforms->fColor[2] = 0;
    uniforms->fColor[3] = 1;
    uniformBuffer->unmap();
    commandBuffer->bindUniformBuffer(uniformBuffer, 0);
    commandBuffer->draw(PrimitiveType::kTriangleStrip, 0, 4);

    commandBuffer->endRenderPass();

    // TODO: add 4-byte transfer buffer alignment for Mac to Caps
    //       add bpp to Caps
    size_t rowBytes = 4*kTextureWidth;
    size_t bufferSize = rowBytes*kTextureHeight;
    sk_sp<Buffer> copyBuffer = gpu->resourceProvider()->findOrCreateBuffer(
            bufferSize, BufferType::kXferGpuToCpu, PrioritizeGpuReads::kNo);
    REPORTER_ASSERT(reporter, copyBuffer);
    SkIRect srcRect = { 0, 0, kTextureWidth, kTextureHeight };
    commandBuffer->copyTextureToBuffer(texture, srcRect, copyBuffer, 0, rowBytes);

    bool result = gpu->submit(commandBuffer);
    REPORTER_ASSERT(reporter, result);

    gpu->checkForFinishedWork(skgpu::SyncToCpu::kYes);
    uint32_t* pixels = (uint32_t*)(copyBuffer->map());
    REPORTER_ASSERT(reporter, pixels[0] == 0xff00ffff);
    copyBuffer->unmap();

#if GRAPHITE_TEST_UTILS && CAPTURE_COMMANDBUFFER
    gpu->testingOnly_endCapture();
#endif
}
