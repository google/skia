/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DawnTestContext_DEFINED
#define DawnTestContext_DEFINED

#include "TestContext.h"

#ifdef SK_DAWN

#include "dawn/GrDawnBackendContext.h"

namespace sk_gpu_test {
class DawnTestContext : public TestContext {
public:
    virtual GrBackend backend() override { return kDawn_GrBackend; }

    sk_sp<const GrDawnBackendContext> getDawnBackendContext() {
        return fDawn;
    }

protected:
    DawnTestContext(sk_sp<const GrDawnBackendContext> dawn) : fDawn(std::move(dawn)) {}

    sk_sp<const GrDawnBackendContext> fDawn;

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
