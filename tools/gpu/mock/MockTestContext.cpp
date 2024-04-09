
/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GLTestContext_DEFINED
#define GLTestContext_DEFINED

#include "tools/gpu/mock/MockTestContext.h"

#include "include/gpu/GrDirectContext.h"

namespace {

class MockTestContext : public sk_gpu_test::TestContext {
public:
    MockTestContext() {}
    ~MockTestContext() override {}

    GrBackendApi backend() override { return GrBackendApi::kMock; }

    void testAbandon() override {}

    sk_sp<GrDirectContext> makeContext(const GrContextOptions& options) override {
        return GrDirectContext::MakeMock(nullptr, options);
    }

protected:
    void teardown() override {}
    void onPlatformMakeNotCurrent() const override {}
    void onPlatformMakeCurrent() const override {}
    std::function<void()> onPlatformGetAutoContextRestore() const override { return nullptr; }

private:
    using INHERITED = sk_gpu_test::TestContext;
};

} // anonymous namespace

namespace sk_gpu_test {

TestContext* CreateMockTestContext(TestContext*) { return new MockTestContext(); }

}  // namespace sk_gpu_test
#endif
