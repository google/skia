/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSL.h"
#include "include/sksl/DSLBlock.h"

#include "tests/Test.h"

#include <utility>

namespace sk_gpu_test { class ContextInfo; }

// This file verifies that DSL code compiles with only a DSL.h import. We don't bother with any
// 'real' tests here, as those are all in SkSLDSLTest.cpp.

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
