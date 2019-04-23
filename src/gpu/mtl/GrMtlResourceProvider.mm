/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlResourceProvider.h"

#include "GrMtlCopyManager.h"
#include "GrMtlGpu.h"
#include "GrMtlPipelineState.h"
#include "GrMtlUtil.h"

#include "SkSLCompiler.h"

GrMtlResourceProvider::GrMtlResourceProvider(GrMtlGpu* gpu)
    : fGpu(gpu) {
    fPipelineStateCache.reset(new PipelineStateCache(gpu));
}

GrMtlCopyPipelineState* GrMtlResourceProvider::findOrCreateCopyPipelineState(
        MTLPixelFormat dstPixelFormat,
        id<MTLFunction> vertexFunction,
        id<MTLFunction> fragmentFunction,
        MTLVertexDescriptor* vertexDescriptor) {

    for (const auto& copyPipelineState: fCopyPipelineStateCache) {
        if (GrMtlCopyManager::IsCompatible(copyPipelineState.get(), dstPixelFormat)) {
            return copyPipelineState.get();
        }
    }

    fCopyPipelineStateCache.emplace_back(GrMtlCopyPipelineState::CreateCopyPipelineState(
             fGpu, dstPixelFormat, vertexFunction, fragmentFunction, vertexDescriptor));
    return fCopyPipelineStateCache.back().get();
}

GrMtlPipelineState* GrMtlResourceProvider::findOrCreateCompatiblePipelineState(
        GrRenderTarget* renderTarget, GrSurfaceOrigin origin,
        const GrPipeline& pipeline, const GrPrimitiveProcessor& proc,
        const GrTextureProxy* const primProcProxies[], GrPrimitiveType primType) {
    return fPipelineStateCache->refPipelineState(renderTarget, origin, proc, primProcProxies,
                                                 pipeline, primType);
}

////////////////////////////////////////////////////////////////////////////////////////////////

id<MTLDepthStencilState> GrMtlResourceProvider::getDisabledDepthStencilState() {
    if (nil == fDisabledDepthStencilState) {
        MTLDepthStencilDescriptor* desc = [[MTLDepthStencilDescriptor alloc] init];
        fDisabledDepthStencilState = [fGpu->device() newDepthStencilStateWithDescriptor:desc];
    }

    return fDisabledDepthStencilState;
}

MTLStencilOperation skia_stencil_op_to_mtl(GrStencilOp op) {
    switch (op) {
        case GrStencilOp::kKeep:
            return MTLStencilOperationKeep;
        case GrStencilOp::kZero:
            return MTLStencilOperationZero;
        case GrStencilOp::kReplace:
            return MTLStencilOperationReplace;
        case GrStencilOp::kInvert:
            return MTLStencilOperationInvert;
        case GrStencilOp::kIncWrap:
            return MTLStencilOperationIncrementWrap;
        case GrStencilOp::kDecWrap:
            return MTLStencilOperationDecrementWrap;
        case GrStencilOp::kIncClamp:
            return MTLStencilOperationIncrementClamp;
        case GrStencilOp::kDecClamp:
            return MTLStencilOperationDecrementClamp;
    }
}

MTLStencilDescriptor* skia_stencil_to_mtl(GrStencilSettings::Face face) {
    MTLStencilDescriptor* result = [[MTLStencilDescriptor alloc] init];
    switch (face.fTest) {
        case GrStencilTest::kAlways:
            result.stencilCompareFunction = MTLCompareFunctionAlways;
            break;
        case GrStencilTest::kNever:
            result.stencilCompareFunction = MTLCompareFunctionNever;
            break;
        case GrStencilTest::kGreater:
            result.stencilCompareFunction = MTLCompareFunctionGreater;
            break;
        case GrStencilTest::kGEqual:
            result.stencilCompareFunction = MTLCompareFunctionGreaterEqual;
            break;
        case GrStencilTest::kLess:
            result.stencilCompareFunction = MTLCompareFunctionLess;
            break;
        case GrStencilTest::kLEqual:
            result.stencilCompareFunction = MTLCompareFunctionLessEqual;
            break;
        case GrStencilTest::kEqual:
            result.stencilCompareFunction = MTLCompareFunctionEqual;
            break;
        case GrStencilTest::kNotEqual:
            result.stencilCompareFunction = MTLCompareFunctionNotEqual;
            break;
    }
    result.readMask = face.fTestMask;
    result.writeMask = face.fWriteMask;
    result.depthStencilPassOperation = skia_stencil_op_to_mtl(face.fPassOp);
    result.stencilFailureOperation = skia_stencil_op_to_mtl(face.fFailOp);
    return result;
}

