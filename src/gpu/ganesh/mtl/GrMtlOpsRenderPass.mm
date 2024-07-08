/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/mtl/GrMtlOpsRenderPass.h"

#include "src/gpu/ganesh/GrBackendUtils.h"
#include "src/gpu/ganesh/GrColor.h"
#include "src/gpu/ganesh/GrNativeRect.h"
#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/mtl/GrMtlCommandBuffer.h"
#include "src/gpu/ganesh/mtl/GrMtlPipelineState.h"
#include "src/gpu/ganesh/mtl/GrMtlPipelineStateBuilder.h"
#include "src/gpu/ganesh/mtl/GrMtlRenderCommandEncoder.h"
#include "src/gpu/ganesh/mtl/GrMtlRenderTarget.h"
#include "src/gpu/ganesh/mtl/GrMtlTexture.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GR_NORETAIN_BEGIN

GrMtlOpsRenderPass::GrMtlOpsRenderPass(GrMtlGpu* gpu, GrRenderTarget* rt,
                                       sk_sp<GrMtlFramebuffer> framebuffer, GrSurfaceOrigin origin,
                                       const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
                                       const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo)
        : INHERITED(rt, origin)
        , fGpu(gpu)
        , fFramebuffer(std::move(framebuffer)) {
    this->setupRenderPass(colorInfo, stencilInfo);
}

GrMtlOpsRenderPass::~GrMtlOpsRenderPass() {
}

void GrMtlOpsRenderPass::submit() {
    if (!fFramebuffer) {
        return;
    }
    SkIRect iBounds;
    fBounds.roundOut(&iBounds);
    fGpu->submitIndirectCommandBuffer(fRenderTarget, fOrigin, &iBounds);
    fActiveRenderCmdEncoder = nullptr;
}

static MTLPrimitiveType gr_to_mtl_primitive(GrPrimitiveType primitiveType) {
    const static MTLPrimitiveType mtlPrimitiveType[] {
        MTLPrimitiveTypeTriangle,
        MTLPrimitiveTypeTriangleStrip,
        MTLPrimitiveTypePoint,
        MTLPrimitiveTypeLine,
        MTLPrimitiveTypeLineStrip
    };
    static_assert((int)GrPrimitiveType::kTriangles == 0);
    static_assert((int)GrPrimitiveType::kTriangleStrip == 1);
    static_assert((int)GrPrimitiveType::kPoints == 2);
    static_assert((int)GrPrimitiveType::kLines == 3);
    static_assert((int)GrPrimitiveType::kLineStrip == 4);

    SkASSERT(primitiveType <= GrPrimitiveType::kLineStrip);
    return mtlPrimitiveType[static_cast<int>(primitiveType)];
}

bool GrMtlOpsRenderPass::onBindPipeline(const GrProgramInfo& programInfo,
                                        const SkRect& drawBounds) {
    const GrMtlCaps& caps = fGpu->mtlCaps();
    GrProgramDesc programDesc = caps.makeDesc(fRenderTarget, programInfo,
                                              GrCaps::ProgramDescOverrideFlags::kNone);
    if (!programDesc.isValid()) {
        return false;
    }

    fActivePipelineState = fGpu->resourceProvider().findOrCreateCompatiblePipelineState(
            programDesc, programInfo);
    if (!fActivePipelineState) {
        return false;
    }

    fActivePipelineState->setData(fFramebuffer.get(), programInfo);
    fCurrentVertexStride = programInfo.geomProc().vertexStride();

    if (!fActiveRenderCmdEncoder) {
        this->setupRenderCommandEncoder(fActivePipelineState);
        if (!fActiveRenderCmdEncoder) {
            return false;
        }
        fGpu->commandBuffer()->addGrSurface(
                sk_ref_sp<GrMtlAttachment>(fFramebuffer->colorAttachment()));
    }

    fActiveRenderCmdEncoder->setRenderPipelineState(
            fActivePipelineState->pipeline()->mtlPipelineState());
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    if (!fDebugGroupActive) {
        fActiveRenderCmdEncoder->pushDebugGroup(@"bindAndDraw");
        fDebugGroupActive = true;
    }
#endif
    fActivePipelineState->setDrawState(fActiveRenderCmdEncoder,
                                       programInfo.pipeline().writeSwizzle(),
                                       programInfo.pipeline().getXferProcessor());
    if (this->gpu()->caps()->wireframeMode() || programInfo.pipeline().isWireframe()) {
        fActiveRenderCmdEncoder->setTriangleFillMode(MTLTriangleFillModeLines);
    } else {
        fActiveRenderCmdEncoder->setTriangleFillMode(MTLTriangleFillModeFill);
    }

    if (!programInfo.pipeline().isScissorTestEnabled()) {
        // "Disable" scissor by setting it to the full pipeline bounds.
        SkISize dimensions = fFramebuffer->colorAttachment()->dimensions();
        GrMtlPipelineState::SetDynamicScissorRectState(fActiveRenderCmdEncoder,
                                                       dimensions, fOrigin,
                                                       SkIRect::MakeWH(dimensions.width(),
                                                                       dimensions.height()));
    }

    fActivePrimitiveType = gr_to_mtl_primitive(programInfo.primitiveType());
    fBounds.join(drawBounds);
    return true;
}

