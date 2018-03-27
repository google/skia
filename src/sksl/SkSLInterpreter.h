/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_INTERPRETER
#define SKSL_INTERPRETER

#include "ir/SkSLAppendStage.h"
#include "ir/SkSLExpression.h"
#include "ir/SkSLFunctionCall.h"
#include "ir/SkSLFunctionDefinition.h"
#include "ir/SkSLProgram.h"
#include "ir/SkSLStatement.h"

#include <stack>

class SkRasterPipeline;

namespace SkSL {

class Interpreter {
    typedef int StackIndex;

    struct StatementIndex {
        const Statement* fStatement;
        size_t fIndex;
    };

public:
    union Value {
        Value(float f)
        : fFloat(f) {}

        Value(int i)
        : fInt(i) {}

        Value(bool b)
        : fBool(b) {}

        float fFloat;
        int fInt;
        bool fBool;
    };

    enum TypeKind {
        kFloat_TypeKind,
        kInt_TypeKind,
        kBool_TypeKind
    };

    Interpreter(std::unique_ptr<Program> program, SkRasterPipeline* pipeline, std::vector<Value>* stack)
    : fProgram(std::move(program))
    , fPipeline(*pipeline)
    , fStack(*stack) {}

    void run();

    void run(const FunctionDefinition& f);

    void push(Value value);

    Value pop();

    StackIndex stackAlloc(int count);

    void runStatement();

    StackIndex getLValue(const Expression& expr);

    Value call(const FunctionCall& c);

    void appendStage(const AppendStage& c);

    Value evaluate(const Expression& expr);

private:
    std::unique_ptr<Program> fProgram;
    SkRasterPipeline& fPipeline;
    std::vector<StatementIndex> fCurrentIndex;
    std::vector<std::map<const Variable*, StackIndex>> fVars;
    std::vector<Value> &fStack;
};

} // namespace

#endif
