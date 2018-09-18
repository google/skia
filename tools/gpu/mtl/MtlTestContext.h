/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef MtlTestContext_h
#define MtlTestContext_h

#include "TestContext.h"

#ifdef SK_METAL

namespace sk_gpu_test {
TestContext* CreatePlatformMtlTestContext(TestContext*);
}  // namespace sk_gpu_test

#endif


#endif /* MtlTestContext_h */
