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

    template <typename T> class MikeStack {
        T fStorage[1000];
        T* fPtr = &fStorage[0];

    public:
        MikeStack() {}

        T& operator[](int index) {
            SkASSERT(index >= 0 && index < (int)this->size());
            return fStorage[index];
        }
        T* data() { return fStorage; }
        T* end() { return fPtr; }

        size_t size() const {
            SkASSERT(fPtr >= fStorage);
            return fPtr - fStorage;
        }

        void clear() { fPtr = &fStorage[0]; }
        void push_back(T v) { *fPtr++ = v; }
        void pop_back() { --fPtr; }
        T& back() { return fPtr[-1]; }
        void insert(T* cursor, T value) {
            if (cursor < fPtr) {
                memmove(cursor + 1, cursor, (fPtr - cursor) * sizeof(T));
            }
            *cursor = value;
            fPtr += 1;
        }
    };

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

    void run();

    void push(Value v);

    Value pop();

    void swizzle();

    void disassemble(const ByteCodeFunction& f);

    void dumpStack();

    std::unique_ptr<Program> fProgram;
    std::unique_ptr<ByteCode> fByteCode;
    const ByteCodeFunction* fCurrentFunction;
    std::vector<Value> fGlobals;
    MikeStack<Value> fStack;
};

} // namespace

#endif
