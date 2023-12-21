/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/mtl/GrMtlPipelineStateBuilder.h"

#include "include/gpu/GrDirectContext.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkTraceEvent.h"
#include "src/core/SkWriteBuffer.h"
#include "src/gpu/PipelineUtils.h"
#include "src/gpu/ganesh/GrAutoLocaleSetter.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrPersistentCacheUtils.h"
#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/utils/SkShaderUtils.h"

#include "src/gpu/ganesh/mtl/GrMtlGpu.h"
#include "src/gpu/ganesh/mtl/GrMtlPipelineState.h"
#include "src/gpu/ganesh/mtl/GrMtlUtil.h"
#include "src/gpu/mtl/MtlUtilsPriv.h"

#import <simd/simd.h>

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GR_NORETAIN_BEGIN

GrMtlPipelineState* GrMtlPipelineStateBuilder::CreatePipelineState(
        GrMtlGpu* gpu, const GrProgramDesc& desc, const GrProgramInfo& programInfo,
        const GrMtlPrecompiledLibraries* precompiledLibs) {
    GrAutoLocaleSetter als("C");
    GrMtlPipelineStateBuilder builder(gpu, desc, programInfo);

    if (!builder.emitAndInstallProcs()) {
        return nullptr;
    }
    return builder.finalize(desc, programInfo, precompiledLibs);
}

GrMtlPipelineStateBuilder::GrMtlPipelineStateBuilder(GrMtlGpu* gpu,
                                                     const GrProgramDesc& desc,
                                                     const GrProgramInfo& programInfo)
        : INHERITED(desc, programInfo)
        , fGpu(gpu)
        , fUniformHandler(this)
        , fVaryingHandler(this) {
}

const GrCaps* GrMtlPipelineStateBuilder::caps() const {
    return fGpu->caps();
}

void GrMtlPipelineStateBuilder::finalizeFragmentSecondaryColor(GrShaderVar& outputColor) {
    outputColor.addLayoutQualifier("location = 0, index = 1");
}

static constexpr SkFourByteTag kMSL_Tag = SkSetFourByteTag('M', 'S', 'L', ' ');
static constexpr SkFourByteTag kSKSL_Tag = SkSetFourByteTag('S', 'K', 'S', 'L');

void GrMtlPipelineStateBuilder::storeShadersInCache(const std::string shaders[],
                                                    const SkSL::Program::Interface interfaces[],
                                                    SkSL::ProgramSettings* settings,
                                                    sk_sp<SkData> pipelineData,
                                                    bool isSkSL) {
    sk_sp<SkData> key = SkData::MakeWithoutCopy(this->desc().asKey(),
                                                this->desc().keyLength());
    SkString description = GrProgramDesc::Describe(fProgramInfo, *this->caps());
    // cache metadata to allow for a complete precompile in either case
    GrPersistentCacheUtils::ShaderMetadata meta;
    meta.fSettings = settings;
    meta.fPlatformData = std::move(pipelineData);
    SkFourByteTag tag = isSkSL ? kSKSL_Tag : kMSL_Tag;
    sk_sp<SkData> data = GrPersistentCacheUtils::PackCachedShaders(tag, shaders, interfaces,
                                                                   kGrShaderTypeCount, &meta);
    fGpu->getContext()->priv().getPersistentCache()->store(*key, *data, description);
}

