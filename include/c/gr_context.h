/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef gr_context_DEFINED
#define gr_context_DEFINED

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API void gr_recording_context_unref(gr_recording_context_t* context);
SK_C_API int gr_recording_context_get_max_surface_sample_count_for_color_type(gr_recording_context_t* context, sk_colortype_t colorType);
SK_C_API gr_backend_t gr_recording_context_get_backend(gr_recording_context_t* context);

// GrDirectContext

SK_C_API gr_direct_context_t* gr_direct_context_make_gl(const gr_glinterface_t* glInterface);
SK_C_API gr_direct_context_t* gr_direct_context_make_gl_with_options(const gr_glinterface_t* glInterface, const gr_context_options_t* options);
SK_C_API gr_direct_context_t* gr_direct_context_make_vulkan(const gr_vk_backendcontext_t vkBackendContext);
SK_C_API gr_direct_context_t* gr_direct_context_make_vulkan_with_options(const gr_vk_backendcontext_t vkBackendContext, const gr_context_options_t* options);
SK_C_API gr_direct_context_t* gr_direct_context_make_metal(void* device, void* queue);
SK_C_API gr_direct_context_t* gr_direct_context_make_metal_with_options(void* device, void* queue, const gr_context_options_t* options);

// TODO: the overloads with GrContextOptions

SK_C_API bool gr_direct_context_is_abandoned(gr_direct_context_t* context);
SK_C_API void gr_direct_context_abandon_context(gr_direct_context_t* context);
SK_C_API void gr_direct_context_release_resources_and_abandon_context(gr_direct_context_t* context);
SK_C_API size_t gr_direct_context_get_resource_cache_limit(gr_direct_context_t* context);
SK_C_API void gr_direct_context_set_resource_cache_limit(gr_direct_context_t* context, size_t maxResourceBytes);
SK_C_API void gr_direct_context_get_resource_cache_usage(gr_direct_context_t* context, int* maxResources, size_t* maxResourceBytes);
SK_C_API void gr_direct_context_flush(gr_direct_context_t* context);
SK_C_API bool gr_direct_context_submit(gr_direct_context_t* context, bool syncCpu);
SK_C_API void gr_direct_context_flush_and_submit(gr_direct_context_t* context, bool syncCpu);
SK_C_API void gr_direct_context_reset_context(gr_direct_context_t* context, uint32_t state);
SK_C_API void gr_direct_context_dump_memory_statistics(const gr_direct_context_t* context, sk_tracememorydump_t* dump);
SK_C_API void gr_direct_context_free_gpu_resources(gr_direct_context_t* context);
SK_C_API void gr_direct_context_perform_deferred_cleanup(gr_direct_context_t* context, long long ms);
SK_C_API void gr_direct_context_purge_unlocked_resources_bytes(gr_direct_context_t* context, size_t bytesToPurge, bool preferScratchResources);
SK_C_API void gr_direct_context_purge_unlocked_resources(gr_direct_context_t* context, bool scratchResourcesOnly);


// GrGLInterface

SK_C_API const gr_glinterface_t* gr_glinterface_create_native_interface(void);
SK_C_API const gr_glinterface_t* gr_glinterface_assemble_interface(void* ctx, gr_gl_get_proc get);
SK_C_API const gr_glinterface_t* gr_glinterface_assemble_gl_interface(void* ctx, gr_gl_get_proc get);
SK_C_API const gr_glinterface_t* gr_glinterface_assemble_gles_interface(void* ctx, gr_gl_get_proc get);
SK_C_API const gr_glinterface_t* gr_glinterface_assemble_webgl_interface(void* ctx, gr_gl_get_proc get);

SK_C_API void gr_glinterface_unref(const gr_glinterface_t* glInterface);
SK_C_API bool gr_glinterface_validate(const gr_glinterface_t* glInterface);
SK_C_API bool gr_glinterface_has_extension(const gr_glinterface_t* glInterface, const char* extension);

// GrVkExtensions

SK_C_API gr_vk_extensions_t* gr_vk_extensions_new(void);
SK_C_API void gr_vk_extensions_delete(gr_vk_extensions_t* extensions);
SK_C_API void gr_vk_extensions_init(gr_vk_extensions_t* extensions, gr_vk_get_proc getProc, void* userData, vk_instance_t* instance, vk_physical_device_t* physDev, uint32_t instanceExtensionCount, const char** instanceExtensions, uint32_t deviceExtensionCount, const char** deviceExtensions);
SK_C_API bool gr_vk_extensions_has_extension(gr_vk_extensions_t* extensions, const char* ext, uint32_t minVersion);

// GrBackendTexture

SK_C_API gr_backendtexture_t* gr_backendtexture_new_gl(int width, int height, bool mipmapped, const gr_gl_textureinfo_t* glInfo);
SK_C_API gr_backendtexture_t* gr_backendtexture_new_vulkan(int width, int height, const gr_vk_imageinfo_t* vkInfo);
SK_C_API gr_backendtexture_t* gr_backendtexture_new_metal(int width, int height, bool mipmapped, const gr_mtl_textureinfo_t* mtlInfo);
SK_C_API void gr_backendtexture_delete(gr_backendtexture_t* texture);

SK_C_API bool gr_backendtexture_is_valid(const gr_backendtexture_t* texture);
SK_C_API int gr_backendtexture_get_width(const gr_backendtexture_t* texture);
SK_C_API int gr_backendtexture_get_height(const gr_backendtexture_t* texture);
SK_C_API bool gr_backendtexture_has_mipmaps(const gr_backendtexture_t* texture);
SK_C_API gr_backend_t gr_backendtexture_get_backend(const gr_backendtexture_t* texture);
SK_C_API bool gr_backendtexture_get_gl_textureinfo(const gr_backendtexture_t* texture, gr_gl_textureinfo_t* glInfo);


// GrBackendRenderTarget

SK_C_API gr_backendrendertarget_t* gr_backendrendertarget_new_gl(int width, int height, int samples, int stencils, const gr_gl_framebufferinfo_t* glInfo);
SK_C_API gr_backendrendertarget_t* gr_backendrendertarget_new_vulkan(int width, int height, int samples, const gr_vk_imageinfo_t* vkImageInfo);
SK_C_API gr_backendrendertarget_t* gr_backendrendertarget_new_metal(int width, int height, int samples, const gr_mtl_textureinfo_t* mtlInfo);

SK_C_API void gr_backendrendertarget_delete(gr_backendrendertarget_t* rendertarget);

SK_C_API bool gr_backendrendertarget_is_valid(const gr_backendrendertarget_t* rendertarget);
SK_C_API int gr_backendrendertarget_get_width(const gr_backendrendertarget_t* rendertarget);
SK_C_API int gr_backendrendertarget_get_height(const gr_backendrendertarget_t* rendertarget);
SK_C_API int gr_backendrendertarget_get_samples(const gr_backendrendertarget_t* rendertarget);
SK_C_API int gr_backendrendertarget_get_stencils(const gr_backendrendertarget_t* rendertarget);
SK_C_API gr_backend_t gr_backendrendertarget_get_backend(const gr_backendrendertarget_t* rendertarget);
SK_C_API bool gr_backendrendertarget_get_gl_framebufferinfo(const gr_backendrendertarget_t* rendertarget, gr_gl_framebufferinfo_t* glInfo);


SK_C_PLUS_PLUS_END_GUARD

#endif
