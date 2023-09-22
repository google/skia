/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/mtl/GrMtlResourceProvider.h"

#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrProgramDesc.h"
#include "src/gpu/ganesh/mtl/GrMtlCommandBuffer.h"
#include "src/gpu/ganesh/mtl/GrMtlGpu.h"
#include "src/gpu/ganesh/mtl/GrMtlPipelineState.h"
#include "src/gpu/ganesh/mtl/GrMtlUtil.h"

#include "src/sksl/SkSLCompiler.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GR_NORETAIN_BEGIN

GrMtlResourceProvider::GrMtlResourceProvider(GrMtlGpu* gpu)
    : fGpu(gpu) {
    fPipelineStateCache.reset(new PipelineStateCache(gpu));
}

GrMtlPipelineState* GrMtlResourceProvider::findOrCreateCompatiblePipelineState(
        const GrProgramDesc& programDesc,
        const GrProgramInfo& programInfo,
        GrThreadSafePipelineBuilder::Stats::ProgramCacheResult* stat) {
    return fPipelineStateCache->refPipelineState(programDesc, programInfo, stat);
}

bool GrMtlResourceProvider::precompileShader(const SkData& key, const SkData& data) {
    return fPipelineStateCache->precompileShader(key, data);
}

////////////////////////////////////////////////////////////////////////////////////////////////

GrMtlDepthStencil* GrMtlResourceProvider::findOrCreateCompatibleDepthStencilState(
        const GrStencilSettings& stencil, GrSurfaceOrigin origin) {
    GrMtlDepthStencil* depthStencilState;
    GrMtlDepthStencil::Key key = GrMtlDepthStencil::GenerateKey(stencil, origin);
    depthStencilState = fDepthStencilStates.find(key);
    if (!depthStencilState) {
        depthStencilState = GrMtlDepthStencil::Create(fGpu, stencil, origin);
        fDepthStencilStates.add(depthStencilState);
    }
    SkASSERT(depthStencilState);
    return depthStencilState;
}

GrMtlSampler* GrMtlResourceProvider::findOrCreateCompatibleSampler(GrSamplerState params) {
    GrMtlSampler* sampler;
    sampler = fSamplers.find(GrMtlSampler::GenerateKey(params));
    if (!sampler) {
        sampler = GrMtlSampler::Create(fGpu, params);
        fSamplers.add(sampler);
    }
    SkASSERT(sampler);
    return sampler;
}

const GrMtlRenderPipeline* GrMtlResourceProvider::findOrCreateMSAALoadPipeline(
        MTLPixelFormat colorFormat, int sampleCount, MTLPixelFormat stencilFormat) {
    if (!fMSAALoadLibrary) {
        TRACE_EVENT0("skia", TRACE_FUNC);

        std::string shaderText;
        shaderText.append(
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
                "    uint2 uTextureSize;\n"
                "} VertexUniforms;\n"
                "\n"
                "vertex VertexOutput vertexMain(constant VertexUniforms& uniforms [[buffer(0)]],\n"
                "                               uint vertexID [[vertex_id]]) {\n"
                "    VertexOutput out;\n"
                "    float2 position = float2(float(vertexID >> 1), float(vertexID & 1));\n"
                "    out.position.xy = position * uniforms.uPosXform.xy + uniforms.uPosXform.zw;\n"
                "    out.position.zw = float2(0.0, 1.0);\n"
                "    return out;\n"
                "}\n"
                "\n"
                "fragment float4 fragmentMain(VertexOutput in [[stage_in]],\n"
                "                             texture2d<half> colorMap [[texture(0)]]) {\n"
                "    uint2 coords = uint2(in.position.x, in.position.y);"
                "    half4 colorSample   = colorMap.read(coords);\n"
                "    return float4(colorSample);\n"
                "}"
        );

        auto errorHandler = fGpu->getContext()->priv().getShaderErrorHandler();
        fMSAALoadLibrary = GrCompileMtlShaderLibrary(fGpu, shaderText, errorHandler);
        if (!fMSAALoadLibrary) {
            return nullptr;
        }
    }

    for (int i = 0; i < fMSAALoadPipelines.size(); ++i) {
        if (fMSAALoadPipelines[i].fColorFormat == colorFormat &&
            fMSAALoadPipelines[i].fSampleCount == sampleCount &&
            fMSAALoadPipelines[i].fStencilFormat == stencilFormat) {
            return fMSAALoadPipelines[i].fPipeline.get();
        }
    }

    auto pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];

    pipelineDescriptor.label = @"loadMSAAFromResolve";

    pipelineDescriptor.vertexFunction =
            [fMSAALoadLibrary newFunctionWithName: @"vertexMain"];
    pipelineDescriptor.fragmentFunction =
            [fMSAALoadLibrary newFunctionWithName: @"fragmentMain"];

    auto mtlColorAttachment = [[MTLRenderPipelineColorAttachmentDescriptor alloc] init];

    mtlColorAttachment.pixelFormat = colorFormat;
    mtlColorAttachment.blendingEnabled = FALSE;
    mtlColorAttachment.writeMask = MTLColorWriteMaskAll;

    pipelineDescriptor.colorAttachments[0] = mtlColorAttachment;
    pipelineDescriptor.rasterSampleCount = sampleCount;

    pipelineDescriptor.stencilAttachmentPixelFormat = stencilFormat;

    NSError* error;
    auto pso =
            [fGpu->device() newRenderPipelineStateWithDescriptor: pipelineDescriptor
                                                          error: &error];
    if (!pso) {
        SkDebugf("Error creating pipeline: %s\n",
                 [[error localizedDescription] cStringUsingEncoding: NSASCIIStringEncoding]);
    }

    auto renderPipeline = GrMtlRenderPipeline::Make(pso);

    fMSAALoadPipelines.push_back({renderPipeline, colorFormat, sampleCount, stencilFormat});
    return fMSAALoadPipelines[fMSAALoadPipelines.size()-1].fPipeline.get();
}

