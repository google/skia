/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlPipelineStateBuilder.h"

#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"

#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlPipelineState.h"
#include "src/gpu/mtl/GrMtlUtil.h"

#include "src/gpu/GrRenderTargetPriv.h"

#import <simd/simd.h>

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GrMtlPipelineState* GrMtlPipelineStateBuilder::CreatePipelineState(GrMtlGpu* gpu,
                                                                   GrRenderTarget* renderTarget,
                                                                   const GrProgramInfo& programInfo,
                                                                   Desc* desc) {
    GrMtlPipelineStateBuilder builder(gpu, renderTarget, programInfo, desc);

    if (!builder.emitAndInstallProcs()) {
        return nullptr;
    }
    return builder.finalize(renderTarget, programInfo, desc);
}

GrMtlPipelineStateBuilder::GrMtlPipelineStateBuilder(GrMtlGpu* gpu,
                                                     GrRenderTarget* renderTarget,
                                                     const GrProgramInfo& programInfo,
                                                     GrProgramDesc* desc)
        : INHERITED(renderTarget, programInfo, desc)
        , fGpu(gpu)
        , fUniformHandler(this)
        , fVaryingHandler(this) {
}

const GrCaps* GrMtlPipelineStateBuilder::caps() const {
    return fGpu->caps();
}

void GrMtlPipelineStateBuilder::finalizeFragmentOutputColor(GrShaderVar& outputColor) {
    outputColor.addLayoutQualifier("location = 0, index = 0");
}

void GrMtlPipelineStateBuilder::finalizeFragmentSecondaryColor(GrShaderVar& outputColor) {
    outputColor.addLayoutQualifier("location = 0, index = 1");
}

id<MTLLibrary> GrMtlPipelineStateBuilder::createMtlShaderLibrary(
        const GrGLSLShaderBuilder& builder,
        SkSL::Program::Kind kind,
        const SkSL::Program::Settings& settings,
        GrProgramDesc* desc) {
    SkSL::Program::Inputs inputs;
    id<MTLLibrary> shaderLibrary = GrCompileMtlShaderLibrary(fGpu, builder.fCompilerString.c_str(),
                                                             kind, settings, &inputs);
    if (shaderLibrary == nil) {
        return nil;
    }
    if (inputs.fRTHeight) {
        this->addRTHeightUniform(SKSL_RTHEIGHT_NAME);
    }
    return shaderLibrary;
}

static inline MTLVertexFormat attribute_type_to_mtlformat(GrVertexAttribType type) {
    switch (type) {
        case kFloat_GrVertexAttribType:
            return MTLVertexFormatFloat;
        case kFloat2_GrVertexAttribType:
            return MTLVertexFormatFloat2;
        case kFloat3_GrVertexAttribType:
            return MTLVertexFormatFloat3;
        case kFloat4_GrVertexAttribType:
            return MTLVertexFormatFloat4;
        case kHalf_GrVertexAttribType:
            if (@available(macOS 10.13, iOS 11.0, *)) {
                return MTLVertexFormatHalf;
            } else {
                return MTLVertexFormatInvalid;
            }
        case kHalf2_GrVertexAttribType:
            return MTLVertexFormatHalf2;
        case kHalf3_GrVertexAttribType:
            return MTLVertexFormatHalf3;
        case kHalf4_GrVertexAttribType:
            return MTLVertexFormatHalf4;
        case kInt2_GrVertexAttribType:
            return MTLVertexFormatInt2;
        case kInt3_GrVertexAttribType:
            return MTLVertexFormatInt3;
        case kInt4_GrVertexAttribType:
            return MTLVertexFormatInt4;
        case kByte_GrVertexAttribType:
            if (@available(macOS 10.13, iOS 11.0, *)) {
                return MTLVertexFormatChar;
            } else {
                return MTLVertexFormatInvalid;
            }
        case kByte2_GrVertexAttribType:
            return MTLVertexFormatChar2;
        case kByte3_GrVertexAttribType:
            return MTLVertexFormatChar3;
        case kByte4_GrVertexAttribType:
            return MTLVertexFormatChar4;
        case kUByte_GrVertexAttribType:
            if (@available(macOS 10.13, iOS 11.0, *)) {
                return MTLVertexFormatUChar;
            } else {
                return MTLVertexFormatInvalid;
            }
        case kUByte2_GrVertexAttribType:
            return MTLVertexFormatUChar2;
        case kUByte3_GrVertexAttribType:
            return MTLVertexFormatUChar3;
        case kUByte4_GrVertexAttribType:
            return MTLVertexFormatUChar4;
        case kUByte_norm_GrVertexAttribType:
            if (@available(macOS 10.13, iOS 11.0, *)) {
                return MTLVertexFormatUCharNormalized;
            } else {
                return MTLVertexFormatInvalid;
            }
        case kUByte4_norm_GrVertexAttribType:
            return MTLVertexFormatUChar4Normalized;
        case kShort2_GrVertexAttribType:
            return MTLVertexFormatShort2;
        case kShort4_GrVertexAttribType:
            return MTLVertexFormatShort4;
        case kUShort2_GrVertexAttribType:
            return MTLVertexFormatUShort2;
        case kUShort2_norm_GrVertexAttribType:
            return MTLVertexFormatUShort2Normalized;
        case kInt_GrVertexAttribType:
            return MTLVertexFormatInt;
        case kUint_GrVertexAttribType:
            return MTLVertexFormatUInt;
        case kUShort_norm_GrVertexAttribType:
            if (@available(macOS 10.13, iOS 11.0, *)) {
                return MTLVertexFormatUShortNormalized;
            } else {
                return MTLVertexFormatInvalid;
            }
        case kUShort4_norm_GrVertexAttribType:
            return MTLVertexFormatUShort4Normalized;
    }
    SK_ABORT("Unknown vertex attribute type");
}

