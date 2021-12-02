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
#include "experimental/graphite/src/ContextUtils.h"
#include "experimental/graphite/src/DrawBufferManager.h"
#include "experimental/graphite/src/DrawWriter.h"
#include "experimental/graphite/src/Gpu.h"
#include "experimental/graphite/src/GraphicsPipeline.h"
#include "experimental/graphite/src/Renderer.h"
#include "experimental/graphite/src/ResourceProvider.h"
#include "experimental/graphite/src/Texture.h"
#include "experimental/graphite/src/TextureProxy.h"
#include "experimental/graphite/src/UniformManager.h"
#include "experimental/graphite/src/geom/Shape.h"
#include "experimental/graphite/src/geom/Transform_graphite.h"

#if GRAPHITE_TEST_UTILS
// set to 1 if you want to do GPU capture of the commandBuffer
#define CAPTURE_COMMANDBUFFER 0
#endif

using namespace skgpu;

namespace {

class UniformRectDraw final : public RenderStep {
public:
    ~UniformRectDraw() override {}

    static const RenderStep* Singleton() {
        static const UniformRectDraw kSingleton;
        return &kSingleton;
    }

    const char* name() const override { return "uniform-rect"; }

    const char* vertexMSL() const override {
        return "float2 position = float2(float(vertexID >> 1), float(vertexID & 1));\n"
               "out.position.xy = position * uniforms.scale + uniforms.translate;\n"
               "out.position.zw = float2(0.0, 1.0);\n";
    }

    void writeVertices(DrawWriter* writer, const Transform&, const Shape&) const override {
        // The shape is upload via uniforms, so this just needs to record 4 data-less vertices
        writer->draw({}, {}, 4);
    }

    sk_sp<UniformData> writeUniforms(Layout layout,
                                     const Transform&,
                                     const Shape& shape) const override {
        SkASSERT(shape.isRect());
        // TODO: A << API for uniforms would be nice, particularly if it could take pre-computed
        // offsets for each uniform.
        auto uniforms = UniformData::Make(this->numUniforms(), this->uniforms().data(),
                                          sizeof(float) * 4);
        float2 scale = shape.rect().size();
        float2 translate = shape.rect().topLeft();
        memcpy(uniforms->data(), &scale, sizeof(float2));
        memcpy(uniforms->data() + sizeof(float2), &translate, sizeof(float2));
        return uniforms;
    }

private:
    UniformRectDraw() : RenderStep(Flags::kPerformsShading,
                                   /*uniforms=*/{{"scale",     SLType::kFloat2},
                                                 {"translate", SLType::kFloat2}},
                                   PrimitiveType::kTriangleStrip,
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

    const char* name() const override { return "triangle-rect"; }

    const char* vertexMSL() const override {
        return "float2 position = vtx.position;\n"
               "out.position.xy = position * uniforms.scale + uniforms.translate;\n"
               "out.position.zw = float2(0.0, 1.0);\n";
    }

    void writeVertices(DrawWriter* writer, const Transform&, const Shape& shape) const override {
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

        writer->draw(vertices, indices, 6);
    }

    sk_sp<UniformData> writeUniforms(Layout layout, const Transform&, const Shape&) const override {
        auto uniforms = UniformData::Make(this->numUniforms(), this->uniforms().data(),
                                          sizeof(float) * 4);
        float data[4] = {2.f, 2.f, -1.f, -1.f};
        memcpy(uniforms->data(), data, 4 * sizeof(float));
        return uniforms;
    }

private:
    TriangleRectDraw()
            : RenderStep(Flags::kPerformsShading,
                         /*uniforms=*/{{"scale",     SLType::kFloat2},
                                       {"translate", SLType::kFloat2}},
                         PrimitiveType::kTriangles,
                         /*vertexAttrs=*/{{"position", VertexAttribType::kFloat2, SLType::kFloat2}},
                         /*instanceAttrs=*/{}) {}
};

class InstanceRectDraw final : public RenderStep {
public:
    ~InstanceRectDraw() override {}

    static const RenderStep* Singleton() {
        static const InstanceRectDraw kSingleton;
        return &kSingleton;
    }

    const char* name() const override { return "instance-rect"; }

