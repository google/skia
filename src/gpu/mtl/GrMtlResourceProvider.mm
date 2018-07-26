/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlResourceProvider.h"

#include "GrMtlGpu.h"
#include "GrMtlUtil.h"

#include "SkSLCompiler.h"

#import <simd/simd.h>

void GrMtlResourceProvider::initCopyPipelineResources() {
    // Create shaders required by pipeline state
    const GrShaderCaps* shaderCaps = fGpu->caps()->shaderCaps();
    const char* version = shaderCaps->versionDeclString();
    SkString vertShaderText(version);
    vertShaderText.append(
        "#extension GL_ARB_separate_shader_objects : enable\n"
        "#extension GL_ARB_shading_language_420pack : enable\n"

        "layout(set = 1, binding = 0) uniform vertexUniformBuffer {"
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
        "}"
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

    // Create vertex descriptor for pipeline state
    // Expected [[stage_in]] (vertex attribute) MSL format for copies:
    //
    // struct Input {
    //     float2 inPosition [[attribute(0)]];
    // };
    MTLVertexDescriptor* vertexDescriptor = [[MTLVertexDescriptor alloc] init];
    vertexDescriptor.attributes[0].format = MTLVertexFormatFloat2;
    vertexDescriptor.attributes[0].offset = 0;
    vertexDescriptor.attributes[0].bufferIndex = 0;

    vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
    vertexDescriptor.layouts[0].stepRate = 1;
    vertexDescriptor.layouts[0].stride = sizeof(simd::float2);

    fCopyPipelineResources.fVertexFunction = vertexFunction;
    fCopyPipelineResources.fFragmentFunction = fragmentFunction;
    fCopyPipelineResources.fVertexDescriptor = vertexDescriptor;
}

id<MTLRenderPipelineState> GrMtlResourceProvider::findOrCreateCopyPipelineState(
        id<MTLTexture> dstTex) {

    for (const auto& pipelineState: fCopyPipelineStateCache) {
        if (pipelineState.fPixelFormat == dstTex.pixelFormat) {
            SkASSERT(pipelineState.fPipelineState);
            return pipelineState.fPipelineState;
        }
    }

    if (fCopyPipelineResources.fVertexFunction == nil) {
        SkASSERT(fCopyPipelineResources.fFragmentFunction == nil);
        SkASSERT(fCopyPipelineResources.fVertexDescriptor == nil);
        this->initCopyPipelineResources();
    }

    // Create pipeline state for copy as draw
    MTLRenderPipelineDescriptor* pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineDescriptor.vertexFunction = fCopyPipelineResources.fVertexFunction;
    pipelineDescriptor.fragmentFunction = fCopyPipelineResources.fFragmentFunction;
    pipelineDescriptor.vertexDescriptor = fCopyPipelineResources.fVertexDescriptor;
    pipelineDescriptor.colorAttachments[0].pixelFormat = dstTex.pixelFormat;

    NSError* error = nil;
    id<MTLRenderPipelineState> pipelineState =
            [fGpu->device() newRenderPipelineStateWithDescriptor: pipelineDescriptor
                                                           error: &error];
    if (error) {
        SkDebugf("Error creating pipeline: %s\n",
                 [[error localizedDescription] cStringUsingEncoding: NSASCIIStringEncoding]);
        return nil;
    }

    SkASSERT(pipelineState);
    fCopyPipelineStateCache.emplace_back(MtlCopyPipelineStateInfo{pipelineState,
                                                                  dstTex.pixelFormat});
    return pipelineState;
}
