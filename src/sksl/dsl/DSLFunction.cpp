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
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLReturnStatement.h"

namespace SkSL {

namespace dsl {

void DSLFunction::init(DSLModifiers modifiers, const DSLType& returnType, skstd::string_view name,
                       SkTArray<DSLParameter*> params) {
    // Conservatively assume all user-defined functions have side effects.
    if (!DSLWriter::IsModule()) {
        modifiers.fModifiers.fFlags |= Modifiers::kHasSideEffects_Flag;
    }

    if (DSLWriter::Settings().fForceNoInline) {
        // Apply the `noinline` modifier to every function. This allows us to test Runtime
        // Effects without any inlining, even when the code is later added to a paint.
        modifiers.fModifiers.fFlags &= ~Modifiers::kInline_Flag;
        modifiers.fModifiers.fFlags |= Modifiers::kNoInline_Flag;
    }

    std::vector<std::unique_ptr<Variable>> paramVars;
    paramVars.reserve(params.size());
    bool isMain = name == "main";
    auto typeIsValidForColor = [&](const SkSL::Type& type) {
        return type == *DSLWriter::Context().fTypes.fHalf4 ||
               type == *DSLWriter::Context().fTypes.fFloat4;
    };
    for (DSLParameter* param : params) {
        if (param->fDeclared) {
            DSLWriter::ReportError("error: parameter has already been used in another function\n");
        }
        SkASSERT(!param->fInitialValue.valid());
        SkASSERT(!param->fDeclaration);
        param->fDeclared = true;
        if (isMain && DSLWriter::Context().fConfig->isRuntimeEffect()) {
            const SkSL::Type& type = param->fType.skslType();
            // We verify that the signature is fully correct later. For now, if this is a runtime
            // effect of any flavor, a float2 param is supposed to be the coords, and a half4/float
            // parameter is supposed to be the input or destination color.
            if (type == *DSLWriter::Context().fTypes.fFloat2) {
                param->fModifiers.fModifiers.fLayout.fBuiltin = SK_MAIN_COORDS_BUILTIN;
            } else if (typeIsValidForColor(type)) {
                // TODO(skia:12257): add support for blender destination color
                param->fModifiers.fModifiers.fLayout.fBuiltin = SK_INPUT_COLOR_BUILTIN;
            }
        }
        std::unique_ptr<SkSL::Variable> paramVar = DSLWriter::CreateParameterVar(*param);
        if (!paramVar) {
            return;
        }
        paramVars.push_back(std::move(paramVar));
    }
    SkASSERT(paramVars.size() == params.size());
    fDecl = SkSL::FunctionDeclaration::Convert(DSLWriter::Context(),
                                               *DSLWriter::SymbolTable(),
                                               /*offset=*/-1,
                                               DSLWriter::Modifiers(modifiers.fModifiers),
                                               isMain ? name : DSLWriter::Name(name),
                                               std::move(paramVars), &returnType.skslType(),
                                               DSLWriter::IsModule());
    DSLWriter::ReportErrors();
    if (fDecl) {
        for (size_t i = 0; i < params.size(); ++i) {
            params[i]->fVar = fDecl->parameters()[i];
        }
    }
}

void DSLFunction::define(DSLBlock block) {
    if (!fDecl) {
        // Evidently we failed to create the declaration; error should already have been reported.
        // Release the block so we don't fail its destructor assert.
        block.release();
        return;
    }
    SkASSERTF(!fDecl->definition(), "function '%s' already defined", fDecl->description().c_str());
    std::unique_ptr<Block> body = block.release();
    body = DSLWriter::IRGenerator().finalizeFunction(*fDecl, std::move(body));
    auto function = std::make_unique<SkSL::FunctionDefinition>(/*offset=*/-1, fDecl,
                                                               /*builtin=*/false, std::move(body));
    DSLWriter::ReportErrors();
    fDecl->fDefinition = function.get();
    DSLWriter::ProgramElements().push_back(std::move(function));
}

DSLExpression DSLFunction::call(SkTArray<DSLWrapper<DSLExpression>> args) {
    ExpressionArray released;
    released.reserve_back(args.size());
    for (DSLWrapper<DSLExpression>& arg : args) {
        released.push_back(arg->release());
    }
    std::unique_ptr<SkSL::Expression> result = DSLWriter::Call(*fDecl, std::move(released));
    return result ? DSLExpression(std::move(result)) : DSLExpression();
}

} // namespace dsl

} // namespace SkSL
