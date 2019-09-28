/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DawnTestContext_DEFINED
#define DawnTestContext_DEFINED

#include "tools/gpu/TestContext.h"

#ifdef SK_DAWN

namespace sk_gpu_test {
class DawnTestContext : public TestContext {
public:
    virtual GrBackend backend() override { return GrBackendApi::kDawn; }

    const dawn::Device& getDevice() {
        return fDevice;
    }

protected:
    DawnTestContext(const dawn::Device& device) : fDevice(device) {}

    dawn::Device fDevice;

private:
    typedef TestContext INHERITED;
};

/**
 * Creates Dawn context object bound to the Dawn library.
 */
DawnTestContext* CreatePlatformDawnTestContext(DawnTestContext*);

}  // namespace sk_gpu_test

#endif

#endif