id<MTLLibrary> GrMtlPipelineStateBuilder::compileMtlShaderLibrary(
        const std::string& shader,
        SkSL::Program::Interface interface,
        GrContextOptions::ShaderErrorHandler* errorHandler) {
    id<MTLLibrary> shaderLibrary = GrCompileMtlShaderLibrary(fGpu, shader, errorHandler);
    if (shaderLibrary != nil &&
        interface.fRTFlipUniform != SkSL::Program::Interface::kRTFlip_None) {
        this->addRTFlipUniform(SKSL_RTFLIP_NAME);
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
            if (@available(macOS 10.13, iOS 11.0, tvOS 11.0, *)) {
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
            if (@available(macOS 10.13, iOS 11.0, tvOS 11.0, *)) {
                return MTLVertexFormatChar;
            } else {
                return MTLVertexFormatInvalid;
            }
        case kByte2_GrVertexAttribType:
            return MTLVertexFormatChar2;
        case kByte4_GrVertexAttribType:
            return MTLVertexFormatChar4;
        case kUByte_GrVertexAttribType:
            if (@available(macOS 10.13, iOS 11.0, tvOS 11.0, *)) {
                return MTLVertexFormatUChar;
            } else {
                return MTLVertexFormatInvalid;
            }
        case kUByte2_GrVertexAttribType:
            return MTLVertexFormatUChar2;
        case kUByte4_GrVertexAttribType:
            return MTLVertexFormatUChar4;
        case kUByte_norm_GrVertexAttribType:
            if (@available(macOS 10.13, iOS 11.0, tvOS 11.0, *)) {
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
        case kUInt_GrVertexAttribType:
            return MTLVertexFormatUInt;
        case kUShort_norm_GrVertexAttribType:
            if (@available(macOS 10.13, iOS 11.0, tvOS 11.0, *)) {
                return MTLVertexFormatUShortNormalized;
            } else {
                return MTLVertexFormatInvalid;
            }
        case kUShort4_norm_GrVertexAttribType:
            return MTLVertexFormatUShort4Normalized;
    }
    SK_ABORT("Unknown vertex attribute type");
}

static MTLVertexDescriptor* create_vertex_descriptor(const GrGeometryProcessor& geomProc,
                                                     SkBinaryWriteBuffer* writer) {
    uint32_t vertexBinding = 0, instanceBinding = 0;

    int nextBinding = GrMtlUniformHandler::kLastUniformBinding + 1;
    if (geomProc.hasVertexAttributes()) {
        vertexBinding = nextBinding++;
    }

    if (geomProc.hasInstanceAttributes()) {
        instanceBinding = nextBinding;
    }
    if (writer) {
        writer->writeUInt(vertexBinding);
        writer->writeUInt(instanceBinding);
    }

    auto vertexDescriptor = [[MTLVertexDescriptor alloc] init];
    int attributeIndex = 0;

    int vertexAttributeCount = geomProc.numVertexAttributes();
    if (writer) {
        writer->writeInt(vertexAttributeCount);
    }
    for (auto attribute : geomProc.vertexAttributes()) {
        MTLVertexAttributeDescriptor* mtlAttribute = vertexDescriptor.attributes[attributeIndex];
        MTLVertexFormat format = attribute_type_to_mtlformat(attribute.cpuType());
        SkASSERT(MTLVertexFormatInvalid != format);
        mtlAttribute.format = format;
        mtlAttribute.offset = *attribute.offset();
        mtlAttribute.bufferIndex = vertexBinding;
        if (writer) {
            writer->writeInt(format);
            writer->writeUInt(*attribute.offset());
            writer->writeUInt(vertexBinding);
        }

        attributeIndex++;
    }

    if (vertexAttributeCount) {
        MTLVertexBufferLayoutDescriptor* vertexBufferLayout =
                vertexDescriptor.layouts[vertexBinding];
        vertexBufferLayout.stepFunction = MTLVertexStepFunctionPerVertex;
        vertexBufferLayout.stepRate = 1;
        vertexBufferLayout.stride = geomProc.vertexStride();
        if (writer) {
            writer->writeUInt(geomProc.vertexStride());
        }
    }

    int instanceAttributeCount = geomProc.numInstanceAttributes();
    if (writer) {
        writer->writeInt(instanceAttributeCount);
    }
    for (auto attribute : geomProc.instanceAttributes()) {
        MTLVertexAttributeDescriptor* mtlAttribute = vertexDescriptor.attributes[attributeIndex];
        MTLVertexFormat format = attribute_type_to_mtlformat(attribute.cpuType());
        SkASSERT(MTLVertexFormatInvalid != format);
        mtlAttribute.format = format;
        mtlAttribute.offset = *attribute.offset();
        mtlAttribute.bufferIndex = instanceBinding;
        if (writer) {
            writer->writeInt(format);
            writer->writeUInt(*attribute.offset());
            writer->writeUInt(instanceBinding);
        }

        attributeIndex++;
    }

    if (instanceAttributeCount) {
        MTLVertexBufferLayoutDescriptor* instanceBufferLayout =
                vertexDescriptor.layouts[instanceBinding];
        instanceBufferLayout.stepFunction = MTLVertexStepFunctionPerInstance;
        instanceBufferLayout.stepRate = 1;
        instanceBufferLayout.stride = geomProc.instanceStride();
        if (writer) {
            writer->writeUInt(geomProc.instanceStride());
        }
    }
    return vertexDescriptor;
}

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
        MTLPixelFormat format, const GrPipeline& pipeline, SkBinaryWriteBuffer* writer) {
    auto mtlColorAttachment = [[MTLRenderPipelineColorAttachmentDescriptor alloc] init];

    // pixel format
    mtlColorAttachment.pixelFormat = format;
    if (writer) {
        writer->writeInt(format);
    }

    // blending
    const skgpu::BlendInfo& blendInfo = pipeline.getXferProcessor().getBlendInfo();

    skgpu::BlendEquation equation = blendInfo.fEquation;
    skgpu::BlendCoeff srcCoeff = blendInfo.fSrcBlend;
    skgpu::BlendCoeff dstCoeff = blendInfo.fDstBlend;
    bool blendOn = !skgpu::BlendShouldDisable(equation, srcCoeff, dstCoeff);

    mtlColorAttachment.blendingEnabled = blendOn;
    if (writer) {
        writer->writeBool(blendOn);
    }
    if (blendOn) {
        mtlColorAttachment.sourceRGBBlendFactor = blend_coeff_to_mtl_blend(srcCoeff);
        mtlColorAttachment.destinationRGBBlendFactor = blend_coeff_to_mtl_blend(dstCoeff);
        mtlColorAttachment.rgbBlendOperation = blend_equation_to_mtl_blend_op(equation);
        mtlColorAttachment.sourceAlphaBlendFactor = blend_coeff_to_mtl_blend(srcCoeff);
        mtlColorAttachment.destinationAlphaBlendFactor = blend_coeff_to_mtl_blend(dstCoeff);
        mtlColorAttachment.alphaBlendOperation = blend_equation_to_mtl_blend_op(equation);
        if (writer) {
            writer->writeInt(mtlColorAttachment.sourceRGBBlendFactor);
            writer->writeInt(mtlColorAttachment.destinationRGBBlendFactor);
            writer->writeInt(mtlColorAttachment.rgbBlendOperation);
            writer->writeInt(mtlColorAttachment.sourceAlphaBlendFactor);
            writer->writeInt(mtlColorAttachment.destinationAlphaBlendFactor);
            writer->writeInt(mtlColorAttachment.alphaBlendOperation);
        }
    }

    if (blendInfo.fWritesColor) {
        mtlColorAttachment.writeMask = MTLColorWriteMaskAll;
    } else {
        mtlColorAttachment.writeMask = MTLColorWriteMaskNone;
    }
    if (writer) {
        writer->writeBool(blendInfo.fWritesColor);
    }
    return mtlColorAttachment;
}

static uint32_t buffer_size(uint32_t offset, uint32_t maxAlignment) {
    // Metal expects the buffer to be padded at the end according to the alignment
    // of the largest element in the buffer.
    uint32_t offsetDiff = offset & maxAlignment;
    if (offsetDiff != 0) {
        offsetDiff = maxAlignment - offsetDiff + 1;
    }
    return offset + offsetDiff;
}

static MTLRenderPipelineDescriptor* read_pipeline_data(SkReadBuffer* reader) {
    auto pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];

#ifdef SK_ENABLE_MTL_DEBUG_INFO
    // set label
    {
        SkString description;
        reader->readString(&description);
        pipelineDescriptor.label = @(description.c_str());
    }
#endif

    // set up vertex descriptor
    {
        auto vertexDescriptor = [[MTLVertexDescriptor alloc] init];
        uint32_t vertexBinding = reader->readUInt();
        uint32_t instanceBinding = reader->readUInt();

        int attributeIndex = 0;

        // vertex attributes
        int vertexAttributeCount = reader->readInt();
        for (int i = 0; i < vertexAttributeCount; ++i) {
            MTLVertexAttributeDescriptor* mtlAttribute = vertexDescriptor.attributes[attributeIndex];
            mtlAttribute.format = (MTLVertexFormat) reader->readInt();
            mtlAttribute.offset = reader->readUInt();
            mtlAttribute.bufferIndex = reader->readUInt();
            ++attributeIndex;
        }
        if (vertexAttributeCount) {
            MTLVertexBufferLayoutDescriptor* vertexBufferLayout =
                    vertexDescriptor.layouts[vertexBinding];
            vertexBufferLayout.stepFunction = MTLVertexStepFunctionPerVertex;
            vertexBufferLayout.stepRate = 1;
            vertexBufferLayout.stride = reader->readUInt();
        }

        // instance attributes
        int instanceAttributeCount = reader->readInt();
        for (int i = 0; i < instanceAttributeCount; ++i) {
            MTLVertexAttributeDescriptor* mtlAttribute = vertexDescriptor.attributes[attributeIndex];
            mtlAttribute.format = (MTLVertexFormat) reader->readInt();
            mtlAttribute.offset = reader->readUInt();
            mtlAttribute.bufferIndex = reader->readUInt();
            ++attributeIndex;
        }
        if (instanceAttributeCount) {
            MTLVertexBufferLayoutDescriptor* instanceBufferLayout =
                    vertexDescriptor.layouts[instanceBinding];
            instanceBufferLayout.stepFunction = MTLVertexStepFunctionPerInstance;
            instanceBufferLayout.stepRate = 1;
            instanceBufferLayout.stride = reader->readUInt();
        }
        pipelineDescriptor.vertexDescriptor = vertexDescriptor;
    }

    // set up color attachments
    {
        auto mtlColorAttachment = [[MTLRenderPipelineColorAttachmentDescriptor alloc] init];

        mtlColorAttachment.pixelFormat = (MTLPixelFormat) reader->readInt();
        mtlColorAttachment.blendingEnabled = reader->readBool();
        if (mtlColorAttachment.blendingEnabled) {
            mtlColorAttachment.sourceRGBBlendFactor = (MTLBlendFactor) reader->readInt();
            mtlColorAttachment.destinationRGBBlendFactor = (MTLBlendFactor) reader->readInt();
            mtlColorAttachment.rgbBlendOperation = (MTLBlendOperation) reader->readInt();
            mtlColorAttachment.sourceAlphaBlendFactor = (MTLBlendFactor) reader->readInt();
            mtlColorAttachment.destinationAlphaBlendFactor = (MTLBlendFactor) reader->readInt();
            mtlColorAttachment.alphaBlendOperation = (MTLBlendOperation) reader->readInt();
        }
        if (reader->readBool()) {
            mtlColorAttachment.writeMask = MTLColorWriteMaskAll;
        } else {
            mtlColorAttachment.writeMask = MTLColorWriteMaskNone;
        }

        pipelineDescriptor.colorAttachments[0] = mtlColorAttachment;
    }

    pipelineDescriptor.stencilAttachmentPixelFormat = (MTLPixelFormat) reader->readInt();

    return pipelineDescriptor;
}

