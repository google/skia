/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "webgpu/webgpu_cpp.h"
#include "tools/gpu/dawn/DawnTestContext.h"

#ifdef SK_BUILD_FOR_UNIX
#include "GL/glx.h"
#endif

#ifdef SK_BUILD_FOR_WIN
#include <windows.h>
#endif

#define USE_OPENGL_BACKEND 0

#ifdef SK_DAWN
#include "webgpu/webgpu.h"
#include "dawn/dawn_proc.h"
#include "include/gpu/GrDirectContext.h"
#include "tools/AutoreleasePool.h"
#if USE_OPENGL_BACKEND
#include "dawn/native/OpenGLBackend.h"
#elif defined(SK_BUILD_FOR_MAC)
#include "dawn/native/MetalBackend.h"
#elif defined(SK_BUILD_FOR_WIN)
#include "dawn/native/D3D12Backend.h"
#elif defined(SK_BUILD_FOR_UNIX) || (defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26)
#include "dawn/native/VulkanBackend.h"
#endif

#if defined(SK_BUILD_FOR_MAC) && USE_OPENGL_BACKEND
#include <dlfcn.h>
static void* getProcAddressMacOS(const char* procName) {
    return dlsym(RTLD_DEFAULT, procName);
}
#endif

namespace {

#ifdef SK_BUILD_FOR_WIN
class ProcGetter {
public:
    typedef void(*Proc)();

    ProcGetter()
      : fModule(LoadLibraryA("opengl32.dll")) {
        SkASSERT(!fInstance);
        fInstance = this;
    }

    ~ProcGetter() {
        if (fModule) {
            FreeLibrary(fModule);
        }
        fInstance = nullptr;
    }

    static void* getProcAddress(const char* name) {
        return fInstance->getProc(name);
    }

private:
    Proc getProc(const char* name) {
        PROC proc;
        if ((proc = GetProcAddress(fModule, name))) {
            return (Proc) proc;
        }
        if ((proc = wglGetProcAddress(name))) {
            return (Proc) proc;
        }
        return nullptr;
    }

    HMODULE fModule;
    static ProcGetter* fInstance;
};

ProcGetter* ProcGetter::fInstance;
#endif

static void PrintDeviceError(WGPUErrorType, const char* message, void*) {
    SkDebugf("Device error: %s\n", message);
}

static void PrintDeviceLostMessage(WGPUDeviceLostReason reason, const char* message, void*) {
    if (reason != WGPUDeviceLostReason_Destroyed) {
        SkDebugf("Device lost: %s\n", message);
    }
}

class DawnTestContextImpl : public sk_gpu_test::DawnTestContext {
public:
    static wgpu::Device createDevice(const dawn::native::Instance& instance,
                                     wgpu::BackendType type) {
        DawnProcTable backendProcs = dawn::native::GetProcs();
        dawnProcSetProcs(&backendProcs);

        std::vector<dawn::native::Adapter> adapters = instance.GetAdapters();
        for (dawn::native::Adapter adapter : adapters) {
            wgpu::AdapterProperties properties;
            adapter.GetProperties(&properties);
            if (properties.backendType == type) {
                return wgpu::Device::Acquire(adapter.CreateDevice());
            }
        }
        return nullptr;
    }

    static DawnTestContext* Create(DawnTestContext* sharedContext) {
        std::unique_ptr<dawn::native::Instance> instance = std::make_unique<dawn::native::Instance>();
        wgpu::Device device;
        if (sharedContext) {
            device = sharedContext->getDevice();
        } else {
            wgpu::BackendType type;
#if USE_OPENGL_BACKEND
            type = wgpu::BackendType::OpenGL;
            dawn::native::opengl::AdapterDiscoveryOptions adapterOptions(
                    static_cast<WGPUBackendType>(type));
            adapterOptions.getProc = reinterpret_cast<void*(*)(const char*)>(
#if defined(SK_BUILD_FOR_UNIX)
                glXGetProcAddress
#elif defined(SK_BUILD_FOR_MAC)
                getProcAddressMacOS
#elif defined(SK_BUILD_FOR_WIN)
                ProcGetter::getProcAddress
#endif
            );
#else  // !USE_OPENGL_BACKEND
#if defined(SK_BUILD_FOR_MAC)
            type = wgpu::BackendType::Metal;
            dawn::native::metal::AdapterDiscoveryOptions adapterOptions;
#elif defined(SK_BUILD_FOR_WIN)
            type = wgpu::BackendType::D3D12;
            dawn::native::d3d12::AdapterDiscoveryOptions adapterOptions;
#elif defined(SK_BUILD_FOR_UNIX) || (defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26)
            type = wgpu::BackendType::Vulkan;
            dawn::native::vulkan::AdapterDiscoveryOptions adapterOptions;
#endif
#endif  // USE_OPENGL_BACKEND
            instance->DiscoverAdapters(&adapterOptions);
            device = createDevice(*instance, type);
            if (device) {
                device.SetUncapturedErrorCallback(PrintDeviceError, 0);
                device.SetDeviceLostCallback(PrintDeviceLostMessage, 0);
            }
        }
        if (!device) {
            return nullptr;
        }
        return new DawnTestContextImpl(std::move(instance), device);
    }

    ~DawnTestContextImpl() override { this->teardown(); }

    void testAbandon() override {}

    void finish() override {}

    sk_sp<GrDirectContext> makeContext(const GrContextOptions& options) override {
        return GrDirectContext::MakeDawn(fDevice, options);
    }

protected:
    void teardown() override {
        INHERITED::teardown();
    }

private:
    DawnTestContextImpl(std::unique_ptr<dawn::native::Instance> instance,
                        const wgpu::Device& device)
            : DawnTestContext(std::move(instance), device) {
        fFenceSupport = true;
    }

    void onPlatformMakeNotCurrent() const override {}
    void onPlatformMakeCurrent() const override {}
    std::function<void()> onPlatformGetAutoContextRestore() const override  { return nullptr; }

    using INHERITED = sk_gpu_test::DawnTestContext;
};
}  // anonymous namespace

namespace sk_gpu_test {
DawnTestContext* CreatePlatformDawnTestContext(DawnTestContext* sharedContext) {
    return DawnTestContextImpl::Create(sharedContext);
}
}  // namespace sk_gpu_test

#endif
