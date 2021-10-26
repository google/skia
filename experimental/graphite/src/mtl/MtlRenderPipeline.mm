/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlRenderPipeline.h"

#include "experimental/graphite/src/RenderPipelineDesc.h"
#include "experimental/graphite/src/mtl/MtlGpu.h"
#include "experimental/graphite/src/mtl/MtlUtils.h"
#include "include/private/SkSLString.h"

namespace skgpu::mtl {

static const char* kTestingOnlyShaders[] = {
    // clear viewport to blue
    "#include <metal_stdlib>\n"
    "#include <simd/simd.h>\n"
    "using namespace metal;\n"
    "\n"
    "typedef struct {\n"
    "    float4 position [[position]];\n"
    "} VertexOutput;\n"
    "\n"
    "vertex VertexOutput vertexMain(uint vertexID [[vertex_id]]) {\n"
    "    VertexOutput out;\n"
    "    float2 position = float2(float(vertexID >> 1), float(vertexID & 1));\n"
    "    out.position.xy = position * 2 - 1;\n"
    "    out.position.zw = float2(0.0, 1.0);\n"
    "    return out;\n"
    "}\n"
    "\n"
    "fragment float4 fragmentMain(VertexOutput in [[stage_in]]) {\n"
    "    return float4(0.0, 0.0, 1.0, 1.0);\n"
    "}",

    // clear subarea to given color, using uniform buffer
    "#include <metal_stdlib>\n"
    "#include <simd/simd.h>\n"
    "using namespace metal;\n"
    "\n"
    "typedef struct {\n"
    "    float4 position [[position]];\n"
    "} VertexOutput;\n"
    "\n"
    "typedef struct {\n"
    "    float4 uPosXform;\n"
    "    float4 uColor;\n"
    "} UniformData;\n"
    "\n"
    "vertex VertexOutput vertexMain(constant UniformData& uniforms [[buffer(0)]],\n"
    "                               uint vertexID [[vertex_id]]) {\n"
    "    VertexOutput out;\n"
    "    float2 position = float2(float(vertexID >> 1), float(vertexID & 1));\n"
    "    out.position.xy = position * uniforms.uPosXform.xy + uniforms.uPosXform.zw;\n"
    "    out.position.zw = float2(0.0, 1.0);\n"
    "    return out;\n"
    "}\n"
    "\n"
    "fragment float4 fragmentMain(constant UniformData& uniforms [[buffer(0)]],\n"
    "                             VertexOutput in [[stage_in]]) {\n"
    "    return uniforms.uColor;\n"
    "}",
};

static constexpr NSString* kTestingOnlyShaderLabels[]  = {
    @"Clear viewport to blue",
    @"Clear rect with uniforms"
};

sk_sp<RenderPipeline> RenderPipeline::Make(const Gpu* gpu, const skgpu::RenderPipelineDesc& desc) {
    sk_cfp<MTLRenderPipelineDescriptor*> psoDescriptor([[MTLRenderPipelineDescriptor alloc] init]);

    // Temp pipeline for now that just fills the viewport with blue
    int shaderIndex = desc.testingOnlyShaderIndex();
    SkSL::String shaderText;
    shaderText.append(kTestingOnlyShaders[shaderIndex]);

    auto metallib = CompileShaderLibrary(gpu, shaderText);
    if (!metallib) {
        return nullptr;
    }

    (*psoDescriptor).label = kTestingOnlyShaderLabels[shaderIndex];

    (*psoDescriptor).vertexFunction =
            [*metallib newFunctionWithName: @"vertexMain"];
    (*psoDescriptor).fragmentFunction =
            [*metallib newFunctionWithName: @"fragmentMain"];

    // TODO: I *think* this gets cleaned up by the pipelineDescriptor?
    auto mtlColorAttachment = [[MTLRenderPipelineColorAttachmentDescriptor alloc] init];

    mtlColorAttachment.pixelFormat = MTLPixelFormatRGBA8Unorm;
    mtlColorAttachment.blendingEnabled = FALSE;
    mtlColorAttachment.writeMask = MTLColorWriteMaskAll;

    (*psoDescriptor).colorAttachments[0] = mtlColorAttachment;
    (*psoDescriptor).sampleCount = 1;

    NSError* error;
    sk_cfp<id<MTLRenderPipelineState>> pso(
            [gpu->device() newRenderPipelineStateWithDescriptor:psoDescriptor.get()
                                                          error:&error]);
    return sk_sp<RenderPipeline>(new RenderPipeline(std::move(pso)));
}

} // namespace skgpu::mtl
