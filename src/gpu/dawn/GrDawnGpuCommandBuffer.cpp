/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnGpuCommandBuffer.h"

#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrMesh.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/dawn/GrDawnBuffer.h"
#include "src/gpu/dawn/GrDawnGpu.h"
#include "src/gpu/dawn/GrDawnProgramBuilder.h"
#include "src/gpu/dawn/GrDawnRenderTarget.h"
#include "src/gpu/dawn/GrDawnStencilAttachment.h"
#include "src/gpu/dawn/GrDawnTexture.h"
#include "src/gpu/dawn/GrDawnUtil.h"
#include "src/sksl/SkSLCompiler.h"

GrDawnGpuTextureCommandBuffer::GrDawnGpuTextureCommandBuffer(GrDawnGpu* gpu,
                                                             GrTexture* texture,
                                                             GrSurfaceOrigin origin)
    : INHERITED(texture, origin)
    , fGpu(gpu) {
    fEncoder = fGpu->device().CreateCommandEncoder();
}

void GrDawnGpuTextureCommandBuffer::copy(GrSurface* src, const SkIRect& srcRect,
                                         const SkIPoint& dstPoint) {
    if (!src->asTexture()) {
        return;
    }
    uint32_t width = srcRect.width(), height = srcRect.height();
    size_t rowBytes = srcRect.width() * GrBytesPerPixel(src->config());
    rowBytes = GrDawnRoundRowBytes(rowBytes);
    size_t sizeInBytes = height * rowBytes;

    dawn::BufferDescriptor desc;
    desc.usage = dawn::BufferUsageBit::CopySrc | dawn::BufferUsageBit::CopyDst;
    desc.size = sizeInBytes;

    dawn::Buffer buffer = fGpu->device().CreateBuffer(&desc);

    dawn::TextureCopyView srcTextureView, dstTextureView;
    srcTextureView.texture = static_cast<GrDawnTexture*>(src->asTexture())->texture();
    srcTextureView.origin = {(uint32_t) srcRect.x(), (uint32_t) srcRect.y(), 0};
    dstTextureView.texture = static_cast<GrDawnTexture*>(fTexture)->texture();
    dstTextureView.origin = {(uint32_t) dstPoint.x(), (uint32_t) dstPoint.y(), 0};

    dawn::BufferCopyView bufferView;
    bufferView.buffer = buffer;
    bufferView.offset = 0;
    bufferView.rowPitch = rowBytes;
    bufferView.imageHeight = height;

    dawn::Extent3D copySize = {width, height, 1};
    fEncoder.CopyTextureToBuffer(&srcTextureView, &bufferView, &copySize);
    fEncoder.CopyBufferToTexture(&bufferView, &dstTextureView, &copySize);
}

void GrDawnGpuTextureCommandBuffer::transferFrom(const SkIRect& srcRect,
                                                 GrColorType surfaceColorType,
                                                 GrColorType bufferColorType,
                                                 GrGpuBuffer* transferBuffer,
                                                 size_t offset) {
    fGpu->transferPixelsFrom(fTexture, srcRect.fLeft, srcRect.fTop, srcRect.width(),
                             srcRect.height(), surfaceColorType, bufferColorType, transferBuffer,
                             offset);
}

void GrDawnGpuTextureCommandBuffer::submit() {
    dawn::CommandBuffer commandBuffer = fEncoder.Finish();
    if (commandBuffer) {
        fGpu->queue().Submit(1, &commandBuffer);
    }
}

GrDawnGpuTextureCommandBuffer::~GrDawnGpuTextureCommandBuffer() {}

////////////////////////////////////////////////////////////////////////////////

static dawn::LoadOp to_dawn_load_op(GrLoadOp loadOp) {
    switch (loadOp) {
        case GrLoadOp::kLoad:
            return dawn::LoadOp::Load;
        case GrLoadOp::kDiscard:
            // Use LoadOp::Load to emulate DontCare.
            // Dawn doesn't have DontCare, for security reasons.
            // Load should be equivalent to DontCare for desktop; Clear would
            // probably be better for tilers. If Dawn does add DontCare
            // as an extension, use it here.
            return dawn::LoadOp::Load;
        case GrLoadOp::kClear:
            return dawn::LoadOp::Clear;
        default:
            SK_ABORT("Invalid LoadOp");
    }
}

