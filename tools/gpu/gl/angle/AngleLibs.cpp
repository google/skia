/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/gl/angle/AngleLibs.h"

#include "include/private/SkOnce.h"
#include "src/ports/SkOSLibrary.h"

namespace sk_gpu_test {

AngleLibs* AngleLibs::Get() {
    static AngleLibs gLibs;

    static SkOnce gOnce;
    gOnce([gLibs = &gLibs] {
        AngleLibs libs;
        // We load the ANGLE library and never let it go
#if defined _WIN32
        libs.fGLLib = DynamicLoadLibrary("libGLESv2_angle.dll");
        libs.fEGLLib = DynamicLoadLibrary("libEGL_angle.dll");
#elif defined SK_BUILD_FOR_MAC
        libs.fGLLib = DynamicLoadLibrary("libGLESv2_angle.dylib");
        libs.fEGLLib = DynamicLoadLibrary("libEGL_angle.dylib");
#else
        libs.fGLLib = DynamicLoadLibrary("libGLESv2_angle.so");
        libs.fEGLLib = DynamicLoadLibrary("libEGL_angle.so");
#endif
        if (nullptr == libs.fGLLib || nullptr == libs.fEGLLib) {
            // We can't setup the interface correctly w/o the so
            return;
        }
        if (!(libs.fEGLGetProcAddress = (PFNEGLGETPROCADDRESSPROC)GetProcedureAddress(
                      libs.fEGLLib, "eglGetProcAddress"))) {
            return;
        }
        if (!(libs.fEGLChooseConfig = (PFNEGLCHOOSECONFIGPROC)GetProcedureAddress(
                      libs.fEGLLib, "eglChooseConfig"))) {
            return;
        }
        if (!(libs.fEGLCreateContext = (PFNEGLCREATECONTEXTPROC)GetProcedureAddress(
                      libs.fEGLLib, "eglCreateContext"))) {
            return;
        }
        if (!(libs.fEGLCreatePbufferSurface = (PFNEGLCREATEPBUFFERSURFACEPROC)GetProcedureAddress(
                      libs.fEGLLib, "eglCreatePbufferSurface"))) {
            return;
        }
        if (!(libs.fEGLDestroyContext = (PFNEGLDESTROYCONTEXTPROC)GetProcedureAddress(
                      libs.fEGLLib, "eglDestroyContext"))) {
            return;
        }
        if (!(libs.fEGLDestroySurface = (PFNEGLDESTROYSURFACEPROC)GetProcedureAddress(
                      libs.fEGLLib, "eglDestroySurface"))) {
            return;
        }
        if (!(libs.fEGLGetCurrentContext = (PFNEGLGETCURRENTCONTEXTPROC)GetProcedureAddress(
                      libs.fEGLLib, "eglGetCurrentContext"))) {
            return;
        }
        if (!(libs.fEGLGetCurrentDisplay = (PFNEGLGETCURRENTDISPLAYPROC)GetProcedureAddress(
                      libs.fEGLLib, "eglGetCurrentDisplay"))) {
            return;
        }
        if (!(libs.fEGLGetCurrentSurface = (PFNEGLGETCURRENTSURFACEPROC)GetProcedureAddress(
                      libs.fEGLLib, "eglGetCurrentSurface"))) {
            return;
        }
        if (!(libs.fEGLGetError =
                      (PFNEGLGETERRORPROC)GetProcedureAddress(libs.fEGLLib, "eglGetError"))) {
            return;
        }
        if (!(libs.fEGLGetProcAddress = (PFNEGLGETPROCADDRESSPROC)GetProcedureAddress(
                      libs.fEGLLib, "eglGetProcAddress"))) {
            return;
        }
        if (!(libs.fEGLInitialize =
                      (PFNEGLINITIALIZEPROC)GetProcedureAddress(libs.fEGLLib, "eglInitialize"))) {
            return;
        }
        if (!(libs.fEGLMakeCurrent =
                      (PFNEGLMAKECURRENTPROC)GetProcedureAddress(libs.fEGLLib, "eglMakeCurrent"))) {
            return;
        }
        if (!(libs.fEGLQueryString =
                      (PFNEGLQUERYSTRINGPROC)GetProcedureAddress(libs.fEGLLib, "eglQueryString"))) {
            return;
        }
        if (!(libs.fEGLSwapBuffers =
                      (PFNEGLSWAPBUFFERSPROC)GetProcedureAddress(libs.fEGLLib, "eglSwapBuffers"))) {
            return;
        }
        if (!(libs.fEGLTerminate =
                      (PFNEGLTERMINATEPROC)GetProcedureAddress(libs.fEGLLib, "eglTerminate"))) {
            return;
        }
        if (!(libs.fEGLGetPlatformDisplay = (PFNEGLGETPLATFORMDISPLAYEXTPROC)GetProcedureAddress(
                      libs.fEGLLib, "eglGetPlatformDisplayEXT"))) {
            return;
        }

        *gLibs = libs;
    });
    return gLibs.fGLLib ? &gLibs : nullptr;
}

}  // namespace sk_gpu_test
