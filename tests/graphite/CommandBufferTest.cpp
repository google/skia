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
    auto gpu = context->priv().gpu();
    REPORTER_ASSERT(reporter, gpu);

#if GRAPHITE_TEST_UTILS && CAPTURE_COMMANDBUFFER
    gpu->testingOnly_startCapture();
#endif
    auto commandBuffer = gpu->resourceProvider()->createCommandBuffer();

    SkISize textureSize = { 1024, 768 };
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
    renderPassDesc.fClearColor = { 1, 0, 0, 1 };

    commandBuffer->beginRenderPass(renderPassDesc);

    RenderPipelineDesc pipelineDesc;
    auto renderPipeline = gpu->resourceProvider()->findOrCreateRenderPipeline(pipelineDesc);
    commandBuffer->setRenderPipeline(std::move(renderPipeline));
    commandBuffer->draw(PrimitiveType::kTriangleStrip, 0, 4);

    commandBuffer->endRenderPass();

    sk_sp<Buffer> buffer = gpu->resourceProvider()->findOrCreateBuffer(1024*768*4,
                                                                       BufferType::kXferGpuToCpu,
                                                                       PrioritizeGpuReads::kNo);
    REPORTER_ASSERT(reporter, buffer);
    SkIRect srcRect = { 0, 0, 1024, 768 };
    size_t rowBytes = 1024*4;
    commandBuffer->copyTextureToBuffer(texture, srcRect, buffer, 0, rowBytes);

    bool result = gpu->submit(commandBuffer);
    REPORTER_ASSERT(reporter, result);

    gpu->checkForFinishedWork(skgpu::SyncToCpu::kYes);
    uint32_t* pixels = (uint32_t*)(buffer->map());
    REPORTER_ASSERT(reporter, pixels[0] == 0xffff0000);

#if GRAPHITE_TEST_UTILS && CAPTURE_COMMANDBUFFER
    gpu->testingOnly_endCapture();
#endif
}
