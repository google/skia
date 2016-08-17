/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"
#include "gl/GrGLInterface.h"

#include "gr_context.h"

#include "sk_types_priv.h"

gr_context_t* gr_context_create_with_defaults(gr_backend_t backend, gr_backendcontext_t backendContext) {
    return ToGrContext(GrContext::Create((GrBackend)backend, backendContext));
}

void gr_context_unref(gr_context_t* context) {
    AsGrContext(context)->unref();
}

const gr_gl_interface_t* gr_gl_default_interface() {
    return ToGrGLInterface(GrGLDefaultInterface());
}

const gr_gl_interface_t* gr_gl_create_native_interface() {
    return ToGrGLInterface(GrGLCreateNativeInterface());
}

void gr_gl_interface_unref(gr_gl_interface_t* glInterface) {
    AsGrGLInterface(glInterface)->unref();
}