void GrMtlOpsRenderPass::onSetScissorRect(const SkIRect& scissor) {
    SkASSERT(fActivePipelineState);
    SkASSERT(fActiveRenderCmdEncoder);
    GrMtlPipelineState::SetDynamicScissorRectState(fActiveRenderCmdEncoder,
                                                   fFramebuffer->colorAttachment()->dimensions(),
                                                   fOrigin, scissor);
}

bool GrMtlOpsRenderPass::onBindTextures(const GrGeometryProcessor& geomProc,
                                        const GrSurfaceProxy* const geomProcTextures[],
                                        const GrPipeline& pipeline) {
    SkASSERT(fActivePipelineState);
    SkASSERT(fActiveRenderCmdEncoder);
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    if (!fDebugGroupActive) {
        fActiveRenderCmdEncoder->pushDebugGroup(@"bindAndDraw");
        fDebugGroupActive = true;
    }
#endif
    fActivePipelineState->setTextures(geomProc, pipeline, geomProcTextures);
    fActivePipelineState->bindTextures(fActiveRenderCmdEncoder);
    return true;
}

void GrMtlOpsRenderPass::onClear(const GrScissorState& scissor, std::array<float, 4> color) {
    // Partial clears are not supported
    SkASSERT(!scissor.enabled());

    // Ideally we should never end up here since all clears should either be done as draws or
    // load ops in metal. However, if a client inserts a wait op we need to handle it.
    auto colorAttachment = fRenderPassDesc.colorAttachments[0];
    colorAttachment.clearColor = MTLClearColorMake(color[0], color[1], color[2], color[3]);
    colorAttachment.loadAction = MTLLoadActionClear;
    if (!this->setupResolve()) {
        this->setupRenderCommandEncoder(nullptr);
    }
}

void GrMtlOpsRenderPass::onClearStencilClip(const GrScissorState& scissor, bool insideStencilMask) {
    // Partial clears are not supported
    SkASSERT(!scissor.enabled());

    GrAttachment* sb = fFramebuffer->stencilAttachment();
    // this should only be called internally when we know we have a
    // stencil buffer.
    SkASSERT(sb);
    int stencilBitCount = GrBackendFormatStencilBits(sb->backendFormat());

    // The contract with the callers does not guarantee that we preserve all bits in the stencil
    // during this clear. Thus we will clear the entire stencil to the desired value.
    auto stencilAttachment = fRenderPassDesc.stencilAttachment;
    if (insideStencilMask) {
        stencilAttachment.clearStencil = (1 << (stencilBitCount - 1));
    } else {
        stencilAttachment.clearStencil = 0;
    }

    stencilAttachment.loadAction = MTLLoadActionClear;
    if (!this->setupResolve()) {
        this->setupRenderCommandEncoder(nullptr);
    }
}

