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

#include <memory>
#include <type_traits>

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

    struct FreeModule { void operator()(HMODULE m) { (void)FreeLibrary(m); } };
    std::unique_ptr<typename std::remove_pointer<HMODULE>::type, FreeModule> module(
            LoadLibraryA("opengl32.dll"));
    if (!module) {
        return nullptr;
    }
    const GrGLGetProc win_get_gl_proc = [](void* ctx, const char* name) {
        SkASSERT(wglGetCurrentContext());
        if (GrGLFuncPtr p = (GrGLFuncPtr)GetProcAddress((HMODULE)ctx, name)) {
            return p;
        }
        if (GrGLFuncPtr p = (GrGLFuncPtr)wglGetProcAddress(name)) {
            return p;
        }
        return (GrGLFuncPtr)nullptr;
    };

    GrGLGetStringFn* getString =
        (GrGLGetStringFn*)win_get_gl_proc((void*)module.get(), "glGetString");
    if (!getString) {
        return nullptr;
    }
    const char* verStr = reinterpret_cast<const char*>(getString(GR_GL_VERSION));
    GrGLStandard standard = GrGLGetStandardInUseFromString(verStr);

    if (GR_IS_GR_GL_ES(standard)) {
        return GrGLMakeAssembledGLESInterface((void*)module.get(), win_get_gl_proc);
    } else if (GR_IS_GR_GL(standard)) {
        return GrGLMakeAssembledGLInterface((void*)module.get(), win_get_gl_proc);
    }
    return nullptr;
}

#endif // ARM64

const GrGLInterface* GrGLCreateNativeInterface() { return GrGLMakeNativeInterface().release(); }

#endif//defined(SK_BUILD_FOR_WIN)
