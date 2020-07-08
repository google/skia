/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// ##### GENERATED CODE, DO NOT MODIFY #####

#include "src/sksl/SkSLIncludes.h"
namespace SkSL {
void Compiler::generated_load_sksl_vert(std::vector<std::unique_ptr<ProgramElement>>& elements, std::shared_ptr<SymbolTable> symbols, Context& context) {
Modifiers m;
Layout l;
elements.emplace_back(new SkSL::InterfaceBlock(-1, new Variable(-1, Modifiers(l, 4), "sk_PerVertex", *new Type(71, "sk_PerVertex", { Type::Field(Modifiers(Layout::builtin(0), 0), "sk_Position", (Type*) symbols->get("float4")), Type::Field(Modifiers(Layout::builtin(1), 0), "sk_PointSize", (Type*) symbols->get("float")), Type::Field(Modifiers(Layout::builtin(3), 0), "sk_ClipDistance", new Type("float[1]", Type::kArray_Kind, *(Type*) symbols->get("float"), 1)) }), (Variable::Storage) 0, nullptr), "sk_PerVertex", "", make_vector<Expression>(0), nullptr));
symbols->addWithoutOwnership("sk_PerVertex_$var", &((InterfaceBlock&) *elements.back()).fVariable);
symbols->add("sk_VertexID", std::unique_ptr<Symbol>(new Variable(-1, Modifiers(Layout::builtin(42), 2), "sk_VertexID", *(Type*) symbols->get("int"), (Variable::Storage) 0, nullptr)));
symbols->add("sk_ClipDistance", std::unique_ptr<Symbol>(new Field(-1, *(Variable*) (*symbols)["sk_PerVertex_$var"], 2)));
symbols->add("sk_InstanceID", std::unique_ptr<Symbol>(new Variable(-1, Modifiers(Layout::builtin(43), 2), "sk_InstanceID", *(Type*) symbols->get("int"), (Variable::Storage) 0, nullptr)));
symbols->add("sk_PointSize", std::unique_ptr<Symbol>(new Field(-1, *(Variable*) (*symbols)["sk_PerVertex_$var"], 1)));
symbols->add("sk_Position", std::unique_ptr<Symbol>(new Field(-1, *(Variable*) (*symbols)["sk_PerVertex_$var"], 0)));
elements.emplace_back(new VarDeclarations(-1, (Type*) symbols->get("int"), make_vector<VarDeclaration>(1, new VarDeclaration((Variable*) symbols->get("sk_VertexID"), make_vector<Expression>(0), std::unique_ptr<Expression>(nullptr)))));
elements.emplace_back(new VarDeclarations(-1, (Type*) symbols->get("int"), make_vector<VarDeclaration>(1, new VarDeclaration((Variable*) symbols->get("sk_InstanceID"), make_vector<Expression>(0), std::unique_ptr<Expression>(nullptr)))));
}
}
