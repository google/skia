/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/mtl/MtlResourceProvider.h"

#include "include/gpu/ShaderErrorHandler.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "src/sksl/SkSLProgramKind.h"

#include "src/core/SkSLTypeShared.h"
#include "src/gpu/Blend.h"
#include "src/gpu/PipelineUtils.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/graphite/ComputePipelineDesc.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/compute/ComputeStep.h"
#include "src/gpu/graphite/mtl/MtlBuffer.h"
#include "src/gpu/graphite/mtl/MtlCommandBuffer.h"
#include "src/gpu/graphite/mtl/MtlComputePipeline.h"
#include "src/gpu/graphite/mtl/MtlGraphicsPipeline.h"
#include "src/gpu/graphite/mtl/MtlGraphiteUtilsPriv.h"
#include "src/gpu/graphite/mtl/MtlSampler.h"
#include "src/gpu/graphite/mtl/MtlSharedContext.h"
#include "src/gpu/graphite/mtl/MtlTexture.h"
#include "src/gpu/mtl/MtlUtilsPriv.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLProgramSettings.h"

#import <Metal/Metal.h>

namespace skgpu::graphite {

MtlResourceProvider::MtlResourceProvider(SharedContext* sharedContext,
                                         SingleOwner* singleOwner,
                                         uint32_t recorderID,
                                         size_t resourceBudget)
        : ResourceProvider(sharedContext, singleOwner, recorderID, resourceBudget) {}

const MtlSharedContext* MtlResourceProvider::mtlSharedContext() {
    return static_cast<const MtlSharedContext*>(fSharedContext);
}

sk_sp<MtlGraphicsPipeline> MtlResourceProvider::findOrCreateLoadMSAAPipeline(
        const RenderPassDesc& renderPassDesc) {
    uint64_t renderPassKey =
            this->mtlSharedContext()->mtlCaps().getRenderPassDescKey(renderPassDesc);
    sk_sp<MtlGraphicsPipeline> pipeline = fLoadMSAAPipelines[renderPassKey];
    if (!pipeline) {
        static const char* kLoadMSAAShaderText = R"(
                #include <metal_stdlib>
                #include <simd/simd.h>
                using namespace metal;

                typedef struct {
                    float4 position [[position]];
                } VertexOutput;

                vertex VertexOutput vertexMain(uint vertexID [[vertex_id]]) {
                    VertexOutput out;
                    float2 position = float2(float(vertexID >> 1), float(vertexID & 1));
                    out.position = float4(2.0 * position - 1.0, 0.0, 1.0);
                    return out;
                }

                fragment float4 fragmentMain(VertexOutput in [[stage_in]],
                                             texture2d<half> colorMap [[texture(0)]]) {
                    uint2 coords = uint2(in.position.x, in.position.y);
                    half4 colorSample   = colorMap.read(coords);
                    return float4(colorSample);
                }
        )";

        auto mtlLibrary = MtlCompileShaderLibrary(this->mtlSharedContext(),
                                                  kLoadMSAAShaderText,
                                                  fSharedContext->caps()->shaderErrorHandler());

        BlendInfo noBlend{}; // default is equivalent to kSrc blending
        sk_cfp<id<MTLDepthStencilState>> ignoreDS =
                this->findOrCreateCompatibleDepthStencilState({});
        pipeline = MtlGraphicsPipeline::Make(this->mtlSharedContext(),
                                             "LoadMSAAFromResolve",
                                             {mtlLibrary.get(), "vertexMain"},
                                             /*vertexAttrs=*/{},
                                             /*instanceAttrs=*/{},
                                             {mtlLibrary.get(), "fragmentMain"},
                                             std::move(ignoreDS),
                                             /*stencilRefValue=*/0,
                                             noBlend,
                                             renderPassDesc,
                                             /*pipelineShaders=*/nullptr);
        if (pipeline) {
            fLoadMSAAPipelines.set(renderPassKey, pipeline);
        }
    }

    return pipeline;
}