static MTLVertexDescriptor* create_vertex_descriptor(const GrPrimitiveProcessor& primProc) {
    uint32_t vertexBinding = 0, instanceBinding = 0;

    int nextBinding = GrMtlUniformHandler::kLastUniformBinding + 1;
    if (primProc.hasVertexAttributes()) {
        vertexBinding = nextBinding++;
    }

    if (primProc.hasInstanceAttributes()) {
        instanceBinding = nextBinding;
    }

    auto vertexDescriptor = [[MTLVertexDescriptor alloc] init];
    int attributeIndex = 0;

    int vertexAttributeCount = primProc.numVertexAttributes();
    size_t vertexAttributeOffset = 0;
    for (const auto& attribute : primProc.vertexAttributes()) {
        MTLVertexAttributeDescriptor* mtlAttribute = vertexDescriptor.attributes[attributeIndex];
        mtlAttribute.format = attribute_type_to_mtlformat(attribute.cpuType());
        SkASSERT(MTLVertexFormatInvalid != mtlAttribute.format);
        mtlAttribute.offset = vertexAttributeOffset;
        mtlAttribute.bufferIndex = vertexBinding;

        vertexAttributeOffset += attribute.sizeAlign4();
        attributeIndex++;
    }
    SkASSERT(vertexAttributeOffset == primProc.vertexStride());

    if (vertexAttributeCount) {
        MTLVertexBufferLayoutDescriptor* vertexBufferLayout =
                vertexDescriptor.layouts[vertexBinding];
        vertexBufferLayout.stepFunction = MTLVertexStepFunctionPerVertex;
        vertexBufferLayout.stepRate = 1;
        vertexBufferLayout.stride = vertexAttributeOffset;
    }

    int instanceAttributeCount = primProc.numInstanceAttributes();
    size_t instanceAttributeOffset = 0;
    for (const auto& attribute : primProc.instanceAttributes()) {
        MTLVertexAttributeDescriptor* mtlAttribute = vertexDescriptor.attributes[attributeIndex];
        mtlAttribute.format = attribute_type_to_mtlformat(attribute.cpuType());
        mtlAttribute.offset = instanceAttributeOffset;
        mtlAttribute.bufferIndex = instanceBinding;

        instanceAttributeOffset += attribute.sizeAlign4();
        attributeIndex++;
    }
    SkASSERT(instanceAttributeOffset == primProc.instanceStride());

    if (instanceAttributeCount) {
        MTLVertexBufferLayoutDescriptor* instanceBufferLayout =
                vertexDescriptor.layouts[instanceBinding];
        instanceBufferLayout.stepFunction = MTLVertexStepFunctionPerInstance;
        instanceBufferLayout.stepRate = 1;
        instanceBufferLayout.stride = instanceAttributeOffset;
    }
    return vertexDescriptor;
}

