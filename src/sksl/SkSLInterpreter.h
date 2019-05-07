/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_INTERPRETER
#define SKSL_INTERPRETER

#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/ir/SkSLAppendStage.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLStatement.h"

#include <stack>

namespace SkSL {

class Interpreter {
    typedef int StackIndex;

public:
    union Value {
        Value() {}

        Value(float f)
        : fFloat(f) {}

        Value(int32_t s)
        : fSigned(s) {}

        Value(uint32_t u)
        : fUnsigned(u) {}

        Value(bool b)
        : fBool(b) {}

        float fFloat;
        int32_t fSigned;
        uint32_t fUnsigned;
        bool fBool;
    };

    enum TypeKind {
        kFloat_TypeKind,
        kInt_TypeKind,
        kBool_TypeKind
    };

    /**
     * 'inputs' contains the values of global 'in' variables in source order.
     */
    Interpreter(std::unique_ptr<Program> program, std::unique_ptr<ByteCode> byteCode,
                Value inputs[] = nullptr);

    /**
     * Invokes the specified function with the given arguments. 'out' and 'inout' parameters will
     * result in the 'args' array being modified. The return value is stored in 'outReturn' (may be
     * null, in which case the return value is discarded).
     */
    void run(const ByteCodeFunction& f, Value args[], Value* outReturn);

private:
    StackIndex stackAlloc(int count);

    void run(Value* stack, Value args[], Value* outReturn);

    void swizzle();

    void disassemble(const ByteCodeFunction& f);

    std::unique_ptr<Program> fProgram;
    std::unique_ptr<ByteCode> fByteCode;
    const ByteCodeFunction* fCurrentFunction;
    std::vector<Value> fGlobals;
};

} // namespace

#endif
