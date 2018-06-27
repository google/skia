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
#include "gl/GrGLInterface.h"
#include "gl/GrGLAssembleInterface.h"

#include "gr_context.h"

#include "sk_types_priv.h"

gr_context_t* gr_context_create(gr_backend_t backend, gr_backendcontext_t backendContext, const gr_context_options_t* options) {
    return ToGrContext(GrContext::Create((GrBackend)backend, backendContext, AsGrContextOptions(*options)));
}

gr_context_t* gr_context_create_with_defaults(gr_backend_t backend, gr_backendcontext_t backendContext) {
    return ToGrContext(GrContext::Create((GrBackend)backend, backendContext));
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

int gr_context_get_recommended_sample_count(gr_context_t* context, gr_pixelconfig_t config, float dpi) {
    return AsGrContext(context)->getRecommendedSampleCount((GrPixelConfig)config, dpi);
}

void gr_context_flush(gr_context_t* context) {
    AsGrContext(context)->flush();
}

void gr_context_reset_context(gr_context_t* context, uint32_t state) {
    AsGrContext(context)->resetContext(state);
}


const gr_glinterface_t* gr_glinterface_default_interface() {
    return ToGrGLInterface(GrGLDefaultInterface());
}

const gr_glinterface_t* gr_glinterface_create_native_interface() {
    return ToGrGLInterface(GrGLCreateNativeInterface());
}

const gr_glinterface_t* gr_glinterface_assemble_interface(void* ctx, gr_gl_get_proc get) {
    return ToGrGLInterface(GrGLAssembleInterface(ctx, get));
}

const gr_glinterface_t* gr_glinterface_assemble_gl_interface(void* ctx, gr_gl_get_proc get) {
    return ToGrGLInterface(GrGLAssembleGLInterface(ctx, get));
}

const gr_glinterface_t* gr_glinterface_assemble_gles_interface(void* ctx, gr_gl_get_proc get) {
    return ToGrGLInterface(GrGLAssembleGLESInterface(ctx, get));
}

void gr_glinterface_unref(gr_glinterface_t* glInterface) {
    SkSafeUnref(AsGrGLInterface(glInterface));
}

gr_glinterface_t* gr_glinterface_clone(gr_glinterface_t* glInterface) {
    return ToGrGLInterface(GrGLInterface::NewClone(AsGrGLInterface(glInterface)));
}

bool gr_glinterface_validate(gr_glinterface_t* glInterface) {
    return AsGrGLInterface(glInterface)->validate();
}

bool gr_glinterface_has_extension(gr_glinterface_t* glInterface, const char* extension) {
    return AsGrGLInterface(glInterface)->hasExtension(extension);
}

#else // !SK_SUPPORT_GPU

#include "gr_context.h"
#include "sk_types_priv.h"

gr_context_t* gr_context_create(gr_backend_t backend, gr_backendcontext_t backendContext, const gr_context_options_t* options) {
    return nullptr;
}

gr_context_t* gr_context_create_with_defaults(gr_backend_t backend, gr_backendcontext_t backendContext) {
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

int gr_context_get_recommended_sample_count(gr_context_t* context, gr_pixelconfig_t config, float dpi) {
    return 0;
}

void gr_context_flush(gr_context_t* context) {
}

const gr_glinterface_t* gr_glinterface_default_interface() {
    return nullptr;
}

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

gr_glinterface_t* gr_glinterface_clone(gr_glinterface_t* glInterface) {
    return nullptr;
}

bool gr_glinterface_validate(gr_glinterface_t* glInterface) {
    return false;
}

bool gr_glinterface_has_extension(gr_glinterface_t* glInterface, const char* extension) {
    return false;
}

#endif // SK_SUPPORT_GPU
