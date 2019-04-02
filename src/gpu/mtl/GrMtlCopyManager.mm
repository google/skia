/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlCopyManager.h"

#include "GrSurface.h"

#include "GrMtlBuffer.h"
#include "GrMtlCopyPipelineState.h"
#include "GrMtlGpu.h"
#include "GrMtlResourceProvider.h"
#include "GrMtlUtil.h"

#include "SkPoint.h"
#include "SkRect.h"
#include "SkTraceEvent.h"

#import <simd/simd.h>

void GrMtlCopyManager::createCopyProgramBuffer() {
    // Create per vertex attribute data for copy as draw
    static const simd::float2 vdata[4] = {
        {0, 0},
        {0, 1},
        {1, 0},
        {1, 1},
    };
    sk_sp<GrMtlBuffer> mtlBuffer = GrMtlBuffer::Make(fGpu, sizeof(vdata), GrGpuBufferType::kVertex,
                                                     kStatic_GrAccessPattern, vdata);
    fVertexAttributeBuffer = mtlBuffer->mtlBuffer();
}

void GrMtlCopyManager::createCopyProgramShaders() {
     // Create shaders required by pipeline state
    const GrShaderCaps* shaderCaps = fGpu->caps()->shaderCaps();
    const char* version = shaderCaps->versionDeclString();
    SkString vertShaderText(version);
    vertShaderText.appendf(
        "#extension GL_ARB_separate_shader_objects : enable\n"
        "#extension GL_ARB_shading_language_420pack : enable\n"
        "layout(set = %d"/*kUniform_BufferIndex*/", binding = 0) uniform vertexUniformBuffer {"
            "float4 uPosXform;"
            "float4 uTexCoordXform;"
        "};"
        "layout(location = 0) in float2 inPosition;"
        "layout(location = 1) out float2 vTexCoord;"

        "// Copy Program VS\n"
        "void main() {"
            "vTexCoord = inPosition * uTexCoordXform.xy + uTexCoordXform.zw;"
            "sk_Position.xy = inPosition * uPosXform.xy + uPosXform.zw;"
            "sk_Position.zw = float2(0, 1);"
        "}",
        kUniform_BufferIndex
    );

    SkString fragShaderText(version);
    fragShaderText.append(
        "#extension GL_ARB_separate_shader_objects : enable\n"
        "#extension GL_ARB_shading_language_420pack : enable\n"

        "layout(set = 1, binding = 0) uniform sampler2D uTexture;"
        "layout(location = 1) in float2 vTexCoord;"

        "// Copy Program FS\n"
        "void main() {"
            "sk_FragColor = texture(uTexture, vTexCoord);"
        "}"
    );

    SkSL::Program::Settings settings;
    SkSL::Program::Inputs inputs;
    id<MTLLibrary> vertexLibrary = GrCompileMtlShaderLibrary(fGpu, vertShaderText.c_str(),
                                                             SkSL::Program::kVertex_Kind,
                                                             settings, &inputs);
    SkASSERT(inputs.isEmpty());
    SkASSERT(vertexLibrary);

    id<MTLLibrary> fragmentLibrary = GrCompileMtlShaderLibrary(fGpu, fragShaderText.c_str(),
                                                               SkSL::Program::kFragment_Kind,
                                                               settings, &inputs);
    SkASSERT(inputs.isEmpty());
    SkASSERT(fragmentLibrary);

    id<MTLFunction> vertexFunction = [vertexLibrary newFunctionWithName: @"vertexMain"];
    id<MTLFunction> fragmentFunction = [fragmentLibrary newFunctionWithName: @"fragmentMain"];
    SkASSERT(vertexFunction);
    SkASSERT(fragmentFunction);

    fVertexFunction = vertexFunction;
    fFragmentFunction = fragmentFunction;
}

void GrMtlCopyManager::createCopyProgramVertexDescriptor() {
    // Create vertex descriptor for pipeline state
    // Expected [[stage_in]] (vertex attribute) MSL format for copies:
    //
    // struct Input {
    //     float2 inPosition [[attribute(0)]];
    // };
    MTLVertexDescriptor* vertexDescriptor = [[MTLVertexDescriptor alloc] init];
    vertexDescriptor.attributes[0].format = MTLVertexFormatFloat2;
    vertexDescriptor.attributes[0].offset = 0;
    vertexDescriptor.attributes[0].bufferIndex = kAttribute_BufferIndex;

    vertexDescriptor.layouts[kAttribute_BufferIndex].stepFunction = MTLVertexStepFunctionPerVertex;
    vertexDescriptor.layouts[kAttribute_BufferIndex].stepRate = 1;
    vertexDescriptor.layouts[kAttribute_BufferIndex].stride = sizeof(simd::float2);

    fVertexDescriptor = vertexDescriptor;
}

void GrMtlCopyManager::createCopyProgram() {
    TRACE_EVENT0("skia", TRACE_FUNC);

    MTLSamplerDescriptor* samplerDescriptor = [[MTLSamplerDescriptor alloc] init];
    fSamplerState = [fGpu->device() newSamplerStateWithDescriptor: samplerDescriptor];

    this->createCopyProgramBuffer();
    this->createCopyProgramShaders();
    this->createCopyProgramVertexDescriptor();
}