static MTLBlendFactor blend_coeff_to_mtl_blend(GrBlendCoeff coeff) {
    switch (coeff) {
        case kZero_GrBlendCoeff:
            return MTLBlendFactorZero;
        case kOne_GrBlendCoeff:
            return MTLBlendFactorOne;
        case kSC_GrBlendCoeff:
            return MTLBlendFactorSourceColor;
        case kISC_GrBlendCoeff:
            return MTLBlendFactorOneMinusSourceColor;
        case kDC_GrBlendCoeff:
            return MTLBlendFactorDestinationColor;
        case kIDC_GrBlendCoeff:
            return MTLBlendFactorOneMinusDestinationColor;
        case kSA_GrBlendCoeff:
            return MTLBlendFactorSourceAlpha;
        case kISA_GrBlendCoeff:
            return MTLBlendFactorOneMinusSourceAlpha;
        case kDA_GrBlendCoeff:
            return MTLBlendFactorDestinationAlpha;
        case kIDA_GrBlendCoeff:
            return MTLBlendFactorOneMinusDestinationAlpha;
        case kConstC_GrBlendCoeff:
            return MTLBlendFactorBlendColor;
        case kIConstC_GrBlendCoeff:
            return MTLBlendFactorOneMinusBlendColor;
        case kConstA_GrBlendCoeff:
            return MTLBlendFactorBlendAlpha;
        case kIConstA_GrBlendCoeff:
            return MTLBlendFactorOneMinusBlendAlpha;
        case kS2C_GrBlendCoeff:
            if (@available(macOS 10.12, iOS 11.0, *)) {
                return MTLBlendFactorSource1Color;
            } else {
                return MTLBlendFactorZero;
            }
        case kIS2C_GrBlendCoeff:
            if (@available(macOS 10.12, iOS 11.0, *)) {
                return MTLBlendFactorOneMinusSource1Color;
            } else {
                return MTLBlendFactorZero;
            }
        case kS2A_GrBlendCoeff:
            if (@available(macOS 10.12, iOS 11.0, *)) {
                return MTLBlendFactorSource1Alpha;
            } else {
                return MTLBlendFactorZero;
            }
        case kIS2A_GrBlendCoeff:
            if (@available(macOS 10.12, iOS 11.0, *)) {
                return MTLBlendFactorOneMinusSource1Alpha;
            } else {
                return MTLBlendFactorZero;
            }
        case kIllegal_GrBlendCoeff:
            return MTLBlendFactorZero;
    }

    SK_ABORT("Unknown blend coefficient");
}

static MTLBlendOperation blend_equation_to_mtl_blend_op(GrBlendEquation equation) {
    static const MTLBlendOperation gTable[] = {
        MTLBlendOperationAdd,              // kAdd_GrBlendEquation
        MTLBlendOperationSubtract,         // kSubtract_GrBlendEquation
        MTLBlendOperationReverseSubtract,  // kReverseSubtract_GrBlendEquation
    };
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(gTable) == kFirstAdvancedGrBlendEquation);
    GR_STATIC_ASSERT(0 == kAdd_GrBlendEquation);
    GR_STATIC_ASSERT(1 == kSubtract_GrBlendEquation);
    GR_STATIC_ASSERT(2 == kReverseSubtract_GrBlendEquation);

    SkASSERT((unsigned)equation < kGrBlendEquationCnt);
    return gTable[equation];
}

