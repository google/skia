/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef NXTTestContext_DEFINED
#define NXTTestContext_DEFINED

#include "TestContext.h"

#ifdef SK_NXT

#include "nxt/GrNXTBackendContext.h"

namespace sk_gpu_test {
class NXTTestContext : public TestContext {
public:
    virtual GrBackend backend() override { return kNXT_GrBackend; }
    virtual GrBackendContext backendContext() override {
        return reinterpret_cast<GrBackendContext>(fNXT.get());
    }

    sk_sp<const GrNXTBackendContext> getNXTBackendContext() {
        return fNXT;
    }

protected:
    NXTTestContext(sk_sp<const GrNXTBackendContext> nxt) : fNXT(std::move(nxt)) {}

    sk_sp<const GrNXTBackendContext> fNXT;

private:
    typedef TestContext INHERITED;
};

/**
 * Creates NXT context object bound to the NXT library.
 */
NXTTestContext* CreatePlatformNXTTestContext(NXTTestContext*);

}  // namespace sk_gpu_test

#endif

#endif
