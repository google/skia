/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CODEGENERATOR
#define SKSL_CODEGENERATOR

#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLOutputStream.h"
#include "src/sksl/ir/SkSLProgram.h"

namespace SkSL {

struct ShaderCaps;

/**
 * Abstract superclass of all code generators, which take a Program as input and produce code as
 * output.
 */
class CodeGenerator {
public:
    CodeGenerator(const Context* context,
                  const ShaderCaps* caps,
                  const Program* program,
                  OutputStream* stream)
            : fProgram(*program)
            , fContext(fProgram.fContext->fTypes, *fProgram.fContext->fErrors)
            , fCaps(*caps)
            , fOut(stream) {
        fContext.fConfig = fProgram.fConfig.get();
        fContext.fModule = fProgram.fContext->fModule;
        fContext.fSymbolTable = fProgram.fSymbols.get();
    }

    virtual ~CodeGenerator() = default;

    virtual bool generateCode() = 0;

    // Intended for use by AutoOutputStream.
    OutputStream* outputStream() { return fOut; }
    void setOutputStream(OutputStream* output) { fOut = output; }

protected:
#if defined(SK_USE_LEGACY_MIPMAP_LOD_BIAS)
    static constexpr float kSharpenTexturesBias = -.5f;
#else
    // For SkMipmapMode::kLinear we want a bias such that when the unbiased LOD value is
    // midway between levels we select just the larger level, i.e. a bias of -.5. However, using
    // this bias with kNearest mode with a draw that is a perfect power of two downscale puts us
    // right on the rounding edge where it could go up or down depending on the particular GPU.
    // Experimentally we found that at -.49 most iOS devices (iPhone 7, 8, and iPad Pro
    // [PowerVRGT7800 version]) all round to the level twice as big as the device space footprint
    // for some such draws in our unit tests on GLES. However, the iPhone 11 still fails and so
    // we are using -.475. They do not at -.48. All other GPUs passed tests with -.499. Though, at
    // this time the bias is not implemented in the MSL codegen and so iOS/Metal was not tested.
    static constexpr float kSharpenTexturesBias = -.475f;
#endif

    const Program& fProgram;
    Context fContext;
    const ShaderCaps& fCaps;
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
