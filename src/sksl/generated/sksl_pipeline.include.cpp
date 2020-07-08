/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// ##### GENERATED CODE, DO NOT MODIFY #####

#include "src/sksl/SkSLIncludes.h"
namespace SkSL {
void Compiler::generated_load_sksl_pipeline(std::vector<std::unique_ptr<ProgramElement>>& elements, std::shared_ptr<SymbolTable> symbols, Context& context) {
Modifiers m;
Layout l;
symbols->add("sample", std::unique_ptr<Symbol>(new FunctionDeclaration(-1, m, "sample", { simple_parameter("fp", "fragmentProcessor", symbols) }, *(Type*) symbols->get("half4"), 1)));
symbols->add("sample", std::unique_ptr<Symbol>(new FunctionDeclaration(-1, m, "sample", { simple_parameter("fp", "fragmentProcessor", symbols), simple_parameter("coords", "float2", symbols) }, *(Type*) symbols->get("half4"), 1)));
symbols->add("sample", std::unique_ptr<Symbol>(new FunctionDeclaration(-1, m, "sample", { simple_parameter("fp", "fragmentProcessor", symbols), simple_parameter("transform", "float3x3", symbols) }, *(Type*) symbols->get("half4"), 1)));
symbols->add("sk_FragCoord", std::unique_ptr<Symbol>(new Variable(-1, Modifiers(Layout::builtin(15), 0), "sk_FragCoord", *(Type*) symbols->get("float4"), (Variable::Storage) 0, nullptr)));
elements.emplace_back(new VarDeclarations(-1, (Type*) symbols->get("float4"), make_vector<VarDeclaration>(1, new VarDeclaration((Variable*) symbols->get("sk_FragCoord"), make_vector<Expression>(0), std::unique_ptr<Expression>(nullptr)))));
}
}