void GrMtlOpsRenderPass::inlineUpload(GrOpFlushState* state, GrDeferredTextureUploadFn& upload) {
    state->doUpload(upload);

    // If the previous renderCommandEncoder did a resolve without an MSAA store
    // (e.g., if the color attachment is memoryless) we need to copy the contents of
    // the resolve attachment to the MSAA attachment at this point.
    if (!this->setupResolve()) {
        // If setting up for the resolve didn't create an encoder, it's probably reasonable to
        // create a new encoder at this point, though maybe not necessary.
        this->setupRenderCommandEncoder(nullptr);
    }
}

void GrMtlOpsRenderPass::initRenderState(GrMtlRenderCommandEncoder* encoder) {
    if (!encoder) {
        return;
    }
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    encoder->pushDebugGroup(@"initRenderState");
#endif
    encoder->setFrontFacingWinding(MTLWindingCounterClockwise);
    SkISize colorAttachmentDimensions = fFramebuffer->colorAttachment()->dimensions();
    // Strictly speaking we shouldn't have to set this, as the default viewport is the size of
    // the drawable used to generate the renderCommandEncoder -- but just in case.
    MTLViewport viewport = { 0.0, 0.0,
                             (double) colorAttachmentDimensions.width(),
                             (double) colorAttachmentDimensions.height(),
                             0.0, 1.0 };
    encoder->setViewport(viewport);
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    encoder->popDebugGroup();
#endif
}