sk_sp<GraphicsPipeline> MtlResourceProvider::createGraphicsPipeline(
        const RuntimeEffectDictionary* runtimeDict,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc) {
    std::string vsMSL, fsMSL;
    SkSL::Program::Interface vsInterface, fsInterface;
    SkSL::ProgramSettings settings;

    settings.fForceNoRTFlip = true;

    SkSL::Compiler skslCompiler(fSharedContext->caps()->shaderCaps());
    ShaderErrorHandler* errorHandler = fSharedContext->caps()->shaderErrorHandler();

    const RenderStep* step =
            fSharedContext->rendererProvider()->lookup(pipelineDesc.renderStepID());

    bool useShadingSsboIndex =
            fSharedContext->caps()->storageBufferPreferred() && step->performsShading();

    FragSkSLInfo fsSkSLInfo = BuildFragmentSkSL(fSharedContext->caps(),
                                                fSharedContext->shaderCodeDictionary(),
                                                runtimeDict,
                                                step,
                                                pipelineDesc.paintParamsID(),
                                                useShadingSsboIndex,
                                                renderPassDesc.fWriteSwizzle);
    std::string& fsSkSL = fsSkSLInfo.fSkSL;
    const BlendInfo& blendInfo = fsSkSLInfo.fBlendInfo;
    const bool localCoordsNeeded = fsSkSLInfo.fRequiresLocalCoords;
    if (!SkSLToMSL(&skslCompiler,
                   fsSkSL,
                   SkSL::ProgramKind::kGraphiteFragment,
                   settings,
                   &fsMSL,
                   &fsInterface,
                   errorHandler)) {
        return nullptr;
    }

    std::string vsSkSL = BuildVertexSkSL(fSharedContext->caps()->resourceBindingRequirements(),
                                         step,
                                         useShadingSsboIndex,
                                         localCoordsNeeded);
    if (!SkSLToMSL(&skslCompiler,
                   vsSkSL,
                   SkSL::ProgramKind::kGraphiteVertex,
                   settings,
                   &vsMSL,
                   &vsInterface,
                   errorHandler)) {
        return nullptr;
    }

    auto vsLibrary = MtlCompileShaderLibrary(this->mtlSharedContext(), vsMSL, errorHandler);
    auto fsLibrary = MtlCompileShaderLibrary(this->mtlSharedContext(), fsMSL, errorHandler);

    sk_cfp<id<MTLDepthStencilState>> dss =
            this->findOrCreateCompatibleDepthStencilState(step->depthStencilSettings());

#if defined(GRAPHITE_TEST_UTILS)
    GraphicsPipeline::PipelineInfo pipelineInfo = {pipelineDesc.renderStepID(),
                                                   pipelineDesc.paintParamsID(),
                                                   std::move(vsSkSL),
                                                   std::move(fsSkSL),
                                                   std::move(vsMSL),
                                                   std::move(fsMSL) };
    GraphicsPipeline::PipelineInfo* pipelineInfoPtr = &pipelineInfo;
#else
    GraphicsPipeline::PipelineInfo* pipelineInfoPtr = nullptr;
#endif
    return MtlGraphicsPipeline::Make(this->mtlSharedContext(),
                                     step->name(),
                                     {vsLibrary.get(), "vertexMain"},
                                     step->vertexAttributes(),
                                     step->instanceAttributes(),
                                     {fsLibrary.get(), "fragmentMain"},
                                     std::move(dss),
                                     step->depthStencilSettings().fStencilReferenceValue,
                                     blendInfo,
                                     renderPassDesc,
                                     pipelineInfoPtr);
}

sk_sp<ComputePipeline> MtlResourceProvider::createComputePipeline(
        const ComputePipelineDesc& pipelineDesc) {
    sk_cfp<id<MTLLibrary>> library;
    std::string entryPointName;
    ShaderErrorHandler* errorHandler = fSharedContext->caps()->shaderErrorHandler();
    if (pipelineDesc.computeStep()->supportsNativeShader()) {
        auto nativeShader = pipelineDesc.computeStep()->nativeShaderSource(
                ComputeStep::NativeShaderFormat::kMSL);
        library = MtlCompileShaderLibrary(
                this->mtlSharedContext(), nativeShader.fSource, errorHandler);
        if (library == nil) {
            return nullptr;
        }
        entryPointName = std::move(nativeShader.fEntryPoint);
    } else {
        std::string msl;
        SkSL::Program::Interface interface;
        SkSL::ProgramSettings settings;

        SkSL::Compiler skslCompiler(fSharedContext->caps()->shaderCaps());
        std::string sksl = BuildComputeSkSL(fSharedContext->caps(), pipelineDesc.computeStep());
        if (!SkSLToMSL(&skslCompiler,
                       sksl,
                       SkSL::ProgramKind::kCompute,
                       settings,
                       &msl,
                       &interface,
                       errorHandler)) {
            return nullptr;
        }
        library = MtlCompileShaderLibrary(this->mtlSharedContext(), msl, errorHandler);
        entryPointName = "computeMain";
    }
    return MtlComputePipeline::Make(this->mtlSharedContext(),
                                    pipelineDesc.computeStep()->name(),
                                    {library.get(), std::move(entryPointName)});
}

sk_sp<Texture> MtlResourceProvider::createTexture(SkISize dimensions,
                                                  const TextureInfo& info,
                                                  skgpu::Budgeted budgeted) {
    return MtlTexture::Make(this->mtlSharedContext(), dimensions, info, budgeted);
}

sk_sp<Texture> MtlResourceProvider::createWrappedTexture(const BackendTexture& texture) {
    MtlHandle mtlHandleTexture = texture.getMtlTexture();
    if (!mtlHandleTexture) {
        return nullptr;
    }
    sk_cfp<id<MTLTexture>> mtlTexture = sk_ret_cfp((id<MTLTexture>)mtlHandleTexture);
    return MtlTexture::MakeWrapped(this->mtlSharedContext(),
                                   texture.dimensions(),
                                   texture.info(),
                                   std::move(mtlTexture));
}

