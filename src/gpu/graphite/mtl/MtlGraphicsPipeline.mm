/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/mtl/MtlGraphicsPipeline.h"

#include "include/gpu/graphite/TextureInfo.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/mtl/MtlResourceProvider.h"
#include "src/gpu/graphite/mtl/MtlSharedContext.h"
#include "src/gpu/mtl/MtlUtilsPriv.h"

namespace skgpu::graphite {

namespace {

inline MTLVertexFormat attribute_type_to_mtlformat(VertexAttribType type) {
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
            if (@available(macOS 10.13, iOS 11.0, tvOS 11.0, *)) {
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
            if (@available(macOS 10.13, iOS 11.0, tvOS 11.0, *)) {
                return MTLVertexFormatChar;
            } else {
                return MTLVertexFormatInvalid;
            }
        case VertexAttribType::kByte2:
            return MTLVertexFormatChar2;
        case VertexAttribType::kByte4:
            return MTLVertexFormatChar4;
        case VertexAttribType::kUByte:
            if (@available(macOS 10.13, iOS 11.0, tvOS 11.0, *)) {
                return MTLVertexFormatUChar;
            } else {
                return MTLVertexFormatInvalid;
            }
        case VertexAttribType::kUByte2:
            return MTLVertexFormatUChar2;
        case VertexAttribType::kUByte4:
            return MTLVertexFormatUChar4;
        case VertexAttribType::kUByte_norm:
            if (@available(macOS 10.13, iOS 11.0, tvOS 11.0, *)) {
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
            if (@available(macOS 10.13, iOS 11.0, tvOS 11.0, *)) {
                return MTLVertexFormatUShortNormalized;
            } else {
                return MTLVertexFormatInvalid;
            }
        case VertexAttribType::kUShort4_norm:
            return MTLVertexFormatUShort4Normalized;
    }
    SK_ABORT("Unknown vertex attribute type");
}

MTLVertexDescriptor* create_vertex_descriptor(SkSpan<const Attribute> vertexAttrs,
                                              SkSpan<const Attribute> instanceAttrs) {
    auto vertexDescriptor = [[MTLVertexDescriptor alloc] init];
    int attributeIndex = 0;

    size_t vertexAttributeOffset = 0;
    for (const auto& attribute : vertexAttrs) {
        MTLVertexAttributeDescriptor* mtlAttribute = vertexDescriptor.attributes[attributeIndex];
        MTLVertexFormat format = attribute_type_to_mtlformat(attribute.cpuType());
        SkASSERT(MTLVertexFormatInvalid != format);
        mtlAttribute.format = format;
        mtlAttribute.offset = vertexAttributeOffset;
        mtlAttribute.bufferIndex = MtlGraphicsPipeline::kVertexBufferIndex;

        vertexAttributeOffset += attribute.sizeAlign4();
        attributeIndex++;
    }

    if (vertexAttributeOffset) {
        MTLVertexBufferLayoutDescriptor* vertexBufferLayout =
                vertexDescriptor.layouts[MtlGraphicsPipeline::kVertexBufferIndex];
        vertexBufferLayout.stepFunction = MTLVertexStepFunctionPerVertex;
        vertexBufferLayout.stepRate = 1;
        vertexBufferLayout.stride = vertexAttributeOffset;
    }

    size_t instanceAttributeOffset = 0;
    for (const auto& attribute : instanceAttrs) {
        MTLVertexAttributeDescriptor* mtlAttribute = vertexDescriptor.attributes[attributeIndex];
        MTLVertexFormat format = attribute_type_to_mtlformat(attribute.cpuType());
        SkASSERT(MTLVertexFormatInvalid != format);
        mtlAttribute.format = format;
        mtlAttribute.offset = instanceAttributeOffset;
        mtlAttribute.bufferIndex = MtlGraphicsPipeline::kInstanceBufferIndex;

        instanceAttributeOffset += attribute.sizeAlign4();
        attributeIndex++;
    }

    if (instanceAttributeOffset) {
        MTLVertexBufferLayoutDescriptor* instanceBufferLayout =
                vertexDescriptor.layouts[MtlGraphicsPipeline::kInstanceBufferIndex];
        instanceBufferLayout.stepFunction = MTLVertexStepFunctionPerInstance;
        instanceBufferLayout.stepRate = 1;
        instanceBufferLayout.stride = instanceAttributeOffset;
    }
    return vertexDescriptor;
}

// TODO: share this w/ Ganesh Metal backend?
static MTLBlendFactor blend_coeff_to_mtl_blend(skgpu::BlendCoeff coeff) {
    switch (coeff) {
        case skgpu::BlendCoeff::kZero:
            return MTLBlendFactorZero;
        case skgpu::BlendCoeff::kOne:
            return MTLBlendFactorOne;
        case skgpu::BlendCoeff::kSC:
            return MTLBlendFactorSourceColor;
        case skgpu::BlendCoeff::kISC:
            return MTLBlendFactorOneMinusSourceColor;
        case skgpu::BlendCoeff::kDC:
            return MTLBlendFactorDestinationColor;
        case skgpu::BlendCoeff::kIDC:
            return MTLBlendFactorOneMinusDestinationColor;
        case skgpu::BlendCoeff::kSA:
            return MTLBlendFactorSourceAlpha;
        case skgpu::BlendCoeff::kISA:
            return MTLBlendFactorOneMinusSourceAlpha;
        case skgpu::BlendCoeff::kDA:
            return MTLBlendFactorDestinationAlpha;
        case skgpu::BlendCoeff::kIDA:
            return MTLBlendFactorOneMinusDestinationAlpha;
        case skgpu::BlendCoeff::kConstC:
            return MTLBlendFactorBlendColor;
        case skgpu::BlendCoeff::kIConstC:
            return MTLBlendFactorOneMinusBlendColor;
        case skgpu::BlendCoeff::kS2C:
            if (@available(macOS 10.12, iOS 11.0, tvOS 11.0, *)) {
                return MTLBlendFactorSource1Color;
            } else {
                return MTLBlendFactorZero;
            }
        case skgpu::BlendCoeff::kIS2C:
            if (@available(macOS 10.12, iOS 11.0, tvOS 11.0, *)) {
                return MTLBlendFactorOneMinusSource1Color;
            } else {
                return MTLBlendFactorZero;
            }
        case skgpu::BlendCoeff::kS2A:
            if (@available(macOS 10.12, iOS 11.0, tvOS 11.0, *)) {
                return MTLBlendFactorSource1Alpha;
            } else {
                return MTLBlendFactorZero;
            }
        case skgpu::BlendCoeff::kIS2A:
            if (@available(macOS 10.12, iOS 11.0, tvOS 11.0, *)) {
                return MTLBlendFactorOneMinusSource1Alpha;
            } else {
                return MTLBlendFactorZero;
            }
        case skgpu::BlendCoeff::kIllegal:
            return MTLBlendFactorZero;
    }

    SK_ABORT("Unknown blend coefficient");
}

// TODO: share this w/ Ganesh Metal backend?
static MTLBlendOperation blend_equation_to_mtl_blend_op(skgpu::BlendEquation equation) {
    static const MTLBlendOperation gTable[] = {
            MTLBlendOperationAdd,              // skgpu::BlendEquation::kAdd
            MTLBlendOperationSubtract,         // skgpu::BlendEquation::kSubtract
            MTLBlendOperationReverseSubtract,  // skgpu::BlendEquation::kReverseSubtract
    };
    static_assert(std::size(gTable) == (int)skgpu::BlendEquation::kFirstAdvanced);
    static_assert(0 == (int)skgpu::BlendEquation::kAdd);
    static_assert(1 == (int)skgpu::BlendEquation::kSubtract);
    static_assert(2 == (int)skgpu::BlendEquation::kReverseSubtract);

    SkASSERT((unsigned)equation < skgpu::kBlendEquationCnt);
    return gTable[(int)equation];
}

static MTLRenderPipelineColorAttachmentDescriptor* create_color_attachment(
        MTLPixelFormat format,
        const BlendInfo& blendInfo) {

    skgpu::BlendEquation equation = blendInfo.fEquation;
    skgpu::BlendCoeff srcCoeff = blendInfo.fSrcBlend;
    skgpu::BlendCoeff dstCoeff = blendInfo.fDstBlend;
    bool blendOn = !skgpu::BlendShouldDisable(equation, srcCoeff, dstCoeff);

    // TODO: I *think* this gets cleaned up by the pipelineDescriptor?
    auto mtlColorAttachment = [[MTLRenderPipelineColorAttachmentDescriptor alloc] init];

    mtlColorAttachment.pixelFormat = format;

    mtlColorAttachment.blendingEnabled = blendOn;

    if (blendOn) {
        mtlColorAttachment.sourceRGBBlendFactor = blend_coeff_to_mtl_blend(srcCoeff);
        mtlColorAttachment.destinationRGBBlendFactor = blend_coeff_to_mtl_blend(dstCoeff);
        mtlColorAttachment.rgbBlendOperation = blend_equation_to_mtl_blend_op(equation);
        mtlColorAttachment.sourceAlphaBlendFactor = blend_coeff_to_mtl_blend(srcCoeff);
        mtlColorAttachment.destinationAlphaBlendFactor = blend_coeff_to_mtl_blend(dstCoeff);
        mtlColorAttachment.alphaBlendOperation = blend_equation_to_mtl_blend_op(equation);
    }

    mtlColorAttachment.writeMask = blendInfo.fWritesColor ? MTLColorWriteMaskAll
                                                          : MTLColorWriteMaskNone;

    return mtlColorAttachment;
}

} // anonymous namespace

sk_sp<MtlGraphicsPipeline> MtlGraphicsPipeline::Make(const MtlSharedContext* sharedContext,
                                                     const std::string& label,
                                                     MSLFunction vertexMain,
                                                     SkSpan<const Attribute> vertexAttrs,
                                                     SkSpan<const Attribute> instanceAttrs,
                                                     MSLFunction fragmentMain,
                                                     sk_cfp<id<MTLDepthStencilState>> dss,
                                                     uint32_t stencilRefValue,
                                                     const BlendInfo& blendInfo,
                                                     const RenderPassDesc& renderPassDesc,
                                                     PipelineInfo* pipelineInfo) {
    id<MTLLibrary> vsLibrary = std::get<0>(vertexMain);
    id<MTLLibrary> fsLibrary = std::get<0>(fragmentMain);
    if (!vsLibrary || !fsLibrary) {
        return nullptr;
    }

    sk_cfp<MTLRenderPipelineDescriptor*> psoDescriptor([[MTLRenderPipelineDescriptor alloc] init]);

    NSString* labelName =  [NSString stringWithUTF8String: label.c_str()];
    NSString* vsFuncName = [NSString stringWithUTF8String: std::get<1>(vertexMain).c_str()];
    NSString* fsFuncName = [NSString stringWithUTF8String: std::get<1>(fragmentMain).c_str()];

    (*psoDescriptor).label = labelName;
    (*psoDescriptor).vertexFunction = [vsLibrary newFunctionWithName: vsFuncName];
    (*psoDescriptor).fragmentFunction = [fsLibrary newFunctionWithName: fsFuncName];

    // TODO: I *think* this gets cleaned up by the pipelineDescriptor?
    (*psoDescriptor).vertexDescriptor = create_vertex_descriptor(vertexAttrs, instanceAttrs);

    const MtlTextureSpec& mtlColorSpec =
            renderPassDesc.fColorAttachment.fTextureInfo.mtlTextureSpec();
    auto mtlColorAttachment = create_color_attachment((MTLPixelFormat)mtlColorSpec.fFormat,
                                                      blendInfo);
    (*psoDescriptor).colorAttachments[0] = mtlColorAttachment;

    (*psoDescriptor).rasterSampleCount =
            renderPassDesc.fColorAttachment.fTextureInfo.numSamples();

    const MtlTextureSpec& mtlDSSpec =
            renderPassDesc.fDepthStencilAttachment.fTextureInfo.mtlTextureSpec();
    MTLPixelFormat depthStencilFormat = (MTLPixelFormat)mtlDSSpec.fFormat;
    if (MtlFormatIsStencil(depthStencilFormat)) {
        (*psoDescriptor).stencilAttachmentPixelFormat = depthStencilFormat;
    } else {
        (*psoDescriptor).stencilAttachmentPixelFormat = MTLPixelFormatInvalid;
    }
    if (MtlFormatIsDepth(depthStencilFormat)) {
        (*psoDescriptor).depthAttachmentPixelFormat = depthStencilFormat;
    } else {
        (*psoDescriptor).depthAttachmentPixelFormat = MTLPixelFormatInvalid;
    }

    NSError* error;
    sk_cfp<id<MTLRenderPipelineState>> pso(
            [sharedContext->device() newRenderPipelineStateWithDescriptor:psoDescriptor.get()
                                                                    error:&error]);
    if (!pso) {
        SKGPU_LOG_E("Render pipeline creation failure:\n%s", error.debugDescription.UTF8String);
        return nullptr;
    }

    return sk_sp<MtlGraphicsPipeline>(new MtlGraphicsPipeline(sharedContext,
                                                              pipelineInfo,
                                                              std::move(pso),
                                                              std::move(dss),
                                                              stencilRefValue));
}

MtlGraphicsPipeline::MtlGraphicsPipeline(const skgpu::graphite::SharedContext* sharedContext,
                                         PipelineInfo* pipelineInfo,
                                         sk_cfp<id<MTLRenderPipelineState>> pso,
                                         sk_cfp<id<MTLDepthStencilState>> dss,
                                         uint32_t refValue)
        : GraphicsPipeline(sharedContext, pipelineInfo)
        , fPipelineState(std::move(pso))
        , fDepthStencilState(std::move(dss))
        , fStencilReferenceValue(refValue) {}

void MtlGraphicsPipeline::freeGpuData() {
    fPipelineState.reset();
}

} // namespace skgpu::graphite
