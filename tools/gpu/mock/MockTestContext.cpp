
/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GLTestContext_DEFINED
#define GLTestContext_DEFINED

#include "MockTestContext.h"

namespace {

class MockTestContext : public sk_gpu_test::TestContext {
public:
    MockTestContext() {}
    ~MockTestContext() override {}

    virtual GrBackend backend() override { return kMock_GrBackend; }
    virtual GrBackendContext backendContext() override {
        return reinterpret_cast<GrBackendContext>(nullptr);
    }
    void testAbandon() override {}
    void submit() override {}
    void finish() override {}

protected:
    void teardown() override {}
    void onPlatformMakeCurrent() const override {}
    void onPlatformSwapBuffers() const override {}

private:
    typedef sk_gpu_test::TestContext INHERITED;
};

} // anonymous namespace

namespace sk_gpu_test {

TestContext* CreateMockTestContext(TestContext*) { return new MockTestContext(); }

}  // namespace sk_gpu_test
#endif
