/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrTypes.h"
#include "tools/gpu/ContextType.h"

const char* skgpu::ContextTypeName(skgpu::ContextType type) {
    switch (type) {
        case skgpu::ContextType::kGL:
            return "OpenGL";
        case skgpu::ContextType::kGLES:
            return "OpenGLES";
        case skgpu::ContextType::kANGLE_D3D9_ES2:
            return "ANGLE D3D9 ES2";
        case skgpu::ContextType::kANGLE_D3D11_ES2:
            return "ANGLE D3D11 ES2";
        case skgpu::ContextType::kANGLE_D3D11_ES3:
            return "ANGLE D3D11 ES3";
        case skgpu::ContextType::kANGLE_GL_ES2:
            return "ANGLE GL ES2";
        case skgpu::ContextType::kANGLE_GL_ES3:
            return "ANGLE GL ES3";
        case skgpu::ContextType::kANGLE_Metal_ES2:
            return "ANGLE Metal ES2";
        case skgpu::ContextType::kANGLE_Metal_ES3:
            return "ANGLE Metal ES3";
        case skgpu::ContextType::kVulkan:
            return "Vulkan";
        case skgpu::ContextType::kMetal:
            return "Metal";
        case skgpu::ContextType::kDirect3D:
            return "Direct3D";
        case skgpu::ContextType::kDawn:
            return "Dawn";
        case skgpu::ContextType::kDawn_D3D11:
            return "Dawn D3D11";
        case skgpu::ContextType::kDawn_D3D12:
            return "Dawn D3D12";
        case skgpu::ContextType::kDawn_Metal:
            return "Dawn Metal";
        case skgpu::ContextType::kDawn_Vulkan:
            return "Dawn Vulkan";
        case skgpu::ContextType::kDawn_OpenGL:
            return "Dawn OpenGL";
        case skgpu::ContextType::kDawn_OpenGLES:
            return "Dawn OpenGLES";
        case skgpu::ContextType::kMock:
            return "Mock";
    }
    SkUNREACHABLE;
}

bool skgpu::IsNativeBackend(skgpu::ContextType type) {
    switch (type) {
        case ContextType::kDirect3D:
        case ContextType::kGL:
        case ContextType::kGLES:
        case ContextType::kMetal:
        case ContextType::kVulkan:
            return true;

        default:
            // Mock doesn't use the GPU, and Dawn and ANGLE add a layer between Skia and the native
            // GPU backend.
            return false;
    }
}

bool skgpu::IsRenderingContext(ContextType type) {
    return type != ContextType::kMock;
}

GrBackendApi skgpu::ganesh::ContextTypeBackend(skgpu::ContextType type) {
    switch (type) {
        case skgpu::ContextType::kGL:
        case skgpu::ContextType::kGLES:
        case skgpu::ContextType::kANGLE_D3D9_ES2:
        case skgpu::ContextType::kANGLE_D3D11_ES2:
        case skgpu::ContextType::kANGLE_D3D11_ES3:
        case skgpu::ContextType::kANGLE_GL_ES2:
        case skgpu::ContextType::kANGLE_GL_ES3:
        case skgpu::ContextType::kANGLE_Metal_ES2:
        case skgpu::ContextType::kANGLE_Metal_ES3:
            return GrBackendApi::kOpenGL;

        case ContextType::kVulkan:
            return GrBackendApi::kVulkan;

        case ContextType::kMetal:
            return GrBackendApi::kMetal;

        case ContextType::kDirect3D:
            return GrBackendApi::kDirect3D;

        case ContextType::kDawn:
        case ContextType::kDawn_D3D11:
        case ContextType::kDawn_D3D12:
        case ContextType::kDawn_Metal:
        case ContextType::kDawn_Vulkan:
        case ContextType::kDawn_OpenGL:
        case ContextType::kDawn_OpenGLES:
            return GrBackendApi::kUnsupported;

        case ContextType::kMock:
            return GrBackendApi::kMock;
    }
    SkUNREACHABLE;
}

skgpu::BackendApi skgpu::graphite::ContextTypeBackend(ContextType type) {
    switch (type) {
        case skgpu::ContextType::kGL:
        case skgpu::ContextType::kGLES:
        case skgpu::ContextType::kANGLE_D3D9_ES2:
        case skgpu::ContextType::kANGLE_D3D11_ES2:
        case skgpu::ContextType::kANGLE_D3D11_ES3:
        case skgpu::ContextType::kANGLE_GL_ES2:
        case skgpu::ContextType::kANGLE_GL_ES3:
        case skgpu::ContextType::kANGLE_Metal_ES2:
        case skgpu::ContextType::kANGLE_Metal_ES3:
        case skgpu::ContextType::kDirect3D:
            return BackendApi::kUnsupported;

        case ContextType::kVulkan:
            return BackendApi::kVulkan;

        case ContextType::kMetal:
            return BackendApi::kMetal;

        case ContextType::kDawn:
        case ContextType::kDawn_D3D11:
        case ContextType::kDawn_D3D12:
        case ContextType::kDawn_Metal:
        case ContextType::kDawn_Vulkan:
        case ContextType::kDawn_OpenGL:
        case ContextType::kDawn_OpenGLES:
            return BackendApi::kDawn;

        case ContextType::kMock:
            return BackendApi::kMock;
    }
    SkUNREACHABLE;
}
