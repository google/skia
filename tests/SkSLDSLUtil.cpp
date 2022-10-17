/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkTypes.h"

#ifdef SK_ENABLE_SKSL
#include "include/gpu/GrDirectContext.h"
#include "include/sksl/DSLCore.h"
#include "include/sksl/DSLExpression.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "tools/gpu/GrContextFactory.h"

using namespace SkSL::dsl;

// for use from SkSLDSLOnlyTest.cpp
void StartDSL(const sk_gpu_test::ContextInfo ctxInfo) {
    Start(ctxInfo.directContext()->priv().getGpu()->shaderCompiler());
}
#endif
