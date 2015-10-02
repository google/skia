
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GrGLInterface.h"
#include "gl/GrGLAssembleInterface.h"
#include "../ports/SkOSLibrary.h"

#include <EGL/egl.h>

static GrGLFuncPtr angle_get_gl_proc(void* ctx, const char name[]) {
    GrGLFuncPtr proc = (GrGLFuncPtr) GetProcedureAddress(ctx, name);
    if (proc) {
        return proc;
    }
    return eglGetProcAddress(name);
}

const GrGLInterface* GrGLCreateANGLEInterface() {
    static void* gANGLELib = nullptr;

    if (nullptr == gANGLELib) {
        // We load the ANGLE library and never let it go
#if defined _WIN32
        gANGLELib = DynamicLoadLibrary("libGLESv2.dll");
#else
        gANGLELib = DynamicLoadLibrary("libGLESv2.so");
#endif // defined _WIN32
    }

    if (nullptr == gANGLELib) {
        // We can't setup the interface correctly w/o the so
        return nullptr;
    }

    return GrGLAssembleGLESInterface(gANGLELib, angle_get_gl_proc);
}
