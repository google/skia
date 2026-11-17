/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/gpu/graphite/vk/VulkanGraphiteContext.h"
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "include/gpu/vk/VulkanBackendContext.h"
#include "include/gpu/vk/VulkanMemoryAllocator.h"
#include "include/gpu/vk/VulkanTypes.h"
#include "include/private/gpu/vk/SkiaVulkan.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/buffer/Buffer.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/gpu/ContextType.h"
#include "tools/graphite/GraphiteTestContext.h"
#include "tools/graphite/vk/GraphiteVulkanTestContext.h"

#include <atomic>
#include <cstdint>
#include <memory>
#include <utility>

using namespace skgpu::graphite;

namespace {

class OOMTestMemoryAllocator final : public skgpu::VulkanMemoryAllocator {
public:
    explicit OOMTestMemoryAllocator(sk_sp<skgpu::VulkanMemoryAllocator> wrapped)
            : fWrapped(std::move(wrapped)) {}

    void setFailResult(VkResult result) { fFailResult.store(result, std::memory_order_relaxed); }

    VkResult allocateImageMemory(VkImage image,
                                 uint32_t allocationPropertyFlags,
                                 skgpu::VulkanBackendMemory* memory) override {
        VkResult failResult = fFailResult.load(std::memory_order_relaxed);
        if (failResult != VK_SUCCESS) {
            return failResult;
        }
        return fWrapped->allocateImageMemory(image, allocationPropertyFlags, memory);
    }

    VkResult allocateBufferMemory(VkBuffer buffer,
                                  BufferUsage usage,
                                  uint32_t allocationPropertyFlags,
                                  skgpu::VulkanBackendMemory* memory) override {
        VkResult failResult = fFailResult.load(std::memory_order_relaxed);
        if (failResult != VK_SUCCESS) {
            return failResult;
        }
        return fWrapped->allocateBufferMemory(buffer, usage, allocationPropertyFlags, memory);
    }

    void getAllocInfo(const skgpu::VulkanBackendMemory& memory,
                      skgpu::VulkanAlloc* alloc) const override {
        fWrapped->getAllocInfo(memory, alloc);
    }

    VkResult mapMemory(const skgpu::VulkanBackendMemory& memory, void** data) override {
        return fWrapped->mapMemory(memory, data);
    }

    void unmapMemory(const skgpu::VulkanBackendMemory& memory) override {
        fWrapped->unmapMemory(memory);
    }

    VkResult flushMemory(const skgpu::VulkanBackendMemory& memory,
                         VkDeviceSize offset,
                         VkDeviceSize size) override {
        return fWrapped->flushMemory(memory, offset, size);
    }

    VkResult invalidateMemory(const skgpu::VulkanBackendMemory& memory,
                              VkDeviceSize offset,
                              VkDeviceSize size) override {
        return fWrapped->invalidateMemory(memory, offset, size);
    }

    void freeMemory(const skgpu::VulkanBackendMemory& memory) override {
        fWrapped->freeMemory(memory);
    }

    std::pair<uint64_t, uint64_t> totalAllocatedAndUsedMemory() const override {
        return fWrapped->totalAllocatedAndUsedMemory();
    }

private:
    sk_sp<skgpu::VulkanMemoryAllocator> fWrapped;
    std::atomic<VkResult> fFailResult{VK_SUCCESS};
};

}  // namespace

DEF_GRAPHITE_TEST_FOR_CONTEXTS(VulkanOOMTest,
                               skiatest::IsVulkanContextType,
                               reporter,
                               /*context*/,
                               testContext,
                               CtsEnforcement::kNever) {
    auto* vkTestContext = static_cast<skiatest::graphite::VulkanTestContext*>(testContext);
    skgpu::VulkanBackendContext backendContext = vkTestContext->getBackendContext();
    auto testAllocator = sk_make_sp<OOMTestMemoryAllocator>(backendContext.fMemoryAllocator);
    backendContext.fMemoryAllocator = testAllocator;

    std::unique_ptr<Context> oomContext =
            ContextFactory::MakeVulkan(backendContext, ContextOptions{});
    REPORTER_ASSERT(reporter, oomContext);
    if (!oomContext) {
        return;
    }

    REPORTER_ASSERT(reporter, !oomContext->oomed());

    auto recorder = oomContext->makeRecorder();
    REPORTER_ASSERT(reporter, recorder);

    const bool isProtected = oomContext->priv().caps()->protectedSupport();
    VulkanTextureInfo vkTexInfo;
    vkTexInfo.fSampleCount = SampleCount::k1;
    vkTexInfo.fMipmapped = skgpu::Mipmapped::kNo;
    vkTexInfo.fFlags = isProtected ? VK_IMAGE_CREATE_PROTECTED_BIT : 0;
    vkTexInfo.fFormat = VK_FORMAT_R8G8B8A8_UNORM;
    vkTexInfo.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    vkTexInfo.fImageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    vkTexInfo.fSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    const TextureInfo texInfo = TextureInfos::MakeVulkan(vkTexInfo);
    constexpr SkISize kSize = {16, 16};

    {
        skiatest::ReporterContext subtest(reporter,
                                          SkString("Texture VK_ERROR_OUT_OF_DEVICE_MEMORY"));
        testAllocator->setFailResult(VK_ERROR_OUT_OF_DEVICE_MEMORY);

        BackendTexture beTex = recorder->createBackendTexture(kSize, texInfo);
        REPORTER_ASSERT(reporter, !beTex.isValid());
        REPORTER_ASSERT(reporter, oomContext->oomed());
        REPORTER_ASSERT(
                reporter, !oomContext->oomed(), "oomed() should reset after returning true");
    }

    {
        skiatest::ReporterContext subtest(reporter,
                                          SkString("Buffer VK_ERROR_OUT_OF_HOST_MEMORY"));
        testAllocator->setFailResult(VK_ERROR_OUT_OF_HOST_MEMORY);

        sk_sp<Buffer> buffer =
                oomContext->priv().resourceProvider()->findOrCreateNonShareableBuffer(
                        /*size=*/256,
                        BufferType::kVertex,
                        AccessPattern::kGpuOnly,
                        "OOMTestBuffer");
        REPORTER_ASSERT(reporter, !buffer);
        REPORTER_ASSERT(reporter, oomContext->oomed());
    }

    {
        skiatest::ReporterContext subtest(reporter,
                                          SkString("Non-OOM error does not set oomed()"));
        testAllocator->setFailResult(VK_ERROR_INITIALIZATION_FAILED);

        BackendTexture beTex = recorder->createBackendTexture(kSize, texInfo);
        REPORTER_ASSERT(reporter, !beTex.isValid());
        REPORTER_ASSERT(reporter, !oomContext->oomed());
    }

    {
        skiatest::ReporterContext subtest(reporter,
                                          SkString("Subsequent allocations succeed cleanly"));
        testAllocator->setFailResult(VK_SUCCESS);

        BackendTexture beTex = recorder->createBackendTexture(kSize, texInfo);
        REPORTER_ASSERT(reporter, beTex.isValid());
        REPORTER_ASSERT(reporter, !oomContext->oomed());
        oomContext->deleteBackendTexture(beTex);
    }
}
