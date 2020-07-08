/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// ##### GENERATED CODE, DO NOT MODIFY #####

#include "src/sksl/SkSLIncludes.h"
namespace SkSL {
void Compiler::generated_load_sksl_geom(std::vector<std::unique_ptr<ProgramElement>>& elements, std::shared_ptr<SymbolTable> symbols, Context& context) {
Modifiers m;
Layout l;
elements.emplace_back(new SkSL::InterfaceBlock(-1, new Variable(-1, Modifiers(Layout::builtin(10007), 4), "sk_PerVertex", *new Type(274, "sk_PerVertex", { Type::Field(Modifiers(Layout::builtin(0), 0), "sk_Position", (Type*) (*symbols)["float4"]), Type::Field(Modifiers(Layout::builtin(1), 0), "sk_PointSize", (Type*) (*symbols)["float"]), Type::Field(Modifiers(Layout::builtin(3), 0), "sk_ClipDistance", new Type("float[1]", Type::kArray_Kind, *(Type*) (*symbols)["float"], 1)) }), (Variable::Storage) 0, nullptr), "sk_PerVertex", "", make_vector<Expression>(0), nullptr));
symbols->addWithoutOwnership("sk_PerVertex_$var", &((InterfaceBlock&) *elements.back()).fVariable);
create_simple_function(symbols, Modifiers(l, 4096), "EndPrimitive", "void", 0);
symbols->add("sk_ClipDistance", std::unique_ptr<Symbol>(new Field(-1, *(Variable*) (*symbols)["sk_PerVertex_$var"], 2)));
symbols->add("sk_Position", std::unique_ptr<Symbol>(new Field(-1, *(Variable*) (*symbols)["sk_PerVertex_$var"], 0)));
create_simple_function(symbols, Modifiers(l, 4096), "EmitStreamVertex", "void", 1, "stream", "int");
symbols->add("sk_InvocationID", std::unique_ptr<Symbol>(new Variable(-1, Modifiers(Layout::builtin(8), 2), "sk_InvocationID", *(Type*) (*symbols)["int"], (Variable::Storage) 0, nullptr)));
create_simple_function(symbols, Modifiers(l, 4096), "EndStreamPrimitive", "void", 1, "stream", "int");
symbols->add("sk_PointSize", std::unique_ptr<Symbol>(new Field(-1, *(Variable*) (*symbols)["sk_PerVertex_$var"], 1)));
create_simple_function(symbols, Modifiers(l, 4096), "EmitVertex", "void", 0);
symbols->add("sk_in", std::unique_ptr<Symbol>(new Variable(-1, Modifiers(Layout::builtin(10002), 2), "sk_in", *new Type("sk_PerVertex[1]", Type::kArray_Kind, *new Type(94, "sk_PerVertex", { Type::Field(Modifiers(Layout::builtin(0), 0), "sk_Position", (Type*) (*symbols)["float4"]), Type::Field(Modifiers(Layout::builtin(1), 0), "sk_PointSize", (Type*) (*symbols)["float"]), Type::Field(Modifiers(Layout::builtin(3), 0), "sk_ClipDistance", new Type("float[1]", Type::kArray_Kind, *(Type*) (*symbols)["float"], 1)) }), 1), (Variable::Storage) 0, nullptr)));
elements.emplace_back(new SkSL::InterfaceBlock(-1, new Variable(-1, Modifiers(Layout::builtin(10002), 2), "sk_in", *new Type("sk_PerVertex[1]", Type::kArray_Kind, *new Type(94, "sk_PerVertex", { Type::Field(Modifiers(Layout::builtin(0), 0), "sk_Position", (Type*) (*symbols)["float4"]), Type::Field(Modifiers(Layout::builtin(1), 0), "sk_PointSize", (Type*) (*symbols)["float"]), Type::Field(Modifiers(Layout::builtin(3), 0), "sk_ClipDistance", new Type("float[1]", Type::kArray_Kind, *(Type*) (*symbols)["float"], 1)) }), 1), (Variable::Storage) 0, nullptr), "sk_PerVertex", "sk_in", make_vector<Expression>(1, new IntLiteral(context, -1, 1)), nullptr));
elements.emplace_back(new VarDeclarations(-1, (Type*) (*symbols)["int"], make_vector<VarDeclaration>(1, new VarDeclaration((Variable*) (*symbols)["sk_InvocationID"], make_vector<Expression>(0), std::unique_ptr<Expression>(nullptr)))));
}
}
