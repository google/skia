/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlPipelineStateBuilder.h"

#include "include/gpu/GrContext.h"
#include "src/core/SkReader32.h"
#include "src/gpu/GrAutoLocaleSetter.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrPersistentCacheUtils.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrShaderUtils.h"

#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlPipelineState.h"
#include "src/gpu/mtl/GrMtlUtil.h"

#import <simd/simd.h>

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GrMtlPipelineState* GrMtlPipelineStateBuilder::CreatePipelineState(
                                                                GrMtlGpu* gpu,
                                                                GrRenderTarget* renderTarget,
                                                                const GrProgramDesc& desc,
                                                                const GrProgramInfo& programInfo) {
    GrAutoLocaleSetter als("C");
    GrMtlPipelineStateBuilder builder(gpu, renderTarget, desc, programInfo);

    if (!builder.emitAndInstallProcs()) {
        return nullptr;
    }
    return builder.finalize(renderTarget, desc, programInfo);
}

GrMtlPipelineStateBuilder::GrMtlPipelineStateBuilder(GrMtlGpu* gpu,
                                                     GrRenderTarget* renderTarget,
                                                     const GrProgramDesc& desc,
                                                     const GrProgramInfo& programInfo)
        : INHERITED(renderTarget, desc, programInfo)
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

static constexpr SkFourByteTag kMSL_Tag = SkSetFourByteTag('M', 'S', 'L', ' ');
static constexpr SkFourByteTag kSKSL_Tag = SkSetFourByteTag('S', 'K', 'S', 'L');


void GrMtlPipelineStateBuilder::loadShadersFromCache(SkReader32* cached,
                                                     __strong id<MTLLibrary> outLibraries[]) {
    SkSL::String shaders[kGrShaderTypeCount];
    SkSL::Program::Inputs inputs[kGrShaderTypeCount];

    GrPersistentCacheUtils::UnpackCachedShaders(cached, shaders, inputs, kGrShaderTypeCount);

    outLibraries[kVertex_GrShaderType] = this->compileMtlShaderLibrary(
                                              shaders[kVertex_GrShaderType],
                                              inputs[kVertex_GrShaderType]);
    outLibraries[kFragment_GrShaderType] = this->compileMtlShaderLibrary(
                                                shaders[kFragment_GrShaderType],
                                                inputs[kFragment_GrShaderType]);

    // Geometry shaders are not supported
    SkASSERT(shaders[kGeometry_GrShaderType].empty());

    SkASSERT(outLibraries[kVertex_GrShaderType]);
    SkASSERT(outLibraries[kFragment_GrShaderType]);
}

void GrMtlPipelineStateBuilder::storeShadersInCache(const SkSL::String shaders[],
                                                    const SkSL::Program::Inputs inputs[],
                                                    bool isSkSL) {
    // Here we shear off the Mtl-specific portion of the Desc in order to create the
    // persistent key. This is because Mtl only caches the MSL code, not the fully compiled
    // program, and that only depends on the base GrProgramDesc data.
    sk_sp<SkData> key = SkData::MakeWithoutCopy(this->desc().asKey(),
                                                this->desc().initialKeyLength());
    sk_sp<SkData> data = GrPersistentCacheUtils::PackCachedShaders(isSkSL ? kSKSL_Tag : kMSL_Tag,
                                                                   shaders,
                                                                   inputs, kGrShaderTypeCount);
    fGpu->getContext()->priv().getPersistentCache()->store(*key, *data);
}

id<MTLLibrary> GrMtlPipelineStateBuilder::generateMtlShaderLibrary(
        const SkSL::String& shader,
        SkSL::Program::Kind kind,
        const SkSL::Program::Settings& settings,
        SkSL::String* msl,
        SkSL::Program::Inputs* inputs) {
    id<MTLLibrary> shaderLibrary = GrGenerateMtlShaderLibrary(fGpu, shader,
                                                              kind, settings, msl, inputs);
    if (shaderLibrary != nil && inputs->fRTHeight) {
        this->addRTHeightUniform(SKSL_RTHEIGHT_NAME);
    }
    return shaderLibrary;
}