id<MTLDepthStencilState> GrMtlResourceProvider::findOrCreateCompatibleDepthStencilState(
        const GrStencilSettings& stencil, const GrSurfaceOrigin& origin) {
    if (stencil.isDisabled()) {
        return this->getDisabledDepthStencilState();
    }

    // TODO: cache this
    MTLDepthStencilDescriptor* desc = [[MTLDepthStencilDescriptor alloc] init];
    if (!stencil.isDisabled()) {
        if (stencil.isTwoSided()) {
            desc.frontFaceStencil = skia_stencil_to_mtl(stencil.front(origin));
            desc.backFaceStencil = skia_stencil_to_mtl(stencil.back(origin));
        }
        else {
            desc.frontFaceStencil = skia_stencil_to_mtl(stencil.frontAndBack());
            desc.backFaceStencil = desc.frontFaceStencil;
        }
    }

    id<MTLDepthStencilState> state = [fGpu->device() newDepthStencilStateWithDescriptor:desc];
    return state;
}

// Finds or creates a compatible MTLSamplerState based on the GrSamplerState.
GrMtlSampler* GrMtlResourceProvider::findOrCreateCompatibleSampler(const GrSamplerState& params,
                                                                   uint32_t maxMipLevel) {
    GrMtlSampler* sampler;
    sampler = fSamplers.find(GrMtlSampler::GenerateKey(params, maxMipLevel));
    if (!sampler) {
        sampler = GrMtlSampler::Create(fGpu, params, maxMipLevel);
        fSamplers.add(sampler);
    }
    SkASSERT(sampler);
    return sampler;
}

////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_PIPELINE_STATE_CACHE_STATS
// Display pipeline state cache usage
static const bool c_DisplayMtlPipelineCache{false};
#endif

struct GrMtlResourceProvider::PipelineStateCache::Entry {
    Entry(GrMtlGpu* gpu, GrMtlPipelineState* pipelineState)
    : fGpu(gpu)
    , fPipelineState(pipelineState) {}

    GrMtlGpu* fGpu;
    std::unique_ptr<GrMtlPipelineState> fPipelineState;
};

GrMtlResourceProvider::PipelineStateCache::PipelineStateCache(GrMtlGpu* gpu)
    : fMap(kMaxEntries)
    , fGpu(gpu)
#ifdef GR_PIPELINE_STATE_CACHE_STATS
    , fTotalRequests(0)
    , fCacheMisses(0)
#endif
{}

GrMtlResourceProvider::PipelineStateCache::~PipelineStateCache() {
    // TODO: determine if we need abandon/release methods
    fMap.reset();
    // dump stats
#ifdef GR_PIPELINE_STATE_CACHE_STATS
    if (c_DisplayMtlPipelineCache) {
        SkDebugf("--- Pipeline State Cache ---\n");
        SkDebugf("Total requests: %d\n", fTotalRequests);
        SkDebugf("Cache misses: %d\n", fCacheMisses);
        SkDebugf("Cache miss %%: %f\n", (fTotalRequests > 0) ?
                 100.f * fCacheMisses / fTotalRequests :
                 0.f);
        SkDebugf("---------------------\n");
    }
#endif
}

GrMtlPipelineState* GrMtlResourceProvider::PipelineStateCache::refPipelineState(
        GrRenderTarget* renderTarget,
        GrSurfaceOrigin origin,
        const GrPrimitiveProcessor& primProc,
        const GrTextureProxy* const primProcProxies[],
        const GrPipeline& pipeline,
        GrPrimitiveType primType) {
#ifdef GR_PIPELINE_STATE_CACHE_STATS
    ++fTotalRequests;
#endif
    // Get GrMtlProgramDesc
    GrMtlPipelineStateBuilder::Desc desc;
    if (!GrMtlPipelineStateBuilder::Desc::Build(&desc, renderTarget, primProc, pipeline, primType,
                                                fGpu)) {
        GrCapsDebugf(fGpu->caps(), "Failed to build mtl program descriptor!\n");
        return nullptr;
    }

    std::unique_ptr<Entry>* entry = fMap.find(desc);
    if (!entry) {
        // Didn't find an origin-independent version, check with the specific origin
        desc.setSurfaceOriginKey(GrGLSLFragmentShaderBuilder::KeyForSurfaceOrigin(origin));
        entry = fMap.find(desc);
    }
    if (!entry) {
#ifdef GR_PIPELINE_STATE_CACHE_STATS
        ++fCacheMisses;
#endif
        GrMtlPipelineState* pipelineState(GrMtlPipelineStateBuilder::CreatePipelineState(
                fGpu, renderTarget, origin, primProc, primProcProxies, pipeline, &desc));
        if (nullptr == pipelineState) {
            return nullptr;
        }
        entry = fMap.insert(desc, std::unique_ptr<Entry>(new Entry(fGpu, pipelineState)));
        return (*entry)->fPipelineState.get();
    }
    return (*entry)->fPipelineState.get();
}