void GrMtlOpsRenderPass::setupRenderPass(
        const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
        const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo) {
    const static MTLLoadAction mtlLoadAction[] {
        MTLLoadActionLoad,
        MTLLoadActionClear,
        MTLLoadActionDontCare
    };
    static_assert((int)GrLoadOp::kLoad == 0);
    static_assert((int)GrLoadOp::kClear == 1);
    static_assert((int)GrLoadOp::kDiscard == 2);
    SkASSERT(colorInfo.fLoadOp <= GrLoadOp::kDiscard);
    SkASSERT(stencilInfo.fLoadOp <= GrLoadOp::kDiscard);

    const static MTLStoreAction mtlStoreAction[] {
        MTLStoreActionStore,
        MTLStoreActionDontCare
    };
    static_assert((int)GrStoreOp::kStore == 0);
    static_assert((int)GrStoreOp::kDiscard == 1);
    SkASSERT(colorInfo.fStoreOp <= GrStoreOp::kDiscard);
    SkASSERT(stencilInfo.fStoreOp <= GrStoreOp::kDiscard);

    fRenderPassDesc = [MTLRenderPassDescriptor new];
    auto colorAttachment = fRenderPassDesc.colorAttachments[0];
    auto color = fFramebuffer->colorAttachment();
    colorAttachment.texture = color->mtlTexture();
    const std::array<float, 4>& clearColor = colorInfo.fClearColor;
    colorAttachment.clearColor =
            MTLClearColorMake(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    colorAttachment.loadAction = mtlLoadAction[static_cast<int>(colorInfo.fLoadOp)];
    colorAttachment.storeAction = mtlStoreAction[static_cast<int>(colorInfo.fStoreOp)];

    auto stencil = fFramebuffer->stencilAttachment();
    auto mtlStencil = fRenderPassDesc.stencilAttachment;
    if (stencil) {
        mtlStencil.texture = stencil->mtlTexture();
    }
    mtlStencil.clearStencil = 0;
    mtlStencil.loadAction = mtlLoadAction[static_cast<int>(stencilInfo.fLoadOp)];
    mtlStencil.storeAction = mtlStoreAction[static_cast<int>(stencilInfo.fStoreOp)];

    if (!this->setupResolve()) {
        // Manage initial clears
        if (colorInfo.fLoadOp == GrLoadOp::kClear || stencilInfo.fLoadOp == GrLoadOp::kClear)  {
            fBounds = SkRect::MakeWH(color->dimensions().width(),
                                     color->dimensions().height());
            this->setupRenderCommandEncoder(nullptr);
        } else {
            fBounds.setEmpty();
            // For now, we lazily create the renderCommandEncoder because we may have no draws,
            // and an empty renderCommandEncoder can still produce output. This can cause issues
            // when we clear a texture upon creation -- we'll subsequently discard the contents.
            // This can be removed when that ordering is fixed.
        }
    }
}

bool GrMtlOpsRenderPass::setupResolve() {
    fActiveRenderCmdEncoder = nullptr;
    auto resolve = fFramebuffer->resolveAttachment();
    if (resolve) {
        auto colorAttachment = fRenderPassDesc.colorAttachments[0];
        colorAttachment.resolveTexture = resolve->mtlTexture();
        // TODO: For framebufferOnly attachments we should do StoreAndMultisampleResolve if
        // the storeAction is Store. But for the moment they don't take this path.
        colorAttachment.storeAction = MTLStoreActionMultisampleResolve;
        if (colorAttachment.loadAction == MTLLoadActionLoad) {
            auto color = fFramebuffer->colorAttachment();
            auto dimensions = color->dimensions();
            // for now use the full bounds
            auto nativeBounds = GrNativeRect::MakeIRectRelativeTo(
                    fOrigin, dimensions.height(), SkIRect::MakeSize(dimensions));
            fActiveRenderCmdEncoder =
                    fGpu->loadMSAAFromResolve(color, resolve, nativeBounds,
                                              fRenderPassDesc.stencilAttachment);
        }
    }

    return (fActiveRenderCmdEncoder != nullptr);
}

void GrMtlOpsRenderPass::setupRenderCommandEncoder(GrMtlPipelineState* pipelineState) {
    fActiveRenderCmdEncoder =
            fGpu->commandBuffer()->getRenderCommandEncoder(fRenderPassDesc, pipelineState, this);
    // Any future RenderCommandEncoders we create for this OpsRenderPass should load,
    // unless onClear or onClearStencilClip are explicitly called.
    auto colorAttachment = fRenderPassDesc.colorAttachments[0];
    colorAttachment.loadAction = MTLLoadActionLoad;
    auto stencilAttachment = fRenderPassDesc.stencilAttachment;
    stencilAttachment.loadAction = MTLLoadActionLoad;
}

void GrMtlOpsRenderPass::onBindBuffers(sk_sp<const GrBuffer> indexBuffer,
                                       sk_sp<const GrBuffer> instanceBuffer,
                                       sk_sp<const GrBuffer> vertexBuffer,
                                       GrPrimitiveRestart primRestart) {
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    if (!fDebugGroupActive) {
        fActiveRenderCmdEncoder->pushDebugGroup(@"bindAndDraw");
        fDebugGroupActive = true;
    }
#endif
    SkASSERT(GrPrimitiveRestart::kNo == primRestart);
    int inputBufferIndex = 0;
    if (vertexBuffer) {
        SkASSERT(!vertexBuffer->isCpuBuffer());
        SkASSERT(!static_cast<const GrGpuBuffer*>(vertexBuffer.get())->isMapped());
        fActiveVertexBuffer = std::move(vertexBuffer);
        fGpu->commandBuffer()->addGrBuffer(fActiveVertexBuffer);
        ++inputBufferIndex;
    }
    if (instanceBuffer) {
        SkASSERT(!instanceBuffer->isCpuBuffer());
        SkASSERT(!static_cast<const GrGpuBuffer*>(instanceBuffer.get())->isMapped());
        this->setVertexBuffer(fActiveRenderCmdEncoder, instanceBuffer.get(), 0, inputBufferIndex++);
        fActiveInstanceBuffer = std::move(instanceBuffer);
        fGpu->commandBuffer()->addGrBuffer(fActiveInstanceBuffer);
    }
    if (indexBuffer) {
        SkASSERT(!indexBuffer->isCpuBuffer());
        SkASSERT(!static_cast<const GrGpuBuffer*>(indexBuffer.get())->isMapped());
        fActiveIndexBuffer = std::move(indexBuffer);
        fGpu->commandBuffer()->addGrBuffer(fActiveIndexBuffer);
    }
}

void GrMtlOpsRenderPass::onDraw(int vertexCount, int baseVertex) {
    SkASSERT(fActivePipelineState);
    SkASSERT(nil != fActiveRenderCmdEncoder);
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    if (!fDebugGroupActive) {
        fActiveRenderCmdEncoder->pushDebugGroup(@"bindAndDraw");
        fDebugGroupActive = true;
    }
#endif
    this->setVertexBuffer(fActiveRenderCmdEncoder, fActiveVertexBuffer.get(), 0, 0);

    fActiveRenderCmdEncoder->drawPrimitives(fActivePrimitiveType, baseVertex, vertexCount);
    fGpu->stats()->incNumDraws();
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    SkASSERT(fDebugGroupActive);
    fActiveRenderCmdEncoder->popDebugGroup();
    fDebugGroupActive = false;
#endif
}

void GrMtlOpsRenderPass::onDrawIndexed(int indexCount, int baseIndex, uint16_t minIndexValue,
                                       uint16_t maxIndexValue, int baseVertex) {
    SkASSERT(fActivePipelineState);
    SkASSERT(nil != fActiveRenderCmdEncoder);
    SkASSERT(fActiveIndexBuffer);
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    if (!fDebugGroupActive) {
        fActiveRenderCmdEncoder->pushDebugGroup(@"bindAndDraw");
        fDebugGroupActive = true;
    }
#endif
    this->setVertexBuffer(fActiveRenderCmdEncoder, fActiveVertexBuffer.get(),
                          fCurrentVertexStride * baseVertex, 0);

    auto mtlIndexBuffer = static_cast<const GrMtlBuffer*>(fActiveIndexBuffer.get());
    size_t indexOffset = sizeof(uint16_t) * baseIndex;
    id<MTLBuffer> indexBuffer = mtlIndexBuffer->mtlBuffer();
    fActiveRenderCmdEncoder->drawIndexedPrimitives(fActivePrimitiveType, indexCount,
                                                   MTLIndexTypeUInt16, indexBuffer, indexOffset);
    fGpu->stats()->incNumDraws();
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    SkASSERT(fDebugGroupActive);
    fActiveRenderCmdEncoder->popDebugGroup();
    fDebugGroupActive = false;
#endif
}

void GrMtlOpsRenderPass::onDrawInstanced(int instanceCount, int baseInstance, int vertexCount,
                                         int baseVertex) {
    SkASSERT(fActivePipelineState);
    SkASSERT(nil != fActiveRenderCmdEncoder);
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    if (!fDebugGroupActive) {
        fActiveRenderCmdEncoder->pushDebugGroup(@"bindAndDraw");
        fDebugGroupActive = true;
    }
#endif
    this->setVertexBuffer(fActiveRenderCmdEncoder, fActiveVertexBuffer.get(), 0, 0);

    if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
        fActiveRenderCmdEncoder->drawPrimitives(fActivePrimitiveType, baseVertex, vertexCount,
                                                instanceCount, baseInstance);
    } else {
        SkASSERT(false);
    }
    fGpu->stats()->incNumDraws();
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    SkASSERT(fDebugGroupActive);
    fActiveRenderCmdEncoder->popDebugGroup();
    fDebugGroupActive = false;
#endif
}

