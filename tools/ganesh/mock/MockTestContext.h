
/*
 * Copyright 2017 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef MockTestContext_DEFINED
#define MockTestContext_DEFINED

#include "tools/ganesh/TestContext.h"
#include <memory>

class SkContext;
struct SkContextOptions;

namespace sk_gpu_test {

/**
 * Creates mock context object for use with GrContexts created with GrBackendApi::kMock. It will
 * trivially succeed at everything.
 */
TestContext* CreateMockTestContext(TestContext* shareContext = nullptr);

}  // namespace sk_gpu_test

namespace SkContexts {

// Creates a context wrapping a Ganesh GPU backend with Mock
std::unique_ptr<SkContext> MakeGanesh(const SkContextOptions& options,
                            sk_gpu_test::TestContext* shareContext = nullptr);

}  // namespace SkContexts

#endif
