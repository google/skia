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

void DSLFunction::init(const DSLType& returnType, const char* name,
                       std::vector<DSLVar*> params) {
    std::vector<std::unique_ptr<Variable>> paramVars;
    paramVars.reserve(params.size());
    bool isMain = !strcmp(name, "main");
    auto typeIsValidForColor = [&](const SkSL::Type& type) {
        return type == *DSLWriter::Context().fTypes.fHalf4 ||
               type == *DSLWriter::Context().fTypes.fFloat4;
    };
    for (DSLVar* param : params) {
        // This counts as declaring the variable; make sure it hasn't been previously declared and
        // then kill its pending declaration statement. Otherwise the statement will hang around
        // until after the Var is destroyed, which is probably after the End() call and therefore
        // after the Pool's destruction. Freeing a pooled object after the Pool's destruction is a
        // Bad Thing.
        if (param->fDeclared) {
            DSLWriter::ReportError("error: using an already-declared variable as a function "
                                   "parameter\n");
        }
        if (param->fInitialValue.release()) {
            DSLWriter::ReportError("error: variables used as function parameters cannot have "
                                   "initial values\n");
        }
        param->fDeclared = true;
        param->fStorage = SkSL::VariableStorage::kParameter;
        if (paramVars.empty()) {
            SkSL::ProgramKind kind = DSLWriter::Context().fConfig->fKind;
            if (isMain && (kind == ProgramKind::kRuntimeColorFilter ||
                           kind == ProgramKind::kRuntimeShader ||
                           kind == ProgramKind::kFragmentProcessor)) {
                const SkSL::Type& type = param->fType.skslType();
                // We verify that the signature is fully correct later. For now, if this is an .fp
                // or runtime effect of any flavor, a float2 param is supposed to be the coords, and
                // a half4/float parameter is supposed to be the input color:
                if (type == *DSLWriter::Context().fTypes.fFloat2) {
                    param->fModifiers.fModifiers.fLayout.fBuiltin = SK_MAIN_COORDS_BUILTIN;
                } else if (typeIsValidForColor(type)) {
                    param->fModifiers.fModifiers.fLayout.fBuiltin = SK_INPUT_COLOR_BUILTIN;
                }
            }
        }
        std::unique_ptr<SkSL::Variable> paramVar = DSLWriter::ParameterVar(*param);
        SkASSERT(paramVar);
        paramVars.push_back(std::move(paramVar));
        param->fDeclaration = nullptr;
    }
    fDecl = SkSL::FunctionDeclaration::Convert(DSLWriter::Context(),
                                               *DSLWriter::SymbolTable(),
                                               *DSLWriter::Context().fModifiersPool, /*offset=*/-1,
                                               DSLWriter::Modifiers(SkSL::Modifiers()),
                                               isMain ? name : DSLWriter::Name(name),
                                               std::move(paramVars), fReturnType,
                                               /*isBuiltin=*/false);
}

void DSLFunction::define(DSLBlock block) {
    if (!fDecl) {
        return;
    }
    SkASSERTF(!fDecl->definition(), "function '%s' already defined", fDecl->description().c_str());
    std::unique_ptr<Statement> body = block.release();
    DSLWriter::IRGenerator().finalizeFunction(*fDecl, body.get());
    auto function = std::make_unique<SkSL::FunctionDefinition>(/*offset=*/-1, fDecl,
                                                               /*builtin=*/false, std::move(body));
    if (DSLWriter::Compiler().errorCount()) {
        DSLWriter::ReportError(DSLWriter::Compiler().errorText(/*showCount=*/false).c_str());
        DSLWriter::Compiler().setErrorCount(0);
        SkASSERT(!DSLWriter::Compiler().errorCount());
    }
    fDecl->fDefinition = function.get();
    DSLWriter::ProgramElements().push_back(std::move(function));
}

DSLExpression DSLFunction::call(SkTArray<DSLExpression> args) {
    ExpressionArray released;
    released.reserve_back(args.size());
    for (DSLExpression& arg : args) {
        released.push_back(arg.release());
    }
    return DSLWriter::Call(*fDecl, std::move(released));
}

} // namespace dsl

} // namespace SkSL
