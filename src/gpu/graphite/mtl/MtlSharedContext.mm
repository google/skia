/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/mtl/MtlSharedContext.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/mtl/MtlCommandBuffer.h"
#include "src/gpu/graphite/mtl/MtlResourceProvider.h"
#include "src/gpu/graphite/mtl/MtlTexture.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"
#include "src/gpu/mtl/MtlMemoryAllocatorImpl.h"

namespace skgpu::graphite {

sk_sp<SharedContext> MtlSharedContext::Make(const MtlBackendContext& context,
                                            const ContextOptions& options) {
    if (@available(macOS 10.15, iOS 13.0, tvOS 13.0, *)) {
        // no warning needed
    } else {
        SKGPU_LOG_E("Skia's Graphite backend no longer supports this OS version.");
#ifdef SK_BUILD_FOR_IOS
        SKGPU_LOG_E("Minimum supported version is iOS/tvOS 13.0.");
#else
        SKGPU_LOG_E("Minimum supported version is MacOS 10.15.");
#endif
        return nullptr;
    }

    sk_cfp<id<MTLDevice>> device = sk_ret_cfp((id<MTLDevice>)(context.fDevice.get()));

    std::unique_ptr<const MtlCaps> caps(new MtlCaps(device.get(), options));

    // TODO: Add memory allocator to context once we figure out synchronization
    sk_sp<MtlMemoryAllocator> memoryAllocator = skgpu::MtlMemoryAllocatorImpl::Make(device.get());
    if (!memoryAllocator) {
        SkDEBUGFAIL("No supplied Metal memory allocator and unable to create one internally.");
        return nullptr;
    }

    return sk_sp<SharedContext>(new MtlSharedContext(std::move(device),
                                                     std::move(memoryAllocator),
                                                     std::move(caps),
                                                     options.fExecutor,
                                                     options.fUserDefinedKnownRuntimeEffects));
}

MtlSharedContext::MtlSharedContext(sk_cfp<id<MTLDevice>> device,
                                   sk_sp<skgpu::MtlMemoryAllocator> memoryAllocator,
                                   std::unique_ptr<const MtlCaps> caps,
                                   SkExecutor* executor,
                                   SkSpan<sk_sp<SkRuntimeEffect>> userDefinedKnownRuntimeEffects)
        : SharedContext(std::move(caps),
                        BackendApi::kMetal,
                        executor,
                        userDefinedKnownRuntimeEffects)
        , fMemoryAllocator(std::move(memoryAllocator))
        , fDevice(std::move(device)) {
    fThreadSafeResourceProvider = std::make_unique<MtlThreadSafeResourceProvider>(
        this->makeResourceProvider(&fSingleOwner,
                                   SK_InvalidGenID,
                                   kThreadedSafeResourceBudget));

    static constexpr DepthStencilSettings kIgnoreDSS;

    for (const DepthStencilSettings& dss : { kDirectDepthLessPass,
                                             kDirectDepthLEqualPass,
                                             kWindingStencilPass,
                                             kEvenOddStencilPass,
                                             kRegularCoverPass,
                                             kInverseCoverPass,
                                             kIgnoreDSS }) {
        this->createCompatibleDepthStencilState(dss);
    }
}

MtlSharedContext::~MtlSharedContext() {
    fThreadSafeResourceProvider.reset();

    // need to clear out resources before the allocator (if any) is removed
    this->globalCache()->deleteResources();
}

MtlThreadSafeResourceProvider* MtlSharedContext::threadSafeResourceProvider() const {
    return static_cast<MtlThreadSafeResourceProvider*>(fThreadSafeResourceProvider.get());
}

std::unique_ptr<ResourceProvider> MtlSharedContext::makeResourceProvider(
        SingleOwner* singleOwner,
        uint32_t recorderID,
        size_t resourceBudget) {
    return std::unique_ptr<ResourceProvider>(new MtlResourceProvider(this,
                                                                     singleOwner,
                                                                     recorderID,
                                                                     resourceBudget));
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

sk_cfp<id<MTLDepthStencilState>> MtlSharedContext::getCompatibleDepthStencilState(
            const DepthStencilSettings& depthStencilSettings) const {

    sk_cfp<id<MTLDepthStencilState>>* depthStencilState;
    depthStencilState = fDepthStencilStates.find(depthStencilSettings);

    // We've explicitly initialized fDepthStencilStates with all the common depth stencil settings
    // in the ctor - since there are so few of them. This frees us from concurrency concerns (i.e.,
    // if we were to lazily create them and store them in a map). However, if a new one is
    // encountered we will need to either add its initialization to the ctor or reconsider this
    // approach.
    SkAssertResult(depthStencilState);
    return *depthStencilState;
}

void MtlSharedContext::createCompatibleDepthStencilState(
                const DepthStencilSettings& depthStencilSettings) {

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
            [this->device() newDepthStencilStateWithDescriptor: desc]);

    fDepthStencilStates.set(depthStencilSettings, std::move(dss));
}

sk_sp<GraphicsPipeline> MtlSharedContext::createGraphicsPipeline(
        const RuntimeEffectDictionary* runtimeDict,
        const UniqueKey& pipelineKey,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc,
        SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags,
        uint32_t compilationID) {
    return MtlGraphicsPipeline::Make(this,
                                     runtimeDict, pipelineKey, pipelineDesc, renderPassDesc,
                                     pipelineCreationFlags, compilationID);
}

} // namespace skgpu::graphite
