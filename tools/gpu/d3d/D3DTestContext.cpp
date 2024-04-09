/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/d3d/D3DTestContext.h"

#ifdef SK_DIRECT3D

#include "include/gpu/GrDirectContext.h"
#include "tools/gpu/d3d/D3DTestUtils.h"

namespace {

class D3DTestContextImpl : public sk_gpu_test::D3DTestContext {
public:
    static D3DTestContext* Create(D3DTestContext* sharedContext) {
        GrD3DBackendContext backendContext;
        bool ownsContext;
        if (sharedContext) {
            // take from the given context
            ownsContext = false;
            backendContext = sharedContext->getD3DBackendContext();
        } else {
            // create our own
            if (!sk_gpu_test::CreateD3DBackendContext(&backendContext)) {
                return nullptr;
            }

            ownsContext = true;
        }
        return new D3DTestContextImpl(backendContext, ownsContext);
    }

    ~D3DTestContextImpl() override { this->teardown(); }

    void testAbandon() override {}

    sk_sp<GrDirectContext> makeContext(const GrContextOptions& options) override {
        return GrDirectContext::MakeDirect3D(fD3D, options);
    }

protected:
    void teardown() override {
        INHERITED::teardown();
        if (fOwnsContext) {
            // delete all the D3D objects in the backend context
        }
    }

private:
    D3DTestContextImpl(const GrD3DBackendContext& backendContext, bool ownsContext)
            : D3DTestContext(backendContext, ownsContext) {
        fFenceSupport = true;
    }

    void onPlatformMakeNotCurrent() const override {}
    void onPlatformMakeCurrent() const override {}
    std::function<void()> onPlatformGetAutoContextRestore() const override  { return nullptr; }

    using INHERITED = sk_gpu_test::D3DTestContext;
};
}  // anonymous namespace

namespace sk_gpu_test {
D3DTestContext* CreatePlatformD3DTestContext(D3DTestContext* sharedContext) {
    return D3DTestContextImpl::Create(sharedContext);
}
}  // namespace sk_gpu_test

#endif
