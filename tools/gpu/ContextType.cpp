/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "tools/gpu/ContextType.h"

namespace skgpu {

const char* ContextTypeName(skgpu::ContextType contextType) {
    switch (contextType) {
        case ContextType::kGL:
            return "OpenGL";
        case ContextType::kGLES:
            return "OpenGLES";
        case ContextType::kANGLE_D3D9_ES2:
            return "ANGLE D3D9 ES2";
        case ContextType::kANGLE_D3D11_ES2:
            return "ANGLE D3D11 ES2";
        case ContextType::kANGLE_D3D11_ES3:
            return "ANGLE D3D11 ES3";
        case ContextType::kANGLE_GL_ES2:
            return "ANGLE GL ES2";
        case ContextType::kANGLE_GL_ES3:
            return "ANGLE GL ES3";
        case ContextType::kANGLE_Metal_ES2:
            return "ANGLE Metal ES2";
        case ContextType::kANGLE_Metal_ES3:
            return "ANGLE Metal ES3";
        case ContextType::kVulkan:
            return "Vulkan";
        case ContextType::kMetal:
            return "Metal";
        case ContextType::kDirect3D:
            return "Direct3D";
        case ContextType::kDawn:
            return "Dawn";
        case ContextType::kDawn_D3D11:
            return "Dawn D3D11";
        case ContextType::kDawn_D3D12:
            return "Dawn D3D12";
        case ContextType::kDawn_Metal:
            return "Dawn Metal";
        case ContextType::kDawn_Vulkan:
            return "Dawn Vulkan";
        case ContextType::kDawn_OpenGL:
            return "Dawn OpenGL";
        case ContextType::kDawn_OpenGLES:
            return "Dawn OpenGLES";
        case ContextType::kMock:
            return "Mock";
    }
    SkUNREACHABLE;
}

}  // namespace skgpu
