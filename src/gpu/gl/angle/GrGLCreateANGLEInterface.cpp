
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gl/GrGLInterface.h"
#include "gl/GrGLAssembleInterface.h"

#if defined _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif // defined _WIN32

#include <EGL/egl.h>

static GrGLFuncPtr angle_get_gl_proc(void* ctx, const char name[]) {
#if defined _WIN32
    GrGLFuncPtr proc = (GrGLFuncPtr) GetProcAddress((HMODULE)ctx, name);
#else
    GrGLFuncPtr proc = (GrGLFuncPtr) dlsym(ctx, name);
#endif // defined _WIN32
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
        gANGLELib = LoadLibrary("libGLESv2.dll");
#else
        gANGLELib = dlopen("libGLESv2.so", RTLD_LAZY);
#endif // defined _WIN32
    }

    if (nullptr == gANGLELib) {
        // We can't setup the interface correctly w/o the so
        return nullptr;
    }

    return GrGLAssembleGLESInterface(gANGLELib, angle_get_gl_proc);
}