GrDawnGpuRTCommandBuffer::GrDawnGpuRTCommandBuffer(GrDawnGpu* gpu,
                                                   GrRenderTarget* rt, GrSurfaceOrigin origin,
                                                   const LoadAndStoreInfo& colorInfo,
                                                   const StencilLoadAndStoreInfo& stencilInfo)
        : INHERITED(rt, origin)
        , fGpu(gpu)
        , fColorInfo(colorInfo) {
    fEncoder = fGpu->device().CreateCommandEncoder();
    dawn::LoadOp colorOp = to_dawn_load_op(colorInfo.fLoadOp);
    dawn::LoadOp stencilOp = to_dawn_load_op(stencilInfo.fLoadOp);
    fPassEncoder = beginRenderPass(colorOp, stencilOp);
}

dawn::RenderPassEncoder GrDawnGpuRTCommandBuffer::beginRenderPass(dawn::LoadOp colorOp,
                                                                  dawn::LoadOp stencilOp) {
    dawn::Texture texture = static_cast<GrDawnRenderTarget*>(fRenderTarget)->texture();
    auto stencilAttachment = static_cast<GrDawnStencilAttachment*>(
        fRenderTarget->renderTargetPriv().getStencilAttachment());
    dawn::TextureView colorView = texture.CreateDefaultView();
    const float *c = fColorInfo.fClearColor.vec();

    dawn::RenderPassColorAttachmentDescriptor colorAttachment;
    colorAttachment.attachment = colorView;
    colorAttachment.resolveTarget = nullptr;
    colorAttachment.clearColor = { c[0], c[1], c[2], c[3] };
    colorAttachment.loadOp = colorOp;
    colorAttachment.storeOp = dawn::StoreOp::Store;
    dawn::RenderPassColorAttachmentDescriptor* colorAttachments = { &colorAttachment };
    dawn::RenderPassDescriptor renderPassDescriptor;
    renderPassDescriptor.colorAttachmentCount = 1;
    renderPassDescriptor.colorAttachments = &colorAttachments;
    if (stencilAttachment) {
        dawn::RenderPassDepthStencilAttachmentDescriptor depthStencilAttachment;
        depthStencilAttachment.attachment = stencilAttachment->view();
        depthStencilAttachment.depthLoadOp = stencilOp;
        depthStencilAttachment.stencilLoadOp = stencilOp;
        depthStencilAttachment.clearDepth = 1.0f;
        depthStencilAttachment.clearStencil = 0;
        depthStencilAttachment.depthStoreOp = dawn::StoreOp::Store;
        depthStencilAttachment.stencilStoreOp = dawn::StoreOp::Store;
        renderPassDescriptor.depthStencilAttachment = &depthStencilAttachment;
    } else {
        renderPassDescriptor.depthStencilAttachment = nullptr;
    }
    return fEncoder.BeginRenderPass(&renderPassDescriptor);
}

GrDawnGpuRTCommandBuffer::~GrDawnGpuRTCommandBuffer() {
}

GrGpu* GrDawnGpuRTCommandBuffer::gpu() { return fGpu; }

void GrDawnGpuRTCommandBuffer::end() {
    fPassEncoder.EndPass();
}

void GrDawnGpuRTCommandBuffer::submit() {
    dawn::CommandBuffer commandBuffer = fEncoder.Finish();
    if (commandBuffer) {
        fGpu->queue().Submit(1, &commandBuffer);
    }
}

void GrDawnGpuRTCommandBuffer::insertEventMarker(const char* msg) {
    SkASSERT(!"unimplemented");
}

void GrDawnGpuRTCommandBuffer::transferFrom(const SkIRect& srcRect, GrColorType surfaceColorType,
                                            GrColorType bufferColorType,
                                            GrGpuBuffer* transferBuffer, size_t offset) {
    fGpu->transferPixelsFrom(fRenderTarget, srcRect.fLeft, srcRect.fTop, srcRect.width(),
                             srcRect.height(), surfaceColorType, bufferColorType, transferBuffer,
                             offset);
}

void GrDawnGpuRTCommandBuffer::onClearStencilClip(const GrFixedClip& clip, bool insideStencilMask) {
    fPassEncoder.EndPass();
    fPassEncoder = beginRenderPass(dawn::LoadOp::Load, dawn::LoadOp::Clear);
}

void GrDawnGpuRTCommandBuffer::onClear(const GrFixedClip& clip, const SkPMColor4f& color) {
    fPassEncoder.EndPass();
    fPassEncoder = beginRenderPass(dawn::LoadOp::Clear, dawn::LoadOp::Load);
}

////////////////////////////////////////////////////////////////////////////////

void GrDawnGpuRTCommandBuffer::inlineUpload(GrOpFlushState* state,
                                            GrDeferredTextureUploadFn& upload) {
    SkASSERT(!"unimplemented");
}