GrMtlPipelineState* GrMtlPipelineStateBuilder::finalize(
        const GrProgramDesc& desc, const GrProgramInfo& programInfo,
        const GrMtlPrecompiledLibraries* precompiledLibs) {
    TRACE_EVENT0("skia.shaders", TRACE_FUNC);

    // Set up for cache if needed
    std::unique_ptr<SkBinaryWriteBuffer> writer;

    sk_sp<SkData> cached;
    auto persistentCache = fGpu->getContext()->priv().getPersistentCache();
    if (persistentCache && !precompiledLibs) {
        sk_sp<SkData> key = SkData::MakeWithoutCopy(desc.asKey(), desc.keyLength());
        cached = persistentCache->load(*key);
    }
    if (persistentCache && !cached) {
        writer = std::make_unique<SkBinaryWriteBuffer>(SkSerialProcs{});
    }

    // Ordering in how we set these matters. If it changes adjust read_pipeline_data, above.
    auto pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    SkString description = GrProgramDesc::Describe(programInfo, *fGpu->caps());
    int split = description.find("\n");
    description.resize(split);
    pipelineDescriptor.label = @(description.c_str());
    if (writer) {
        writer->writeString(description.c_str());
    }
#endif

    pipelineDescriptor.vertexDescriptor = create_vertex_descriptor(programInfo.geomProc(),
                                                                   writer.get());

    MTLPixelFormat pixelFormat = GrBackendFormatAsMTLPixelFormat(programInfo.backendFormat());
    if (pixelFormat == MTLPixelFormatInvalid) {
        return nullptr;
    }

    pipelineDescriptor.colorAttachments[0] = create_color_attachment(pixelFormat,
                                                                     programInfo.pipeline(),
                                                                     writer.get());
    pipelineDescriptor.rasterSampleCount = programInfo.numSamples();

    const GrMtlCaps* mtlCaps = (const GrMtlCaps*)this->caps();
    pipelineDescriptor.stencilAttachmentPixelFormat = mtlCaps->getStencilPixelFormat(desc);
    if (writer) {
        writer->writeInt(pipelineDescriptor.stencilAttachmentPixelFormat);
    }
    SkASSERT(pipelineDescriptor.vertexDescriptor);
    SkASSERT(pipelineDescriptor.colorAttachments[0]);

    if (precompiledLibs) {
        SkASSERT(precompiledLibs->fVertexLibrary);
        SkASSERT(precompiledLibs->fFragmentLibrary);
        pipelineDescriptor.vertexFunction =
                [precompiledLibs->fVertexLibrary newFunctionWithName: @"vertexMain"];
        pipelineDescriptor.fragmentFunction =
                [precompiledLibs->fFragmentLibrary newFunctionWithName: @"fragmentMain"];
        SkASSERT(pipelineDescriptor.vertexFunction);
        SkASSERT(pipelineDescriptor.fragmentFunction);
        if (precompiledLibs->fRTFlip) {
            this->addRTFlipUniform(SKSL_RTFLIP_NAME);
        }
    } else {
        id<MTLLibrary> shaderLibraries[kGrShaderTypeCount];

        this->finalizeShaders();

        SkSL::ProgramSettings settings;
        settings.fSharpenTextures = true;
        SkASSERT(!this->fragColorIsInOut());

        SkReadBuffer reader;
        SkFourByteTag shaderType = 0;
        if (persistentCache && cached) {
            reader.setMemory(cached->data(), cached->size());
            shaderType = GrPersistentCacheUtils::GetType(&reader);
        }

        auto errorHandler = fGpu->getContext()->priv().getShaderErrorHandler();
        std::string msl[kGrShaderTypeCount];
        SkSL::Program::Interface interfaces[kGrShaderTypeCount];

        // Unpack any stored shaders from the persistent cache
        if (cached) {
            switch (shaderType) {
                case kMSL_Tag: {
                    GrPersistentCacheUtils::UnpackCachedShaders(
                            &reader, msl, interfaces, kGrShaderTypeCount);
                    break;
                }

                case kSKSL_Tag: {
                    std::string cached_sksl[kGrShaderTypeCount];
                    if (GrPersistentCacheUtils::UnpackCachedShaders(
                                &reader, cached_sksl, interfaces, kGrShaderTypeCount)) {
                        bool success = skgpu::SkSLToMSL(mtlCaps->shaderCaps(),
                                                        cached_sksl[kVertex_GrShaderType],
                                                        SkSL::ProgramKind::kVertex,
                                                        settings,
                                                        &msl[kVertex_GrShaderType],
                                                        &interfaces[kVertex_GrShaderType],
                                                        errorHandler);
                        success = success && skgpu::SkSLToMSL(mtlCaps->shaderCaps(),
                                                              cached_sksl[kFragment_GrShaderType],
                                                              SkSL::ProgramKind::kFragment,
                                                              settings,
                                                              &msl[kFragment_GrShaderType],
                                                              &interfaces[kFragment_GrShaderType],
                                                              errorHandler);
                        if (!success) {
                            return nullptr;
                        }
                    }
                    break;
                }

                default: {
                    break;
                }
            }
        }

        // Create any MSL shaders from pipeline data if necessary and cache
        if (msl[kVertex_GrShaderType].empty() || msl[kFragment_GrShaderType].empty()) {
            bool success = true;
            if (msl[kVertex_GrShaderType].empty()) {
                success = skgpu::SkSLToMSL(mtlCaps->shaderCaps(),
                                           fVS.fCompilerString,
                                           SkSL::ProgramKind::kVertex,
                                           settings,
                                           &msl[kVertex_GrShaderType],
                                           &interfaces[kVertex_GrShaderType],
                                           errorHandler);
            }
            if (success && msl[kFragment_GrShaderType].empty()) {
                success = skgpu::SkSLToMSL(mtlCaps->shaderCaps(),
                                           fFS.fCompilerString,
                                           SkSL::ProgramKind::kFragment,
                                           settings,
                                           &msl[kFragment_GrShaderType],
                                           &interfaces[kFragment_GrShaderType],
                                           errorHandler);
            }
            if (!success) {
                return nullptr;
            }

            if (persistentCache && !cached) {
                sk_sp<SkData> pipelineData = writer->snapshotAsData();
                if (fGpu->getContext()->priv().options().fShaderCacheStrategy ==
                        GrContextOptions::ShaderCacheStrategy::kSkSL) {
                    std::string sksl[kGrShaderTypeCount];
                    sksl[kVertex_GrShaderType] = SkShaderUtils::PrettyPrint(fVS.fCompilerString);
                    sksl[kFragment_GrShaderType] = SkShaderUtils::PrettyPrint(fFS.fCompilerString);
                    this->storeShadersInCache(sksl, interfaces, &settings,
                                              std::move(pipelineData), true);
                } else {
                    /*** dump pipeline data here */
                    this->storeShadersInCache(msl, interfaces, nullptr,
                                              std::move(pipelineData), false);
                }
            }
        }

        // Compile MSL to libraries
        shaderLibraries[kVertex_GrShaderType] = this->compileMtlShaderLibrary(
                                                        msl[kVertex_GrShaderType],
                                                        interfaces[kVertex_GrShaderType],
                                                        errorHandler);
        shaderLibraries[kFragment_GrShaderType] = this->compileMtlShaderLibrary(
                                                        msl[kFragment_GrShaderType],
                                                        interfaces[kFragment_GrShaderType],
                                                        errorHandler);
        if (!shaderLibraries[kVertex_GrShaderType] || !shaderLibraries[kFragment_GrShaderType]) {
            return nullptr;
        }

        pipelineDescriptor.vertexFunction =
                [shaderLibraries[kVertex_GrShaderType] newFunctionWithName: @"vertexMain"];
        pipelineDescriptor.fragmentFunction =
                [shaderLibraries[kFragment_GrShaderType] newFunctionWithName: @"fragmentMain"];
    }

    if (pipelineDescriptor.vertexFunction == nil) {
        SkDebugf("Couldn't find vertexMain() in library\n");
        return nullptr;
    }
    if (pipelineDescriptor.fragmentFunction == nil) {
        SkDebugf("Couldn't find fragmentMain() in library\n");
        return nullptr;
    }
    SkASSERT(pipelineDescriptor.vertexFunction);
    SkASSERT(pipelineDescriptor.fragmentFunction);

    NSError* error = nil;
#if GR_METAL_SDK_VERSION >= 230
    if (@available(macOS 11.0, iOS 14.0, tvOS 14.0, *)) {
        id<MTLBinaryArchive> archive = fGpu->binaryArchive();
        if (archive) {
            NSArray* archiveArray = [NSArray arrayWithObjects:archive, nil];
            pipelineDescriptor.binaryArchives = archiveArray;
            BOOL result;
            {
                TRACE_EVENT0("skia.shaders", "addRenderPipelineFunctionsWithDescriptor");
                result = [archive addRenderPipelineFunctionsWithDescriptor: pipelineDescriptor
                                                                            error: &error];
            }
            if (!result && error) {
                SkDebugf("Error storing pipeline: %s\n",
                        [[error localizedDescription] cStringUsingEncoding: NSASCIIStringEncoding]);
            }
        }
    }
#endif

    id<MTLRenderPipelineState> pipelineState;
    {
        TRACE_EVENT0("skia.shaders", "newRenderPipelineStateWithDescriptor");
        if (@available(macOS 10.15, *)) {
            pipelineState = [fGpu->device() newRenderPipelineStateWithDescriptor: pipelineDescriptor
                                                                           error: &error];
        } else {
            pipelineState = GrMtlNewRenderPipelineStateWithDescriptor(
                    fGpu->device(), pipelineDescriptor, &error);
        }
    }
    if (error) {
        SkDebugf("Error creating pipeline: %s\n",
                 [[error localizedDescription] cStringUsingEncoding: NSASCIIStringEncoding]);
        return nullptr;
    }
    if (!pipelineState) {
        return nullptr;
    }

    sk_sp<GrMtlRenderPipeline> renderPipeline = GrMtlRenderPipeline::Make(pipelineState);

    uint32_t bufferSize = buffer_size(fUniformHandler.fCurrentUBOOffset,
                                      fUniformHandler.fCurrentUBOMaxAlignment);
    return new GrMtlPipelineState(fGpu,
                                  std::move(renderPipeline),
                                  pipelineDescriptor.colorAttachments[0].pixelFormat,
                                  fUniformHandles,
                                  fUniformHandler.fUniforms,
                                  bufferSize,
                                  (uint32_t)fUniformHandler.numSamplers(),
                                  std::move(fGPImpl),
                                  std::move(fXPImpl),
                                  std::move(fFPImpls));
}

