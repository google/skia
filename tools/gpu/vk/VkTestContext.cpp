/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "VkTestContext.h"

#ifdef SK_VULKAN

namespace {
// TODO: Implement fence syncs and swap buffers
class VkTestContextImpl : public sk_gpu_test::VkTestContext {
public:
    VkTestContextImpl()
        : VkTestContext(sk_sp<const GrVkBackendContext>(GrVkBackendContext::Create())) {}

    ~VkTestContextImpl() override { this->teardown(); }

    void testAbandon() override {}

protected:
    void teardown() override { fVk.reset(nullptr); }

private:
    void onPlatformMakeCurrent() const override {}
    void onPlatformSwapBuffers() const override {}

    typedef sk_gpu_test::VkTestContext INHERITED;
};
}

namespace sk_gpu_test {
VkTestContext* CreatePlatformVkTestContext() {
    return new VkTestContextImpl;
}
}  // namespace sk_gpu_test

#endif