void GrDawnGpuRTCommandBuffer::copy(GrSurface* src, const SkIRect& srcRect,
                                    const SkIPoint& dstPoint) {
    auto s = static_cast<GrDawnTexture*>(src->asTexture());
    auto d = static_cast<GrDawnTexture*>(fRenderTarget->asTexture());

    if (!s || !d) {
        return;
    }

    dawn::Texture srcTex = s->texture();
    dawn::Texture dstTex = d->texture();

    uint32_t x = srcRect.x();
    uint32_t y = srcRect.y();
    uint32_t width = srcRect.width();
    uint32_t height = srcRect.height();
    int rowPitch = GrDawnRoundRowBytes(width * GrBytesPerPixel(src->config()));
    int sizeInBytes = rowPitch * height;

    dawn::BufferDescriptor desc;
    desc.usage = dawn::BufferUsageBit::CopySrc | dawn::BufferUsageBit::CopyDst;
    desc.size = sizeInBytes;

    dawn::Buffer buffer = fGpu->device().CreateBuffer(&desc);

    uint32_t dstX = dstPoint.x();
    uint32_t dstY = dstPoint.y();
    fPassEncoder.EndPass();

    dawn::TextureCopyView srcTextureCopyView;
    srcTextureCopyView.texture = srcTex;
    srcTextureCopyView.origin = {x, y, 0};

    dawn::TextureCopyView dstTextureCopyView;
    dstTextureCopyView.texture = dstTex;
    dstTextureCopyView.origin = {dstX, dstY, 0};

    dawn::BufferCopyView bufferCopyView;
    bufferCopyView.buffer = buffer;
    bufferCopyView.offset = 0;
    bufferCopyView.rowPitch = rowPitch;
    bufferCopyView.imageHeight = height;

    dawn::Extent3D copySize = {width, height, 1};
    fEncoder.CopyTextureToBuffer(&srcTextureCopyView, &bufferCopyView, &copySize);
    fEncoder.CopyBufferToTexture(&bufferCopyView, &dstTextureCopyView, &copySize);
    fPassEncoder = beginRenderPass(dawn::LoadOp::Load, dawn::LoadOp::Load);
}

////////////////////////////////////////////////////////////////////////////////

static dawn::VertexFormat to_dawn_vertex_format(GrVertexAttribType type) {
    switch (type) {
    case kFloat_GrVertexAttribType:
    case kHalf_GrVertexAttribType:
        return dawn::VertexFormat::Float;
    case kFloat2_GrVertexAttribType:
    case kHalf2_GrVertexAttribType:
        return dawn::VertexFormat::Float2;
    case kFloat3_GrVertexAttribType:
    case kHalf3_GrVertexAttribType:
        return dawn::VertexFormat::Float3;
    case kFloat4_GrVertexAttribType:
    case kHalf4_GrVertexAttribType:
        return dawn::VertexFormat::Float4;
    case kUShort2_GrVertexAttribType:
        return dawn::VertexFormat::UShort2;
    case kInt_GrVertexAttribType:
        return dawn::VertexFormat::Int;
    case kUByte4_norm_GrVertexAttribType:
        return dawn::VertexFormat::UChar4Norm;
    default:
        SkASSERT(!"unsupported vertex format");
        return dawn::VertexFormat::Float4;
    }
}

static dawn::PrimitiveTopology to_dawn_primitive_topology(GrPrimitiveType primitiveType) {
    switch (primitiveType) {
        case GrPrimitiveType::kTriangles:
            return dawn::PrimitiveTopology::TriangleList;
        case GrPrimitiveType::kTriangleStrip:
            return dawn::PrimitiveTopology::TriangleStrip;
        case GrPrimitiveType::kPoints:
            return dawn::PrimitiveTopology::PointList;
        case GrPrimitiveType::kLines:
            return dawn::PrimitiveTopology::LineList;
        case GrPrimitiveType::kLineStrip:
            return dawn::PrimitiveTopology::LineStrip;
        case GrPrimitiveType::kLinesAdjacency:
        default:
            SkASSERT(!"unsupported primitive topology");
            return dawn::PrimitiveTopology::TriangleList;
    }
}

