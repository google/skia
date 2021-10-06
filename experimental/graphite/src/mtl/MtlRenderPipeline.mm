/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlRenderPipeline.h"

#include "experimental/graphite/src/mtl/MtlGpu.h"
#include "experimental/graphite/src/mtl/MtlUtils.h"
#include "include/private/SkSLString.h"

namespace skgpu::mtl {

std::unique_ptr<RenderPipeline> RenderPipeline::Make(const Gpu* gpu,
                                                     const skgpu::RenderPipelineDesc&) {
    sk_cfp<MTLRenderPipelineDescriptor*> psoDescriptor([[MTLRenderPipelineDescriptor alloc] init]);

    // Temp pipeline for now that just fills the viewport with red
    SkSL::String shaderText;
    shaderText.append(
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
            "    return float4(1.0, 0.0, 0.0, 1.0);\n"
            "}"
    );

    auto metallib = CompileShaderLibrary(gpu, shaderText);
    if (!metallib) {
        return nullptr;
    }

    (*psoDescriptor).label = @"testPipeline";

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
    return std::unique_ptr<RenderPipeline>(new RenderPipeline(std::move(pso)));
}

} // namespace skgpu::mtl