sk_sp<Buffer> MtlResourceProvider::createBuffer(size_t size,
                                                BufferType type,
                                                AccessPattern accessPattern) {
    return MtlBuffer::Make(this->mtlSharedContext(), size, type, accessPattern);
}

sk_sp<Sampler> MtlResourceProvider::createSampler(const SkSamplingOptions& samplingOptions,
                                                  SkTileMode xTileMode,
                                                  SkTileMode yTileMode) {
    return MtlSampler::Make(this->mtlSharedContext(), samplingOptions, xTileMode, yTileMode);
}

namespace {
MTLCompareFunction compare_op_to_mtl(CompareOp op) {
    switch (op) {
        case CompareOp::kAlways:
            return MTLCompareFunctionAlways;
        case CompareOp::kNever:
            return MTLCompareFunctionNever;
        case CompareOp::kGreater:
            return MTLCompareFunctionGreater;
        case CompareOp::kGEqual:
            return MTLCompareFunctionGreaterEqual;
        case CompareOp::kLess:
            return MTLCompareFunctionLess;
        case CompareOp::kLEqual:
            return MTLCompareFunctionLessEqual;
        case CompareOp::kEqual:
            return MTLCompareFunctionEqual;
        case CompareOp::kNotEqual:
            return MTLCompareFunctionNotEqual;
    }
}

MTLStencilOperation stencil_op_to_mtl(StencilOp op) {
    switch (op) {
        case StencilOp::kKeep:
            return MTLStencilOperationKeep;
        case StencilOp::kZero:
            return MTLStencilOperationZero;
        case StencilOp::kReplace:
            return MTLStencilOperationReplace;
        case StencilOp::kInvert:
            return MTLStencilOperationInvert;
        case StencilOp::kIncWrap:
            return MTLStencilOperationIncrementWrap;
        case StencilOp::kDecWrap:
            return MTLStencilOperationDecrementWrap;
        case StencilOp::kIncClamp:
            return MTLStencilOperationIncrementClamp;
        case StencilOp::kDecClamp:
            return MTLStencilOperationDecrementClamp;
    }
}

MTLStencilDescriptor* stencil_face_to_mtl(DepthStencilSettings::Face face) {
    MTLStencilDescriptor* result = [[MTLStencilDescriptor alloc] init];
    result.stencilCompareFunction = compare_op_to_mtl(face.fCompareOp);
    result.readMask = face.fReadMask;
    result.writeMask = face.fWriteMask;
    result.depthStencilPassOperation = stencil_op_to_mtl(face.fDepthStencilPassOp);
    result.stencilFailureOperation = stencil_op_to_mtl(face.fStencilFailOp);
    return result;
}
}  // anonymous namespace

sk_cfp<id<MTLDepthStencilState>> MtlResourceProvider::findOrCreateCompatibleDepthStencilState(
            const DepthStencilSettings& depthStencilSettings) {
    sk_cfp<id<MTLDepthStencilState>>* depthStencilState;
    depthStencilState = fDepthStencilStates.find(depthStencilSettings);
    if (!depthStencilState) {
        MTLDepthStencilDescriptor* desc = [[MTLDepthStencilDescriptor alloc] init];
        SkASSERT(depthStencilSettings.fDepthTestEnabled ||
                 depthStencilSettings.fDepthCompareOp == CompareOp::kAlways);
        desc.depthCompareFunction = compare_op_to_mtl(depthStencilSettings.fDepthCompareOp);
        if (depthStencilSettings.fDepthTestEnabled) {
            desc.depthWriteEnabled = depthStencilSettings.fDepthWriteEnabled;
        }
        if (depthStencilSettings.fStencilTestEnabled) {
            desc.frontFaceStencil = stencil_face_to_mtl(depthStencilSettings.fFrontStencil);
            desc.backFaceStencil = stencil_face_to_mtl(depthStencilSettings.fBackStencil);
        }

        sk_cfp<id<MTLDepthStencilState>> dss(
                [this->mtlSharedContext()->device() newDepthStencilStateWithDescriptor: desc]);
        depthStencilState = fDepthStencilStates.set(depthStencilSettings, std::move(dss));
    }

    SkASSERT(depthStencilState);
    return *depthStencilState;
}

BackendTexture MtlResourceProvider::onCreateBackendTexture(SkISize dimensions,
                                                           const TextureInfo& info) {
    sk_cfp<id<MTLTexture>> texture = MtlTexture::MakeMtlTexture(this->mtlSharedContext(),
                                                                dimensions,
                                                                info);
    if (!texture) {
        return {};
    }
    return BackendTexture(dimensions, (Handle)texture.release());
}

void MtlResourceProvider::onDeleteBackendTexture(const BackendTexture& texture) {
    SkASSERT(texture.backend() == BackendApi::kMetal);
    MtlHandle texHandle = texture.getMtlTexture();
    SkCFSafeRelease(texHandle);
}

} // namespace skgpu::graphite
