/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSL.h"
#include "tests/Test.h"

#include <string_view>

struct GrContextOptions;

// This file verifies that DSL code compiles with only a DSL.h import. We don't bother with any
// 'real' tests here, as those are all in SkSLDSLTest.cpp.
// IWYU pragma: no_include "include/sksl/DSLCore.h"
// IWYU pragma: no_include "include/sksl/DSLExpression.h"
// IWYU pragma: no_include "include/sksl/DSLStatement.h"
// IWYU pragma: no_include "include/sksl/DSLType.h"

using namespace SkSL::dsl;

// Defined in SkSLDSLUtil.cpp (so that we don't have to put the required extra includes here)
void StartDSL(const sk_gpu_test::ContextInfo ctxInfo);

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLImportOnly, r, ctxInfo) {
    StartDSL(ctxInfo);
    Parameter x(kInt_Type, "x");
    Function(kInt_Type, "test", x).define(
        If(x >= 0,
            Block(Return(x)),
            Block(Return(-x)))
    );
    End();
}