void GrDawnGpuRTCommandBuffer::setScissorState(
        const GrPipeline& pipeline,
        const GrPipeline::FixedDynamicState* fixedDynamicState,
        const GrPipeline::DynamicStateArrays* dynamicStateArrays) {
    SkIRect rect;
    if (pipeline.isScissorEnabled()) {
        constexpr SkIRect kBogusScissor{0, 0, 1, 1};
        rect = fixedDynamicState ? fixedDynamicState->fScissorRect : kBogusScissor;
        if (kBottomLeft_GrSurfaceOrigin == fOrigin) {
            rect.setXYWH(rect.x(), fRenderTarget->height() - rect.bottom(),
                         rect.width(), rect.height());
        }
    } else {
        rect = SkIRect::MakeWH(fRenderTarget->width(), fRenderTarget->height());
    }
    fPassEncoder.SetScissorRect(rect.x(), rect.y(), rect.width(), rect.height());
}

void GrDawnGpuRTCommandBuffer::applyState(const GrPipeline& pipeline,
                                          const GrPrimitiveProcessor& primProc,
                                          const GrTextureProxy* const primProcProxies[],
                                          const GrPipeline::FixedDynamicState* fixedDynamicState,
                                          const GrPipeline::DynamicStateArrays* dynamicStateArrays,
                                          const GrPrimitiveType primitiveType,
                                          bool hasPoints) {
    GrProgramDesc desc;
    GrProgramDesc::Build(&desc, fRenderTarget, primProc, hasPoints, pipeline, fGpu);
    dawn::TextureFormat colorFormat;
    SkAssertResult(GrPixelConfigToDawnFormat(fRenderTarget->config(), &colorFormat));
    dawn::TextureFormat stencilFormat = dawn::TextureFormat::Depth24PlusStencil8;
    bool hasDepthStencil = fRenderTarget->renderTargetPriv().getStencilAttachment() != nullptr;
    sk_sp<GrDawnProgram> program = GrDawnProgramBuilder::Build(fGpu, fRenderTarget, fOrigin,
                                                               pipeline, primProc, primProcProxies,
                                                               colorFormat, hasDepthStencil,
                                                               stencilFormat, &desc);
    SkASSERT(program);
    auto bindGroup = program->setData(fGpu, fRenderTarget, fOrigin, primProc, pipeline,
                                      primProcProxies);

    std::vector<dawn::VertexBufferDescriptor> inputs;
    std::vector<dawn::VertexAttributeDescriptor> vertexAttributes;
    if (primProc.numVertexAttributes() > 0) {
        size_t offset = 0;
        int i = 0;
        for (const auto& attrib : primProc.vertexAttributes()) {
            dawn::VertexAttributeDescriptor attribute;
            attribute.shaderLocation = i;
            attribute.offset = offset;
            attribute.format = to_dawn_vertex_format(attrib.cpuType());
            vertexAttributes.push_back(attribute);
            offset += attrib.sizeAlign4();
            i++;
        }
        dawn::VertexBufferDescriptor input;
        input.stride = offset;
        input.stepMode = dawn::InputStepMode::Vertex;
        input.attributes = &vertexAttributes.front();
        input.attributeCount = vertexAttributes.size();
        inputs.push_back(input);
    }
    std::vector<dawn::VertexAttributeDescriptor> instanceAttributes;
    if (primProc.numInstanceAttributes() > 0) {
        size_t offset = 0;
        int i = 0;
        for (const auto& attrib : primProc.instanceAttributes()) {
            dawn::VertexAttributeDescriptor attribute;
            attribute.shaderLocation = i;
            attribute.offset = offset;
            attribute.format = to_dawn_vertex_format(attrib.cpuType());
            instanceAttributes.push_back(attribute);
            offset += attrib.sizeAlign4();
            i++;
        }
        dawn::VertexBufferDescriptor input;
        input.stride = offset;
        input.stepMode = dawn::InputStepMode::Instance;
        input.attributes = &instanceAttributes.front();
        input.attributeCount = instanceAttributes.size();
        inputs.push_back(input);
    }
    dawn::VertexInputDescriptor vertexInput;
    vertexInput.bufferCount = inputs.size();
    vertexInput.buffers = &inputs.front();
    vertexInput.indexFormat = dawn::IndexFormat::Uint16;

    dawn::PipelineStageDescriptor vsDesc;
    vsDesc.module = program->fVSModule;
    vsDesc.entryPoint = "main";

    dawn::PipelineStageDescriptor fsDesc;
    fsDesc.module = program->fFSModule;
    fsDesc.entryPoint = "main";

    dawn::RasterizationStateDescriptor rastDesc;

    rastDesc.frontFace = dawn::FrontFace::CW;
    rastDesc.cullMode = dawn::CullMode::None;
    rastDesc.depthBias = 0;
    rastDesc.depthBiasSlopeScale = 0.0f;
    rastDesc.depthBiasClamp = 0.0f;

    dawn::RenderPipelineDescriptor rpDesc;
    rpDesc.layout = program->fPipelineLayout;
    rpDesc.vertexStage = &vsDesc;
    rpDesc.fragmentStage = &fsDesc;
    rpDesc.vertexInput = &vertexInput;
    rpDesc.rasterizationState = &rastDesc;
    rpDesc.primitiveTopology = to_dawn_primitive_topology(primitiveType);
    rpDesc.sampleCount = 1;
    rpDesc.depthStencilState = hasDepthStencil ? &program->fDepthStencilState : nullptr;
    rpDesc.colorStateCount = 1;
    dawn::ColorStateDescriptor* colorStates[] = { &program->fColorState };
    rpDesc.colorStates = colorStates;
    dawn::RenderPipeline renderPipeline = fGpu->device().CreateRenderPipeline(&rpDesc);
    fPassEncoder.SetPipeline(renderPipeline);
    fPassEncoder.SetBindGroup(0, bindGroup, 0, nullptr);
    if (pipeline.isStencilEnabled()) {
        fPassEncoder.SetStencilReference(pipeline.getUserStencil()->fFront.fRef);
    }
    GrXferProcessor::BlendInfo blendInfo = pipeline.getXferProcessor().getBlendInfo();
    const float* c = blendInfo.fBlendConstant.vec();
    dawn::Color color{c[0], c[1], c[2], c[3]};
    fPassEncoder.SetBlendColor(&color);
    this->setScissorState(pipeline, fixedDynamicState, dynamicStateArrays);
}

