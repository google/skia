
/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GLTestContext_DEFINED
#define GLTestContext_DEFINED

#include "tools/gpu/mock/MockTestContext.h"

#include "include/gpu/GrContext.h"

namespace {

class MockTestContext : public sk_gpu_test::TestContext {
public:
    MockTestContext() {}
    ~MockTestContext() override {}

    virtual GrBackendApi backend() override { return GrBackendApi::kMock; }

    void testAbandon() override {}
    void submit() override {}
    void finish() override {}

    sk_sp<GrContext> makeGrContext(const GrContextOptions& options) override {
        return GrContext::MakeMock(nullptr, options);
    }

protected:
    void teardown() override {}
    void onPlatformMakeCurrent() const override {}
    std::function<void()> onPlatformGetAutoContextRestore() const override { return nullptr; }
    void onPlatformSwapBuffers() const override {}

private:
    typedef sk_gpu_test::TestContext INHERITED;
};

} // anonymous namespace

namespace sk_gpu_test {

TestContext* CreateMockTestContext(TestContext*) { return new MockTestContext(); }

}  // namespace sk_gpu_test
#endif
