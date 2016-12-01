/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_CODEGENERATOR
#define SKSL_CODEGENERATOR

#include "ir/SkSLProgram.h"
#include "SkSLCompiler.h"

namespace SkSL {

/**
 * Abstract superclass of all code generators, which take a Program as input and produce code as
 * output.
 */
class CodeGenerator {
public:
    CodeGenerator(const Compiler::Settings* settings, const Program* program, ErrorReporter* errors,
                  SkWStream* out, Compiler::Inputs* outInputs)
    : fSettings(*settings)
    , fProgram(*program)
    , fErrors(*errors)
    , fOut(out)
    , fInputs(outInputs) {}

    virtual ~CodeGenerator() {}

    virtual void generateCode() = 0;

protected:

    const Compiler::Settings& fSettings;
    const Program& fProgram;
    ErrorReporter& fErrors;
    SkWStream* fOut;
    Compiler::Inputs* fInputs;
};

} // namespace

#endif