bool GrMtlCopyManager::copySurfaceAsDraw(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                                         GrSurface* src, GrSurfaceOrigin srcOrigin,
                                         const SkIRect& srcRect, const SkIPoint& dstPoint,
                                         bool canDiscardOutsideDstRect) {
    SkASSERT(fGpu->mtlCaps().canCopyAsDraw(dst->config(), SkToBool(dst->asRenderTarget()),
                                           src->config(), SkToBool(src->asTexture())));

    id<MTLTexture> dstTex = GrGetMTLTextureFromSurface(dst, false);
    id<MTLTexture> srcTex = GrGetMTLTextureFromSurface(src, false);

    if (fSamplerState == nil) {
        SkASSERT(fVertexAttributeBuffer == nil);
        SkASSERT(fVertexFunction == nil);
        SkASSERT(fFragmentFunction == nil);
        SkASSERT(fVertexDescriptor == nil);

        this->createCopyProgram();
    }

    if (!(fSamplerState && fVertexAttributeBuffer && fVertexFunction &&
          fFragmentFunction && fVertexDescriptor)) {
        SkASSERT(false);
        return false;
    }

    // UPDATE UNIFORM DESCRIPTOR SET
    int w = srcRect.width();
    int h = srcRect.height();

    // dst rect edges in NDC (-1 to 1)
    int dw = dstTex.width;
    int dh = dstTex.height;
    float dx0 = 2.f * dstPoint.fX / dw - 1.f;
    float dx1 = 2.f * (dstPoint.fX + w) / dw - 1.f;
    float dy0 = 2.f * dstPoint.fY / dh - 1.f;
    float dy1 = 2.f * (dstPoint.fY + h) / dh - 1.f;
    if (kBottomLeft_GrSurfaceOrigin == dstOrigin) {
        dy0 = -dy0;
        dy1 = -dy1;
    }

    float sx0 = (float)srcRect.fLeft;
    float sx1 = (float)(srcRect.fLeft + w);
    float sy0 = (float)srcRect.fTop;
    float sy1 = (float)(srcRect.fTop + h);
    int sh = srcTex.height;
    if (kBottomLeft_GrSurfaceOrigin == srcOrigin) {
        sy0 = sh - sy0;
        sy1 = sh - sy1;
    }

    // src rect edges in normalized texture space (0 to 1).
    int sw = srcTex.width;
    sx0 /= sw;
    sx1 /= sw;
    sy0 /= sh;
    sy1 /= sh;

    const simd::float4 vertexUniformBuffer[2] = {
        {dx1 - dx0, dy1 - dy0, dx0, dy0}, // posXform
        {sx1 - sx0, sy1 - sy0, sx0, sy0}, // texCoordXform
    };

    MTLRenderPassDescriptor* renderPassDesc = [MTLRenderPassDescriptor renderPassDescriptor];
    renderPassDesc.colorAttachments[0].texture = dstTex;
    renderPassDesc.colorAttachments[0].slice = 0;
    renderPassDesc.colorAttachments[0].level = 0;
    renderPassDesc.colorAttachments[0].loadAction = canDiscardOutsideDstRect ? MTLLoadActionDontCare
                                                                             : MTLLoadActionLoad;
    renderPassDesc.colorAttachments[0].storeAction = MTLStoreActionStore;

    id<MTLRenderCommandEncoder> renderCmdEncoder =
            [fGpu->commandBuffer() renderCommandEncoderWithDescriptor: renderPassDesc];
    GrMtlCopyPipelineState* copyPipelineState =
            fGpu->resourceProvider().findOrCreateCopyPipelineState(dstTex.pixelFormat,
                                                                   fVertexFunction,
                                                                   fFragmentFunction,
                                                                   fVertexDescriptor);
    [renderCmdEncoder setRenderPipelineState: copyPipelineState->mtlCopyPipelineState()];
    [renderCmdEncoder setVertexBuffer: fVertexAttributeBuffer
                               offset: 0
                              atIndex: kAttribute_BufferIndex];
    [renderCmdEncoder setVertexBytes: vertexUniformBuffer
                              length: sizeof(vertexUniformBuffer)
                             atIndex: kUniform_BufferIndex];
    [renderCmdEncoder setFragmentTexture: srcTex
                                 atIndex: 0];
    [renderCmdEncoder setFragmentSamplerState: fSamplerState
                                      atIndex: 0];
    [renderCmdEncoder drawPrimitives: MTLPrimitiveTypeTriangleStrip
                         vertexStart: 0
                         vertexCount: 4];
    [renderCmdEncoder endEncoding];
    return true;
}

bool GrMtlCopyManager::IsCompatible(const GrMtlCopyPipelineState* pipelineState,
                                    MTLPixelFormat dstPixelFormat) {
    return pipelineState->fPixelFormat == dstPixelFormat;
}
