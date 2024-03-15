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

#include "include/gpu/ganesh/mtl/GrMtlBackendContext.h"

namespace sk_gpu_test {
class MtlTestContext : public TestContext {
public:
    GrBackendApi backend() override { return GrBackendApi::kMetal; }

    const GrMtlBackendContext& getMtlBackendContext() const {
        return fMtl;
    }

protected:
    MtlTestContext(const GrMtlBackendContext& mtl)
            : fMtl(mtl) {}

    GrMtlBackendContext fMtl;

private:
    using INHERITED = TestContext;
};

/**
 * Creates Metal context object bound to the native Metal library.
 */
MtlTestContext* CreatePlatformMtlTestContext(MtlTestContext*);

}  // namespace sk_gpu_test

#endif

#endif /* MtlTestContext_h */
