/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBlender.h"
#include "include/core/SkCombinationBuilder.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/mtl/MtlTypes.h"
#include "src/core/SkKeyContext.h"
#include "src/core/SkKeyHelpers.h"
#include "src/core/SkShaderCodeDictionary.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/DrawBufferManager.h"
#include "src/gpu/graphite/DrawGeometry.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/Gpu.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Sampler.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/geom/Shape.h"
#include "src/gpu/graphite/geom/Transform_graphite.h"

#if GRAPHITE_TEST_UTILS
// set to 1 if you want to do GPU capture of the commandBuffer
#define CAPTURE_COMMANDBUFFER 0
#endif

using namespace skgpu::graphite;

namespace {

const DepthStencilSettings kTestDepthStencilSettings = {
    // stencil
    {},
    {},
    0,
    true,
    // depth
    CompareOp::kAlways,
    true,
    false,
};

class UniformRectDraw final : public RenderStep {
public:
    ~UniformRectDraw() override {}

    static const RenderStep* Singleton() {
        static const UniformRectDraw kSingleton;
        return &kSingleton;
    }

    const char* vertexSkSL() const override {
        return "float2 tmpPosition = float2(float(sk_VertexID >> 1), float(sk_VertexID & 1));\n"
               "float4 devPosition = float4(tmpPosition * scale + translate, 0.0, 1.0);\n";
    }

    void writeVertices(DrawWriter* writer, const DrawGeometry&) const override {
        // The shape is upload via uniforms, so this just needs to record 4 data-less vertices
        writer->draw({}, 4);
    }

    void writeUniforms(const DrawGeometry& geom, SkPipelineDataGatherer* gatherer) const override {
        SkASSERT(geom.shape().isRect());

#ifdef SK_DEBUG
        static constexpr int kNumRectUniforms = 2;
        static constexpr SkUniform kRectUniforms[kNumRectUniforms] = {
                { "scale",      SkSLType::kFloat2 },
                { "translate",  SkSLType::kFloat2 },
        };

        UniformExpectationsValidator uev(gatherer, SkMakeSpan(kRectUniforms, kNumRectUniforms));
#endif

        // TODO: A << API for uniforms would be nice, particularly if it could take pre-computed
        // offsets for each uniform.
        gatherer->write(geom.shape().rect().size());
        gatherer->write(geom.shape().rect().topLeft());
    }

private:
    UniformRectDraw() : RenderStep("UniformRectDraw", "test-only",
                                   Flags::kPerformsShading,
                                   /*uniforms=*/{{"scale",     SkSLType::kFloat2},
                                                 {"translate", SkSLType::kFloat2}},
                                   PrimitiveType::kTriangleStrip,
                                   {{},
                                    {},
                                    0,
                                    true,
                                    CompareOp::kAlways,
                                    false,
                                    false},
                                   /*vertexAttrs=*/{},
                                   /*instanceAttrs=*/{}) {}
};

class TriangleRectDraw final : public RenderStep {
public:
    ~TriangleRectDraw() override {}

    static const RenderStep* Singleton() {
        static const TriangleRectDraw kSingleton;
        return &kSingleton;
    }

    const char* vertexSkSL() const override {
        return "float4 devPosition = float4(position * scale + translate, 0.0, 1.0);\n";
    }

    void writeVertices(DrawWriter* writer, const DrawGeometry& geom) const override {
        const Shape& shape = geom.shape();
        DrawBufferManager* bufferMgr = writer->bufferManager();
        auto [vertexWriter, vertices] = bufferMgr->getVertexWriter(4 * this->vertexStride());
        vertexWriter << 0.5f * (shape.rect().left() + 1.f)  << 0.5f * (shape.rect().top() + 1.f)
                     << 0.5f * (shape.rect().left() + 1.f)  << 0.5f * (shape.rect().bot() + 1.f)
                     << 0.5f * (shape.rect().right() + 1.f) << 0.5f * (shape.rect().top() + 1.f)
                     << 0.5f * (shape.rect().right() + 1.f) << 0.5f * (shape.rect().bot() + 1.f);

        // TODO: Would be nice to re-use this
        auto [indexWriter, indices] = bufferMgr->getIndexWriter(6 * sizeof(uint16_t));
        indexWriter << 0 << 1 << 2
                    << 2 << 1 << 3;

        writer->drawIndexed(vertices, indices, 6);
    }

