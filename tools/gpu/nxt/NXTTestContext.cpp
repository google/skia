/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "dawn/dawncpp.h"
#include "NXTTestContext.h"

#ifdef SK_BUILD_FOR_UNIX
#include "GL/glx.h"
#endif

#ifdef SK_BUILD_FOR_WIN
#include <windows.h>
#endif

#ifdef SK_NXT
#include "dawn/dawn.h"

#ifdef SK_NXT_OPENGL
namespace backend {
    namespace opengl {
        void Init(void* (*getProc)(const char*), dawnProcTable* procs, dawnDevice* device);
    }
}
#endif
#ifdef SK_NXT_D3D12
namespace backend {
    namespace d3d12 {
        void Init(dawnProcTable* procs, dawnDevice* device);
    }
}
#endif
#ifdef SK_NXT_VULKAN
#include <vector>
namespace backend {
    namespace vulkan {
        void Init(dawnProcTable* procs,
                  dawnDevice* device,
                  const std::vector<const char*>& requiredInstanceExtensions);
    }
}
#endif

#include "GrContext.h"
#ifdef SK_NXT_METAL
#include "NXTMTLUtils.h"
#endif

#if defined(SK_BUILD_FOR_MAC) && SK_NXT_OPENGL
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

class NXTFence {
public:
    NXTFence(const dawn::Device& device, const dawn::Buffer& buffer)
      : fDevice(device.Clone()), fBuffer(buffer.Clone()), fCalled(false) {
        fBuffer.MapReadAsync(0, 1, callback, reinterpret_cast<dawn::CallbackUserdata>(this));
    }

    bool wait() {
        while (!fCalled) {
            fDevice.Tick();
        }
        return true;
    }

    ~NXTFence() {
    }

    static void callback(dawnBufferMapAsyncStatus status, const void* data, dawnCallbackUserdata userData) {
        NXTFence* fence = reinterpret_cast<NXTFence*>(userData);
        fence->fCalled = true;
    }
    dawn::Buffer buffer() { return fBuffer.Clone(); }

private:
    dawn::Device                   fDevice;
    dawn::Buffer                   fBuffer;
    bool                           fCalled;
};

/**
 * Implements sk_gpu_test::FenceSync for NXT.
 */
class NXTFenceSync : public sk_gpu_test::FenceSync {
public:
    NXTFenceSync(dawn::Device device) : fDevice(device.Clone()) {
#ifdef SK_NXT_METAL
        fAutoreleasePool = CreateAutoreleasePool();
#endif
    }

    ~NXTFenceSync() override {
#ifdef SK_NXT_METAL
        DestroyAutoreleasePool(fAutoreleasePool);
#endif
    }

    sk_gpu_test::PlatformFence SK_WARN_UNUSED_RESULT insertFence() const override {
        dawn::Buffer buffer;
        if (fBuffers.empty()) {
            buffer = fDevice.CreateBufferBuilder()
                .SetAllowedUsage(dawn::BufferUsageBit::MapRead | dawn::BufferUsageBit::TransferDst)
                .SetSize(1)
                .GetResult();
        } else {
            buffer = fBuffers.back().Clone();
            fBuffers.pop_back();
        }
        NXTFence* fence = new NXTFence(fDevice, buffer);
        return reinterpret_cast<sk_gpu_test::PlatformFence>(fence);
    }

    bool waitFence(sk_gpu_test::PlatformFence opaqueFence) const override {
#ifdef SK_NXT_METAL
        DrainAutoreleasePool(fAutoreleasePool);
        fAutoreleasePool = CreateAutoreleasePool();
#endif
        NXTFence* fence = reinterpret_cast<NXTFence*>(opaqueFence);
        return fence->wait();
    }

    void deleteFence(sk_gpu_test::PlatformFence opaqueFence) const override {
        NXTFence* fence = reinterpret_cast<NXTFence*>(opaqueFence);
        fBuffers.push_back(fence->buffer());
        delete fence;
    }

private:
    dawn::Device                      fDevice;
    mutable std::vector<dawn::Buffer> fBuffers;
#ifdef SK_NXT_METAL
    mutable void*                     fAutoreleasePool;
#endif
    typedef sk_gpu_test::FenceSync INHERITED;
};

class NXTTestContextImpl : public sk_gpu_test::NXTTestContext {
public:
    static NXTTestContext* Create(NXTTestContext* sharedContext) {
        sk_sp<const GrNXTBackendContext> backendContext;
        if (sharedContext) {
            backendContext = sharedContext->getNXTBackendContext();
        } else {
            dawnDevice backendDevice;
            dawnProcTable backendProcs;
#ifdef SK_NXT_OPENGL
#if defined(SK_BUILD_FOR_UNIX)
            backend::opengl::Init(reinterpret_cast<void*(*)(const char*)>(glXGetProcAddress), &backendProcs, &backendDevice);
#elif defined(SK_BUILD_FOR_MAC)
            backend::opengl::Init(getProcAddressMacOS, &backendProcs, &backendDevice);
#elif defined(SK_BUILD_FOR_WIN)
            backend::opengl::Init(ProcGetter::getProcAddress, &backendProcs, &backendDevice);
#endif
#elif SK_NXT_METAL
            InitNXTMTLSystemDefaultDevice(&backendProcs, &backendDevice);
#elif SK_NXT_D3D12
            backend::d3d12::Init(&backendProcs, &backendDevice);
#elif SK_NXT_VULKAN
            std::vector<const char *> requiredInstanceExtensions;
            backend::vulkan::Init(&backendProcs, &backendDevice, requiredInstanceExtensions);
#endif
            dawnSetProcs(&backendProcs);
            dawnQueue backendQueue = dawnDeviceCreateQueue(backendDevice);
            backendContext.reset(new GrNXTBackendContext(backendDevice, backendQueue));
        }
        if (!backendContext) {
            return nullptr;
        }
        return new NXTTestContextImpl(std::move(backendContext));
    }

    ~NXTTestContextImpl() override { this->teardown(); }

    void testAbandon() override {}

    // There is really nothing to here since we don't own any unqueued command buffers here.
    void submit() override {}

    void finish() override {}

    sk_sp<GrContext> makeGrContext(const GrContextOptions& options) override {
        return GrContext::MakeNXT(fNXT, options);
    }

protected:
    void teardown() override {
        INHERITED::teardown();
        fNXT.reset(nullptr);
    }

private:
    NXTTestContextImpl(sk_sp<const GrNXTBackendContext> backendContext)
            : NXTTestContext(std::move(backendContext)) {
        fFenceSync.reset(new NXTFenceSync(dawn::Device::Acquire(fNXT->fDevice)));
    }

    void onPlatformMakeCurrent() const override {}
    std::function<void()> onPlatformGetAutoContextRestore() const override  { return nullptr; }
    void onPlatformSwapBuffers() const override {}

    typedef sk_gpu_test::NXTTestContext INHERITED;
};
}  // anonymous namespace

namespace sk_gpu_test {
NXTTestContext* CreatePlatformNXTTestContext(NXTTestContext* sharedContext) {
    return NXTTestContextImpl::Create(sharedContext);
}
}  // namespace sk_gpu_test

#endif
