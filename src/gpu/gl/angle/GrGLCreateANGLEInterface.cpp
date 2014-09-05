
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gl/GrGLInterface.h"
#include "gl/GrGLAssembleInterface.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "EGL/egl.h"

static GrGLFuncPtr angle_get_gl_proc(void* ctx, const char name[]) {
    GrGLFuncPtr proc = (GrGLFuncPtr) GetProcAddress((HMODULE)ctx, name);
    if (proc) {
        return proc;
    }
    return eglGetProcAddress(name);
}

const GrGLInterface* GrGLCreateANGLEInterface() {

    static HMODULE ghANGLELib = NULL;

    if (NULL == ghANGLELib) {
        // We load the ANGLE library and never let it go
        ghANGLELib = LoadLibrary("libGLESv2.dll");
    }
    if (NULL == ghANGLELib) {
        // We can't setup the interface correctly w/o the DLL
        return NULL;
    }

    return GrGLAssembleGLESInterface(ghANGLELib, angle_get_gl_proc);
}