static MTLRenderPipelineColorAttachmentDescriptor* create_color_attachment(
        GrPixelConfig config, const GrPipeline& pipeline) {
    auto mtlColorAttachment = [[MTLRenderPipelineColorAttachmentDescriptor alloc] init];

    // pixel format
    MTLPixelFormat format;
    SkAssertResult(GrPixelConfigToMTLFormat(config, &format));
    mtlColorAttachment.pixelFormat = format;

    // blending
    const GrXferProcessor::BlendInfo& blendInfo = pipeline.getXferProcessor().getBlendInfo();

    GrBlendEquation equation = blendInfo.fEquation;
    GrBlendCoeff srcCoeff = blendInfo.fSrcBlend;
    GrBlendCoeff dstCoeff = blendInfo.fDstBlend;
    bool blendOff = (kAdd_GrBlendEquation == equation || kSubtract_GrBlendEquation == equation) &&
                    kOne_GrBlendCoeff == srcCoeff && kZero_GrBlendCoeff == dstCoeff;

    mtlColorAttachment.blendingEnabled = !blendOff;
    if (!blendOff) {
        mtlColorAttachment.sourceRGBBlendFactor = blend_coeff_to_mtl_blend(srcCoeff);
        mtlColorAttachment.destinationRGBBlendFactor = blend_coeff_to_mtl_blend(dstCoeff);
        mtlColorAttachment.rgbBlendOperation = blend_equation_to_mtl_blend_op(equation);
        mtlColorAttachment.sourceAlphaBlendFactor = blend_coeff_to_mtl_blend(srcCoeff);
        mtlColorAttachment.destinationAlphaBlendFactor = blend_coeff_to_mtl_blend(dstCoeff);
        mtlColorAttachment.alphaBlendOperation = blend_equation_to_mtl_blend_op(equation);
    }

    if (!blendInfo.fWriteColor) {
        mtlColorAttachment.writeMask = MTLColorWriteMaskNone;
    } else {
        mtlColorAttachment.writeMask = MTLColorWriteMaskAll;
    }
    return mtlColorAttachment;
}

uint32_t buffer_size(uint32_t offset, uint32_t maxAlignment) {
    // Metal expects the buffer to be padded at the end according to the alignment
    // of the largest element in the buffer.
    uint32_t offsetDiff = offset & maxAlignment;
    if (offsetDiff != 0) {
        offsetDiff = maxAlignment - offsetDiff + 1;
    }
    return offset + offsetDiff;
}