    void writeUniforms(const DrawGeometry&, SkPipelineDataGatherer* gatherer) const override {
#ifdef SK_DEBUG
        static constexpr int kNumRectUniforms = 2;
        static constexpr SkUniform kRectUniforms[kNumRectUniforms] = {
                { "scale",      SkSLType::kFloat2 },
                { "translate",  SkSLType::kFloat2 },
        };
        UniformExpectationsValidator uev(gatherer, SkMakeSpan(kRectUniforms, kNumRectUniforms));
#endif

        gatherer->write(SkPoint::Make(2.0f, 2.0f));
        gatherer->write(SkPoint::Make(-1.0f, -1.0f));
    }

private:
    TriangleRectDraw()
            : RenderStep("TriangleRectDraw", "test-only",
                         Flags::kPerformsShading,
                         /*uniforms=*/{{"scale",     SkSLType::kFloat2},
                                       {"translate", SkSLType::kFloat2}},
                         PrimitiveType::kTriangles,
                         kTestDepthStencilSettings,
                         /*vertexAttrs=*/{{"position",
                                           VertexAttribType::kFloat2,
                                           SkSLType::kFloat2}},
                         /*instanceAttrs=*/{}) {}
};

class InstanceRectDraw final : public RenderStep {
public:
    ~InstanceRectDraw() override {}

    static const RenderStep* Singleton() {
        static const InstanceRectDraw kSingleton;
        return &kSingleton;
    }

    const char* vertexSkSL() const override {
        return "float2 tmpPosition = float2(float(sk_VertexID >> 1), float(sk_VertexID & 1));\n"
               "float4 devPosition = float4(tmpPosition * dims + position, 0.0, 1.0);\n";
    }

    void writeVertices(DrawWriter* writer, const DrawGeometry& geom) const override {
        SkASSERT(geom.shape().isRect());

        DrawBufferManager* bufferMgr = writer->bufferManager();

        // TODO: To truly test draw merging, this index buffer needs to remembered across
        // writeVertices calls
        auto [indexWriter, indices] = bufferMgr->getIndexWriter(6 * sizeof(uint16_t));
        indexWriter << 0 << 1 << 2
                    << 2 << 1 << 3;

        DrawWriter::Instances instances{*writer, {}, indices, 6};
        instances.append(1) << geom.shape().rect().topLeft() << geom.shape().rect().size();
    }

    void writeUniforms(const DrawGeometry&, SkPipelineDataGatherer*) const override { }

private:
    InstanceRectDraw()
            : RenderStep("InstanceRectDraw", "test-only",
                         Flags::kPerformsShading,
                         /*uniforms=*/{},
                         PrimitiveType::kTriangles,
                         kTestDepthStencilSettings,
                         /*vertexAttrs=*/{},
                         /*instanceAttrs=*/ {
                                { "position", VertexAttribType::kFloat2, SkSLType::kFloat2 },
                                { "dims",     VertexAttribType::kFloat2, SkSLType::kFloat2 }
                         }) {}
};

} // anonymous namespace

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
    auto recorder = context->makeRecorder();
    SkKeyContext keyContext(recorder.get(), {});
    auto resourceProvider = recorder->priv().resourceProvider();
    auto commandBuffer = resourceProvider->createCommandBuffer();

    SkISize textureSize = { kTextureWidth, kTextureHeight };
