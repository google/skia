/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/dsl/DSL.h"
#include "src/sksl/dsl/priv/DSLWriter.h"

#include "tests/Test.h"

using namespace SkSL::dsl;

class AutoDSLContext {
public:
    AutoDSLContext(GrGpu* gpu) {
        Start(gpu->shaderCompiler());
        DSLWriter::Instance().fMangle = false;
    }

    ~AutoDSLContext() {
        End();
    }
};

class ExpectError : public ErrorHandler {
public:
    ExpectError(skiatest::Reporter* reporter, const char* msg)
        : fMsg(msg)
        , fReporter(reporter) {
        SetErrorHandler(this);
    }

    ~ExpectError() override {
        REPORTER_ASSERT(fReporter, !fMsg);
        SetErrorHandler(nullptr);
    }

    void handleError(const char* msg) override {
        REPORTER_ASSERT(fReporter, !strcmp(msg, fMsg),
                        "Error mismatch: expected:\n%sbut received:\n%s", fMsg, msg);
        fMsg = nullptr;
    }

private:
    const char* fMsg;
    skiatest::Reporter* fReporter;
};

DEF_GPUTEST_FOR_ALL_CONTEXTS(DSLPlus, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    // Just a basic sanity test for now; full testing coming once DSLVar is added
    Expression e1 = Expression(1) + Expression(2);
    REPORTER_ASSERT(r, e1.release()->description() == "3");
}
