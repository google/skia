/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_WIN)

#include "src/core/SkLeanWindows.h"

#include "include/gpu/gl/GrGLAssembleInterface.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "src/gpu/gl/GrGLUtil.h"

#if defined(_M_ARM64)

sk_sp<const GrGLInterface> GrGLMakeNativeInterface() { return nullptr; }

#else
/*
 * Windows makes the GL funcs all be __stdcall instead of __cdecl :(
 * This implementation will only work if GR_GL_FUNCTION_TYPE is __stdcall.
 * Otherwise, a springboard would be needed that hides the calling convention.
 */
sk_sp<const GrGLInterface> GrGLMakeNativeInterface() {
    if (nullptr == wglGetCurrentContext()) {
        return nullptr;
    }

    std::unique_ptr<void, SkFunctionWrapper<int(void*), FreeLibrary>> module(
            LoadLibraryA("opengl32.dll"));
    if (!module) {
        return nullptr;
    }
    const GrGLGetProc win_get_gl_proc = [] getProc(void* ctx, const char* name) {
        SkASSERT(wglGetCurrentContext());
        if ((GrGLFuncPtr p = (GrGLFuncPtr)GetProcAddress((HMODULE)ctx, name))) {
            return p;
        }
        if ((GrGLFuncPtr p = (GrGLFuncPtr)wglGetProcAddress(name))) {
            return p;
        }
        return nullptr;
    }

    GrGLGetStringFn* getString = (GrGLGetStringFn*)win_get_gl_proc(module.get(), "glGetString");
    if (nullptr == getString) {
        return nullptr;
    }
    const char* verStr = reinterpret_cast<const char*>(getString(GR_GL_VERSION));
    GrGLStandard standard = GrGLGetStandardInUseFromString(verStr);

    if (GR_IS_GR_GL_ES(standard)) {
        return GrGLMakeAssembledGLESInterface(module.get(), win_get_gl_proc);
    } else if (GR_IS_GR_GL(standard)) {
        return GrGLMakeAssembledGLInterface(module.get(), win_get_gl_proc);
    }
    return nullptr;
}

#endif // ARM64

const GrGLInterface* GrGLCreateNativeInterface() { return GrGLMakeNativeInterface().release(); }

#endif//defined(SK_BUILD_FOR_WIN)
