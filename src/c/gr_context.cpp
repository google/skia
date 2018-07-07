/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h" // required to make sure SK_SUPPORT_GPU is defined

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrBackendSurface.h"
#include "gl/GrGLInterface.h"
#include "gl/GrGLAssembleInterface.h"

#include "gr_context.h"

#include "sk_types_priv.h"


// GrContext

gr_context_t* gr_context_make_gl(const gr_glinterface_t* glInterface) {
    return ToGrContext(GrContext::MakeGL(sk_ref_sp(AsGrGLInterface(glInterface))).release());
}

void gr_context_unref(gr_context_t* context) {
    SkSafeUnref(AsGrContext(context));
}

void gr_context_abandon_context(gr_context_t* context) {
    AsGrContext(context)->abandonContext();
}

void gr_context_release_resources_and_abandon_context(gr_context_t* context) {
    AsGrContext(context)->releaseResourcesAndAbandonContext();
}

void gr_context_get_resource_cache_limits(gr_context_t* context, int* maxResources, size_t* maxResourceBytes) {
    AsGrContext(context)->getResourceCacheLimits(maxResources, maxResourceBytes);
}

void gr_context_set_resource_cache_limits(gr_context_t* context, int maxResources, size_t maxResourceBytes) {
    AsGrContext(context)->setResourceCacheLimits(maxResources, maxResourceBytes);
}

void gr_context_get_resource_cache_usage(gr_context_t* context, int* maxResources, size_t* maxResourceBytes) {
    AsGrContext(context)->getResourceCacheUsage(maxResources, maxResourceBytes);
}

int gr_context_get_max_surface_sample_count_for_color_type(gr_context_t* context, sk_colortype_t colorType) {
    return AsGrContext(context)->maxSurfaceSampleCountForColorType((SkColorType)colorType);
}

void gr_context_flush(gr_context_t* context) {
    AsGrContext(context)->flush();
}

void gr_context_reset_context(gr_context_t* context, uint32_t state) {
    AsGrContext(context)->resetContext(state);
}

gr_backend_t gr_context_get_backend(gr_context_t* context) {
    return (gr_backend_t)AsGrContext(context)->backend();
}


// GrGLInterface

const gr_glinterface_t* gr_glinterface_create_native_interface() {
    return ToGrGLInterface(GrGLMakeNativeInterface().release());
}

const gr_glinterface_t* gr_glinterface_assemble_interface(void* ctx, gr_gl_get_proc get) {
    return ToGrGLInterface(GrGLMakeAssembledInterface(ctx, get).release());
}

const gr_glinterface_t* gr_glinterface_assemble_gl_interface(void* ctx, gr_gl_get_proc get) {
    return ToGrGLInterface(GrGLMakeAssembledGLInterface(ctx, get).release());
}

const gr_glinterface_t* gr_glinterface_assemble_gles_interface(void* ctx, gr_gl_get_proc get) {
    return ToGrGLInterface(GrGLMakeAssembledGLESInterface(ctx, get).release());
}

void gr_glinterface_unref(gr_glinterface_t* glInterface) {
    SkSafeUnref(AsGrGLInterface(glInterface));
}

bool gr_glinterface_validate(gr_glinterface_t* glInterface) {
    return AsGrGLInterface(glInterface)->validate();
}

bool gr_glinterface_has_extension(gr_glinterface_t* glInterface, const char* extension) {
    return AsGrGLInterface(glInterface)->hasExtension(extension);
}


// GrBackendTexture

gr_backendtexture_t* gr_backendtexture_new_gl(int width, int height, bool mipmapped, const gr_gl_textureinfo_t* glInfo) {
    return ToGrBackendTexture(new GrBackendTexture(width, height, (GrMipMapped)mipmapped, *AsGrGLTextureInfo(glInfo)));
}

void gr_backendtexture_delete(gr_backendtexture_t* texture) {
    delete AsGrBackendTexture(texture);
}

bool gr_backendtexture_is_valid(const gr_backendtexture_t* texture) {
    return AsGrBackendTexture(texture)->isValid();
}

int gr_backendtexture_get_width(const gr_backendtexture_t* texture) {
    return AsGrBackendTexture(texture)->width();
}

int gr_backendtexture_get_height(const gr_backendtexture_t* texture) {
    return AsGrBackendTexture(texture)->height();
}

bool gr_backendtexture_has_mipmaps(const gr_backendtexture_t* texture) {
    return AsGrBackendTexture(texture)->hasMipMaps();
}

gr_backend_t gr_backendtexture_get_backend(const gr_backendtexture_t* texture) {
    return (gr_backend_t)AsGrBackendTexture(texture)->backend();
}

bool gr_backendtexture_get_gl_textureinfo(const gr_backendtexture_t* texture, gr_gl_textureinfo_t* glInfo) {
    return AsGrBackendTexture(texture)->getGLTextureInfo(AsGrGLTextureInfo(glInfo));
}


// GrBackendRenderTarget

gr_backendrendertarget_t* gr_backendrendertarget_new_gl(int width, int height, int samples, int stencils, const gr_gl_framebufferinfo_t* glInfo) {
    return ToGrBackendRenderTarget(new GrBackendRenderTarget(width, height, samples, stencils, *AsGrGLFramebufferInfo(glInfo)));
}

void gr_backendrendertarget_delete(gr_backendrendertarget_t* rendertarget) {
    delete AsGrBackendRenderTarget(rendertarget);
}

