/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef D3dTestContext_DEFINED
#define D3dTestContext_DEFINED

#include "tools/gpu/TestContext.h"

#ifdef SK_DIRECT3D

#include "include/gpu/d3d/GrD3dBackendContext.h"

namespace sk_gpu_test {
class D3dTestContext : public TestContext {
public:
    virtual GrBackendApi backend() override { return GrBackendApi::kDirect3D; }

protected:
    D3dTestContext(const GrD3dBackendContext& d3d, bool ownsContext)
            : fD3d(d3d)
            , fOwnsContext(ownsContext) {}

    GrD3dBackendContext fD3d;
    bool fOwnsContext;

private:
    typedef TestContext INHERITED;
};

/**
 * Creates D3D context object bound to the native D3D library.
 */
D3dTestContext* CreatePlatformD3dTestContext(D3dTestContext*);

}  // namespace sk_gpu_test

#endif

#endif
