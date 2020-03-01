/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef D3DTestContext_DEFINED
#define D3DTestContext_DEFINED

#include "tools/gpu/TestContext.h"

#ifdef SK_DIRECT3D

#include "include/gpu/d3d/GrD3DBackendContext.h"

namespace sk_gpu_test {
class D3DTestContext : public TestContext {
public:
    virtual GrBackendApi backend() override { return GrBackendApi::kDirect3D; }

    const GrD3DBackendContext& getD3DBackendContext() const {
        return fD3D;
    }

protected:
    D3DTestContext(const GrD3DBackendContext& d3d, bool ownsContext)
            : fD3D(d3d)
            , fOwnsContext(ownsContext) {}

    GrD3DBackendContext fD3D;
    bool fOwnsContext;

private:
    typedef TestContext INHERITED;
};

/**
 * Creates D3D context object bound to the native D3D library.
 */
D3DTestContext* CreatePlatformD3DTestContext(D3DTestContext*);

}  // namespace sk_gpu_test

#endif

#endif