//////////////////////////////////////////////////////////////////////////////

bool GrMtlPipelineStateBuilder::PrecompileShaders(GrMtlGpu* gpu, const SkData& cachedData,
                                                  GrMtlPrecompiledLibraries* precompiledLibs) {
    SkASSERT(precompiledLibs);

    SkReadBuffer reader(cachedData.data(), cachedData.size());
    SkFourByteTag shaderType = GrPersistentCacheUtils::GetType(&reader);

    auto errorHandler = gpu->getContext()->priv().getShaderErrorHandler();

    SkSL::ProgramSettings settings;
    settings.fSharpenTextures = true;
    GrPersistentCacheUtils::ShaderMetadata meta;
    meta.fSettings = &settings;

    std::string shaders[kGrShaderTypeCount];
    SkSL::Program::Interface interfaces[kGrShaderTypeCount];
    if (!GrPersistentCacheUtils::UnpackCachedShaders(
                &reader, shaders, interfaces, kGrShaderTypeCount, &meta)) {
        return false;
    }

    // skip the size
    reader.readUInt();
    auto pipelineDescriptor = read_pipeline_data(&reader);
    if (!reader.isValid()) {
        return false;
    }

    switch (shaderType) {
        case kMSL_Tag: {
            precompiledLibs->fVertexLibrary =
                    GrCompileMtlShaderLibrary(gpu, shaders[kVertex_GrShaderType], errorHandler);
            precompiledLibs->fFragmentLibrary =
                    GrCompileMtlShaderLibrary(gpu, shaders[kFragment_GrShaderType], errorHandler);
            break;
        }

        case kSKSL_Tag: {
            std::string msl[kGrShaderTypeCount];
            if (!skgpu::SkSLToMSL(gpu->caps()->shaderCaps(),
                                  shaders[kVertex_GrShaderType],
                                  SkSL::ProgramKind::kVertex,
                                  settings,
                                  &msl[kVertex_GrShaderType],
                                  &interfaces[kVertex_GrShaderType],
                                  errorHandler)) {
                return false;
            }
            if (!skgpu::SkSLToMSL(gpu->caps()->shaderCaps(),
                                  shaders[kFragment_GrShaderType],
                                  SkSL::ProgramKind::kFragment,
                                  settings,
                                  &msl[kFragment_GrShaderType],
                                  &interfaces[kFragment_GrShaderType],
                                  errorHandler)) {
                return false;
            }
            precompiledLibs->fVertexLibrary =
                    GrCompileMtlShaderLibrary(gpu, msl[kVertex_GrShaderType], errorHandler);
            precompiledLibs->fFragmentLibrary =
                    GrCompileMtlShaderLibrary(gpu, msl[kFragment_GrShaderType], errorHandler);
            break;
        }

        default: {
            return false;
        }
    }

    pipelineDescriptor.vertexFunction =
            [precompiledLibs->fVertexLibrary newFunctionWithName: @"vertexMain"];
    pipelineDescriptor.fragmentFunction =
            [precompiledLibs->fFragmentLibrary newFunctionWithName: @"fragmentMain"];

#if GR_METAL_SDK_VERSION >= 230
    if (@available(macOS 11.0, iOS 14.0, tvOS 14.0, *)) {
        id<MTLBinaryArchive> archive = gpu->binaryArchive();
        if (archive) {
            NSArray* archiveArray = [NSArray arrayWithObjects:archive, nil];
            pipelineDescriptor.binaryArchives = archiveArray;
            BOOL result;
            NSError* error = nil;
            {
                TRACE_EVENT0("skia.shaders", "addRenderPipelineFunctionsWithDescriptor");
                result = [archive addRenderPipelineFunctionsWithDescriptor: pipelineDescriptor
                                                                            error: &error];
            }
            if (!result && error) {
                SkDebugf("Error storing pipeline: %s\n",
                        [[error localizedDescription] cStringUsingEncoding: NSASCIIStringEncoding]);
            }
        }
    }
#endif
    {
        TRACE_EVENT0("skia.shaders", "newRenderPipelineStateWithDescriptor");
        MTLNewRenderPipelineStateCompletionHandler completionHandler =
                 ^(id<MTLRenderPipelineState> state, NSError* error) {
                     if (error) {
                         SkDebugf("Error creating pipeline: %s\n",
                                  [[error localizedDescription]
                                           cStringUsingEncoding: NSASCIIStringEncoding]);
                     }
                 };

        // kick off asynchronous pipeline build and depend on Apple's cache to manage it
        [gpu->device() newRenderPipelineStateWithDescriptor: pipelineDescriptor
                                          completionHandler: completionHandler];
    }

    precompiledLibs->fRTFlip = (interfaces[kFragment_GrShaderType].fRTFlipUniform !=
                                SkSL::Program::Interface::kRTFlip_None);
    return true;
}

GR_NORETAIN_END
