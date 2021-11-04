/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlGraphicsPipeline.h"

#include "experimental/graphite/src/GraphicsPipelineDesc.h"
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

    // draw triangles with given color, using uniform buffer and vertex data
    "#include <metal_stdlib>\n"
    "#include <simd/simd.h>\n"
    "using namespace metal;\n"
    "\n"
    "typedef struct {\n"
    "    float2 position [[attribute(0)]];\n"
    "} VertexInput;\n"
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
    "vertex VertexOutput vertexMain(VertexInput in [[stage_in]],\n"
    "                               constant UniformData& uniforms [[buffer(0)]],\n"
    "                               uint vertexID [[vertex_id]]) {\n"
    "    VertexOutput out;\n"
    "    float2 position = in.position;\n"
    "    out.position.xy = position * uniforms.uPosXform.xy + uniforms.uPosXform.zw;\n"
    "    out.position.zw = float2(0.0, 1.0);\n"
    "    return out;\n"
    "}\n"
    "\n"
    "fragment float4 fragmentMain(constant UniformData& uniforms [[buffer(0)]],\n"
    "                             VertexOutput in [[stage_in]]) {\n"
    "    return uniforms.uColor;\n"
    "}",

    // draw triangles with vertex ID and instance buffer
    "#include <metal_stdlib>\n"
    "#include <simd/simd.h>\n"
    "using namespace metal;\n"
    "\n"
    "typedef struct {\n"
    "    float2 position [[attribute(0)]];\n"
    "    float2 dims [[attribute(1)]];\n"
    "    float4 color [[attribute(2)]];\n"
    "} InstanceInput;\n"
    "\n"
    "typedef struct {\n"
    "    float4 position [[position]];\n"
    "    float4 color;\n"
    "} VertexOutput;\n"
    "\n"
    "vertex VertexOutput vertexMain(InstanceInput in [[stage_in]],\n"
    "                               uint vertexID [[vertex_id]]) {\n"
    "    VertexOutput out;\n"
    "    float2 position = float2(float(vertexID >> 1), float(vertexID & 1));\n"
    "    out.position.xy = position * in.dims + in.position;\n"
    "    out.position.zw = float2(0.0, 1.0);\n"
    "    out.color = in.color;"
    "    return out;\n"
    "}\n"
    "\n"
    "fragment float4 fragmentMain(VertexOutput in [[stage_in]]) {\n"
    "    return in.color;\n"
    "}",
};

static constexpr NSString* kTestingOnlyShaderLabels[]  = {
    @"Clear viewport to blue",
    @"Clear rect with uniforms",
    @"Draw triangles with uniform color",
    @"Draw triangles with instance buffer"
};

static inline MTLVertexFormat attribute_type_to_mtlformat(VertexAttribType type) {
    switch (type) {
        case VertexAttribType::kFloat:
            return MTLVertexFormatFloat;
        case VertexAttribType::kFloat2:
            return MTLVertexFormatFloat2;
        case VertexAttribType::kFloat3:
            return MTLVertexFormatFloat3;
        case VertexAttribType::kFloat4:
            return MTLVertexFormatFloat4;
        case VertexAttribType::kHalf:
            if (@available(macOS 10.13, iOS 11.0, *)) {
                return MTLVertexFormatHalf;
            } else {
                return MTLVertexFormatInvalid;
            }
        case VertexAttribType::kHalf2:
            return MTLVertexFormatHalf2;
        case VertexAttribType::kHalf4:
            return MTLVertexFormatHalf4;
        case VertexAttribType::kInt2:
            return MTLVertexFormatInt2;
        case VertexAttribType::kInt3:
            return MTLVertexFormatInt3;
        case VertexAttribType::kInt4:
            return MTLVertexFormatInt4;
        case VertexAttribType::kByte:
            if (@available(macOS 10.13, iOS 11.0, *)) {
                return MTLVertexFormatChar;
            } else {
                return MTLVertexFormatInvalid;
            }
        case VertexAttribType::kByte2:
            return MTLVertexFormatChar2;
        case VertexAttribType::kByte4:
            return MTLVertexFormatChar4;
        case VertexAttribType::kUByte:
            if (@available(macOS 10.13, iOS 11.0, *)) {
                return MTLVertexFormatUChar;
            } else {
                return MTLVertexFormatInvalid;
            }
        case VertexAttribType::kUByte2:
            return MTLVertexFormatUChar2;
        case VertexAttribType::kUByte4:
            return MTLVertexFormatUChar4;
        case VertexAttribType::kUByte_norm:
            if (@available(macOS 10.13, iOS 11.0, *)) {
                return MTLVertexFormatUCharNormalized;
            } else {
                return MTLVertexFormatInvalid;
            }
        case VertexAttribType::kUByte4_norm:
            return MTLVertexFormatUChar4Normalized;
        case VertexAttribType::kShort2:
            return MTLVertexFormatShort2;
        case VertexAttribType::kShort4:
            return MTLVertexFormatShort4;
        case VertexAttribType::kUShort2:
            return MTLVertexFormatUShort2;
        case VertexAttribType::kUShort2_norm:
            return MTLVertexFormatUShort2Normalized;
        case VertexAttribType::kInt:
            return MTLVertexFormatInt;
        case VertexAttribType::kUInt:
            return MTLVertexFormatUInt;
        case VertexAttribType::kUShort_norm:
            if (@available(macOS 10.13, iOS 11.0, *)) {
                return MTLVertexFormatUShortNormalized;
            } else {
                return MTLVertexFormatInvalid;
            }
        case VertexAttribType::kUShort4_norm:
            return MTLVertexFormatUShort4Normalized;
    }
    SK_ABORT("Unknown vertex attribute type");
}

