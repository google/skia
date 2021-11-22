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
#include "experimental/graphite/src/DrawBufferManager.h"
#include "experimental/graphite/src/DrawWriter.h"
#include "experimental/graphite/src/Gpu.h"
#include "experimental/graphite/src/GraphicsPipeline.h"
#include "experimental/graphite/src/ResourceProvider.h"
#include "experimental/graphite/src/Texture.h"
#include "experimental/graphite/src/TextureProxy.h"

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
        false,  // framebufferOnly
    };
    TextureInfo textureInfo(mtlTextureInfo);
#else
    TextureInfo textureInfo;
#endif

    auto target = sk_sp<TextureProxy>(new TextureProxy(textureSize, textureInfo));
    REPORTER_ASSERT(reporter, target);

    RenderPassDesc renderPassDesc = {};
    renderPassDesc.fColorAttachment.fTextureProxy = target;
    renderPassDesc.fColorAttachment.fLoadOp = LoadOp::kClear;
    renderPassDesc.fColorAttachment.fStoreOp = StoreOp::kStore;
    renderPassDesc.fClearColor = { 1, 0, 0, 1 }; // red

    target->instantiate(gpu->resourceProvider());
    DrawBufferManager bufferMgr(gpu->resourceProvider(), 4);


    commandBuffer->beginRenderPass(renderPassDesc);

    DrawWriter drawWriter(commandBuffer->asDrawDispatcher(), &bufferMgr);

    // Shared uniform buffer
    struct UniformData {
        SkPoint fScale;
        SkPoint fTranslate;
        SkColor4f fColor;
    };
    sk_sp<Buffer> uniformBuffer = gpu->resourceProvider()->findOrCreateBuffer(
            2*sizeof(UniformData), BufferType::kUniform, PrioritizeGpuReads::kNo);
    size_t uniformOffset = 0;

    // Draw blue rectangle over entire rendertarget (which was red)
    GraphicsPipelineDesc pipelineDesc;
    pipelineDesc.setTestingOnlyShaderIndex(0);
    auto graphicsPipeline = gpu->resourceProvider()->findOrCreateGraphicsPipeline(pipelineDesc);
    drawWriter.newPipelineState(PrimitiveType::kTriangleStrip,
                                pipelineDesc.vertexStride(),
                                pipelineDesc.instanceStride());
    commandBuffer->bindGraphicsPipeline(std::move(graphicsPipeline));
    drawWriter.draw({}, {}, 4);

    // Draw inset yellow rectangle using uniforms
    pipelineDesc.setTestingOnlyShaderIndex(1);
    graphicsPipeline = gpu->resourceProvider()->findOrCreateGraphicsPipeline(pipelineDesc);
    drawWriter.newPipelineState(PrimitiveType::kTriangleStrip,
                                pipelineDesc.vertexStride(),
                                pipelineDesc.instanceStride());
    commandBuffer->bindGraphicsPipeline(std::move(graphicsPipeline));
    UniformData* uniforms = (UniformData*)uniformBuffer->map();
    uniforms->fScale = SkPoint({1.8, 1.8});
    uniforms->fTranslate = SkPoint({-0.9, -0.9});
    uniforms->fColor = SkColors::kYellow;
    commandBuffer->bindUniformBuffer(UniformSlot::kRenderStep, uniformBuffer, uniformOffset);
    drawWriter.draw({}, {}, 4);
    uniformOffset += sizeof(UniformData);
    ++uniforms;

    // Draw inset magenta rectangle with triangles in vertex buffer
    pipelineDesc.setTestingOnlyShaderIndex(2);
    skgpu::GraphicsPipelineDesc::Attribute vertexAttributes[1] = {
        { "position", VertexAttribType::kFloat2, SLType::kFloat2 }
    };
    pipelineDesc.setVertexAttributes(vertexAttributes, 1);
    graphicsPipeline = gpu->resourceProvider()->findOrCreateGraphicsPipeline(pipelineDesc);
    drawWriter.newPipelineState(PrimitiveType::kTriangles,
                                pipelineDesc.vertexStride(),
                                pipelineDesc.instanceStride());
    commandBuffer->bindGraphicsPipeline(std::move(graphicsPipeline));

    auto [vertexWriter, vertices] = bufferMgr.getVertexWriter(4 * pipelineDesc.vertexStride());
    vertexWriter << SkPoint{0.25f, 0.25f}
                    << SkPoint{0.25f, 0.75f}
                    << SkPoint{0.75f, 0.25f}
                    << SkPoint{0.75f, 0.75f};
    auto [indexWriter, indices] = bufferMgr.getIndexWriter(6 * sizeof(uint16_t));
    indexWriter << 0 << 1 << 2
                << 2 << 1 << 3;
    uniforms->fScale = SkPoint({2, 2});
    uniforms->fTranslate = SkPoint({-1, -1});
    uniforms->fColor = SkColors::kMagenta;
    commandBuffer->bindUniformBuffer(UniformSlot::kRenderStep, uniformBuffer, uniformOffset);

    drawWriter.draw(vertices, indices, 6);

    // draw rects using instance buffer
    pipelineDesc.setTestingOnlyShaderIndex(3);
    skgpu::GraphicsPipelineDesc::Attribute instanceAttributes[3] = {
        { "position", VertexAttribType::kFloat2, SLType::kFloat2 },
        { "dims", VertexAttribType::kFloat2, SLType::kFloat2 },
        { "color", VertexAttribType::kFloat4, SLType::kFloat4 }
    };
    pipelineDesc.setVertexAttributes(nullptr, 0);
    pipelineDesc.setInstanceAttributes(instanceAttributes, 3);
    graphicsPipeline = gpu->resourceProvider()->findOrCreateGraphicsPipeline(pipelineDesc);
    drawWriter.newPipelineState(PrimitiveType::kTriangles,
                                pipelineDesc.vertexStride(),
                                pipelineDesc.instanceStride());
    commandBuffer->bindGraphicsPipeline(std::move(graphicsPipeline));

    drawWriter.setInstanceTemplate({}, indices, 6);
    auto instanceWriter = drawWriter.appendInstances(2);
    instanceWriter << SkPoint{-0.4f, -0.4f}  << SkPoint{0.4f, 0.4f}   << SkColors::kGreen
                    << SkPoint{0.f, 0.f}      << SkPoint{0.25f, 0.25f} << SkColors::kCyan;

    drawWriter.flush();
    uniformBuffer->unmap();

    bufferMgr.transferToCommandBuffer(commandBuffer.get());
    commandBuffer->endRenderPass();

    // Do readback

    // TODO: add 4-byte transfer buffer alignment for Mac to Caps
    //       add bpp to Caps
    size_t rowBytes = 4*kTextureWidth;
    size_t bufferSize = rowBytes*kTextureHeight;
    sk_sp<Buffer> copyBuffer = gpu->resourceProvider()->findOrCreateBuffer(
            bufferSize, BufferType::kXferGpuToCpu, PrioritizeGpuReads::kNo);
    REPORTER_ASSERT(reporter, copyBuffer);
    SkIRect srcRect = { 0, 0, kTextureWidth, kTextureHeight };
    commandBuffer->copyTextureToBuffer(target->refTexture(), srcRect, copyBuffer, 0, rowBytes);

    bool result = gpu->submit(commandBuffer);
    REPORTER_ASSERT(reporter, result);

    gpu->checkForFinishedWork(skgpu::SyncToCpu::kYes);
    uint32_t* pixels = (uint32_t*)(copyBuffer->map());
    REPORTER_ASSERT(reporter, pixels[0] == 0xffff0000);
    REPORTER_ASSERT(reporter, pixels[51 + 38*kTextureWidth] == 0xff00ffff);
    REPORTER_ASSERT(reporter, pixels[256 + 192*kTextureWidth] == 0xffff00ff);
    copyBuffer->unmap();

#if GRAPHITE_TEST_UTILS && CAPTURE_COMMANDBUFFER
    gpu->testingOnly_endCapture();
#endif
}
