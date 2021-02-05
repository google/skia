/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_PIPELINESTAGECODEGENERATOR
#define SKSL_PIPELINESTAGECODEGENERATOR

#include "src/sksl/SkSLCodeGenerator.h"
#include "src/sksl/SkSLOperators.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

namespace SkSL {

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