void GrMtlResourceProvider::destroyResources() {
    fMSAALoadLibrary = nil;
    fMSAALoadPipelines.clear();

    fSamplers.foreach([&](GrMtlSampler* sampler) { sampler->unref(); });
    fSamplers.reset();

    fDepthStencilStates.foreach([&](GrMtlDepthStencil* stencil) { stencil->unref(); });
    fDepthStencilStates.reset();

    fPipelineStateCache->release();
}

////////////////////////////////////////////////////////////////////////////////////////////////

struct GrMtlResourceProvider::PipelineStateCache::Entry {
    Entry(GrMtlPipelineState* pipelineState)
            : fPipelineState(pipelineState) {}
    Entry(const GrMtlPrecompiledLibraries& precompiledLibraries)
            : fPipelineState(nullptr)
            , fPrecompiledLibraries(precompiledLibraries) {}

    std::unique_ptr<GrMtlPipelineState> fPipelineState;

    // TODO: change to one library once we can build that
    GrMtlPrecompiledLibraries fPrecompiledLibraries;
};

GrMtlResourceProvider::PipelineStateCache::PipelineStateCache(GrMtlGpu* gpu)
    : fMap(gpu->getContext()->priv().options().fRuntimeProgramCacheSize)
    , fGpu(gpu) {}

GrMtlResourceProvider::PipelineStateCache::~PipelineStateCache() {
    SkASSERT(0 == fMap.count());
}

void GrMtlResourceProvider::PipelineStateCache::release() {
    fMap.reset();
}

GrMtlPipelineState* GrMtlResourceProvider::PipelineStateCache::refPipelineState(
        const GrProgramDesc& desc,
        const GrProgramInfo& programInfo,
        Stats::ProgramCacheResult* statPtr) {

    if (!statPtr) {
        // If stat is NULL we are using inline compilation rather than through DDL,
        // so we need to track those stats as well.
        GrThreadSafePipelineBuilder::Stats::ProgramCacheResult stat;
        auto tmp = this->onRefPipelineState(desc, programInfo, &stat);
        if (!tmp) {
            fStats.incNumInlineCompilationFailures();
        } else {
            fStats.incNumInlineProgramCacheResult(stat);
        }
        return tmp;
    } else {
        return this->onRefPipelineState(desc, programInfo, statPtr);
    }
}

GrMtlPipelineState* GrMtlResourceProvider::PipelineStateCache::onRefPipelineState(
        const GrProgramDesc& desc,
        const GrProgramInfo& programInfo,
        Stats::ProgramCacheResult* stat) {
    *stat = Stats::ProgramCacheResult::kHit;
    std::unique_ptr<Entry>* entry = fMap.find(desc);
    if (entry && !(*entry)->fPipelineState) {
        // We've pre-compiled the MSL shaders but don't yet have the pipelineState
        const GrMtlPrecompiledLibraries* precompiledLibs = &((*entry)->fPrecompiledLibraries);
        SkASSERT(precompiledLibs->fVertexLibrary);
        SkASSERT(precompiledLibs->fFragmentLibrary);
        (*entry)->fPipelineState.reset(
                GrMtlPipelineStateBuilder::CreatePipelineState(fGpu, desc, programInfo,
                                                               precompiledLibs));
        if (!(*entry)->fPipelineState) {
            // Should we purge the precompiled shaders from the cache at this point?
            SkDEBUGFAIL("Couldn't create pipelineState from precompiled shaders");
            fStats.incNumCompilationFailures();
            return nullptr;
        }
        // release the libraries
        (*entry)->fPrecompiledLibraries.fVertexLibrary = nil;
        (*entry)->fPrecompiledLibraries.fFragmentLibrary = nil;

        fStats.incNumPartialCompilationSuccesses();
        *stat = Stats::ProgramCacheResult::kPartial;
    } else if (!entry) {
        GrMtlPipelineState* pipelineState(
                GrMtlPipelineStateBuilder::CreatePipelineState(fGpu, desc, programInfo));
        if (!pipelineState) {
            fStats.incNumCompilationFailures();
           return nullptr;
        }
        fStats.incNumCompilationSuccesses();
        entry = fMap.insert(desc, std::unique_ptr<Entry>(new Entry(pipelineState)));
        *stat = Stats::ProgramCacheResult::kMiss;
        return (*entry)->fPipelineState.get();
    }
    return (*entry)->fPipelineState.get();
}

bool GrMtlResourceProvider::PipelineStateCache::precompileShader(const SkData& key,
                                                                 const SkData& data) {
    GrProgramDesc desc;
    if (!GrProgramDesc::BuildFromData(&desc, key.data(), key.size())) {
        return false;
    }

    std::unique_ptr<Entry>* entry = fMap.find(desc);
    if (entry) {
        // We've already seen/compiled this shader
        return true;
    }

    GrMtlPrecompiledLibraries precompiledLibraries;
    if (!GrMtlPipelineStateBuilder::PrecompileShaders(fGpu, data, &precompiledLibraries)) {
        return false;
    }

    fMap.insert(desc, std::make_unique<Entry>(precompiledLibraries));
    return true;

}

GR_NORETAIN_END
