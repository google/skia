/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef MtlTestContext_h
#define MtlTestContext_h

#include "tools/gpu/TestContext.h"

#ifdef SK_METAL

namespace sk_gpu_test {
class MtlTestContext : public TestContext {
public:
    GrBackendApi backend() override { return GrBackendApi::kMetal; }

protected:
    MtlTestContext() {}

private:
    typedef TestContext INHERITED;
};

/**
 * Creates Metal context object bound to the native Metal library.
 */
MtlTestContext* CreatePlatformMtlTestContext(MtlTestContext*);

}  // namespace sk_gpu_test

#endif

#endif /* MtlTestContext_h */