id<MTLLibrary> GrMtlPipelineStateBuilder::compileMtlShaderLibrary(const SkSL::String& shader,
                                                                  SkSL::Program::Inputs inputs) {
    id<MTLLibrary> shaderLibrary = GrCompileMtlShaderLibrary(fGpu, shader);
    if (shaderLibrary != nil && inputs.fRTHeight) {
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
    static_assert(SK_ARRAY_COUNT(gTable) == kFirstAdvancedGrBlendEquation);
    static_assert(0 == kAdd_GrBlendEquation);
    static_assert(1 == kSubtract_GrBlendEquation);
    static_assert(2 == kReverseSubtract_GrBlendEquation);

    SkASSERT((unsigned)equation < kGrBlendEquationCnt);
    return gTable[equation];
}

static MTLRenderPipelineColorAttachmentDescriptor* create_color_attachment(
        MTLPixelFormat format, const GrPipeline& pipeline) {
    auto mtlColorAttachment = [[MTLRenderPipelineColorAttachmentDescriptor alloc] init];

    // pixel format
    mtlColorAttachment.pixelFormat = format;

    // blending
    const GrXferProcessor::BlendInfo& blendInfo = pipeline.getXferProcessor().getBlendInfo();

    GrBlendEquation equation = blendInfo.fEquation;
    GrBlendCoeff srcCoeff = blendInfo.fSrcBlend;
    GrBlendCoeff dstCoeff = blendInfo.fDstBlend;
    bool blendOff = GrBlendShouldDisable(equation, srcCoeff, dstCoeff);

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
                                                        const GrProgramDesc& desc,
                                                        const GrProgramInfo& programInfo) {
    auto pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    id<MTLLibrary> shaderLibraries[kGrShaderTypeCount];

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

    sk_sp<SkData> cached;
    SkReader32 reader;
    SkFourByteTag shaderType = 0;
    auto persistentCache = fGpu->getContext()->priv().getPersistentCache();
    if (persistentCache) {
        // Here we shear off the Mtl-specific portion of the Desc in order to create the
        // persistent key. This is because Mtl only caches the MSL code, not the fully compiled
        // program, and that only depends on the base GrProgramDesc data.
        sk_sp<SkData> key = SkData::MakeWithoutCopy(desc.asKey(), desc.initialKeyLength());
        cached = persistentCache->load(*key);
        if (cached) {
            reader.setMemory(cached->data(), cached->size());
            shaderType = GrPersistentCacheUtils::GetType(&reader);
        }
    }

    SkSL::String shaders[kGrShaderTypeCount];
    if (kMSL_Tag == shaderType) {
        this->loadShadersFromCache(&reader, shaderLibraries);
    } else {
        SkSL::Program::Inputs inputs[kGrShaderTypeCount];

        SkSL::String* sksl[kGrShaderTypeCount] = {
            &fVS.fCompilerString,
            nullptr,              // geometry shaders not supported
            &fFS.fCompilerString,
        };
        SkSL::String cached_sksl[kGrShaderTypeCount];
        if (kSKSL_Tag == shaderType) {
            GrPersistentCacheUtils::UnpackCachedShaders(&reader, cached_sksl, inputs,
                                                        kGrShaderTypeCount);
            for (int i = 0; i < kGrShaderTypeCount; ++i) {
                sksl[i] = &cached_sksl[i];
            }
        }

        shaderLibraries[kVertex_GrShaderType] = this->generateMtlShaderLibrary(
                                                     *sksl[kVertex_GrShaderType],
                                                     SkSL::Program::kVertex_Kind,
                                                     settings,
                                                     &shaders[kVertex_GrShaderType],
                                                     &inputs[kVertex_GrShaderType]);
        shaderLibraries[kFragment_GrShaderType] = this->generateMtlShaderLibrary(
                                                       *sksl[kFragment_GrShaderType],
                                                       SkSL::Program::kFragment_Kind,
                                                       settings,
                                                       &shaders[kFragment_GrShaderType],
                                                       &inputs[kFragment_GrShaderType]);

        // Geometry shaders are not supported
        SkASSERT(!this->primitiveProcessor().willUseGeoShader());

        if (!shaderLibraries[kVertex_GrShaderType] || !shaderLibraries[kFragment_GrShaderType]) {
            return nullptr;
        }

        if (persistentCache && !cached) {
            bool isSkSL = false;
            if (fGpu->getContext()->priv().options().fShaderCacheStrategy ==
                    GrContextOptions::ShaderCacheStrategy::kSkSL) {
                for (int i = 0; i < kGrShaderTypeCount; ++i) {
                    if (sksl[i]) {
                        shaders[i] = GrShaderUtils::PrettyPrint(*sksl[i]);
                    }
                }
                isSkSL = true;
            }
            this->storeShadersInCache(shaders, inputs, isSkSL);
        }
    }

    id<MTLFunction> vertexFunction =
            [shaderLibraries[kVertex_GrShaderType] newFunctionWithName: @"vertexMain"];
    id<MTLFunction> fragmentFunction =
            [shaderLibraries[kFragment_GrShaderType] newFunctionWithName: @"fragmentMain"];

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

    MTLPixelFormat pixelFormat = GrBackendFormatAsMTLPixelFormat(renderTarget->backendFormat());
    if (pixelFormat == MTLPixelFormatInvalid) {
        return nullptr;
    }

    pipelineDescriptor.colorAttachments[0] = create_color_attachment(pixelFormat,
                                                                     programInfo.pipeline());
    pipelineDescriptor.sampleCount = programInfo.numRasterSamples();
    bool hasStencilAttachment = SkToBool(renderTarget->renderTargetPriv().getStencilAttachment());
    GrMtlCaps* mtlCaps = (GrMtlCaps*)this->caps();
    pipelineDescriptor.stencilAttachmentPixelFormat =
        hasStencilAttachment ? mtlCaps->preferredStencilFormat().fInternalFormat
                             : MTLPixelFormatInvalid;

    SkASSERT(pipelineDescriptor.vertexFunction);
    SkASSERT(pipelineDescriptor.fragmentFunction);
    SkASSERT(pipelineDescriptor.vertexDescriptor);
    SkASSERT(pipelineDescriptor.colorAttachments[0]);

    NSError* error = nil;
#if defined(SK_BUILD_FOR_MAC)
    id<MTLRenderPipelineState> pipelineState = GrMtlNewRenderPipelineStateWithDescriptor(
                                                     fGpu->device(), pipelineDescriptor, &error);
#else
    id<MTLRenderPipelineState> pipelineState =
            [fGpu->device() newRenderPipelineStateWithDescriptor: pipelineDescriptor
                                                           error: &error];
#endif
    if (error) {
        SkDebugf("Error creating pipeline: %s\n",
                 [[error localizedDescription] cStringUsingEncoding: NSASCIIStringEncoding]);
        return nullptr;
    }
    if (!pipelineState) {
        return nullptr;
    }

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

