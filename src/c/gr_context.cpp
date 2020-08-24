/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/c/gr_context.h"

#include "src/c/sk_types_priv.h"

// GrContext

gr_context_t* gr_context_make_gl(const gr_glinterface_t* glInterface) {
    return SK_ONLY_GPU(ToGrContext(GrContext::MakeGL(sk_ref_sp(AsGrGLInterface(glInterface))).release()), nullptr);
}

gr_context_t* gr_context_make_vulkan(const gr_vk_backendcontext_t vkBackendContext) {
    return SK_ONLY_VULKAN(ToGrContext(GrContext::MakeVulkan(AsGrVkBackendContext(&vkBackendContext)).release()), nullptr);
}

void gr_context_unref(gr_context_t* context) {
    SK_ONLY_GPU(SkSafeUnref(AsGrContext(context)));
}

void gr_context_abandon_context(gr_context_t* context) {
    SK_ONLY_GPU(AsGrContext(context)->abandonContext());
}

void gr_context_release_resources_and_abandon_context(gr_context_t* context) {
    SK_ONLY_GPU(AsGrContext(context)->releaseResourcesAndAbandonContext());
}

size_t gr_context_get_resource_cache_limit(gr_context_t* context) {
    return SK_ONLY_GPU(AsGrContext(context)->getResourceCacheLimit(), 0);
}

void gr_context_set_resource_cache_limit(gr_context_t* context, size_t maxResourceBytes) {
    SK_ONLY_GPU(AsGrContext(context)->setResourceCacheLimit(maxResourceBytes));
}

void gr_context_get_resource_cache_usage(gr_context_t* context, int* maxResources, size_t* maxResourceBytes) {
    SK_ONLY_GPU(AsGrContext(context)->getResourceCacheUsage(maxResources, maxResourceBytes));
}

int gr_context_get_max_surface_sample_count_for_color_type(gr_context_t* context, sk_colortype_t colorType) {
    return SK_ONLY_GPU(AsGrContext(context)->maxSurfaceSampleCountForColorType((SkColorType)colorType), 0);
}

void gr_context_flush(gr_context_t* context) {
    SK_ONLY_GPU(AsGrContext(context)->flush());
}

void gr_context_reset_context(gr_context_t* context, uint32_t state) {
    SK_ONLY_GPU(AsGrContext(context)->resetContext(state));
}

gr_backend_t gr_context_get_backend(gr_context_t* context) {
    return SK_ONLY_GPU((gr_backend_t)AsGrContext(context)->backend(), (gr_backend_t)0);
}

void gr_context_dump_memory_statistics(const gr_context_t* context, sk_tracememorydump_t* dump) {
    SK_ONLY_GPU(AsGrContext(context)->dumpMemoryStatistics(AsTraceMemoryDump(dump)));
}

void gr_context_free_gpu_resources(gr_context_t* context) {
    SK_ONLY_GPU(AsGrContext(context)->freeGpuResources());
}

void gr_context_perform_deferred_cleanup(gr_context_t* context, long long ms) {
    SK_ONLY_GPU(AsGrContext(context)->performDeferredCleanup(std::chrono::milliseconds(ms)));
}

void gr_context_purge_unlocked_resources_bytes(gr_context_t* context, size_t bytesToPurge, bool preferScratchResources) {
    SK_ONLY_GPU(AsGrContext(context)->purgeUnlockedResources(bytesToPurge, preferScratchResources));
}

void gr_context_purge_unlocked_resources(gr_context_t* context, bool scratchResourcesOnly) {
    SK_ONLY_GPU(AsGrContext(context)->purgeUnlockedResources(scratchResourcesOnly));
}


// GrGLInterface

const gr_glinterface_t* gr_glinterface_create_native_interface(void) {
    return SK_ONLY_GPU(ToGrGLInterface(GrGLMakeNativeInterface().release()), nullptr);
}