void GrMtlOpsRenderPass::onDrawIndexedInstanced(
        int indexCount, int baseIndex, int instanceCount, int baseInstance, int baseVertex) {
    SkASSERT(fActivePipelineState);
    SkASSERT(nil != fActiveRenderCmdEncoder);
    SkASSERT(fActiveIndexBuffer);
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    if (!fDebugGroupActive) {
        fActiveRenderCmdEncoder->pushDebugGroup(@"bindAndDraw");
        fDebugGroupActive = true;
    }
#endif
    this->setVertexBuffer(fActiveRenderCmdEncoder, fActiveVertexBuffer.get(), 0, 0);

    auto mtlIndexBuffer = static_cast<const GrMtlBuffer*>(fActiveIndexBuffer.get());
    size_t indexOffset = sizeof(uint16_t) * baseIndex;
    if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
        fActiveRenderCmdEncoder->drawIndexedPrimitives(fActivePrimitiveType, indexCount,
                                                       MTLIndexTypeUInt16,
                                                       mtlIndexBuffer->mtlBuffer(), indexOffset,
                                                       instanceCount, baseVertex, baseInstance);
    } else {
        SkASSERT(false);
    }
    fGpu->stats()->incNumDraws();
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    SkASSERT(fDebugGroupActive);
    fActiveRenderCmdEncoder->popDebugGroup();
    fDebugGroupActive = false;
