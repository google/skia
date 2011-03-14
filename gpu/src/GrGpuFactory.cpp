/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#include "GrTypes.h"

// must be before GrGLConfig.h
#if GR_WIN32_BUILD
//    #include "GrGpuD3D9.h"
#endif

#include "GrGLConfig.h"


#if GR_SUPPORT_GLES1 || GR_SUPPORT_GLDESKTOP
    #include "GrGpuGLFixed.h"
#endif

#if GR_SUPPORT_GLES2 || GR_SUPPORT_GLDESKTOP
    #include "GrGpuGLShaders2.h"
#endif

#include "GrGpu.h"

GrGpu* GrGpu::Create(Engine engine, Platform3DContext context3D) {
    // If no GL bindings have been installed, fall-back to calling the
    // GL functions that have been linked with the executable.
    if (!GrGLGetGLInterface())
        GrGLSetDefaultGLInterface();

    GrGpu* gpu = NULL;

    switch (engine) {
        case kOpenGL_Shaders_Engine:
            GrAssert(NULL == context3D);
#if GR_SUPPORT_GLES2 || GR_SUPPORT_GLDESKTOP
            gpu = new GrGpuGLShaders2;
#endif
            break;
        case kOpenGL_Fixed_Engine:
            GrAssert(NULL == context3D);
#if GR_SUPPORT_GLES1 || GR_SUPPORT_GLDESKTOP
            gpu = new GrGpuGLFixed;
#endif
            break;
        case kDirect3D9_Engine:
            GrAssert(NULL != context3D);
#if GR_WIN32_BUILD
//            gpu = new GrGpuD3D9((IDirect3DDevice9*)context3D);
#endif
            break;
        default:
            GrAssert(!"unknown engine");
            break;
    }

    return gpu;
}



