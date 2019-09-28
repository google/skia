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


// GrContext

SK_C_API gr_context_t* gr_context_make_gl(const gr_glinterface_t* glInterface);
// TODO: the overloads with GrContextOptions
// TODO: the Vulkan and Metal contexts

SK_C_API void gr_context_unref(gr_context_t* context);
SK_C_API void gr_context_abandon_context(gr_context_t* context);
SK_C_API void gr_context_release_resources_and_abandon_context(gr_context_t* context);
SK_C_API void gr_context_get_resource_cache_limits(gr_context_t* context, int* maxResources, size_t* maxResourceBytes);
SK_C_API void gr_context_set_resource_cache_limits(gr_context_t* context, int maxResources, size_t maxResourceBytes);
SK_C_API void gr_context_get_resource_cache_usage(gr_context_t* context, int* maxResources, size_t* maxResourceBytes);
SK_C_API int gr_context_get_max_surface_sample_count_for_color_type(gr_context_t* context, sk_colortype_t colorType);
SK_C_API void gr_context_flush(gr_context_t* context);
SK_C_API void gr_context_reset_context(gr_context_t* context, uint32_t state);
SK_C_API gr_backend_t gr_context_get_backend(gr_context_t* context);


// GrGLInterface

SK_C_API const gr_glinterface_t* gr_glinterface_create_native_interface(void);
SK_C_API const gr_glinterface_t* gr_glinterface_assemble_interface(void* ctx, gr_gl_get_proc get);
SK_C_API const gr_glinterface_t* gr_glinterface_assemble_gl_interface(void* ctx, gr_gl_get_proc get);
SK_C_API const gr_glinterface_t* gr_glinterface_assemble_gles_interface(void* ctx, gr_gl_get_proc get);

SK_C_API void gr_glinterface_unref(const gr_glinterface_t* glInterface);
SK_C_API bool gr_glinterface_validate(const gr_glinterface_t* glInterface);
SK_C_API bool gr_glinterface_has_extension(const gr_glinterface_t* glInterface, const char* extension);


// GrBackendTexture

SK_C_API gr_backendtexture_t* gr_backendtexture_new_gl(int width, int height, bool mipmapped, const gr_gl_textureinfo_t* glInfo);
SK_C_API void gr_backendtexture_delete(gr_backendtexture_t* texture);

SK_C_API bool gr_backendtexture_is_valid(const gr_backendtexture_t* texture);
SK_C_API int gr_backendtexture_get_width(const gr_backendtexture_t* texture);
SK_C_API int gr_backendtexture_get_height(const gr_backendtexture_t* texture);
SK_C_API bool gr_backendtexture_has_mipmaps(const gr_backendtexture_t* texture);
SK_C_API gr_backend_t gr_backendtexture_get_backend(const gr_backendtexture_t* texture);
SK_C_API bool gr_backendtexture_get_gl_textureinfo(const gr_backendtexture_t* texture, gr_gl_textureinfo_t* glInfo);


// GrBackendRenderTarget

SK_C_API gr_backendrendertarget_t* gr_backendrendertarget_new_gl(int width, int height, int samples, int stencils, const gr_gl_framebufferinfo_t* glInfo);
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
