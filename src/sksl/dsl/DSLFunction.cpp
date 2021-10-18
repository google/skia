/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLFunction.h"

#include "include/sksl/DSLVar.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLFunctionPrototype.h"
#include "src/sksl/ir/SkSLReturnStatement.h"

namespace SkSL {

namespace dsl {

void DSLFunction::init(DSLModifiers modifiers, const DSLType& returnType, skstd::string_view name,
                       SkTArray<DSLParameter*> params, PositionInfo pos) {
    fPosition = pos;
    // Conservatively assume all user-defined functions have side effects.
    if (!ThreadContext::IsModule()) {
        modifiers.fModifiers.fFlags |= Modifiers::kHasSideEffects_Flag;
    }

    if (ThreadContext::Settings().fForceNoInline) {
        // Apply the `noinline` modifier to every function. This allows us to test Runtime
        // Effects without any inlining, even when the code is later added to a paint.
        modifiers.fModifiers.fFlags &= ~Modifiers::kInline_Flag;
        modifiers.fModifiers.fFlags |= Modifiers::kNoInline_Flag;
    }

    std::vector<std::unique_ptr<Variable>> paramVars;
    paramVars.reserve(params.size());
    for (DSLParameter* param : params) {
        if (param->fDeclared) {
            ThreadContext::ReportError("parameter has already been used in another function");
        }
        SkASSERT(!param->fInitialValue.hasValue());
        SkASSERT(!param->fDeclaration);
        param->fDeclared = true;
        std::unique_ptr<SkSL::Variable> paramVar = DSLWriter::CreateParameterVar(*param);
        if (!paramVar) {
            return;
        }
        paramVars.push_back(std::move(paramVar));
    }
    SkASSERT(paramVars.size() == params.size());
    fDecl = SkSL::FunctionDeclaration::Convert(ThreadContext::Context(),
                                               *ThreadContext::SymbolTable(),
                                               pos.line(),
                                               ThreadContext::Modifiers(modifiers.fModifiers),
                                               name == "main" ? name : DSLWriter::Name(name),
                                               std::move(paramVars), &returnType.skslType());
    ThreadContext::ReportErrors(pos);
    if (fDecl) {
        for (size_t i = 0; i < params.size(); ++i) {
            params[i]->fVar = fDecl->parameters()[i];
            params[i]->fInitialized = true;
        }
        // We don't know when this function is going to be defined; go ahead and add a prototype in
        // case the definition is delayed. If we end up defining the function immediately, we'll
        // remove the prototype in define().
        ThreadContext::ProgramElements().push_back(std::make_unique<SkSL::FunctionPrototype>(
                pos.line(), fDecl, ThreadContext::IsModule()));
    }
}

void DSLFunction::define(DSLBlock block, PositionInfo pos) {
    std::unique_ptr<SkSL::Block> body = block.release();
    if (!fDecl) {
        // Evidently we failed to create the declaration; error should already have been reported.
        // Release the block so we don't fail its destructor assert.
        return;
    }
    if (!ThreadContext::ProgramElements().empty()) {
        // If the last ProgramElement was the prototype for this function, it was unnecessary and we
        // can remove it.
        const SkSL::ProgramElement& last = *ThreadContext::ProgramElements().back();
        if (last.is<SkSL::FunctionPrototype>()) {
            const SkSL::FunctionPrototype& prototype = last.as<SkSL::FunctionPrototype>();
            if (&prototype.declaration() == fDecl) {
                ThreadContext::ProgramElements().pop_back();
            }
        }
    }
    if (fDecl->definition()) {
        ThreadContext::ReportError(String::printf("function '%s' was already defined",
                fDecl->description().c_str()), pos);
        block.release();
        return;
    }
    std::unique_ptr<FunctionDefinition> function = FunctionDefinition::Convert(
            ThreadContext::Context(),
            pos.line(),
            *fDecl,
            std::move(body),
            /*builtin=*/false);
    ThreadContext::ReportErrors(fPosition);
    fDecl->setDefinition(function.get());
    ThreadContext::ProgramElements().push_back(std::move(function));
}

DSLExpression DSLFunction::call(SkTArray<DSLWrapper<DSLExpression>> args, PositionInfo pos) {
    ExpressionArray released;
    released.reserve_back(args.size());
    for (DSLWrapper<DSLExpression>& arg : args) {
        released.push_back(arg->release());
    }
    return this->call(std::move(released));
}

DSLExpression DSLFunction::call(ExpressionArray args, PositionInfo pos) {
    std::unique_ptr<SkSL::Expression> result = SkSL::FunctionCall::Convert(ThreadContext::Context(),
            pos.line(), *fDecl, std::move(args));
    return DSLExpression(std::move(result), pos);
}

} // namespace dsl

} // namespace SkSL