#endif
}

void GrMtlOpsRenderPass::onDrawIndirect(const GrBuffer* drawIndirectBuffer,
                                        size_t bufferOffset,
                                        int drawCount) {
    SkASSERT(fGpu->caps()->nativeDrawIndirectSupport());
    SkASSERT(fActivePipelineState);
    SkASSERT(nil != fActiveRenderCmdEncoder);
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    if (!fDebugGroupActive) {
        fActiveRenderCmdEncoder->pushDebugGroup(@"bindAndDraw");
        fDebugGroupActive = true;
    }
#endif
    this->setVertexBuffer(fActiveRenderCmdEncoder, fActiveVertexBuffer.get(), 0, 0);

    auto mtlIndirectBuffer = static_cast<const GrMtlBuffer*>(drawIndirectBuffer);
    const size_t stride = sizeof(GrDrawIndirectCommand);
    while (drawCount >= 1) {
        if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
            fActiveRenderCmdEncoder->drawPrimitives(fActivePrimitiveType,
                                                    mtlIndirectBuffer->mtlBuffer(), bufferOffset);
        } else {
            SkASSERT(false);
        }
        drawCount--;
        bufferOffset += stride;
        fGpu->stats()->incNumDraws();
    }
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    SkASSERT(fDebugGroupActive);
    fActiveRenderCmdEncoder->popDebugGroup();
    fDebugGroupActive = false;
#endif
}

void GrMtlOpsRenderPass::onDrawIndexedIndirect(const GrBuffer* drawIndirectBuffer,
                                               size_t bufferOffset,
                                               int drawCount) {
    SkASSERT(fGpu->caps()->nativeDrawIndirectSupport());
    SkASSERT(fActivePipelineState);
    SkASSERT(nil != fActiveRenderCmdEncoder);
    SkASSERT(fActiveIndexBuffer);
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    if (!fDebugGroupActive) {
        fActiveRenderCmdEncoder->pushDebugGroup(@"bindAndDraw");
        fDebugGroupActive = true;
    }
#endif
    this->setVertexBuffer(fActiveRenderCmdEncoder, fActiveVertexBuffer.get(), 0, 0);

    auto mtlIndexBuffer = static_cast<const GrMtlBuffer*>(fActiveIndexBuffer.get());
    auto mtlIndirectBuffer = static_cast<const GrMtlBuffer*>(drawIndirectBuffer);
    size_t indexOffset = 0;

    const size_t stride = sizeof(GrDrawIndexedIndirectCommand);
    while (drawCount >= 1) {
        if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
            fActiveRenderCmdEncoder->drawIndexedPrimitives(fActivePrimitiveType,
                                                           MTLIndexTypeUInt16,
                                                           mtlIndexBuffer->mtlBuffer(),
                                                           indexOffset,
                                                           mtlIndirectBuffer->mtlBuffer(),
                                                           bufferOffset);
        } else {
            SkASSERT(false);
        }
        drawCount--;
        bufferOffset += stride;
        fGpu->stats()->incNumDraws();
    }
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    SkASSERT(fDebugGroupActive);
    fActiveRenderCmdEncoder->popDebugGroup();
    fDebugGroupActive = false;
#endif
}

void GrMtlOpsRenderPass::setVertexBuffer(GrMtlRenderCommandEncoder* encoder,
                                         const GrBuffer* buffer,
                                         size_t vertexOffset,
                                         size_t inputBufferIndex) {
    if (!buffer) {
        return;
    }

    // point after the uniforms
    constexpr static int kFirstBufferBindingIdx = GrMtlUniformHandler::kLastUniformBinding + 1;
    int index = inputBufferIndex + kFirstBufferBindingIdx;
    SkASSERT(index < 4);
    auto mtlBuffer = static_cast<const GrMtlBuffer*>(buffer);
    id<MTLBuffer> mtlVertexBuffer = mtlBuffer->mtlBuffer();
    SkASSERT(mtlVertexBuffer);
    size_t offset = vertexOffset;
    encoder->setVertexBuffer(mtlVertexBuffer, offset, index);
}

GR_NORETAIN_END
