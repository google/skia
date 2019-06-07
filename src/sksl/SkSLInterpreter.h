/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_INTERPRETER
#define SKSL_INTERPRETER

#include "src/sksl/SkSLDefines.h"

namespace SkSL {

struct ByteCode;
struct ByteCodeFunction;

namespace Interpreter {

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

    /**
     * Invokes the specified function with the given arguments. 'out' and 'inout' parameters will
     * result in the 'args' array being modified. The return value is stored in 'outReturn' (may be
     * null, in which case the return value is discarded).
     */
    void Run(const ByteCode*, const ByteCodeFunction*, Value args[], Value* outReturn,
             Value uniforms[], int uniformCount);

    /**
     * Print bytecode disassembly to stdout.
     */
    void Disassemble(const ByteCodeFunction*);

} // namespace Interpreter
} // namespace SkSL

#endif
