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

    Interpreter(std::unique_ptr<Program> program, std::unique_ptr<ByteCode> byteCode)
    : fProgram(std::move(program))
    , fByteCode(std::move(byteCode)) {}

    /**
     * Invokes the specified function with the given arguments, returning its return value. 'out'
     * and 'inout' parameters will result in the 'args' array being modified.
     */
    Value* run(const ByteCodeFunction& f, Value args[], Value inputs[]);

private:
    StackIndex stackAlloc(int count);

    uint8_t read8();

    uint16_t read16();

    uint32_t read32();

    inline void vectorOp(int count);

    void run();

    void push(Value v);

    Value pop();

    void swizzle();

    void disassemble(const ByteCodeFunction& f);

    void dumpStack();

    std::unique_ptr<Program> fProgram;
    std::unique_ptr<ByteCode> fByteCode;
    int fIP;
    const ByteCodeFunction* fCurrentFunction;
    std::vector<Value> fGlobals;
    std::vector<Value> fStack;
};

} // namespace

#endif
