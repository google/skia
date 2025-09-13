/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/vk/GrVkDirectContext.h"
#include "include/gpu/vk/VulkanBackendContext.h"
#include "include/gpu/vk/VulkanExtensions.h"
#include "include/gpu/vk/VulkanMemoryAllocator.h"

// These are private files. Clients would need to look at these and implement
// similar solutions.
#include "src/gpu/vk/vulkanmemoryallocator/VulkanMemoryAllocatorPriv.h"
#include "src/gpu/GpuTypesPriv.h"
#include "tools/gpu/vk/VkTestUtils.h"

#include <string.h>
#include <vulkan/vulkan_core.h>
#include <functional>
#include <memory>

#define ACQUIRE_INST_VK_PROC(name)                                                           \
    do {                                                                                     \
    fVk##name = reinterpret_cast<PFN_vk##name>(getProc("vk" #name, backendContext.fInstance, \
                                                       VK_NULL_HANDLE));                     \
    if (fVk##name == nullptr) {                                                              \
        SkDebugf("Function ptr for vk%s could not be acquired\n", #name);                    \
        return 1;                                                                            \
    }                                                                                        \
    } while(false)

int main(int argc, char** argv) {
    skgpu::VulkanBackendContext backendContext;
    VkDebugUtilsMessengerEXT debugMessenger;
    std::unique_ptr<skgpu::VulkanExtensions> extensions(new skgpu::VulkanExtensions());
    std::unique_ptr<sk_gpu_test::TestVkFeatures> features(new sk_gpu_test::TestVkFeatures);

    // First we need to create a VulkanBackendContext so that we can make a Vulkan GrDirectContext.
    // The vast majority of this chunk of code is setting up the VkInstance and VkDevice objects.
    // Normally a client will have their own way of creating these objects. This example uses Skia's
    // test helper sk_gpu_test::CreateVkBackendContext to aid in this. Clients can look at this
    // function as a guide on things to consider when setting up Vulkan for themselves, but they
    // should not depend on that function. We may arbitrarily change it as it is meant only for Skia
    // internal testing. Additionally it may do some odd things that a normal Vulkan user wouldn't
    // do because it is only meant for Skia testing.
    //
    // When creating a device on your own, make sure to use skgpu::VulkanPreferredFeatures to let
    // Skia add features and extensions it would like to take advantage of. Performance may suffer
    // otherwise, or some functionality may not be accessible.
    {
        PFN_vkGetInstanceProcAddr instProc;
        if (!sk_gpu_test::LoadVkLibraryAndGetProcAddrFuncs(&instProc)) {
            return 1;
        }

        backendContext.fInstance = VK_NULL_HANDLE;
        backendContext.fDevice = VK_NULL_HANDLE;

        if (!sk_gpu_test::CreateVkBackendContext(
                    instProc, &backendContext, extensions.get(), features.get(), &debugMessenger)) {
            return 1;
        }
    }

    auto getProc = backendContext.fGetProc;
    PFN_vkDestroyInstance fVkDestroyInstance;
    PFN_vkDestroyDebugUtilsMessengerEXT fVkDestroyDebugUtilsMessengerEXT = nullptr;
    PFN_vkDestroyDevice fVkDestroyDevice;
    ACQUIRE_INST_VK_PROC(DestroyInstance);
    if (debugMessenger != VK_NULL_HANDLE) {
        ACQUIRE_INST_VK_PROC(DestroyDebugUtilsMessengerEXT);
    }
    ACQUIRE_INST_VK_PROC(DestroyDevice);

    backendContext.fMemoryAllocator = skgpu::VulkanMemoryAllocators::Make(
            backendContext, skgpu::ThreadSafe::kNo, std::nullopt);

    // Create a GrDirectContext with our VulkanBackendContext
    sk_sp<GrDirectContext> context = GrDirectContexts::MakeVulkan(backendContext);
    if (!context) {
        fVkDestroyDevice(backendContext.fDevice, nullptr);
        if (debugMessenger != VK_NULL_HANDLE) {
            fVkDestroyDebugUtilsMessengerEXT(backendContext.fInstance, debugMessenger, nullptr);
        }
        fVkDestroyInstance(backendContext.fInstance, nullptr);
        return 1;
    }

    SkImageInfo imageInfo = SkImageInfo::Make(16, 16, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    // Create an SkSurface backed by a Vulkan VkImage. Often clients will be getting VkImages from
    // swapchains. In those cases they should use SkSurfaces::WrapBackendTexture or
    // SkSurfaces::WrapBackendRenderTarget to wrap those premade VkImages in Skia. See the
    // HelloWorld example app to see how this is done.
    sk_sp<SkSurface> surface =
            SkSurfaces::RenderTarget(context.get(), skgpu::Budgeted::kYes, imageInfo);
    if (!surface) {
        context.reset();
        fVkDestroyDevice(backendContext.fDevice, nullptr);
        if (debugMessenger != VK_NULL_HANDLE) {
            fVkDestroyDebugUtilsMessengerEXT(backendContext.fInstance, debugMessenger, nullptr);
        }
        fVkDestroyInstance(backendContext.fInstance, nullptr);
        return 1;
    }

    surface->getCanvas()->clear(SK_ColorRED);

    // After drawing to our surface, we must first flush the recorded work (i.e. convert all our
    // recorded SkCanvas calls into a VkCommandBuffer). Then we call submit to submit our
    // VkCommandBuffers to the gpu queue.
    context->flush(surface.get());
    context->submit();

    surface.reset();
    context.reset();

    // Skia doesn't own the VkDevice or VkInstance so the client must manage their lifetime. The
    // client must not delete these objects until cleaning up all Skia objects that may have used
    // them first.
    fVkDestroyDevice(backendContext.fDevice, nullptr);
    if (debugMessenger != VK_NULL_HANDLE) {
        fVkDestroyDebugUtilsMessengerEXT(backendContext.fInstance, debugMessenger, nullptr);
    }
    fVkDestroyInstance(backendContext.fInstance, nullptr);
    return 0;
}
