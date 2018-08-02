/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrMtlPipelineStateBuilder.h"

#include "GrContext.h"
#include "GrContextPriv.h"

#include "GrMtlGpu.h"
#include "GrMtlPipelineState.h"
#include "GrMtlUtil.h"

#import <simd/simd.h>

GrMtlPipelineState* GrMtlPipelineStateBuilder::CreatePipelineState(
        const GrPrimitiveProcessor& primProc,
        const GrPipeline& pipeline,
        GrProgramDesc* desc,
        GrMtlGpu* gpu) {
    GrMtlPipelineStateBuilder builder(primProc, pipeline, desc, gpu);

    if (!builder.emitAndInstallProcs()) {
        return nullptr;
    }
    return builder.finalize(primProc, pipeline, desc);
}

GrMtlPipelineStateBuilder::GrMtlPipelineStateBuilder(const GrPrimitiveProcessor& primProc,
                                                     const GrPipeline& pipeline,
                                                     GrProgramDesc* desc,
                                                     GrMtlGpu* gpu)
        : INHERITED(primProc, pipeline, desc)
        , fGpu(gpu) {
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
    SkString shaderString;
    for (int i = 0; i < builder.fCompilerStrings.count(); ++i) {
        if (builder.fCompilerStrings[i]) {
            shaderString.append(builder.fCompilerStrings[i]);
            shaderString.append("\n");
        }
    }

    SkSL::Program::Inputs inputs;
    id<MTLLibrary> shaderLibrary = GrCompileMtlShaderLibrary(fGpu, shaderString.c_str(),
                                                             kind, settings, &inputs);
    if (shaderLibrary == nil) {
        return nil;
    }
    if (inputs.fRTHeight) {
        this->addRTHeightUniform(SKSL_RTHEIGHT_NAME);
    }
    if (inputs.fFlipY) {
        desc->setSurfaceOriginKey(GrGLSLFragmentShaderBuilder::KeyForSurfaceOrigin(
                                                     this->pipeline().proxy()->origin()));
        desc->finalize();
    }
    return shaderLibrary;
}

static inline MTLVertexFormat attribute_type_to_mtlformat(GrVertexAttribType type) {
    switch (type) {
        case kFloat_GrVertexAttribType:
        case kHalf_GrVertexAttribType:
            return MTLVertexFormatFloat;
        case kFloat2_GrVertexAttribType:
        case kHalf2_GrVertexAttribType:
            return MTLVertexFormatFloat2;
        case kFloat3_GrVertexAttribType:
        case kHalf3_GrVertexAttribType:
            return MTLVertexFormatFloat3;
        case kFloat4_GrVertexAttribType:
        case kHalf4_GrVertexAttribType:
            return MTLVertexFormatFloat4;
        case kInt2_GrVertexAttribType:
            return MTLVertexFormatInt2;
        case kInt3_GrVertexAttribType:
            return MTLVertexFormatInt3;
        case kInt4_GrVertexAttribType:
            return MTLVertexFormatInt4;
        case kByte_GrVertexAttribType:
            return MTLVertexFormatChar;
        case kByte2_GrVertexAttribType:
            return MTLVertexFormatChar2;
        case kByte3_GrVertexAttribType:
            return MTLVertexFormatChar3;
        case kByte4_GrVertexAttribType:
            return MTLVertexFormatChar4;
        case kUByte_GrVertexAttribType:
            return MTLVertexFormatUChar;
        case kUByte2_GrVertexAttribType:
            return MTLVertexFormatUChar2;
        case kUByte3_GrVertexAttribType:
            return MTLVertexFormatUChar3;
        case kUByte4_GrVertexAttribType:
            return MTLVertexFormatUChar4;
        case kUByte_norm_GrVertexAttribType:
            return MTLVertexFormatUCharNormalized;
        case kUByte4_norm_GrVertexAttribType:
            return MTLVertexFormatUChar4Normalized;
        case kShort2_GrVertexAttribType:
            return MTLVertexFormatShort2;
        case kUShort2_GrVertexAttribType:
            return MTLVertexFormatUShort2;
        case kUShort2_norm_GrVertexAttribType:
            return MTLVertexFormatUShort2Normalized;
        case kInt_GrVertexAttribType:
            return MTLVertexFormatInt;
        case kUint_GrVertexAttribType:
            return MTLVertexFormatUInt;
    }
    SK_ABORT("Unknown vertex attribute type");
    return MTLVertexFormatInvalid;
}

