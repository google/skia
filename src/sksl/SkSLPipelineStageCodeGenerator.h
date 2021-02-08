/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_PIPELINESTAGECODEGENERATOR
#define SKSL_PIPELINESTAGECODEGENERATOR

#include "src/sksl/SkSLString.h"

#include <vector>

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

namespace SkSL {

class FunctionDeclaration;
struct Program;

class PipelineStage {
public:
    // An invalid (otherwise unused) character to mark where FormatArgs are inserted
    static constexpr       char  kFormatArgPlaceholder    = '\001';
    static constexpr const char* kFormatArgPlaceholderStr = "\001";

    struct FormatArg {
        enum class Kind {
            kCoords,
            kUniform,
            kChildProcessor,
            kChildProcessorWithMatrix,
            kFunctionName
        };

        FormatArg(Kind kind, int index = 0) : fKind(kind), fIndex(index) {}

        Kind   fKind;
        int    fIndex;
        String fCoords;
    };

    /**
     * Represents the arguments to GrGLSLShaderBuilder::emitFunction.
     */
    struct Function {
        const FunctionDeclaration* fDecl;
        String                     fBody;
        std::vector<FormatArg>     fFormatArgs;
    };

    struct Args {
        String                 fCode;
        std::vector<FormatArg> fFormatArgs;
        std::vector<Function>  fFunctions;
    };

    static void ConvertProgram(const Program& program, Args* outArgs);
};

}  // namespace SkSL

#endif

#endif
