/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef AngleLibs_DEFINED
#define AngleLibs_DEFINED

#define EGL_EGL_PROTOTYPES 1

#include <EGL/egl.h>
#include <EGL/eglext.h>

namespace sk_gpu_test {

class AngleLibs {
public:
    PFNEGLCHOOSECONFIGPROC fEGLChooseConfig = nullptr;
    PFNEGLCREATECONTEXTPROC fEGLCreateContext = nullptr;
    PFNEGLCREATEPBUFFERSURFACEPROC fEGLCreatePbufferSurface = nullptr;
    PFNEGLDESTROYCONTEXTPROC fEGLDestroyContext = nullptr;
    PFNEGLDESTROYSURFACEPROC fEGLDestroySurface = nullptr;
    PFNEGLGETCURRENTCONTEXTPROC fEGLGetCurrentContext = nullptr;
    PFNEGLGETCURRENTDISPLAYPROC fEGLGetCurrentDisplay = nullptr;
    PFNEGLGETCURRENTSURFACEPROC fEGLGetCurrentSurface = nullptr;
    PFNEGLGETERRORPROC fEGLGetError = nullptr;
    PFNEGLGETPROCADDRESSPROC fEGLGetProcAddress = nullptr;
    PFNEGLINITIALIZEPROC fEGLInitialize = nullptr;
    PFNEGLMAKECURRENTPROC fEGLMakeCurrent = nullptr;
    PFNEGLQUERYSTRINGPROC fEGLQueryString = nullptr;
    PFNEGLSWAPBUFFERSPROC fEGLSwapBuffers = nullptr;
    PFNEGLTERMINATEPROC fEGLTerminate = nullptr;
    PFNEGLGETPLATFORMDISPLAYEXTPROC fEGLGetPlatformDisplay = nullptr;

    void* glLib() const { return fGLLib; }
    void* eglLib() const { return fEGLLib; }

    static AngleLibs* Get();

private:
    AngleLibs() = default;
    void* fGLLib = nullptr;
    void* fEGLLib = nullptr;
};

}  // namespace sk_gpu_test

#endif