const gr_glinterface_t* gr_glinterface_assemble_interface(void* ctx, gr_gl_get_proc get) {
    return SK_ONLY_GPU(ToGrGLInterface(GrGLMakeAssembledInterface(ctx, get).release()), nullptr);
}

const gr_glinterface_t* gr_glinterface_assemble_gl_interface(void* ctx, gr_gl_get_proc get) {
    return SK_ONLY_GPU(ToGrGLInterface(GrGLMakeAssembledGLInterface(ctx, get).release()), nullptr);
}

const gr_glinterface_t* gr_glinterface_assemble_gles_interface(void* ctx, gr_gl_get_proc get) {
    return SK_ONLY_GPU(ToGrGLInterface(GrGLMakeAssembledGLESInterface(ctx, get).release()), nullptr);
}

const gr_glinterface_t* gr_glinterface_assemble_webgl_interface(void* ctx, gr_gl_get_proc get) {
    return SK_ONLY_GPU(ToGrGLInterface(GrGLMakeAssembledWebGLInterface(ctx, get).release()), nullptr);
}

void gr_glinterface_unref(const gr_glinterface_t* glInterface) {
    SK_ONLY_GPU(SkSafeUnref(AsGrGLInterface(glInterface)));
}

bool gr_glinterface_validate(const gr_glinterface_t* glInterface) {
    return SK_ONLY_GPU(AsGrGLInterface(glInterface)->validate(), false);
}

bool gr_glinterface_has_extension(const gr_glinterface_t* glInterface, const char* extension) {
    return SK_ONLY_GPU(AsGrGLInterface(glInterface)->hasExtension(extension), false);
}

// GrVkExtensions

gr_vk_extensions_t* gr_vk_extensions_new(void) {
    return SK_ONLY_VULKAN(ToGrVkExtensions(new GrVkExtensions()), nullptr);
}

void gr_vk_extensions_delete(gr_vk_extensions_t* extensions) {
    SK_ONLY_VULKAN(delete AsGrVkExtensions(extensions));
}

void gr_vk_extensions_init(gr_vk_extensions_t* extensions, gr_vk_get_proc getProc, void* userData, vk_instance_t* instance, vk_physical_device_t* physDev, uint32_t instanceExtensionCount, const char** instanceExtensions, uint32_t deviceExtensionCount, const char** deviceExtensions) {
    SK_ONLY_VULKAN(AsGrVkExtensions(extensions)->init(
        [userData, getProc](const char* name, VkInstance instance, VkDevice device) -> PFN_vkVoidFunction {
            return getProc(userData, name, ToVkInstance(instance), ToVkDevice(device));
        },
        AsVkInstance(instance),
        AsVkPhysicalDevice(physDev),
        instanceExtensionCount, instanceExtensions,
        deviceExtensionCount, deviceExtensions));
}

bool gr_vk_extensions_has_extension(gr_vk_extensions_t* extensions, const char* ext, uint32_t minVersion) {
    return SK_ONLY_VULKAN(AsGrVkExtensions(extensions)->hasExtension(ext, minVersion), false);
}

// GrBackendTexture

gr_backendtexture_t* gr_backendtexture_new_gl(int width, int height, bool mipmapped, const gr_gl_textureinfo_t* glInfo) {
    return SK_ONLY_GPU(ToGrBackendTexture(new GrBackendTexture(width, height, (GrMipMapped)mipmapped, *AsGrGLTextureInfo(glInfo))), nullptr);
}

gr_backendtexture_t* gr_backendtexture_new_vulkan(int width, int height, const gr_vk_imageinfo_t* vkInfo) {
    return SK_ONLY_VULKAN(ToGrBackendTexture(new GrBackendTexture(width, height, *AsGrVkImageInfo(vkInfo))), nullptr);
}

void gr_backendtexture_delete(gr_backendtexture_t* texture) {
    SK_ONLY_GPU(delete AsGrBackendTexture(texture));
}

