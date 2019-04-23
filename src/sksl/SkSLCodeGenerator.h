/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CODEGENERATOR
#define SKSL_CODEGENERATOR

#include "src/sksl/SkSLOutputStream.h"
#include "src/sksl/ir/SkSLProgram.h"

namespace SkSL {

/**
 * Abstract superclass of all code generators, which take a Program as input and produce code as
 * output.
 */
class CodeGenerator {
public:
    CodeGenerator(const Program* program, ErrorReporter* errors, OutputStream* out)
    : fProgram(*program)
    , fErrors(*errors)
    , fOut(out) {
        SkASSERT(program->fIsOptimized);
    }

    virtual ~CodeGenerator() {}

    virtual bool generateCode() = 0;

protected:

    const Program& fProgram;
    ErrorReporter& fErrors;
    OutputStream* fOut;
};

} // namespace

#endif
