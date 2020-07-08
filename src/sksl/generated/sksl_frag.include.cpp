/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// ##### GENERATED CODE, DO NOT MODIFY #####

#include "src/sksl/SkSLIncludes.h"
namespace SkSL {
void Compiler::generated_load_sksl_frag(std::vector<std::unique_ptr<ProgramElement>>& elements, std::shared_ptr<SymbolTable> symbols, Context& context) {
Modifiers m;
Layout l;
symbols->add("sk_Width", std::unique_ptr<Symbol>(new Variable(-1, Modifiers(Layout::builtin(10011), 0), "sk_Width", *(Type*) symbols->get("half"), (Variable::Storage) 0, nullptr)));
symbols->add("sk_FragColor", std::unique_ptr<Symbol>(new Variable(-1, Modifiers(Layout(0, 0, -1, -1, 0, -1, 10001, -1, (Layout::Format) -1, (Layout::Primitive) -1, -1, -1, "", "", (Layout::Key) 0, (Layout::CType) 0), 4), "sk_FragColor", *(Type*) symbols->get("half4"), (Variable::Storage) 0, nullptr)));
symbols->add("gl_SecondaryFragColorEXT", std::unique_ptr<Symbol>(new Variable(-1, Modifiers(Layout::builtin(9999), 4), "gl_SecondaryFragColorEXT", *(Type*) symbols->get("half4"), (Variable::Storage) 0, nullptr)));
symbols->add("sk_Height", std::unique_ptr<Symbol>(new Variable(-1, Modifiers(Layout::builtin(10012), 0), "sk_Height", *(Type*) symbols->get("half"), (Variable::Storage) 0, nullptr)));
symbols->add("sk_SampleMask", std::unique_ptr<Symbol>(new Variable(-1, Modifiers(Layout::builtin(20), 4), "sk_SampleMask", *new Type("int[1]", Type::kArray_Kind, *(Type*) symbols->get("int"), 1), (Variable::Storage) 0, nullptr)));
symbols->add("sk_ClipDistance", std::unique_ptr<Symbol>(new Variable(-1, Modifiers(Layout::builtin(3), 0), "sk_ClipDistance", *new Type("float[1]", Type::kArray_Kind, *(Type*) symbols->get("float"), 1), (Variable::Storage) 0, nullptr)));
symbols->add("sk_LastFragColor", std::unique_ptr<Symbol>(new Variable(-1, Modifiers(Layout::builtin(10008), 0), "sk_LastFragColor", *(Type*) symbols->get("half4"), (Variable::Storage) 0, nullptr)));
symbols->add("sk_Clockwise", std::unique_ptr<Symbol>(new Variable(-1, Modifiers(Layout::builtin(17), 2), "sk_Clockwise", *(Type*) symbols->get("bool"), (Variable::Storage) 0, nullptr)));
symbols->add("sk_FragCoord", std::unique_ptr<Symbol>(new Variable(-1, Modifiers(Layout::builtin(15), 2), "sk_FragCoord", *(Type*) symbols->get("float4"), (Variable::Storage) 0, nullptr)));
elements.emplace_back(new VarDeclarations(-1, (Type*) symbols->get("float4"), make_vector<VarDeclaration>(1, new VarDeclaration((Variable*) symbols->get("sk_FragCoord"), make_vector<Expression>(0), std::unique_ptr<Expression>(nullptr)))));
elements.emplace_back(new VarDeclarations(-1, (Type*) symbols->get("bool"), make_vector<VarDeclaration>(1, new VarDeclaration((Variable*) symbols->get("sk_Clockwise"), make_vector<Expression>(0), std::unique_ptr<Expression>(nullptr)))));
elements.emplace_back(new VarDeclarations(-1, (Type*) symbols->get("float"), make_vector<VarDeclaration>(1, new VarDeclaration((Variable*) symbols->get("sk_ClipDistance"), make_vector<Expression>(1, new IntLiteral(context, -1, 1)), std::unique_ptr<Expression>(nullptr)))));
elements.emplace_back(new VarDeclarations(-1, (Type*) symbols->get("int"), make_vector<VarDeclaration>(1, new VarDeclaration((Variable*) symbols->get("sk_SampleMask"), make_vector<Expression>(1, new IntLiteral(context, -1, 1)), std::unique_ptr<Expression>(nullptr)))));
elements.emplace_back(new VarDeclarations(-1, (Type*) symbols->get("half4"), make_vector<VarDeclaration>(1, new VarDeclaration((Variable*) symbols->get("gl_SecondaryFragColorEXT"), make_vector<Expression>(0), std::unique_ptr<Expression>(nullptr)))));
elements.emplace_back(new VarDeclarations(-1, (Type*) symbols->get("half4"), make_vector<VarDeclaration>(1, new VarDeclaration((Variable*) symbols->get("sk_FragColor"), make_vector<Expression>(0), std::unique_ptr<Expression>(nullptr)))));
elements.emplace_back(new VarDeclarations(-1, (Type*) symbols->get("half4"), make_vector<VarDeclaration>(1, new VarDeclaration((Variable*) symbols->get("sk_LastFragColor"), make_vector<Expression>(0), std::unique_ptr<Expression>(nullptr)))));
elements.emplace_back(new VarDeclarations(-1, (Type*) symbols->get("half"), make_vector<VarDeclaration>(1, new VarDeclaration((Variable*) symbols->get("sk_Width"), make_vector<Expression>(0), std::unique_ptr<Expression>(nullptr)))));
elements.emplace_back(new VarDeclarations(-1, (Type*) symbols->get("half"), make_vector<VarDeclaration>(1, new VarDeclaration((Variable*) symbols->get("sk_Height"), make_vector<Expression>(0), std::unique_ptr<Expression>(nullptr)))));
}
}
