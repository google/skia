/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_INTERPRETER
#define SKSL_INTERPRETER

#include "include/private/SkVx.h"
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

    /**
     * Updates the global inputs.
     */
    void setInputs(Value inputs[]);

    /**
     * Print bytecode disassembly to stdout.
     */
    void disassemble(const ByteCodeFunction&);
private:
    void innerRun(const ByteCodeFunction& f, Value* stack, Value* outReturn);


    std::unique_ptr<Program> fProgram;
    std::unique_ptr<ByteCode> fByteCode;
    std::vector<Value> fGlobals;
};

class VecInterpreter {
public:
    union SValue {
        SValue() {}

        SValue(float f)
        : fFloat(f) {}

        SValue(int32_t s)
        : fSigned(s) {}

        SValue(uint32_t u)
        : fUnsigned(u) {}

        SValue(bool b)
        : fBool(b) {}

        float    fFloat;
        int32_t  fSigned;
        uint32_t fUnsigned;
        bool     fBool;
    };

    using float4 = skvx::Vec<4, float>;
    using int4   = skvx::Vec<4, int32_t>;
    using uint4  = skvx::Vec<4, uint32_t>;

    union VValue {
        VValue() {}

        VValue(float4 f)
        : fFloat(f) {}

        VValue(int4 s)
        : fSigned(s) {}

        VValue(uint4 u)
        : fUnsigned(u) {}

        float4 fFloat;
        int4   fSigned;
        uint4  fUnsigned;
    };

    enum TypeKind {
        kFloat_TypeKind,
        kInt_TypeKind,
        kBool_TypeKind
    };

    /**
     * 'inputs' contains the values of global 'in' variables in source order.
     */
    VecInterpreter(std::unique_ptr<Program> program, std::unique_ptr<ByteCode> byteCode,
                   SValue inputs[] = nullptr);

    /**
     * Invokes the specified function with the given arguments. 'out' and 'inout' parameters will
     * result in the 'args' array being modified. The return value is stored in 'outReturn' (may be
     * null, in which case the return value is discarded).
     */
    void run(const ByteCodeFunction& f, SValue args[], SValue* outReturn);

    /**
     * Updates the global inputs.
     */
    void setInputs(SValue inputs[]);

    /**
     * Print bytecode disassembly to stdout.
     */
    void disassemble(const ByteCodeFunction&);
private:
    void innerRun(const ByteCodeFunction& f, VValue* stack, SValue* outReturn);


    std::unique_ptr<Program> fProgram;
    std::unique_ptr<ByteCode> fByteCode;
    std::vector<VValue> fGlobals;
};

} // namespace

#endif