bool gr_backendtexture_is_valid(const gr_backendtexture_t* texture) {
    return SK_ONLY_GPU(AsGrBackendTexture(texture)->isValid(), false);
}

int gr_backendtexture_get_width(const gr_backendtexture_t* texture) {
    return SK_ONLY_GPU(AsGrBackendTexture(texture)->width(), 0);
}

int gr_backendtexture_get_height(const gr_backendtexture_t* texture) {
    return SK_ONLY_GPU(AsGrBackendTexture(texture)->height(), 0);
}

bool gr_backendtexture_has_mipmaps(const gr_backendtexture_t* texture) {
    return SK_ONLY_GPU(AsGrBackendTexture(texture)->hasMipMaps(), false);
}

gr_backend_t gr_backendtexture_get_backend(const gr_backendtexture_t* texture) {
    return SK_ONLY_GPU((gr_backend_t)AsGrBackendTexture(texture)->backend(), (gr_backend_t)0);
}

bool gr_backendtexture_get_gl_textureinfo(const gr_backendtexture_t* texture, gr_gl_textureinfo_t* glInfo) {
    return SK_ONLY_GPU(AsGrBackendTexture(texture)->getGLTextureInfo(AsGrGLTextureInfo(glInfo)), false);
}


// GrBackendRenderTarget

gr_backendrendertarget_t* gr_backendrendertarget_new_gl(int width, int height, int samples, int stencils, const gr_gl_framebufferinfo_t* glInfo) {
    return SK_ONLY_GPU(ToGrBackendRenderTarget(new GrBackendRenderTarget(width, height, samples, stencils, *AsGrGLFramebufferInfo(glInfo))), nullptr);
}

gr_backendrendertarget_t* gr_backendrendertarget_new_vulkan(int width, int height, int samples, const gr_vk_imageinfo_t* vkImageInfo) {
    return SK_ONLY_VULKAN(ToGrBackendRenderTarget(new GrBackendRenderTarget(width, height, samples, *AsGrVkImageInfo(vkImageInfo))), nullptr);
}

void gr_backendrendertarget_delete(gr_backendrendertarget_t* rendertarget) {
    SK_ONLY_GPU(delete AsGrBackendRenderTarget(rendertarget));
}

bool gr_backendrendertarget_is_valid(const gr_backendrendertarget_t* rendertarget) {
    return SK_ONLY_GPU(AsGrBackendRenderTarget(rendertarget)->isValid(), false);
}

int gr_backendrendertarget_get_width(const gr_backendrendertarget_t* rendertarget) {
    return SK_ONLY_GPU(AsGrBackendRenderTarget(rendertarget)->width(), 0);
}

int gr_backendrendertarget_get_height(const gr_backendrendertarget_t* rendertarget) {
    return SK_ONLY_GPU(AsGrBackendRenderTarget(rendertarget)->height(), 0);
}

int gr_backendrendertarget_get_samples(const gr_backendrendertarget_t* rendertarget) {
    return SK_ONLY_GPU(AsGrBackendRenderTarget(rendertarget)->sampleCnt(), 0);
}

int gr_backendrendertarget_get_stencils(const gr_backendrendertarget_t* rendertarget) {
    return SK_ONLY_GPU(AsGrBackendRenderTarget(rendertarget)->stencilBits(), 0);
}

gr_backend_t gr_backendrendertarget_get_backend(const gr_backendrendertarget_t* rendertarget) {
    return SK_ONLY_GPU((gr_backend_t)AsGrBackendRenderTarget(rendertarget)->backend(), (gr_backend_t)0);
}

bool gr_backendrendertarget_get_gl_framebufferinfo(const gr_backendrendertarget_t* rendertarget, gr_gl_framebufferinfo_t* glInfo) {
    return SK_ONLY_GPU(AsGrBackendRenderTarget(rendertarget)->getGLFramebufferInfo(AsGrGLFramebufferInfo(glInfo)), false);
}