bool gr_backendrendertarget_is_valid(const gr_backendrendertarget_t* rendertarget) {
    return AsGrBackendRenderTarget(rendertarget)->isValid();
}

int gr_backendrendertarget_get_width(const gr_backendrendertarget_t* rendertarget) {
    return AsGrBackendRenderTarget(rendertarget)->width();
}

int gr_backendrendertarget_get_height(const gr_backendrendertarget_t* rendertarget) {
    return AsGrBackendRenderTarget(rendertarget)->height();
}

int gr_backendrendertarget_get_samples(const gr_backendrendertarget_t* rendertarget) {
    return AsGrBackendRenderTarget(rendertarget)->sampleCnt();
}

int gr_backendrendertarget_get_stencils(const gr_backendrendertarget_t* rendertarget) {
    return AsGrBackendRenderTarget(rendertarget)->stencilBits();
}

gr_backend_t gr_backendrendertarget_get_backend(const gr_backendrendertarget_t* rendertarget) {
    return (gr_backend_t)AsGrBackendRenderTarget(rendertarget)->backend();
}

bool gr_backendrendertarget_get_gl_framebufferinfo(const gr_backendrendertarget_t* rendertarget, gr_gl_framebufferinfo_t* glInfo) {
    return AsGrBackendRenderTarget(rendertarget)->getGLFramebufferInfo(AsGrGLFramebufferInfo(glInfo));
}


#else // !SK_SUPPORT_GPU

#include "gr_context.h"
#include "sk_types_priv.h"


// GrContext

gr_context_t* gr_context_make_gl(const gr_glinterface_t* glInterface) {
    return nullptr;
}

void gr_context_unref(gr_context_t* context) {
}

void gr_context_abandon_context(gr_context_t* context) {
}

void gr_context_release_resources_and_abandon_context(gr_context_t* context) {
}

void gr_context_get_resource_cache_limits(gr_context_t* context, int* maxResources, size_t* maxResourceBytes) {
}

void gr_context_set_resource_cache_limits(gr_context_t* context, int maxResources, size_t maxResourceBytes) {
}

void gr_context_get_resource_cache_usage(gr_context_t* context, int* maxResources, size_t* maxResourceBytes) {
}

int gr_context_get_max_surface_sample_count_for_color_type(gr_context_t* context, sk_colortype_t colorType) {
    return 0;
}

void gr_context_flush(gr_context_t* context) {
}


// GrGLInterface

const gr_glinterface_t* gr_glinterface_create_native_interface() {
    return nullptr;
}

const gr_glinterface_t* gr_glinterface_assemble_interface(void* ctx, gr_gl_get_proc get) {
    return nullptr;
}

const gr_glinterface_t* gr_glinterface_assemble_gl_interface(void* ctx, gr_gl_get_proc get) {
    return nullptr;
}

const gr_glinterface_t* gr_glinterface_assemble_gles_interface(void* ctx, gr_gl_get_proc get) {
    return nullptr;
}

void gr_glinterface_unref(gr_glinterface_t* glInterface) {
}

bool gr_glinterface_validate(gr_glinterface_t* glInterface) {
    return false;
}

bool gr_glinterface_has_extension(gr_glinterface_t* glInterface, const char* extension) {
    return false;
}


// GrBackendTexture

gr_backendtexture_t* gr_backendtexture_new_gl(int width, int height, bool mipmapped, const gr_gl_textureinfo_t* glInfo) {
    return nullptr;
}

void gr_backendtexture_delete(gr_backendtexture_t* texture) {
}

bool gr_backendtexture_is_valid(const gr_backendtexture_t* texture) {
    return false;
}

int gr_backendtexture_get_width(const gr_backendtexture_t* texture) {
    return 0;
}

int gr_backendtexture_get_height(const gr_backendtexture_t* texture) {
    return 0;
}

bool gr_backendtexture_has_mipmaps(const gr_backendtexture_t* texture) {
    return false;
}

gr_backend_t gr_backendtexture_get_backend(const gr_backendtexture_t* texture) {
    return (gr_backend_t)0;
}

bool gr_backendtexture_get_gl_textureinfo(const gr_backendtexture_t* texture, gr_gl_textureinfo_t* glInfo) {
    return false;
}


// GrBackendRenderTarget

gr_backendrendertarget_t* gr_backendrendertarget_new_gl(int width, int height, int samples, int stencils, const gr_gl_framebufferinfo_t* glInfo) {
    return nullptr;
}

void gr_backendrendertarget_delete(gr_backendrendertarget_t* rendertarget) {
}

bool gr_backendrendertarget_is_valid(const gr_backendrendertarget_t* rendertarget) {
    return false;
}

int gr_backendrendertarget_get_width(const gr_backendrendertarget_t* rendertarget) {
    return 0;
}

int gr_backendrendertarget_get_height(const gr_backendrendertarget_t* rendertarget) {
    return 0;
}

int gr_backendrendertarget_get_samples(const gr_backendrendertarget_t* rendertarget) {
    return 0;
}

int gr_backendrendertarget_get_stencils(const gr_backendrendertarget_t* rendertarget) {
    return 0;
}

gr_backend_t gr_backendrendertarget_get_backend(const gr_backendrendertarget_t* rendertarget) {
    return (gr_backend_t)0;
}

bool gr_backendrendertarget_get_gl_framebufferinfo(const gr_backendrendertarget_t* rendertarget, gr_gl_framebufferinfo_t* glInfo) {
    return false;
}


#endif // SK_SUPPORT_GPU