#ifdef SK_METAL
    skgpu::graphite::MtlTextureInfo mtlTextureInfo = {
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

    SkUniquePaintParamsID uniqueID;
    {
        auto dict = keyContext.dict();

        SkPaintParamsKeyBuilder builder(dict, SkBackend::kGraphite);

        PaintParams pp { SkColors::kRed, nullptr, nullptr };

        pp.toKey(keyContext, &builder, nullptr);

        auto entry = dict->findOrCreate(&builder);

        uniqueID = entry->uniqueID();
    }

    auto target = sk_sp<TextureProxy>(new TextureProxy(textureSize, textureInfo, SkBudgeted::kYes));
    REPORTER_ASSERT(reporter, target);

    RenderPassDesc renderPassDesc = {};
    renderPassDesc.fColorAttachment.fTextureInfo = target->textureInfo();
    renderPassDesc.fColorAttachment.fLoadOp = LoadOp::kClear;
    renderPassDesc.fColorAttachment.fStoreOp = StoreOp::kStore;
    renderPassDesc.fClearColor = { 1, 0, 0, 1 }; // red

    target->instantiate(resourceProvider);
    DrawBufferManager bufferMgr(resourceProvider, gpu->caps()->requiredUniformBufferAlignment());

    TextureInfo depthStencilInfo =
            gpu->caps()->getDefaultDepthStencilTextureInfo(DepthStencilFlags::kDepthStencil,
                                                           1,
                                                           Protected::kNo);
    renderPassDesc.fDepthStencilAttachment.fTextureInfo = depthStencilInfo;
    renderPassDesc.fDepthStencilAttachment.fLoadOp = LoadOp::kDiscard;
    renderPassDesc.fDepthStencilAttachment.fStoreOp = StoreOp::kDiscard;
    sk_sp<Texture> depthStencilTexture =
            resourceProvider->findOrCreateDepthStencilAttachment(textureSize, depthStencilInfo);

    // Create Sampler -- for now, just to test creation
    sk_sp<Sampler> sampler = resourceProvider->findOrCreateCompatibleSampler(
            SkSamplingOptions(SkFilterMode::kLinear), SkTileMode::kClamp, SkTileMode::kDecal);
    REPORTER_ASSERT(reporter, sampler);

    commandBuffer->beginRenderPass(renderPassDesc, target->refTexture(), nullptr,
                                   depthStencilTexture);

    commandBuffer->setViewport(0.f, 0.f, kTextureWidth, kTextureHeight);

    DrawWriter drawWriter(commandBuffer->asDrawDispatcher(), &bufferMgr);

    struct RectAndColor {
        SkRect    fRect;
        SkColor4f fColor;
    };

    SkPipelineDataGatherer gatherer(Layout::kMetal);

    auto draw = [&](const RenderStep* step, std::vector<RectAndColor> draws) {
        GraphicsPipelineDesc pipelineDesc;
        pipelineDesc.setProgram(step, uniqueID);
        drawWriter.newPipelineState(step->primitiveType(),
                                    step->vertexStride(),
                                    step->instanceStride());
        auto pipeline = resourceProvider->findOrCreateGraphicsPipeline(pipelineDesc,
                                                                       renderPassDesc);
        commandBuffer->bindGraphicsPipeline(std::move(pipeline));

        // All of the test RenderSteps ignore the transform, so just use the identity
        static const Transform kIdentity{SkM44()};
        // No set scissor, so use entire render target dimensions
        static const SkIRect kBounds = SkIRect::MakeWH(kTextureWidth, kTextureHeight);

        PaintersDepth depth = DrawOrder::kClearDepth;
        for (auto d : draws) {
            depth = depth.next();

            drawWriter.newDynamicState();
            Shape shape(d.fRect);
            DrawOrder order(depth);
            DrawGeometry geom{kIdentity, shape, {shape.bounds(), kBounds}, order, nullptr};

            SkDEBUGCODE(gatherer.checkReset());
            step->writeUniforms(geom, &gatherer);
            if (gatherer.hasUniforms()) {
                SkUniformDataBlock renderStepUniforms = gatherer.peekUniformData();
                auto [writer, bindInfo] = bufferMgr.getUniformWriter(renderStepUniforms.size());
                writer.write(renderStepUniforms.data(), renderStepUniforms.size());
                commandBuffer->bindUniformBuffer(UniformSlot::kRenderStep,
                                                 sk_ref_sp(bindInfo.fBuffer),
                                                 bindInfo.fOffset);
            }
            gatherer.reset();

            // TODO: Rely on uniform writer and GetUniforms(kSolidColor).
            auto [writer, bindInfo] = bufferMgr.getUniformWriter(sizeof(SkColor4f));
            writer.write(&d.fColor, sizeof(SkColor4f));
            commandBuffer->bindUniformBuffer(UniformSlot::kPaint,
                                             sk_ref_sp(bindInfo.fBuffer),
                                             bindInfo.fOffset);

            step->writeVertices(&drawWriter, geom);
        }
    };

    SkRect fullRect = SkRect::MakeIWH(kTextureWidth, kTextureHeight);
    // Draw blue rectangle over entire rendertarget (which was red)
    draw(UniformRectDraw::Singleton(), {{fullRect, SkColors::kBlue}});

    // Draw inset yellow rectangle using uniforms
    draw(UniformRectDraw::Singleton(),
         {{fullRect.makeInset(kTextureWidth/20.f, kTextureHeight/20.f), SkColors::kYellow}});

    // Draw inset magenta rectangle with triangles in vertex buffer
    draw(TriangleRectDraw::Singleton(),
         {{fullRect.makeInset(kTextureWidth/4.f, kTextureHeight/4.f), SkColors::kMagenta}});

    // Draw green and cyan rects using instance buffer
    draw(InstanceRectDraw::Singleton(),
         { {{kTextureWidth/3.f, kTextureHeight/3.f,
             kTextureWidth/2.f, kTextureHeight/2.f}, SkColors::kGreen},
           {{kTextureWidth/2.f, kTextureHeight/2.f,
             5.f*kTextureWidth/8.f, 5.f*kTextureHeight/8.f}, SkColors::kCyan} });

    drawWriter.flush();
    bufferMgr.transferToCommandBuffer(commandBuffer.get());
    commandBuffer->endRenderPass();

    // Do readback

    // TODO: add 4-byte transfer buffer alignment for Mac to Caps
    //       add bpp to Caps
    size_t rowBytes = 4*kTextureWidth;
    size_t bufferSize = rowBytes*kTextureHeight;
    sk_sp<Buffer> copyBuffer = resourceProvider->findOrCreateBuffer(
            bufferSize, BufferType::kXferGpuToCpu, PrioritizeGpuReads::kNo);
    REPORTER_ASSERT(reporter, copyBuffer);
    SkIRect srcRect = { 0, 0, kTextureWidth, kTextureHeight };
    commandBuffer->copyTextureToBuffer(target->refTexture(), srcRect, copyBuffer, 0, rowBytes);

    bool result = gpu->submit(commandBuffer);
    REPORTER_ASSERT(reporter, result);

    gpu->checkForFinishedWork(SyncToCpu::kYes);
    uint32_t* pixels = (uint32_t*)(copyBuffer->map());
    REPORTER_ASSERT(reporter, pixels[0] == 0xffff0000);
    REPORTER_ASSERT(reporter, pixels[51 + 38*kTextureWidth] == 0xff00ffff);
    REPORTER_ASSERT(reporter, pixels[256 + 192*kTextureWidth] == 0xffff00ff);
    copyBuffer->unmap();

#if GRAPHITE_TEST_UTILS && CAPTURE_COMMANDBUFFER
    gpu->testingOnly_endCapture();
#endif
}
