/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "dawn/dawncpp.h"
#include "dawn_native/DawnNative.h"
#include "DawnTestContext.h"

#ifdef SK_BUILD_FOR_UNIX
#include "GL/glx.h"
#endif

#ifdef SK_BUILD_FOR_WIN
#include <windows.h>
#endif

#ifdef SK_DAWN
#include "dawn/dawn.h"
#ifdef SK_DAWN_OPENGL
#include "dawn_native/OpenGLBackend.h"
#endif
#ifdef SK_DAWN_D3D12
#include "dawn_native/D3D12Backend.h"
#endif
#ifdef SK_DAWN_VULKAN
#include <vector>
#include "dawn_native/VulkanBackend.h"
#endif

#include "GrContext.h"
#ifdef SK_DAWN_METAL
#include "DawnMTLUtils.h"
#endif

#if defined(SK_BUILD_FOR_MAC) && SK_DAWN_OPENGL
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
        if (proc = GetProcAddress(fModule, name)) {
            return (Proc) proc;
        }
        if (proc = wglGetProcAddress(name)) {
            return (Proc) proc;
        }
        return nullptr;
    }

    HMODULE fModule;
    static ProcGetter* fInstance;
};

ProcGetter* ProcGetter::fInstance;
#endif

class DawnFence {
public:
    DawnFence(const dawn::Device& device, const dawn::Buffer& buffer)
      : fDevice(device), fBuffer(buffer), fCalled(false) {
        fBuffer.MapReadAsync(0, 1, callback, reinterpret_cast<dawn::CallbackUserdata>(this));
    }

    bool wait() {
        while (!fCalled) {
            fDevice.Tick();
        }
        return true;
    }

    ~DawnFence() {
    }

    static void callback(dawnBufferMapAsyncStatus status, const void* data, dawnCallbackUserdata userData) {
        DawnFence* fence = reinterpret_cast<DawnFence*>(userData);
        fence->fCalled = true;
    }
    dawn::Buffer buffer() { return fBuffer; }

private:
    dawn::Device                   fDevice;
    dawn::Buffer                   fBuffer;
    bool                           fCalled;
};

/**
 * Implements sk_gpu_test::FenceSync for Dawn.
 */
class DawnFenceSync : public sk_gpu_test::FenceSync {
public:
    DawnFenceSync(dawn::Device device) : fDevice(device) {
#ifdef SK_DAWN_METAL
        fAutoreleasePool = CreateAutoreleasePool();
#endif
    }

    ~DawnFenceSync() override {
#ifdef SK_DAWN_METAL
        DestroyAutoreleasePool(fAutoreleasePool);
#endif
    }

    sk_gpu_test::PlatformFence SK_WARN_UNUSED_RESULT insertFence() const override {
        dawn::Buffer buffer;
        if (fBuffers.empty()) {
            dawn::BufferDescriptor desc;
            desc.usage = dawn::BufferUsageBit::MapRead | dawn::BufferUsageBit::TransferDst;
            desc.size = 1;
            buffer = fDevice.CreateBuffer(&desc);
        } else {
            buffer = fBuffers.back();
            fBuffers.pop_back();
        }
        DawnFence* fence = new DawnFence(fDevice, buffer);
        return reinterpret_cast<sk_gpu_test::PlatformFence>(fence);
    }

    bool waitFence(sk_gpu_test::PlatformFence opaqueFence) const override {
#ifdef SK_DAWN_METAL
        DrainAutoreleasePool(fAutoreleasePool);
        fAutoreleasePool = CreateAutoreleasePool();
#endif
        DawnFence* fence = reinterpret_cast<DawnFence*>(opaqueFence);
        return fence->wait();
    }

    void deleteFence(sk_gpu_test::PlatformFence opaqueFence) const override {
        DawnFence* fence = reinterpret_cast<DawnFence*>(opaqueFence);
        fBuffers.push_back(fence->buffer());
        delete fence;
    }

private:
    dawn::Device                      fDevice;
    mutable std::vector<dawn::Buffer> fBuffers;
#ifdef SK_DAWN_METAL
    mutable void*                     fAutoreleasePool;
#endif
    typedef sk_gpu_test::FenceSync INHERITED;
};

class DawnTestContextImpl : public sk_gpu_test::DawnTestContext {
public:
    static DawnTestContext* Create(DawnTestContext* sharedContext) {
        sk_sp<const GrDawnBackendContext> backendContext;
        if (sharedContext) {
            backendContext = sharedContext->getDawnBackendContext();
        } else {
            dawnDevice backendDevice;
            dawnProcTable backendProcs = dawn_native::GetProcs();
#ifdef SK_DAWN_OPENGL
            backendDevice = dawn_native::opengl::CreateDevice(
#if defined(SK_BUILD_FOR_UNIX)
                reinterpret_cast<void*(*)(const char*)>(glXGetProcAddress)
#elif defined(SK_BUILD_FOR_MAC)
                getProcAddressMacOS
#elif defined(SK_BUILD_FOR_WIN)
                ProcGetter::getProcAddress
#endif
            );
#elif SK_DAWN_METAL
            backendDevice = CreateDawnMTLSystemDefaultDevice();
#elif SK_DAWN_D3D12
            backendDevice = dawn_native::d3d12::CreateDevice();
#elif SK_DAWN_VULKAN
            std::vector<const char *> requiredInstanceExtensions;
            backendDevice = dawn_native::vulkan::CreateDevice(requiredInstanceExtensions);
#endif
            dawnSetProcs(&backendProcs);
            dawnQueue backendQueue = dawnDeviceCreateQueue(backendDevice);
            backendContext.reset(new GrDawnBackendContext(backendDevice, backendQueue));
        }
        if (!backendContext) {
            return nullptr;
        }
        return new DawnTestContextImpl(std::move(backendContext));
    }

    ~DawnTestContextImpl() override { this->teardown(); }

    void testAbandon() override {}

    // There is really nothing to here since we don't own any unqueued command buffers here.
    void submit() override {}

    void finish() override {}

    sk_sp<GrContext> makeGrContext(const GrContextOptions& options) override {
        return GrContext::MakeDawn(fDawn, options);
    }

protected:
    void teardown() override {
        INHERITED::teardown();
        fDawn.reset(nullptr);
    }

private:
    DawnTestContextImpl(sk_sp<const GrDawnBackendContext> backendContext)
            : DawnTestContext(std::move(backendContext)) {
        fFenceSync.reset(new DawnFenceSync(dawn::Device::Acquire(fDawn->fDevice)));
    }

    void onPlatformMakeCurrent() const override {}
    std::function<void()> onPlatformGetAutoContextRestore() const override  { return nullptr; }
    void onPlatformSwapBuffers() const override {}

    typedef sk_gpu_test::DawnTestContext INHERITED;
};
}  // anonymous namespace

namespace sk_gpu_test {
DawnTestContext* CreatePlatformDawnTestContext(DawnTestContext* sharedContext) {
    return DawnTestContextImpl::Create(sharedContext);
}
}  // namespace sk_gpu_test

#endif