void GrDawnGpuRTCommandBuffer::onDraw(const GrPrimitiveProcessor& primProc,
                                      const GrPipeline& pipeline,
                                      const GrPipeline::FixedDynamicState* fixedDynamicState,
                                      const GrPipeline::DynamicStateArrays* dynamicStateArrays,
                                      const GrMesh meshes[],
                                      int meshCount,
                                      const SkRect& bounds) {
    if (!meshCount) {
        return;
    }
    bool hasPoints = false;
    for (int i = 0; i < meshCount; ++i) {
        if (meshes[i].primitiveType() == GrPrimitiveType::kPoints) {
            hasPoints = true;
        }
    }
    const GrTextureProxy* const* primProcProxies = nullptr;
    if (dynamicStateArrays && dynamicStateArrays->fPrimitiveProcessorTextures) {
        primProcProxies = dynamicStateArrays->fPrimitiveProcessorTextures;
    } else if (fixedDynamicState) {
        primProcProxies = fixedDynamicState->fPrimitiveProcessorTextures;
    }
    for (int i = 0; i < meshCount; ++i) {
        applyState(pipeline, primProc, primProcProxies, fixedDynamicState, dynamicStateArrays,
                   meshes[0].primitiveType(), hasPoints);
        meshes[i].sendToGpu(this);
    }
}

void GrDawnGpuRTCommandBuffer::sendInstancedMeshToGpu(GrPrimitiveType,
                                                      const GrBuffer* vertexBuffer,
                                                      int vertexCount,
                                                      int baseVertex,
                                                      const GrBuffer* instanceBuffer,
                                                      int instanceCount,
                                                      int baseInstance) {
    static const uint64_t vertexBufferOffsets[1] = {0};
    dawn::Buffer vb = static_cast<const GrDawnBuffer*>(vertexBuffer)->get();
    fPassEncoder.SetVertexBuffers(0, 1, &vb, vertexBufferOffsets);
    fPassEncoder.Draw(vertexCount, 1, baseVertex, baseInstance);
    fGpu->stats()->incNumDraws();
}

void GrDawnGpuRTCommandBuffer::sendIndexedInstancedMeshToGpu(GrPrimitiveType,
                                                             const GrBuffer* indexBuffer,
                                                             int indexCount,
                                                             int baseIndex,
                                                             const GrBuffer* vertexBuffer,
                                                             int baseVertex,
                                                             const GrBuffer* instanceBuffer,
                                                             int instanceCount,
                                                             int baseInstance,
                                                             GrPrimitiveRestart restart) {
    uint64_t vertexBufferOffsets[1];
    vertexBufferOffsets[0] = 0;
    dawn::Buffer vb = static_cast<const GrDawnBuffer*>(vertexBuffer)->get();
    dawn::Buffer ib = static_cast<const GrDawnBuffer*>(indexBuffer)->get();
    fPassEncoder.SetIndexBuffer(ib, 0);
    fPassEncoder.SetVertexBuffers(0, 1, &vb, vertexBufferOffsets);
    fPassEncoder.DrawIndexed(indexCount, 1, baseIndex, baseVertex, baseInstance);
    fGpu->stats()->incNumDraws();
}
