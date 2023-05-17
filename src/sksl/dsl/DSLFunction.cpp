/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/DSLFunction.h"

#include "include/core/SkTypes.h"
#include "include/private/SkSLDefines.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLIntrinsicList.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/dsl/DSLModifiers.h"
#include "src/sksl/dsl/DSLType.h"
#include "src/sksl/dsl/DSLVar.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLFunctionPrototype.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace skia_private;

namespace SkSL::dsl {

DSLFunction::DSLFunction(std::string_view name,
                         const DSLModifiers& modifiers,
                         const DSLType& returnType,
                         SkSpan<DSLParameter*> parameters,
                         Position pos) {
    this->init(modifiers, returnType, name, parameters, pos);
}

static bool is_intrinsic_in_module(const Context& context, std::string_view name) {
    return context.fConfig->fIsBuiltinCode && SkSL::FindIntrinsicKind(name) != kNotIntrinsic;
}

void DSLFunction::init(DSLModifiers modifiers, const DSLType& returnType, std::string_view name,
                       SkSpan<DSLParameter*> params, Position pos) {
    fPosition = pos;

    TArray<std::unique_ptr<Variable>> paramVars;
    paramVars.reserve_exact(params.size());
    for (DSLParameter* param : params) {
        SkASSERT(!param->fInitialValue.hasValue());
        SkASSERT(!param->fDeclaration);
        std::unique_ptr<SkSL::Variable> paramVar = DSLWriter::CreateParameterVar(*param);
        if (!paramVar) {
            return;
        }
        paramVars.push_back(std::move(paramVar));
    }
    SkASSERT(SkToSizeT(paramVars.size()) == params.size());
    fDecl = SkSL::FunctionDeclaration::Convert(ThreadContext::Context(),
                                               pos,
                                               modifiers.fPosition,
                                               &modifiers.fModifiers,
                                               name,
                                               std::move(paramVars),
                                               pos,
                                               &returnType.skslType());
}

void DSLFunction::prototype() {
    if (!fDecl) {
        // We failed to create the declaration; error should already have been reported.
        return;
    }
    ThreadContext::ProgramElements().push_back(std::make_unique<SkSL::FunctionPrototype>(
            fDecl->fPosition, fDecl, ThreadContext::IsModule()));
}

void DSLFunction::define(DSLStatement block, Position pos) {
    std::unique_ptr<SkSL::Statement> body = block.release();
    SkASSERT(body->is<Block>());
    body->fPosition = pos;
    if (!fDecl) {
        // We failed to create the declaration; error should already have been reported.
        return;
    }
    // We don't allow modules to define actual functions with intrinsic names. (Those should be
    // reserved for actual intrinsics.)
    const Context& context = ThreadContext::Context();
    if (is_intrinsic_in_module(context, fDecl->name())) {
        ThreadContext::ReportError(
                SkSL::String::printf("Intrinsic function '%.*s' should not have a definition",
                                     (int)fDecl->name().size(),
                                     fDecl->name().data()),
                fDecl->fPosition);
        return;
    }

    if (fDecl->definition()) {
        ThreadContext::ReportError(SkSL::String::printf("function '%s' was already defined",
                                                        fDecl->description().c_str()),
                                   fDecl->fPosition);
        return;
    }
    std::unique_ptr<FunctionDefinition> function = FunctionDefinition::Convert(
            ThreadContext::Context(),
            pos,
            *fDecl,
            std::move(body),
            /*builtin=*/false);
    fDecl->setDefinition(function.get());
    ThreadContext::ProgramElements().push_back(std::move(function));
}

DSLExpression DSLFunction::call(ExpressionArray args, Position pos) {
    std::unique_ptr<SkSL::Expression> result =
            SkSL::FunctionCall::Convert(ThreadContext::Context(), pos, *fDecl, std::move(args));
    return DSLExpression(std::move(result), pos);
}

void DSLFunction::addParametersToSymbolTable(const Context& context) {
    if (fDecl) {
        fDecl->addParametersToSymbolTable(context);
    }
}

}  // namespace SkSL::dsl
