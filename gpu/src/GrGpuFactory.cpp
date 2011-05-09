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

#include "GrGpu.h"
#include "GrGpuGLFixed.h"
#include "GrGpuGLShaders.h"

GrGpu* GrGpu::Create(GrEngine engine, GrPlatform3DContext context3D) {

    if (kOpenGL_Shaders_GrEngine == engine ||
        kOpenGL_Fixed_GrEngine == engine) {
        // If no GL bindings have been installed, fall-back to calling the
        // GL functions that have been linked with the executable.
        if (!GrGLGetGLInterface()) {
            GrGLSetDefaultGLInterface();
            // If there is no platform-default then just fail.
            if (!GrGLGetGLInterface()) {
                return NULL;
            }
        }
        if (!GrGLGetGLInterface()->validate(engine)) {
#if GR_DEBUG
            GrPrintf("Failed GL interface validation!");
#endif
            return NULL;
        }
    }

    GrGpu* gpu = NULL;

    switch (engine) {
        case kOpenGL_Shaders_GrEngine:
            GrAssert(NULL == (void*)context3D);
            {
#if 0 // old code path, will be removed soon
                gpu = new GrGpuGLShaders2;
#else
                gpu = new GrGpuGLShaders;
#endif
            }
            break;
        case kOpenGL_Fixed_GrEngine:
            GrAssert(NULL == (void*)context3D);
            gpu = new GrGpuGLFixed;
            break;
        case kDirect3D9_GrEngine:
            GrAssert(NULL != (void*)context3D);
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