    const char* vertexMSL() const override {
        return "float2 position = float2(float(vertexID >> 1), float(vertexID & 1));\n"
               "out.position.xy = position * vtx.dims + vtx.position;\n"
               "out.position.zw = float2(0.0, 1.0);\n";
    }

    void writeVertices(DrawWriter* writer, const Transform&, const Shape& shape) const override {
        SkASSERT(shape.isRect());

        DrawBufferManager* bufferMgr = writer->bufferManager();

        // TODO: To truly test draw merging, this index buffer needs to remembered across
        // writeVertices calls
        auto [indexWriter, indices] = bufferMgr->getIndexWriter(6 * sizeof(uint16_t));
        indexWriter << 0 << 1 << 2
                    << 2 << 1 << 3;

        writer->setInstanceTemplate({}, indices, 6);
        auto instanceWriter = writer->appendInstances(1);
        instanceWriter << shape.rect().topLeft() << shape.rect().size();
    }

    sk_sp<UniformData> writeUniforms(Layout, const Transform&, const Shape&) const override {
        return nullptr;
    }

private:
    InstanceRectDraw()
            : RenderStep(Flags::kPerformsShading,
                         /*uniforms=*/{},
                         PrimitiveType::kTriangles,
                         /*vertexAttrs=*/{},
                         /*instanceAttrs=*/ {
                                { "position", VertexAttribType::kFloat2, SLType::kFloat2 },
                                { "dims",     VertexAttribType::kFloat2, SLType::kFloat2 }
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
    renderPassDesc.fColorAttachment.fTextureInfo = target->textureInfo();
    renderPassDesc.fColorAttachment.fLoadOp = LoadOp::kClear;
    renderPassDesc.fColorAttachment.fStoreOp = StoreOp::kStore;
    renderPassDesc.fClearColor = { 1, 0, 0, 1 }; // red

    target->instantiate(gpu->resourceProvider());
    DrawBufferManager bufferMgr(gpu->resourceProvider(), 4);

    commandBuffer->beginRenderPass(renderPassDesc, target->refTexture(), nullptr, nullptr);

    commandBuffer->setViewport(0.f, 0.f, kTextureWidth, kTextureHeight);

    DrawWriter drawWriter(commandBuffer->asDrawDispatcher(), &bufferMgr);

    struct RectAndColor {
        SkRect    fRect;
        SkColor4f fColor;
    };

    auto draw = [&](const RenderStep* step, std::vector<RectAndColor> draws) {
        Combination shader{ShaderCombo::ShaderType::kSolidColor};
        GraphicsPipelineDesc pipelineDesc;
        pipelineDesc.setProgram(step, shader);
        drawWriter.newPipelineState(step->primitiveType(),
                                    step->vertexStride(),
                                    step->instanceStride());
        auto pipeline = gpu->resourceProvider()->findOrCreateGraphicsPipeline(pipelineDesc);
        commandBuffer->bindGraphicsPipeline(std::move(pipeline));

        // All of the test RenderSteps ignore the transform, so just use the identity
        static const Transform kIdentity{SkM44()};

        for (auto d : draws) {
            drawWriter.newDynamicState();
            Shape shape(d.fRect);

            auto renderStepUniforms = step->writeUniforms(Layout::kMetal, kIdentity, shape);
            if (renderStepUniforms) {
                auto [writer, bindInfo] =
                        bufferMgr.getUniformWriter(renderStepUniforms->dataSize());
                writer.write(renderStepUniforms->data(), renderStepUniforms->dataSize());
                commandBuffer->bindUniformBuffer(UniformSlot::kRenderStep,
                                                 sk_ref_sp(bindInfo.fBuffer),
                                                 bindInfo.fOffset);
            }

            // TODO: Rely on uniform writer and GetUniforms(kSolidColor).
            auto [writer, bindInfo] = bufferMgr.getUniformWriter(sizeof(SkColor4f));
            writer.write(&d.fColor, sizeof(SkColor4f));
            commandBuffer->bindUniformBuffer(UniformSlot::kPaint,
                                             sk_ref_sp(bindInfo.fBuffer),
                                             bindInfo.fOffset);

            step->writeVertices(&drawWriter, kIdentity, shape);
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
