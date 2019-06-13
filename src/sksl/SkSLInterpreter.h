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
    /**
     * Invokes the specified function with the given arguments. 'out' and 'inout' parameters will
     * result in the 'args' array being modified. The return value is stored in 'outReturn' (may be
     * null, in which case the return value is discarded).
     */
    void Run(const ByteCode*, const ByteCodeFunction*,
             void* args, void* outReturn, int N,
             const void* uniforms, int uniformCount);

    /**
     * Print bytecode disassembly to stdout.
     */
    void Disassemble(const ByteCodeFunction*);

} // namespace Interpreter
} // namespace SkSL

#endif
