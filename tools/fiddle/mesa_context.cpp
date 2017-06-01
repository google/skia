/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fiddle_main.h"

#include <GL/osmesa.h>

// create_grcontext implementation for Mesa.
sk_sp<GrContext> create_grcontext(std::ostringstream &driverinfo) {
    // We just leak the OSMesaContext... the process will die soon anyway.
    if (OSMesaContext osMesaContext = OSMesaCreateContextExt(OSMESA_BGRA, 0, 0, 0, nullptr)) {
        static uint32_t buffer[16 * 16];
        OSMesaMakeCurrent(osMesaContext, &buffer, GL_UNSIGNED_BYTE, 16, 16);
    }
    driverinfo << "Mesa";

    auto osmesa_get = [](void* ctx, const char name[]) {
        SkASSERT(nullptr == ctx);
        SkASSERT(OSMesaGetCurrentContext());
        return OSMesaGetProcAddress(name);
    };
    sk_sp<const GrGLInterface> mesa(GrGLAssembleInterface(nullptr, osmesa_get));
    if (!mesa) {
        return nullptr;
    }
    return sk_sp<GrContext>(GrContext::Create(
                kOpenGL_GrBackend,
                reinterpret_cast<intptr_t>(mesa.get())));
}
