/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkTypes.h"

#if !defined(SK_DISABLE_LEGACY_GL_MAKE_NATIVE_INTERFACE)

#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/gpu/ganesh/gl/win/GrGLMakeWinInterface.h"

#if defined(_M_ARM64)

sk_sp<const GrGLInterface> GrGLMakeNativeInterface() { return nullptr; }

#else
/*
 * Windows makes the GL funcs all be __stdcall instead of __cdecl :(
 * This implementation will only work if GR_GL_FUNCTION_TYPE is __stdcall.
 * Otherwise, a springboard would be needed that hides the calling convention.
 */
sk_sp<const GrGLInterface> GrGLMakeNativeInterface() {
    return GrGLInterfaces::MakeWin();
}

#endif // ARM64

#endif // SK_DISABLE_LEGACY_GL_MAKE_NATIVE_INTERFACE
