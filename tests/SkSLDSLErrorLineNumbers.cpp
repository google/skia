/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLIRNode.h"
#include "include/sksl/DSL.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/dsl/priv/DSLWriter.h"

#include "tests/Test.h"

#include <limits>

using namespace SkSL::dsl;

#if defined(__GNUC__) || defined(__clang__)

class ExpectErrorLineNumber : public ErrorHandler {
public:
    ExpectErrorLineNumber(skiatest::Reporter* reporter, const char* msg, int line)
        : fMsg(msg)
        , fLine(line)
        , fReporter(reporter) {
        SetErrorHandler(this);
    }

    ~ExpectErrorLineNumber() override {
        REPORTER_ASSERT(fReporter, !fMsg);
        SetErrorHandler(nullptr);
    }

    void handleError(const char* msg, PositionInfo pos) override {
        REPORTER_ASSERT(fReporter, !strcmp(msg, fMsg),
                        "Error mismatch: expected:\n%sbut received:\n%s", fMsg, msg);
        REPORTER_ASSERT(fReporter, pos.line() == fLine,
                        "Line number mismatch: expected %d, but received %d\n", fLine, pos.line());
        fMsg = nullptr;
    }

private:
    const char* fMsg;
    int fLine;
    skiatest::Reporter* fReporter;
};

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLErrorLineNumbers, r, ctxInfo) {
    Start(ctxInfo.directContext()->priv().getGpu()->shaderCompiler());
    {
        ExpectErrorLineNumber error(r,
                                    "error: type mismatch: '+' cannot operate on 'float', 'bool'\n",
                                    __LINE__ + 1);
        (Float(1) + true).release();
    }

    {
        Var a(kBool_Type);
        DSLWriter::MarkDeclared(a);
        ExpectErrorLineNumber error(r,
                                    "error: type mismatch: '=' cannot operate on 'bool', 'float'\n",
                                    __LINE__ + 1);
        (a = 5.0f).release();
    }

    {
        Var a(Array(kInt_Type, 5));
        DSLWriter::MarkDeclared(a);
        ExpectErrorLineNumber error(r,
                                    "error: expected 'int', but found 'bool'\n",
                                    __LINE__ + 1);
        (a[true]).release();
    }

    {
        Var a(Array(kInt_Type, 5));
        DSLWriter::MarkDeclared(a);
        ExpectErrorLineNumber error(r,
                                    "error: '++' cannot operate on 'int[5]'\n",
                                    __LINE__ + 1);
        (++a).release();
    }

    {
        ExpectErrorLineNumber error(r,
                                    "error: expected 'bool', but found 'int'\n",
                                    __LINE__ + 1);
        Do(Discard(), 5).release();
    }

    {
        ExpectErrorLineNumber error(r,
                                    "error: expected 'bool', but found 'int'\n",
                                    __LINE__ + 1);
        For(DSLStatement(), 5, DSLExpression(), Block()).release();
    }

    {
        ExpectErrorLineNumber error(r,
                                    "error: expected 'bool', but found 'int'\n",
                                    __LINE__ + 1);
        If(5, Discard()).release();
    }

    {
        ExpectErrorLineNumber error(r,
                                    "error: expected 'bool', but found 'int'\n",
                                    __LINE__ + 1);
        While(5, Discard()).release();
    }

    {
        ExpectErrorLineNumber error(r,
                                    "error: no match for abs(bool)\n",
                                    __LINE__ + 1);
        Abs(true).release();
    }
    End();
}

#endif // defined(__GNUC__) || defined(__clang__)
