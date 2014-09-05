
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GrGLInterface.h"
#include "gl/GrGLAssembleInterface.h"
#include "gl/GrGLUtil.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class AutoLibraryUnload {
public:
    AutoLibraryUnload(const char* moduleName) {
        fModule = LoadLibrary(moduleName);
    }
    ~AutoLibraryUnload() {
        if (fModule) {
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

    bool isInitialized() const { return SkToBool(fGLLib.get()); }

    GrGLFuncPtr getProc(const char name[]) const {
        GrGLFuncPtr proc;
        if ((proc = (GrGLFuncPtr) GetProcAddress(fGLLib.get(), name))) {
            return proc;
        }
        if ((proc = (GrGLFuncPtr) wglGetProcAddress(name))) {
            return proc;
        }
        return NULL;
    }

private:
    AutoLibraryUnload fGLLib;
};

static GrGLFuncPtr win_get_gl_proc(void* ctx, const char name[]) {
    SkASSERT(ctx);
    SkASSERT(wglGetCurrentContext());
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

    GrGLGetStringProc getString = (GrGLGetStringProc)getter.getProc("glGetString");
    if (NULL == getString) {
        return NULL;
    }
    const char* verStr = reinterpret_cast<const char*>(getString(GR_GL_VERSION));
    GrGLStandard standard = GrGLGetStandardInUseFromString(verStr);

    if (kGLES_GrGLStandard == standard) {
        return GrGLAssembleGLESInterface(&getter, win_get_gl_proc);
    } else if (kGL_GrGLStandard == standard) {
        return GrGLAssembleGLInterface(&getter, win_get_gl_proc);
    }
    return NULL;
}
