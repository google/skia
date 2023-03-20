/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLFunction.h"

#include "include/core/SkTypes.h"
#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "include/private/SkSLString.h"
#include "include/sksl/DSLType.h"
#include "include/sksl/DSLVar.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLIntrinsicList.h"
#include "src/sksl/SkSLModifiersPool.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLFunctionPrototype.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace SkSL {

namespace dsl {

static bool is_intrinsic_in_module(const Context& context, std::string_view name) {
    return context.fConfig->fIsBuiltinCode && SkSL::FindIntrinsicKind(name) != kNotIntrinsic;
}

void DSLFunction::init(DSLModifiers modifiers, const DSLType& returnType, std::string_view name,
                       SkSpan<DSLParameter*> params, Position pos) {
    fPosition = pos;

    const Context& context = ThreadContext::Context();
    if (context.fConfig->fSettings.fForceNoInline) {
        // Apply the `noinline` modifier to every function. This allows us to test Runtime
        // Effects without any inlining, even when the code is later added to a paint.
        modifiers.fModifiers.fFlags &= ~Modifiers::kInline_Flag;
        modifiers.fModifiers.fFlags |= Modifiers::kNoInline_Flag;
    }

    std::vector<std::unique_ptr<Variable>> paramVars;
    paramVars.reserve(params.size());
    for (DSLParameter* param : params) {
        SkASSERT(!param->fInitialValue.hasValue());
        SkASSERT(!param->fDeclaration);
        std::unique_ptr<SkSL::Variable> paramVar = DSLWriter::CreateParameterVar(*param);
        if (!paramVar) {
            return;
        }
        paramVars.push_back(std::move(paramVar));
    }
    SkASSERT(paramVars.size() == params.size());
    fDecl = SkSL::FunctionDeclaration::Convert(context,
                                               *ThreadContext::SymbolTable(),
                                               pos,
                                               modifiers.fPosition,
                                               context.fModifiersPool->add(modifiers.fModifiers),
                                               name,
                                               std::move(paramVars),
                                               pos,
                                               &returnType.skslType());
    if (fDecl) {
        for (size_t i = 0; i < params.size(); ++i) {
            params[i]->fVar = fDecl->parameters()[i];
            params[i]->fInitialized = true;
        }
    }
}

void DSLFunction::prototype() {
    if (!fDecl) {
        // We failed to create the declaration; error should already have been reported.
        return;
    }
    ThreadContext::ProgramElements().push_back(std::make_unique<SkSL::FunctionPrototype>(
            fDecl->fPosition, fDecl, ThreadContext::IsModule()));
}

void DSLFunction::define(DSLBlock block, Position pos) {
    std::unique_ptr<SkSL::Block> body = block.release();
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

DSLExpression DSLFunction::call(SkSpan<DSLExpression> args, Position pos) {
    ExpressionArray released;
    released.reserve_back(args.size());
    for (DSLExpression& arg : args) {
        released.push_back(arg.release());
    }
    return this->call(std::move(released));
}

DSLExpression DSLFunction::call(ExpressionArray args, Position pos) {
    std::unique_ptr<SkSL::Expression> result =
            SkSL::FunctionCall::Convert(ThreadContext::Context(), pos, *fDecl, std::move(args));
    return DSLExpression(std::move(result), pos);
}

} // namespace dsl

} // namespace SkSL
