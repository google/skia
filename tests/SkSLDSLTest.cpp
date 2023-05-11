/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrDirectContext.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/dsl/DSL.h"
#include "src/sksl/dsl/DSLCore.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "tests/Test.h"

#include <ctype.h>
#include <memory>
#include <string>

using namespace skia_private;

struct GrContextOptions;

using namespace SkSL::dsl;

SkSL::ProgramSettings default_settings() {
    return SkSL::ProgramSettings{};
}

/**
 * Issues an automatic Start() and End().
 */
class AutoDSLContext {
public:
    AutoDSLContext(GrGpu* gpu, SkSL::ProgramSettings settings = default_settings(),
                   SkSL::ProgramKind kind = SkSL::ProgramKind::kFragment) {
        Start(gpu->shaderCompiler(), kind, settings);
    }

    ~AutoDSLContext() {
        End();
    }
};

static bool whitespace_insensitive_compare(const char* a, const char* b) {
    for (;;) {
        while (isspace(*a)) {
            ++a;
        }
        while (isspace(*b)) {
            ++b;
        }
        if (*a != *b) {
            return false;
        }
        if (*a == 0) {
            return true;
        }
        ++a;
        ++b;
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLStartup, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Expression e1 = 1;
    REPORTER_ASSERT(r, e1.release()->description() == "1");
    Expression e2 = 1.0;
    REPORTER_ASSERT(r, e2.release()->description() == "1.0");
    Expression e3 = true;
    REPORTER_ASSERT(r, e3.release()->description() == "true");

    REPORTER_ASSERT(r, whitespace_insensitive_compare("", ""));
    REPORTER_ASSERT(r, !whitespace_insensitive_compare("", "a"));
    REPORTER_ASSERT(r, !whitespace_insensitive_compare("a", ""));
    REPORTER_ASSERT(r, whitespace_insensitive_compare("a", "a"));
    REPORTER_ASSERT(r, whitespace_insensitive_compare("abc", "abc"));
    REPORTER_ASSERT(r, whitespace_insensitive_compare("abc", " abc "));
    REPORTER_ASSERT(r, whitespace_insensitive_compare("a b  c  ", "\n\n\nabc"));
    REPORTER_ASSERT(r, !whitespace_insensitive_compare("a b c  d", "\n\n\nabc"));
}
