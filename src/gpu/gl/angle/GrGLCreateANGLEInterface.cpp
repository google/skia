
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

namespace {
struct Libs {
    void* fGLLib;
    void* fEGLLib;
};
}

static GrGLFuncPtr angle_get_gl_proc(void* ctx, const char name[]) {
    const Libs* libs = reinterpret_cast<const Libs*>(ctx);
    GrGLFuncPtr proc = (GrGLFuncPtr) GetProcedureAddress(libs->fGLLib, name);
    if (proc) {
        return proc;
    }
    proc = (GrGLFuncPtr) GetProcedureAddress(libs->fEGLLib, name);
    if (proc) {
        return proc;
    }
    return eglGetProcAddress(name);
}

const GrGLInterface* GrGLCreateANGLEInterface() {
    static Libs gLibs = { nullptr, nullptr };

    if (nullptr == gLibs.fGLLib) {
        // We load the ANGLE library and never let it go
#if defined _WIN32
        gLibs.fGLLib = DynamicLoadLibrary("libGLESv2.dll");
        gLibs.fEGLLib = DynamicLoadLibrary("libEGL.dll");
#elif defined SK_BUILD_FOR_MAC
        gLibs.fGLLib = DynamicLoadLibrary("libGLESv2.dylib");
        gLibs.fEGLLib = DynamicLoadLibrary("libEGL.dylib");
#else
        gLibs.fGLLib = DynamicLoadLibrary("libGLESv2.so");
        gLibs.fGLLib = DynamicLoadLibrary("libEGL.so");
#endif
    }

    if (nullptr == gLibs.fGLLib || nullptr == gLibs.fEGLLib) {
        // We can't setup the interface correctly w/o the so
        return nullptr;
    }

    return GrGLAssembleGLESInterface(&gLibs, angle_get_gl_proc);
}