static MTLVertexDescriptor* create_vertex_descriptor(const GraphicsPipelineDesc& desc) {
    auto vertexDescriptor = [[MTLVertexDescriptor alloc] init];
    int attributeIndex = 0;

    int vertexAttributeCount = desc.numVertexAttributes();
    size_t vertexAttributeOffset = 0;
    for (const auto& attribute : desc.vertexAttributes()) {
        MTLVertexAttributeDescriptor* mtlAttribute = vertexDescriptor.attributes[attributeIndex];
        MTLVertexFormat format = attribute_type_to_mtlformat(attribute.cpuType());
        SkASSERT(MTLVertexFormatInvalid != format);
        mtlAttribute.format = format;
        mtlAttribute.offset = vertexAttributeOffset;
        mtlAttribute.bufferIndex = GraphicsPipeline::kVertexBufferIndex;

        vertexAttributeOffset += attribute.sizeAlign4();
        attributeIndex++;
    }
    SkASSERT(vertexAttributeOffset == desc.vertexStride());

    if (vertexAttributeCount) {
        MTLVertexBufferLayoutDescriptor* vertexBufferLayout =
                vertexDescriptor.layouts[GraphicsPipeline::kVertexBufferIndex];
        vertexBufferLayout.stepFunction = MTLVertexStepFunctionPerVertex;
        vertexBufferLayout.stepRate = 1;
        vertexBufferLayout.stride = vertexAttributeOffset;
    }

    int instanceAttributeCount = desc.numInstanceAttributes();
    size_t instanceAttributeOffset = 0;
    for (const auto& attribute : desc.instanceAttributes()) {
        MTLVertexAttributeDescriptor* mtlAttribute = vertexDescriptor.attributes[attributeIndex];
        MTLVertexFormat format = attribute_type_to_mtlformat(attribute.cpuType());
        SkASSERT(MTLVertexFormatInvalid != format);
        mtlAttribute.format = format;
        mtlAttribute.offset = instanceAttributeOffset;
        mtlAttribute.bufferIndex = GraphicsPipeline::kInstanceBufferIndex;

        instanceAttributeOffset += attribute.sizeAlign4();
        attributeIndex++;
    }
    SkASSERT(instanceAttributeOffset == desc.instanceStride());

    if (instanceAttributeCount) {
        MTLVertexBufferLayoutDescriptor* instanceBufferLayout =
                vertexDescriptor.layouts[GraphicsPipeline::kInstanceBufferIndex];
        instanceBufferLayout.stepFunction = MTLVertexStepFunctionPerInstance;
        instanceBufferLayout.stepRate = 1;
        instanceBufferLayout.stride = instanceAttributeOffset;
    }
    return vertexDescriptor;
}

sk_sp<GraphicsPipeline> GraphicsPipeline::Make(const Gpu* gpu,
                                               const skgpu::GraphicsPipelineDesc& desc) {
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
    (*psoDescriptor).vertexDescriptor = create_vertex_descriptor(desc);

    // TODO: I *think* this gets cleaned up by the pipelineDescriptor as well?
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
    if (!pso) {
        SkDebugf("Pipeline creation failure\n");
        SkDebugf("Errors:\n%s", error.debugDescription.UTF8String);
        return nullptr;
    }
    return sk_sp<GraphicsPipeline>(new GraphicsPipeline(std::move(pso), desc.vertexStride(),
                                                        desc.instanceStride()));
}

} // namespace skgpu::mtl
