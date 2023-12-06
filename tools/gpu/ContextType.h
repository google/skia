/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ContextType_DEFINED
#define ContextType_DEFINED

enum class GrBackendApi : unsigned;

namespace skgpu {

enum class BackendApi : unsigned;

// The availability of context types is subject to platform and build configuration
// restrictions.
enum class ContextType {
    kGL,               //! OpenGL context.
    kGLES,             //! OpenGL ES context.
    kANGLE_D3D9_ES2,   //! ANGLE on Direct3D9 OpenGL ES 2 context.
    kANGLE_D3D11_ES2,  //! ANGLE on Direct3D11 OpenGL ES 2 context.
    kANGLE_D3D11_ES3,  //! ANGLE on Direct3D11 OpenGL ES 3 context.
    kANGLE_GL_ES2,     //! ANGLE on OpenGL OpenGL ES 2 context.
    kANGLE_GL_ES3,     //! ANGLE on OpenGL OpenGL ES 3 context.
    kANGLE_Metal_ES2,  //! ANGLE on Metal ES 2 context.
    kANGLE_Metal_ES3,  //! ANGLE on Metal ES 3 context.
    kVulkan,           //! Vulkan
    kMetal,            //! Metal
    kDirect3D,         //! Direct3D 12
    kDawn,             //! Dawn
    kDawn_D3D11,       //! Dawn on Direct3D11
    kDawn_D3D12,       //! Dawn on Direct3D12
    kDawn_Metal,       //! Dawn on Metal
    kDawn_Vulkan,      //! Dawn on Vulkan
    kDawn_OpenGL,      //! Dawn on OpenGL
    kDawn_OpenGLES,    //! Dawn on OpenGL ES
    kMock,             //! Mock context that does not draw.
    kLastContextType = kMock
};

static const int kContextTypeCount = (int)ContextType::kLastContextType + 1;

const char* ContextTypeName(skgpu::ContextType type);

bool IsNativeBackend(skgpu::ContextType type);

bool IsDawnBackend(skgpu::ContextType type);

bool IsRenderingContext(skgpu::ContextType type);

namespace ganesh {

GrBackendApi ContextTypeBackend(skgpu::ContextType type);

}  // namespace ganesh

namespace graphite {

skgpu::BackendApi ContextTypeBackend(skgpu::ContextType type);

}  // namespace graphite
}  // namespace skgpu

#endif
