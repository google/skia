/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h" // required to make sure SK_SUPPORT_GPU is defined

#if SK_SUPPORT_GPU

#include "include/gpu/GrContext.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/gl/GrGLAssembleInterface.h"

#define SK_ONLY_GPU(expr) expr
#define SK_ONLY_GPU_RETURN(expr, def) expr

#else // !SK_SUPPORT_GPU

#define SK_ONLY_GPU(expr)
#define SK_ONLY_GPU_RETURN(expr, def) def

#endif // SK_SUPPORT_GPU

#include "include/c/gr_context.h"

#include "src/c/sk_types_priv.h"


// GrContext

gr_context_t* gr_context_make_gl(const gr_glinterface_t* glInterface) {
    return SK_ONLY_GPU_RETURN(ToGrContext(GrContext::MakeGL(sk_ref_sp(AsGrGLInterface(glInterface))).release()), nullptr);
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

void gr_context_get_resource_cache_limits(gr_context_t* context, int* maxResources, size_t* maxResourceBytes) {
    SK_ONLY_GPU(AsGrContext(context)->getResourceCacheLimits(maxResources, maxResourceBytes));
}

void gr_context_set_resource_cache_limits(gr_context_t* context, int maxResources, size_t maxResourceBytes) {
    SK_ONLY_GPU(AsGrContext(context)->setResourceCacheLimits(maxResources, maxResourceBytes));
}

void gr_context_get_resource_cache_usage(gr_context_t* context, int* maxResources, size_t* maxResourceBytes) {
    SK_ONLY_GPU(AsGrContext(context)->getResourceCacheUsage(maxResources, maxResourceBytes));
}

int gr_context_get_max_surface_sample_count_for_color_type(gr_context_t* context, sk_colortype_t colorType) {
    return SK_ONLY_GPU_RETURN(AsGrContext(context)->maxSurfaceSampleCountForColorType((SkColorType)colorType), 0);
}

void gr_context_flush(gr_context_t* context) {
    SK_ONLY_GPU(AsGrContext(context)->flush());
}

void gr_context_reset_context(gr_context_t* context, uint32_t state) {
    SK_ONLY_GPU(AsGrContext(context)->resetContext(state));
}

gr_backend_t gr_context_get_backend(gr_context_t* context) {
    return SK_ONLY_GPU_RETURN((gr_backend_t)AsGrContext(context)->backend(), (gr_backend_t)0);
}


// GrGLInterface

const gr_glinterface_t* gr_glinterface_create_native_interface() {
    return SK_ONLY_GPU_RETURN(ToGrGLInterface(GrGLMakeNativeInterface().release()), nullptr);
}

const gr_glinterface_t* gr_glinterface_assemble_interface(void* ctx, gr_gl_get_proc get) {
    return SK_ONLY_GPU_RETURN(ToGrGLInterface(GrGLMakeAssembledInterface(ctx, get).release()), nullptr);
}

const gr_glinterface_t* gr_glinterface_assemble_gl_interface(void* ctx, gr_gl_get_proc get) {
    return SK_ONLY_GPU_RETURN(ToGrGLInterface(GrGLMakeAssembledGLInterface(ctx, get).release()), nullptr);
}

const gr_glinterface_t* gr_glinterface_assemble_gles_interface(void* ctx, gr_gl_get_proc get) {
    return SK_ONLY_GPU_RETURN(ToGrGLInterface(GrGLMakeAssembledGLESInterface(ctx, get).release()), nullptr);
}

void gr_glinterface_unref(const gr_glinterface_t* glInterface) {
    SK_ONLY_GPU(SkSafeUnref(AsGrGLInterface(glInterface)));
}

bool gr_glinterface_validate(const gr_glinterface_t* glInterface) {
    return SK_ONLY_GPU_RETURN(AsGrGLInterface(glInterface)->validate(), false);
}

bool gr_glinterface_has_extension(const gr_glinterface_t* glInterface, const char* extension) {
    return SK_ONLY_GPU_RETURN(AsGrGLInterface(glInterface)->hasExtension(extension), false);
}


// GrBackendTexture

gr_backendtexture_t* gr_backendtexture_new_gl(int width, int height, bool mipmapped, const gr_gl_textureinfo_t* glInfo) {
    return SK_ONLY_GPU_RETURN(ToGrBackendTexture(new GrBackendTexture(width, height, (GrMipMapped)mipmapped, *AsGrGLTextureInfo(glInfo))), nullptr);
}

void gr_backendtexture_delete(gr_backendtexture_t* texture) {
    SK_ONLY_GPU(delete AsGrBackendTexture(texture));
}

bool gr_backendtexture_is_valid(const gr_backendtexture_t* texture) {
    return SK_ONLY_GPU_RETURN(AsGrBackendTexture(texture)->isValid(), false);
}

int gr_backendtexture_get_width(const gr_backendtexture_t* texture) {
    return SK_ONLY_GPU_RETURN(AsGrBackendTexture(texture)->width(), 0);
}

int gr_backendtexture_get_height(const gr_backendtexture_t* texture) {
    return SK_ONLY_GPU_RETURN(AsGrBackendTexture(texture)->height(), 0);
}

bool gr_backendtexture_has_mipmaps(const gr_backendtexture_t* texture) {
    return SK_ONLY_GPU_RETURN(AsGrBackendTexture(texture)->hasMipMaps(), false);
}

gr_backend_t gr_backendtexture_get_backend(const gr_backendtexture_t* texture) {
    return SK_ONLY_GPU_RETURN((gr_backend_t)AsGrBackendTexture(texture)->backend(), (gr_backend_t)0);
}

bool gr_backendtexture_get_gl_textureinfo(const gr_backendtexture_t* texture, gr_gl_textureinfo_t* glInfo) {
    return SK_ONLY_GPU_RETURN(AsGrBackendTexture(texture)->getGLTextureInfo(AsGrGLTextureInfo(glInfo)), false);
}


// GrBackendRenderTarget

gr_backendrendertarget_t* gr_backendrendertarget_new_gl(int width, int height, int samples, int stencils, const gr_gl_framebufferinfo_t* glInfo) {
    return SK_ONLY_GPU_RETURN(ToGrBackendRenderTarget(new GrBackendRenderTarget(width, height, samples, stencils, *AsGrGLFramebufferInfo(glInfo))), nullptr);
}

void gr_backendrendertarget_delete(gr_backendrendertarget_t* rendertarget) {
    SK_ONLY_GPU(delete AsGrBackendRenderTarget(rendertarget));
}

bool gr_backendrendertarget_is_valid(const gr_backendrendertarget_t* rendertarget) {
    return SK_ONLY_GPU_RETURN(AsGrBackendRenderTarget(rendertarget)->isValid(), false);
}

int gr_backendrendertarget_get_width(const gr_backendrendertarget_t* rendertarget) {
    return SK_ONLY_GPU_RETURN(AsGrBackendRenderTarget(rendertarget)->width(), 0);
}

int gr_backendrendertarget_get_height(const gr_backendrendertarget_t* rendertarget) {
    return SK_ONLY_GPU_RETURN(AsGrBackendRenderTarget(rendertarget)->height(), 0);
}

int gr_backendrendertarget_get_samples(const gr_backendrendertarget_t* rendertarget) {
    return SK_ONLY_GPU_RETURN(AsGrBackendRenderTarget(rendertarget)->sampleCnt(), 0);
}

int gr_backendrendertarget_get_stencils(const gr_backendrendertarget_t* rendertarget) {
    return SK_ONLY_GPU_RETURN(AsGrBackendRenderTarget(rendertarget)->stencilBits(), 0);
}

gr_backend_t gr_backendrendertarget_get_backend(const gr_backendrendertarget_t* rendertarget) {
    return SK_ONLY_GPU_RETURN((gr_backend_t)AsGrBackendRenderTarget(rendertarget)->backend(), (gr_backend_t)0);
}

bool gr_backendrendertarget_get_gl_framebufferinfo(const gr_backendrendertarget_t* rendertarget, gr_gl_framebufferinfo_t* glInfo) {
    return SK_ONLY_GPU_RETURN(AsGrBackendRenderTarget(rendertarget)->getGLFramebufferInfo(AsGrGLFramebufferInfo(glInfo)), false);
}