static MTLVertexDescriptor* create_vertex_descriptor(const GrPrimitiveProcessor& primProc) {
    uint32_t vertexBinding = 0, instanceBinding = 0;

    int nextBinding = 0;
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
    for (int vertexIndex = 0; vertexIndex < vertexAttributeCount; vertexIndex++) {
        const GrGeometryProcessor::Attribute& attribute = primProc.vertexAttribute(vertexIndex);
        MTLVertexAttributeDescriptor* mtlAttribute = vertexDescriptor.attributes[attributeIndex];
        mtlAttribute.format = attribute_type_to_mtlformat(attribute.type());
        mtlAttribute.offset = vertexAttributeOffset;
        mtlAttribute.bufferIndex = vertexBinding;

        SkASSERT(mtlAttribute.offset == primProc.debugOnly_vertexAttributeOffset(vertexIndex));
        vertexAttributeOffset += attribute.sizeAlign4();
        attributeIndex++;
    }
    SkASSERT(vertexAttributeOffset == primProc.debugOnly_vertexStride());

    if (vertexAttributeCount) {
        MTLVertexBufferLayoutDescriptor* perVertexLayout = vertexDescriptor.layouts[vertexBinding];
        perVertexLayout.stepFunction = MTLVertexStepFunctionPerVertex;
        perVertexLayout.stepRate = 1;
        perVertexLayout.stride = vertexAttributeOffset;
    }

    int instanceAttributeCount = primProc.numInstanceAttributes();
    size_t instanceAttributeOffset = 0;
    for (int instanceIndex = 0; instanceIndex < instanceAttributeCount; instanceIndex++) {
        const GrGeometryProcessor::Attribute& attribute = primProc.instanceAttribute(instanceIndex);
        MTLVertexAttributeDescriptor* mtlAttribute = vertexDescriptor.attributes[attributeIndex];
        mtlAttribute.format = attribute_type_to_mtlformat(attribute.type());
        mtlAttribute.offset = instanceAttributeOffset;
        mtlAttribute.bufferIndex = instanceBinding;

        SkASSERT(mtlAttribute.offset == primProc.debugOnly_instanceAttributeOffset(instanceIndex));
        instanceAttributeOffset += attribute.sizeAlign4();
        attributeIndex++;
    }
    SkASSERT(instanceAttributeOffset == primProc.debugOnly_instanceStride());

    if (instanceAttributeCount) {
        MTLVertexBufferLayoutDescriptor* perInstanceLayout =
                vertexDescriptor.layouts[instanceBinding];
        perInstanceLayout.stepFunction = MTLVertexStepFunctionPerInstance;
        perInstanceLayout.stepRate = 1;
        perInstanceLayout.stride = instanceAttributeOffset;
    }
    return vertexDescriptor;
}

static MTLRenderPipelineColorAttachmentDescriptor* create_color_attachment(
        const GrPipeline& pipeline) {
    auto mtlColorAttachment = [[MTLRenderPipelineColorAttachmentDescriptor alloc] init];

    // pixel format
    MTLPixelFormat format;
    SkASSERT(GrPixelConfigToMTLFormat(pipeline.renderTarget()->config(), &format));
    mtlColorAttachment.pixelFormat = format;

    // blending
    GrXferProcessor::BlendInfo blendInfo;
    pipeline.getXferProcessor().getBlendInfo(&blendInfo);

    GrBlendEquation equation = blendInfo.fEquation;
    GrBlendCoeff srcCoeff = blendInfo.fSrcBlend;
    GrBlendCoeff dstCoeff = blendInfo.fDstBlend;
    bool blendOff = (kAdd_GrBlendEquation == equation || kSubtract_GrBlendEquation == equation) &&
                    kOne_GrBlendCoeff == srcCoeff && kZero_GrBlendCoeff == dstCoeff;

    mtlColorAttachment.blendingEnabled = !blendOff;
    if (!blendOff) {
        // TODO: Support blending
        SkASSERT(false);
        return nil;
    }
    return mtlColorAttachment;
}


GrMtlPipelineState* GrMtlPipelineStateBuilder::finalize(const GrPrimitiveProcessor& primProc,
                                                        const GrPipeline& pipeline,
                                                        GrProgramDesc* desc) {
    auto pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];

    fVS.extensions().appendf("#extension GL_ARB_separate_shader_objects : enable\n");
    fFS.extensions().appendf("#extension GL_ARB_separate_shader_objects : enable\n");
    fVS.extensions().appendf("#extension GL_ARB_shading_language_420pack : enable\n");
    fFS.extensions().appendf("#extension GL_ARB_shading_language_420pack : enable\n");

    this->finalizeShaders();

    SkSL::Program::Settings settings;
    settings.fCaps = this->caps()->shaderCaps();
    settings.fFlipY = this->pipeline().proxy()->origin() != kTopLeft_GrSurfaceOrigin;
    settings.fSharpenTextures = fGpu->getContext()->contextPriv().sharpenMipmappedTextures();
    SkASSERT(!this->fragColorIsInOut());

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
    if (this->primitiveProcessor().willUseGeoShader()) {
        SkASSERT(false); // There are no geometry shaders in Metal. TODO: workaround
    }

    SkASSERT(vertexLibrary);
    SkASSERT(fragmentLibrary);

    id<MTLFunction> vertexFunction = [vertexLibrary newFunctionWithName: @"vertexMain"];
    id<MTLFunction> fragmentFunction = [fragmentLibrary newFunctionWithName: @"fragmentMain"];

    pipelineDescriptor.vertexFunction = vertexFunction;
    pipelineDescriptor.fragmentFunction = fragmentFunction;
    pipelineDescriptor.vertexDescriptor = create_vertex_descriptor(primProc);
    pipelineDescriptor.colorAttachments[0] = create_color_attachment(pipeline);

    SkASSERT(pipelineDescriptor.vertexFunction);
    SkASSERT(pipelineDescriptor.fragmentFunction);
    SkASSERT(pipelineDescriptor.vertexDescriptor);
    SkASSERT(pipelineDescriptor.colorAttachments[0]);

    NSError* error = nil;
    id<MTLRenderPipelineState> pipelineState =
            [fGpu->device() newRenderPipelineStateWithDescriptor: pipelineDescriptor
                                                           error: &error];
    if (error) {
        SkDebugf("Error creating pipeline: %s\n",
                 [[error localizedDescription] cStringUsingEncoding: NSASCIIStringEncoding]);
        pipelineState = nil;
    }
    return new GrMtlPipelineState(pipelineState,
                                  pipelineDescriptor.colorAttachments[0].pixelFormat);
}