GrMtlPipelineState* GrMtlPipelineStateBuilder::finalize(GrRenderTarget* renderTarget,
                                                        const GrProgramInfo& programInfo,
                                                        Desc* desc) {
    auto pipelineDescriptor = [MTLRenderPipelineDescriptor new];

    fVS.extensions().appendf("#extension GL_ARB_separate_shader_objects : enable\n");
    fFS.extensions().appendf("#extension GL_ARB_separate_shader_objects : enable\n");
    fVS.extensions().appendf("#extension GL_ARB_shading_language_420pack : enable\n");
    fFS.extensions().appendf("#extension GL_ARB_shading_language_420pack : enable\n");

    this->finalizeShaders();

    SkSL::Program::Settings settings;
    settings.fCaps = this->caps()->shaderCaps();
    settings.fFlipY = this->origin() != kTopLeft_GrSurfaceOrigin;
    settings.fSharpenTextures = fGpu->getContext()->priv().options().fSharpenMipmappedTextures;
    SkASSERT(!this->fragColorIsInOut());

    // TODO: Store shaders in cache
    id<MTLLibrary> vertexLibrary = nil;
    id<MTLLibrary> fragmentLibrary = nil;
    vertexLibrary = this->createMtlShaderLibrary(fVS,
                                                 SkSL::Program::kVertex_Kind,
                                                 settings,
                                                 desc);
    fragmentLibrary = this->createMtlShaderLibrary(fFS,
                                                   SkSL::Program::kFragment_Kind,
                                                   settings,
                                                   desc);
    SkASSERT(!this->primitiveProcessor().willUseGeoShader());

    if (!vertexLibrary || !fragmentLibrary) {
        return nullptr;
    }

    id<MTLFunction> vertexFunction = [vertexLibrary newFunctionWithName: @"vertexMain"];
    id<MTLFunction> fragmentFunction = [fragmentLibrary newFunctionWithName: @"fragmentMain"];

    if (vertexFunction == nil) {
        SkDebugf("Couldn't find vertexMain() in library\n");
        return nullptr;
    }
    if (fragmentFunction == nil) {
        SkDebugf("Couldn't find fragmentMain() in library\n");
        return nullptr;
    }

    pipelineDescriptor.vertexFunction = vertexFunction;
    pipelineDescriptor.fragmentFunction = fragmentFunction;
    pipelineDescriptor.vertexDescriptor = create_vertex_descriptor(programInfo.primProc());
    pipelineDescriptor.colorAttachments[0] = create_color_attachment(renderTarget->config(),
                                                                     programInfo.pipeline());
    pipelineDescriptor.sampleCount = renderTarget->numSamples();
    bool hasStencilAttachment = SkToBool(renderTarget->renderTargetPriv().getStencilAttachment());
    GrMtlCaps* mtlCaps = (GrMtlCaps*)this->caps();
    pipelineDescriptor.stencilAttachmentPixelFormat =
        hasStencilAttachment ? mtlCaps->preferredStencilFormat().fInternalFormat
                             : MTLPixelFormatInvalid;

    SkASSERT(pipelineDescriptor.vertexFunction);
    SkASSERT(pipelineDescriptor.fragmentFunction);
    SkASSERT(pipelineDescriptor.vertexDescriptor);
    SkASSERT(pipelineDescriptor.colorAttachments[0]);

#if defined(SK_BUILD_FOR_MAC) && defined(GR_USE_COMPLETION_HANDLER)
    bool timedout;
    id<MTLRenderPipelineState> pipelineState = GrMtlNewRenderPipelineStateWithDescriptor(
                                                     fGpu->device(), pipelineDescriptor, &timedout);
    if (timedout) {
        // try a second time
        pipelineState = GrMtlNewRenderPipelineStateWithDescriptor(
                                fGpu->device(), pipelineDescriptor, &timedout);
    }
    if (!pipelineState) {
        return nullptr;
    }
#else
    NSError* error = nil;
    id<MTLRenderPipelineState> pipelineState =
            [fGpu->device() newRenderPipelineStateWithDescriptor: pipelineDescriptor
                                                           error: &error];
    if (error) {
        SkDebugf("Error creating pipeline: %s\n",
                 [[error localizedDescription] cStringUsingEncoding: NSASCIIStringEncoding]);
        return nullptr;
    }
#endif

    uint32_t bufferSize = buffer_size(fUniformHandler.fCurrentUBOOffset,
                                      fUniformHandler.fCurrentUBOMaxAlignment);
    return new GrMtlPipelineState(fGpu,
                                  pipelineState,
                                  pipelineDescriptor.colorAttachments[0].pixelFormat,
                                  fUniformHandles,
                                  fUniformHandler.fUniforms,
                                  bufferSize,
                                  (uint32_t)fUniformHandler.numSamplers(),
                                  std::move(fGeometryProcessor),
                                  std::move(fXferProcessor),
                                  std::move(fFragmentProcessors),
                                  fFragmentProcessorCnt);
}

//////////////////////////////////////////////////////////////////////////////

bool GrMtlPipelineStateBuilder::Desc::Build(Desc* desc,
                                            GrRenderTarget* renderTarget,
                                            const GrProgramInfo& programInfo,
                                            GrPrimitiveType primitiveType,
                                            GrMtlGpu* gpu) {
    if (!GrProgramDesc::Build(desc, renderTarget, programInfo, primitiveType, gpu)) {
        return false;
    }

    GrProcessorKeyBuilder b(&desc->key());

    int keyLength = desc->key().count();
    SkASSERT(0 == (keyLength % 4));
    desc->fShaderKeyLength = SkToU32(keyLength);

    b.add32(renderTarget->config());
    b.add32(renderTarget->numSamples());
    bool hasStencilAttachment = SkToBool(renderTarget->renderTargetPriv().getStencilAttachment());
    b.add32(hasStencilAttachment ? gpu->mtlCaps().preferredStencilFormat().fInternalFormat
                                 : MTLPixelFormatInvalid);
    b.add32((uint32_t)programInfo.pipeline().isStencilEnabled());
    // Stencil samples don't seem to be tracked in the MTLRenderPipeline

    b.add32(programInfo.pipeline().getBlendInfoKey());

    b.add32((uint32_t)primitiveType);

    return true;
}
