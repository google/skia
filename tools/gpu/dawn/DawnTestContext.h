/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DawnTestContext_DEFINED
#define DawnTestContext_DEFINED

#include "tools/gpu/TestContext.h"
#include "dawn/native/DawnNative.h"

#ifdef SK_DAWN

namespace sk_gpu_test {
class DawnTestContext : public TestContext {
public:
    virtual GrBackend backend() override { return GrBackendApi::kDawn; }

    const wgpu::Device& getDevice() {
        return fDevice;
    }

protected:
    DawnTestContext(std::unique_ptr<dawn::native::Instance> instance, const wgpu::Device& device)
        : fInstance(std::move(instance)), fDevice(device) {}

    std::unique_ptr<dawn::native::Instance> fInstance;
    wgpu::Device fDevice;

private:
    using INHERITED = TestContext;
};

/**
 * Creates Dawn context object bound to the Dawn library.
 */
DawnTestContext* CreatePlatformDawnTestContext(DawnTestContext*);

}  // namespace sk_gpu_test

#endif

#endif
