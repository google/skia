/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "NXTTestContext.h"

#ifdef SK_BUILD_FOR_UNIX
#include "GL/glx.h"
#endif

#ifdef SK_NXT_OPENGL
namespace backend {
    namespace opengl {
        void Init(void* (*getProc)(const char*), nxtProcTable* procs, nxtDevice* device);
    }
}
#endif
#ifdef SK_NXT_D3D12
namespace backend {
    namespace d3d12 {
        void Init(nxtProcTable* procs, nxtDevice* device);
    }
}
#endif
#ifdef SK_NXT_VULKAN
#include <vector>
namespace backend {
    namespace vulkan {
        void Init(nxtProcTable* procs,
                  nxtDevice* device,
                  const std::vector<const char*>& requiredInstanceExtensions);
    }
}
#endif

#include "GrContext.h"
#include "nxt/GrNXTUtil.h"
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

class NXTFence {
public:
    NXTFence(const nxt::Device& device, const nxt::Buffer& buffer)
      : fDevice(device.Clone()), fBuffer(buffer.Clone()), fCalled(false) {
        fBuffer.MapReadAsync(0, 1, callback, reinterpret_cast<nxt::CallbackUserdata>(this));
    }

    bool wait() {
        while (!fCalled) {
            fDevice.Tick();
        }
        return true;
    }

    ~NXTFence() {
    }

    static void callback(nxtBufferMapAsyncStatus status, const void* data, nxtCallbackUserdata userData) {
        NXTFence* fence = reinterpret_cast<NXTFence*>(userData);
        fence->fCalled = true;
    }
    nxt::Buffer buffer() { return fBuffer.Clone(); }

private:
    nxt::Device                    fDevice;
    nxt::Buffer                    fBuffer;
    bool                           fCalled;
};
/**
 * Implements sk_gpu_test::FenceSync for NXT.
 */
class NXTFenceSync : public sk_gpu_test::FenceSync {
public:
    NXTFenceSync(nxt::Device device) : fDevice(device.Clone()) {
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
        nxt::Buffer buffer;
        if (fBuffers.empty()) {
            buffer = fDevice.CreateBufferBuilder()
                .SetAllowedUsage(nxt::BufferUsageBit::MapRead | nxt::BufferUsageBit::TransferDst)
                .SetInitialUsage(nxt::BufferUsageBit::MapRead)
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
    nxt::Device                       fDevice;
    mutable std::vector<nxt::Buffer>  fBuffers;
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
            nxtDevice backendDevice;
            nxtProcTable backendProcs;
#ifdef SK_NXT_OPENGL
#if defined(SK_BUILD_FOR_UNIX)
            backend::opengl::Init(reinterpret_cast<void*(*)(const char*)>(glXGetProcAddress), &backendProcs, &backendDevice);
#elif defined(SK_BUILD_FOR_MAC)
            backend::opengl::Init(getProcAddressMacOS, &backendProcs, &backendDevice);
#endif
#elif SK_NXT_METAL
            InitNXTMTLSystemDefaultDevice(&backendProcs, &backendDevice);
#elif SK_NXT_D3D12
            backend::d3d12::Init(&backendProcs, &backendDevice);
#elif SK_NXT_VULKAN
            std::vector<const char *> requiredInstanceExtensions;
            backend::vulkan::Init(&backendProcs, &backendDevice, requiredInstanceExtensions);
#endif
            nxtSetProcs(&backendProcs);
            nxt::Device device = nxt::Device::Acquire(backendDevice);
            nxt::Queue queue = device.CreateQueue();
            backendContext.reset(new GrNXTBackendContext(device.Clone(), queue.Clone()));
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
        fFenceSync.reset(new NXTFenceSync(fNXT->fDevice.Clone()));
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
