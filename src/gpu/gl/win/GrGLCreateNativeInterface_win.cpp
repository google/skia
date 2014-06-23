
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GrGLInterface.h"
#include "gl/GrGLAssembleInterface.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class AutoLibraryUnload {
public:
    AutoLibraryUnload(const char* moduleName) {
        fModule = LoadLibrary(moduleName);
    }
    ~AutoLibraryUnload() {
        if (NULL != fModule) {
            FreeLibrary(fModule);
        }
    }
    HMODULE get() const { return fModule; }

private:
    HMODULE fModule;
};

class GLProcGetter {
public:
    GLProcGetter() : fGLLib("opengl32.dll") {}

    bool isInitialized() const { return NULL != fGLLib.get(); }

    GrGLFuncPtr getProc(const char name[]) const {
        GrGLFuncPtr proc;
        if (NULL != (proc = (GrGLFuncPtr) GetProcAddress(fGLLib.get(), name))) {
            return proc;
        }
        if (NULL != (proc = (GrGLFuncPtr) wglGetProcAddress(name))) {
            return proc;
        }
        return NULL;
    }

private:
    AutoLibraryUnload fGLLib;
};

static GrGLFuncPtr win_get_gl_proc(void* ctx, const char name[]) {
    SkASSERT(NULL != ctx);
    SkASSERT(NULL != wglGetCurrentContext());
    const GLProcGetter* getter = (const GLProcGetter*) ctx;
    return getter->getProc(name);
}

/*
 * Windows makes the GL funcs all be __stdcall instead of __cdecl :(
 * This implementation will only work if GR_GL_FUNCTION_TYPE is __stdcall.
 * Otherwise, a springboard would be needed that hides the calling convention.
 */
const GrGLInterface* GrGLCreateNativeInterface() {
    if (NULL == wglGetCurrentContext()) {
        return NULL;
    }

    GLProcGetter getter;
    if (!getter.isInitialized()) {
        return NULL;
    }

    return GrGLAssembleGLInterface(&getter, win_get_gl_proc);
}
