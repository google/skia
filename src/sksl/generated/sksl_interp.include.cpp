/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// ##### GENERATED CODE, DO NOT MODIFY #####

#include "src/sksl/SkSLIncludes.h"
namespace SkSL {
void Compiler::generated_load_sksl_interp(std::vector<std::unique_ptr<ProgramElement>>& elements, std::shared_ptr<SymbolTable> symbols, Context& context) {
Modifiers m;
Layout l;
create_simple_function(symbols, Modifiers(l, 4096), "radians", "float", 1, "deg", "float");
create_simple_function(symbols, Modifiers(l, 4096), "radians", "float2", 1, "deg", "float2");
create_simple_function(symbols, Modifiers(l, 4096), "radians", "float3", 1, "deg", "float3");
create_simple_function(symbols, Modifiers(l, 4096), "radians", "float4", 1, "deg", "float4");
create_simple_function(symbols, Modifiers(l, 4096), "degrees", "float", 1, "radparam", "float");
create_simple_function(symbols, Modifiers(l, 4096), "degrees", "float2", 1, "rad", "float2");
create_simple_function(symbols, Modifiers(l, 4096), "degrees", "float3", 1, "rad", "float3");
create_simple_function(symbols, Modifiers(l, 4096), "degrees", "float4", 1, "rad", "float4");
create_simple_function(symbols, m, "any", "bool", 1, "x", "$bvec");
create_simple_function(symbols, Modifiers(l, 4096), "cross", "float3", 2, "a", "float3", "b", "float3");
create_simple_function(symbols, m, "not", "$bvec", 1, "x", "$bvec");
create_simple_function(symbols, m, "greaterThanEqual", "$bvec", 2, "x", "$vec", "y", "$vec");
create_simple_function(symbols, m, "greaterThanEqual", "$bvec", 2, "x", "$hvec", "y", "$hvec");
create_simple_function(symbols, m, "greaterThanEqual", "$bvec", 2, "x", "$ivec", "y", "$ivec");
create_simple_function(symbols, m, "greaterThanEqual", "$bvec", 2, "x", "$uvec", "y", "$uvec");
create_simple_function(symbols, m, "sqrt", "$genType", 1, "x", "$genType");
create_simple_function(symbols, m, "sqrt", "$genHType", 1, "x", "$genHType");
create_simple_function(symbols, m, "all", "bool", 1, "x", "$bvec");
create_simple_function(symbols, m, "lessThanEqual", "$bvec", 2, "x", "$vec", "y", "$vec");
create_simple_function(symbols, m, "lessThanEqual", "$bvec", 2, "x", "$hvec", "y", "$hvec");
create_simple_function(symbols, m, "lessThanEqual", "$bvec", 2, "x", "$ivec", "y", "$ivec");
create_simple_function(symbols, m, "lessThanEqual", "$bvec", 2, "x", "$uvec", "y", "$uvec");
create_simple_function(symbols, m, "greaterThan", "$bvec", 2, "x", "$vec", "y", "$vec");
create_simple_function(symbols, m, "greaterThan", "$bvec", 2, "x", "$hvec", "y", "$hvec");
create_simple_function(symbols, m, "greaterThan", "$bvec", 2, "x", "$ivec", "y", "$ivec");
create_simple_function(symbols, m, "greaterThan", "$bvec", 2, "x", "$uvec", "y", "$uvec");
create_simple_function(symbols, m, "lessThan", "$bvec", 2, "x", "$vec", "y", "$vec");
create_simple_function(symbols, m, "lessThan", "$bvec", 2, "x", "$hvec", "y", "$hvec");
create_simple_function(symbols, m, "lessThan", "$bvec", 2, "x", "$ivec", "y", "$ivec");
create_simple_function(symbols, m, "lessThan", "$bvec", 2, "x", "$uvec", "y", "$uvec");
create_simple_function(symbols, m, "pow", "$genType", 2, "x", "$genType", "y", "$genType");
create_simple_function(symbols, m, "pow", "$genHType", 2, "x", "$genHType", "y", "$genHType");
create_simple_function(symbols, m, "mix", "$genType", 3, "x", "$genType", "y", "$genType", "a", "$genBType");
create_simple_function(symbols, m, "mix", "$genHType", 3, "x", "$genHType", "y", "$genHType", "a", "$genBType");
create_simple_function(symbols, m, "mix", "$genIType", 3, "x", "$genIType", "y", "$genIType", "a", "$genBType");
create_simple_function(symbols, m, "mix", "$genBType", 3, "x", "$genBType", "y", "$genBType", "a", "$genBType");
create_simple_function(symbols, m, "mix", "$genType", 3, "x", "$genType", "y", "$genType", "a", "$genType");
create_simple_function(symbols, m, "mix", "$genType", 3, "x", "$genType", "y", "$genType", "a", "float");
create_simple_function(symbols, m, "mix", "$genHType", 3, "x", "$genHType", "y", "$genHType", "a", "$genHType");
create_simple_function(symbols, m, "mix", "$genHType", 3, "x", "$genHType", "y", "$genHType", "a", "half");
create_simple_function(symbols, m, "saturate", "$genType", 1, "x", "$genType");
create_simple_function(symbols, m, "saturate", "$genHType", 1, "x", "$genHType");
create_simple_function(symbols, m, "notEqual", "$bvec", 2, "x", "$vec", "y", "$vec");
create_simple_function(symbols, m, "notEqual", "$bvec", 2, "x", "$hvec", "y", "$hvec");
create_simple_function(symbols, m, "notEqual", "$bvec", 2, "x", "$ivec", "y", "$ivec");
create_simple_function(symbols, m, "notEqual", "$bvec", 2, "x", "$uvec", "y", "$uvec");
create_simple_function(symbols, m, "notEqual", "$bvec", 2, "x", "$bvec", "y", "$bvec");
create_simple_function(symbols, m, "tan", "$genType", 1, "x", "$genType");
create_simple_function(symbols, m, "tan", "$genHType", 1, "x", "$genHType");
create_simple_function(symbols, m, "inverse", "float2x2", 1, "m", "float2x2");
create_simple_function(symbols, m, "inverse", "float3x3", 1, "m", "float3x3");
create_simple_function(symbols, m, "inverse", "float4x4", 1, "m", "float4x4");
create_simple_function(symbols, m, "inverse", "half2x2", 1, "m", "half2x2");
create_simple_function(symbols, m, "inverse", "half3x3", 1, "m", "half3x3");
create_simple_function(symbols, m, "inverse", "half4x4", 1, "m", "half4x4");
create_simple_function(symbols, Modifiers(l, 4096), "distance", "float", 2, "a", "float2", "b", "float2");
create_simple_function(symbols, Modifiers(l, 4096), "distance", "float", 2, "a", "float3", "b", "float3");
create_simple_function(symbols, Modifiers(l, 4096), "distance", "float", 2, "a", "float4", "b", "float4");
create_simple_function(symbols, m, "sin", "$genType", 1, "x", "$genType");
create_simple_function(symbols, m, "sin", "$genHType", 1, "x", "$genHType");
create_simple_function(symbols, m, "atan", "$genType", 1, "y_over_x", "$genType");
create_simple_function(symbols, m, "atan", "$genHType", 1, "y_over_x", "$genHType");
create_simple_function(symbols, m, "min", "$genType", 2, "x", "$genType", "y", "$genType");
create_simple_function(symbols, m, "min", "$genType", 2, "x", "$genType", "y", "float");
create_simple_function(symbols, m, "min", "$genHType", 2, "x", "$genHType", "y", "$genHType");
create_simple_function(symbols, m, "min", "$genHType", 2, "x", "$genHType", "y", "half");
create_simple_function(symbols, m, "min", "$genIType", 2, "x", "$genIType", "y", "$genIType");
create_simple_function(symbols, m, "min", "$genIType", 2, "x", "$genIType", "y", "int");
create_simple_function(symbols, m, "max", "$genType", 2, "x", "$genType", "y", "$genType");
create_simple_function(symbols, m, "max", "$genType", 2, "x", "$genType", "y", "float");
create_simple_function(symbols, m, "max", "$genHType", 2, "x", "$genHType", "y", "$genHType");
create_simple_function(symbols, m, "max", "$genHType", 2, "x", "$genHType", "y", "half");
create_simple_function(symbols, m, "max", "$genIType", 2, "x", "$genIType", "y", "$genIType");
create_simple_function(symbols, m, "max", "$genIType", 2, "x", "$genIType", "y", "int");
create_simple_function(symbols, m, "clamp", "$genType", 3, "x", "$genType", "minVal", "$genType", "maxVal", "$genType");
create_simple_function(symbols, m, "clamp", "$genType", 3, "x", "$genType", "minVal", "float", "maxVal", "float");
create_simple_function(symbols, m, "clamp", "$genHType", 3, "x", "$genHType", "minVal", "$genHType", "maxVal", "$genHType");
create_simple_function(symbols, m, "clamp", "$genHType", 3, "x", "$genHType", "minVal", "half", "maxVal", "half");
create_simple_function(symbols, m, "clamp", "$genIType", 3, "x", "$genIType", "minVal", "$genIType", "maxVal", "$genIType");
create_simple_function(symbols, m, "clamp", "$genIType", 3, "x", "$genIType", "minVal", "int", "maxVal", "int");
create_simple_function(symbols, m, "cos", "$genType", 1, "y", "$genType");
create_simple_function(symbols, m, "cos", "$genHType", 1, "y", "$genHType");
create_simple_function(symbols, m, "length", "float", 1, "x", "$genType");
create_simple_function(symbols, m, "length", "half", 1, "x", "$genHType");
create_simple_function(symbols, m, "fract", "$genType", 1, "x", "$genType");
create_simple_function(symbols, m, "fract", "$genHType", 1, "x", "$genHType");
create_simple_function(symbols, m, "equal", "$bvec", 2, "x", "$vec", "y", "$vec");
create_simple_function(symbols, m, "equal", "$bvec", 2, "x", "$hvec", "y", "$hvec");
create_simple_function(symbols, m, "equal", "$bvec", 2, "x", "$ivec", "y", "$ivec");
create_simple_function(symbols, m, "equal", "$bvec", 2, "x", "$uvec", "y", "$uvec");
create_simple_function(symbols, m, "equal", "$bvec", 2, "x", "$bvec", "y", "$bvec");
create_simple_function(symbols, Modifiers(l, 4096), "normalize", "float2", 1, "v", "float2");
create_simple_function(symbols, Modifiers(l, 4096), "normalize", "float3", 1, "v", "float3");
create_simple_function(symbols, Modifiers(l, 4096), "normalize", "float4", 1, "v", "float4");
create_simple_function(symbols, m, "dot", "float", 2, "x", "$genType", "y", "$genType");
create_simple_function(symbols, m, "dot", "half", 2, "x", "$genHType", "y", "$genHType");
elements.emplace_back(((void) symbols.reset(symbol_table_for("float degrees(float)", symbols)), pop_symbols(&symbols, new FunctionDefinition(-1, *get_function(*symbols, "float degrees(float)"), std::unique_ptr<Statement>(new Block(-1, make_vector<Statement>(1, new ReturnStatement(std::unique_ptr<Expression>(new BinaryExpression(-1, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["radparam"], (VariableReference::RefKind) 0)), (Token::Kind) 58, std::unique_ptr<Expression>(new FloatLiteral(context, -1, 57.29578018)), *(Type*) (*symbols)["float"])))), symbols, 1)), {}))));
elements.emplace_back(((void) symbols.reset(symbol_table_for("float2 degrees(float2)", symbols)), pop_symbols(&symbols, new FunctionDefinition(-1, *get_function(*symbols, "float2 degrees(float2)"), std::unique_ptr<Statement>(new Block(-1, make_vector<Statement>(1, new ReturnStatement(std::unique_ptr<Expression>(new BinaryExpression(-1, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["rad"], (VariableReference::RefKind) 0)), (Token::Kind) 58, std::unique_ptr<Expression>(new FloatLiteral(context, -1, 57.29578018)), *(Type*) (*symbols)["float2"])))), symbols, 1)), {}))));
elements.emplace_back(((void) symbols.reset(symbol_table_for("float3 degrees(float3)", symbols)), pop_symbols(&symbols, new FunctionDefinition(-1, *get_function(*symbols, "float3 degrees(float3)"), std::unique_ptr<Statement>(new Block(-1, make_vector<Statement>(1, new ReturnStatement(std::unique_ptr<Expression>(new BinaryExpression(-1, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["rad"], (VariableReference::RefKind) 0)), (Token::Kind) 58, std::unique_ptr<Expression>(new FloatLiteral(context, -1, 57.29578018)), *(Type*) (*symbols)["float3"])))), symbols, 1)), {}))));
elements.emplace_back(((void) symbols.reset(symbol_table_for("float4 degrees(float4)", symbols)), pop_symbols(&symbols, new FunctionDefinition(-1, *get_function(*symbols, "float4 degrees(float4)"), std::unique_ptr<Statement>(new Block(-1, make_vector<Statement>(1, new ReturnStatement(std::unique_ptr<Expression>(new BinaryExpression(-1, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["rad"], (VariableReference::RefKind) 0)), (Token::Kind) 58, std::unique_ptr<Expression>(new FloatLiteral(context, -1, 57.29578018)), *(Type*) (*symbols)["float4"])))), symbols, 1)), {}))));
elements.emplace_back(((void) symbols.reset(symbol_table_for("float radians(float)", symbols)), pop_symbols(&symbols, new FunctionDefinition(-1, *get_function(*symbols, "float radians(float)"), std::unique_ptr<Statement>(new Block(-1, make_vector<Statement>(1, new ReturnStatement(std::unique_ptr<Expression>(new BinaryExpression(-1, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["deg"], (VariableReference::RefKind) 0)), (Token::Kind) 58, std::unique_ptr<Expression>(new FloatLiteral(context, -1, 0.01745329)), *(Type*) (*symbols)["float"])))), symbols, 1)), {}))));
elements.emplace_back(((void) symbols.reset(symbol_table_for("float2 radians(float2)", symbols)), pop_symbols(&symbols, new FunctionDefinition(-1, *get_function(*symbols, "float2 radians(float2)"), std::unique_ptr<Statement>(new Block(-1, make_vector<Statement>(1, new ReturnStatement(std::unique_ptr<Expression>(new BinaryExpression(-1, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["deg"], (VariableReference::RefKind) 0)), (Token::Kind) 58, std::unique_ptr<Expression>(new FloatLiteral(context, -1, 0.01745329)), *(Type*) (*symbols)["float2"])))), symbols, 1)), {}))));
elements.emplace_back(((void) symbols.reset(symbol_table_for("float3 radians(float3)", symbols)), pop_symbols(&symbols, new FunctionDefinition(-1, *get_function(*symbols, "float3 radians(float3)"), std::unique_ptr<Statement>(new Block(-1, make_vector<Statement>(1, new ReturnStatement(std::unique_ptr<Expression>(new BinaryExpression(-1, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["deg"], (VariableReference::RefKind) 0)), (Token::Kind) 58, std::unique_ptr<Expression>(new FloatLiteral(context, -1, 0.01745329)), *(Type*) (*symbols)["float3"])))), symbols, 1)), {}))));
elements.emplace_back(((void) symbols.reset(symbol_table_for("float4 radians(float4)", symbols)), pop_symbols(&symbols, new FunctionDefinition(-1, *get_function(*symbols, "float4 radians(float4)"), std::unique_ptr<Statement>(new Block(-1, make_vector<Statement>(1, new ReturnStatement(std::unique_ptr<Expression>(new BinaryExpression(-1, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["deg"], (VariableReference::RefKind) 0)), (Token::Kind) 58, std::unique_ptr<Expression>(new FloatLiteral(context, -1, 0.01745329)), *(Type*) (*symbols)["float4"])))), symbols, 1)), {}))));
elements.emplace_back(((void) symbols.reset(symbol_table_for("float distance(float2, float2)", symbols)), pop_symbols(&symbols, new FunctionDefinition(-1, *get_function(*symbols, "float distance(float2, float2)"), std::unique_ptr<Statement>(new Block(-1, make_vector<Statement>(1, new ReturnStatement(std::unique_ptr<Expression>(new FunctionCall(-1, *(Type*) (*symbols)["float"], *get_function(*symbols, "float length($genType)"), make_vector<Expression>(1, new BinaryExpression(-1, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["a"], (VariableReference::RefKind) 0)), (Token::Kind) 57, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["b"], (VariableReference::RefKind) 0)), *(Type*) (*symbols)["float2"])))))), symbols, 1)), {}))));
elements.emplace_back(((void) symbols.reset(symbol_table_for("float distance(float3, float3)", symbols)), pop_symbols(&symbols, new FunctionDefinition(-1, *get_function(*symbols, "float distance(float3, float3)"), std::unique_ptr<Statement>(new Block(-1, make_vector<Statement>(1, new ReturnStatement(std::unique_ptr<Expression>(new FunctionCall(-1, *(Type*) (*symbols)["float"], *get_function(*symbols, "float length($genType)"), make_vector<Expression>(1, new BinaryExpression(-1, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["a"], (VariableReference::RefKind) 0)), (Token::Kind) 57, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["b"], (VariableReference::RefKind) 0)), *(Type*) (*symbols)["float3"])))))), symbols, 1)), {}))));
elements.emplace_back(((void) symbols.reset(symbol_table_for("float distance(float4, float4)", symbols)), pop_symbols(&symbols, new FunctionDefinition(-1, *get_function(*symbols, "float distance(float4, float4)"), std::unique_ptr<Statement>(new Block(-1, make_vector<Statement>(1, new ReturnStatement(std::unique_ptr<Expression>(new FunctionCall(-1, *(Type*) (*symbols)["float"], *get_function(*symbols, "float length($genType)"), make_vector<Expression>(1, new BinaryExpression(-1, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["a"], (VariableReference::RefKind) 0)), (Token::Kind) 57, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["b"], (VariableReference::RefKind) 0)), *(Type*) (*symbols)["float4"])))))), symbols, 1)), {}))));
elements.emplace_back(((void) symbols.reset(symbol_table_for("float2 normalize(float2)", symbols)), pop_symbols(&symbols, new FunctionDefinition(-1, *get_function(*symbols, "float2 normalize(float2)"), std::unique_ptr<Statement>(new Block(-1, make_vector<Statement>(1, new ReturnStatement(std::unique_ptr<Expression>(new BinaryExpression(-1, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["v"], (VariableReference::RefKind) 0)), (Token::Kind) 59, std::unique_ptr<Expression>(new FunctionCall(-1, *(Type*) (*symbols)["float"], *get_function(*symbols, "float length($genType)"), make_vector<Expression>(1, new VariableReference(-1, *(Variable*) (*symbols)["v"], (VariableReference::RefKind) 0)))), *(Type*) (*symbols)["float2"])))), symbols, 1)), {}))));
elements.emplace_back(((void) symbols.reset(symbol_table_for("float3 normalize(float3)", symbols)), pop_symbols(&symbols, new FunctionDefinition(-1, *get_function(*symbols, "float3 normalize(float3)"), std::unique_ptr<Statement>(new Block(-1, make_vector<Statement>(1, new ReturnStatement(std::unique_ptr<Expression>(new BinaryExpression(-1, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["v"], (VariableReference::RefKind) 0)), (Token::Kind) 59, std::unique_ptr<Expression>(new FunctionCall(-1, *(Type*) (*symbols)["float"], *get_function(*symbols, "float length($genType)"), make_vector<Expression>(1, new VariableReference(-1, *(Variable*) (*symbols)["v"], (VariableReference::RefKind) 0)))), *(Type*) (*symbols)["float3"])))), symbols, 1)), {}))));
elements.emplace_back(((void) symbols.reset(symbol_table_for("float4 normalize(float4)", symbols)), pop_symbols(&symbols, new FunctionDefinition(-1, *get_function(*symbols, "float4 normalize(float4)"), std::unique_ptr<Statement>(new Block(-1, make_vector<Statement>(1, new ReturnStatement(std::unique_ptr<Expression>(new BinaryExpression(-1, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["v"], (VariableReference::RefKind) 0)), (Token::Kind) 59, std::unique_ptr<Expression>(new FunctionCall(-1, *(Type*) (*symbols)["float"], *get_function(*symbols, "float length($genType)"), make_vector<Expression>(1, new VariableReference(-1, *(Variable*) (*symbols)["v"], (VariableReference::RefKind) 0)))), *(Type*) (*symbols)["float4"])))), symbols, 1)), {}))));
elements.emplace_back(((void) symbols.reset(symbol_table_for("float3 cross(float3, float3)", symbols)), pop_symbols(&symbols, new FunctionDefinition(-1, *get_function(*symbols, "float3 cross(float3, float3)"), std::unique_ptr<Statement>(new Block(-1, make_vector<Statement>(1, new ReturnStatement(std::unique_ptr<Expression>(new Constructor(-1, *(Type*) (*symbols)["float3"], make_vector<Expression>(3, new BinaryExpression(-1, std::unique_ptr<Expression>(new BinaryExpression(-1, std::unique_ptr<Expression>(new Swizzle(context, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["a"], (VariableReference::RefKind) 0)), { 1 })), (Token::Kind) 58, std::unique_ptr<Expression>(new Swizzle(context, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["b"], (VariableReference::RefKind) 0)), { 2 })), *(Type*) (*symbols)["float"])), (Token::Kind) 57, std::unique_ptr<Expression>(new BinaryExpression(-1, std::unique_ptr<Expression>(new Swizzle(context, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["a"], (VariableReference::RefKind) 0)), { 2 })), (Token::Kind) 58, std::unique_ptr<Expression>(new Swizzle(context, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["b"], (VariableReference::RefKind) 0)), { 1 })), *(Type*) (*symbols)["float"])), *(Type*) (*symbols)["float"]), new BinaryExpression(-1, std::unique_ptr<Expression>(new BinaryExpression(-1, std::unique_ptr<Expression>(new Swizzle(context, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["a"], (VariableReference::RefKind) 0)), { 2 })), (Token::Kind) 58, std::unique_ptr<Expression>(new Swizzle(context, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["b"], (VariableReference::RefKind) 0)), { 0 })), *(Type*) (*symbols)["float"])), (Token::Kind) 57, std::unique_ptr<Expression>(new BinaryExpression(-1, std::unique_ptr<Expression>(new Swizzle(context, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["a"], (VariableReference::RefKind) 0)), { 0 })), (Token::Kind) 58, std::unique_ptr<Expression>(new Swizzle(context, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["b"], (VariableReference::RefKind) 0)), { 2 })), *(Type*) (*symbols)["float"])), *(Type*) (*symbols)["float"]), new BinaryExpression(-1, std::unique_ptr<Expression>(new BinaryExpression(-1, std::unique_ptr<Expression>(new Swizzle(context, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["a"], (VariableReference::RefKind) 0)), { 0 })), (Token::Kind) 58, std::unique_ptr<Expression>(new Swizzle(context, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["b"], (VariableReference::RefKind) 0)), { 1 })), *(Type*) (*symbols)["float"])), (Token::Kind) 57, std::unique_ptr<Expression>(new BinaryExpression(-1, std::unique_ptr<Expression>(new Swizzle(context, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["a"], (VariableReference::RefKind) 0)), { 1 })), (Token::Kind) 58, std::unique_ptr<Expression>(new Swizzle(context, std::unique_ptr<Expression>(new VariableReference(-1, *(Variable*) (*symbols)["b"], (VariableReference::RefKind) 0)), { 0 })), *(Type*) (*symbols)["float"])), *(Type*) (*symbols)["float"])))))), symbols, 1)), {}))));
}
}
