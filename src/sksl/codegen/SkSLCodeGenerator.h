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
    CodeGenerator(const Context* context, const Program* program, OutputStream* out)
    : fContext(*context)
    , fProgram(*program)
    , fOut(out) {}

    virtual ~CodeGenerator() {}

    virtual bool generateCode() = 0;

    // Intended for use by AutoOutputStream.
    OutputStream* outputStream() { return fOut; }
    void setOutputStream(OutputStream* output) { fOut = output; }

protected:
    const Context& fContext;
    const Program& fProgram;
    OutputStream* fOut;
};

class AutoOutputStream {
public:
    // Maintains the current indentation level while writing to the new output stream.
    AutoOutputStream(CodeGenerator* codeGen, OutputStream* newOutput)
            : fCodeGen(codeGen)
            , fOldOutput(codeGen->outputStream()) {
        fCodeGen->setOutputStream(newOutput);
    }
    // Resets the indentation when entering the scope, and restores it when leaving.
    AutoOutputStream(CodeGenerator* codeGen, OutputStream* newOutput, int *indentationPtr)
            : fCodeGen(codeGen)
            , fOldOutput(codeGen->outputStream())
            , fIndentationPtr(indentationPtr)
            , fOldIndentation(indentationPtr ? *indentationPtr : 0) {
        fCodeGen->setOutputStream(newOutput);
        *fIndentationPtr = 0;
    }
    ~AutoOutputStream() {
        fCodeGen->setOutputStream(fOldOutput);
        if (fIndentationPtr) {
            *fIndentationPtr = fOldIndentation;
        }
    }

private:
    CodeGenerator* fCodeGen = nullptr;
    OutputStream* fOldOutput = nullptr;
    int *fIndentationPtr = nullptr;
    int fOldIndentation = 0;
};

}  // namespace SkSL

#endif
