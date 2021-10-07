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
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/dsl/priv/DSLWriter.h"

#include "tests/Test.h"

#include <limits>

using namespace SkSL::dsl;

#if defined(__GNUC__) || defined(__clang__)

class ExpectErrorLineNumber : public SkSL::ErrorReporter {
public:
    ExpectErrorLineNumber(skiatest::Reporter* reporter, const char* msg, int line)
        : fMsg(msg)
        , fLine(line)
        , fReporter(reporter)
        , fOldReporter(&GetErrorReporter()) {
        SetErrorReporter(this);
    }

    ~ExpectErrorLineNumber() override {
        REPORTER_ASSERT(fReporter, !fMsg);
        SetErrorReporter(fOldReporter);
    }

    void handleError(skstd::string_view msg, SkSL::PositionInfo pos) override {
        REPORTER_ASSERT(fReporter, msg == fMsg,
                "Error mismatch: expected:\n%sbut received:\n%.*s", fMsg, (int)msg.length(),
                msg.data());
        REPORTER_ASSERT(fReporter, pos.line() == fLine,
                "Line number mismatch: expected %d, but received %d\n", fLine, pos.line());
        SkSL::ThreadContext::Compiler().handleError(msg, pos);
        fMsg = nullptr;
    }

private:
    const char* fMsg;
    int fLine;
    skiatest::Reporter* fReporter;
    ErrorReporter* fOldReporter;
};

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLErrorLineNumbers, r, ctxInfo) {
    Start(ctxInfo.directContext()->priv().getGpu()->shaderCompiler());
    {
        ExpectErrorLineNumber error(r, "type mismatch: '+' cannot operate on 'float', 'bool'",
                                    __LINE__ + 1);
        (Float(1) + true).release();
    }

    {
        Var a(kBool_Type);
        DSLWriter::MarkDeclared(a);
        ExpectErrorLineNumber error(r, "type mismatch: '=' cannot operate on 'bool', 'float'",
                                    __LINE__ + 1);
        (a = 5.0f).release();
    }

    {
        Var a(Array(kInt_Type, 5));
        DSLWriter::MarkDeclared(a);
        ExpectErrorLineNumber error(r, "expected 'int', but found 'bool'", __LINE__ + 1);
        (a[true]).release();
    }

    {
        Var a(Array(kInt_Type, 5));
        DSLWriter::MarkDeclared(a);
        ExpectErrorLineNumber error(r, "'++' cannot operate on 'int[5]'", __LINE__ + 1);
        (++a).release();
    }

    {
        ExpectErrorLineNumber error(r, "expected 'bool', but found 'int'", __LINE__ + 1);
        Do(Discard(), 5).release();
    }

    {
        ExpectErrorLineNumber error(r, "expected 'bool', but found 'int'", __LINE__ + 1);
        For(DSLStatement(), 5, DSLExpression(), Block()).release();
    }

    {
        ExpectErrorLineNumber error(r, "expected 'bool', but found 'int'", __LINE__ + 1);
        If(5, Discard()).release();
    }

    {
        ExpectErrorLineNumber error(r, "expected 'bool', but found 'int'", __LINE__ + 1);
        While(5, Discard()).release();
    }

    {
        ExpectErrorLineNumber error(r, "no match for abs(bool)", __LINE__ + 1);
        Abs(true).release();
    }
    End();
}

#endif // defined(__GNUC__) || defined(__clang__)
