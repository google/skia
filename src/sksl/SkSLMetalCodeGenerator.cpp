/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLMetalCodeGenerator.h"

#include "SkSLCompiler.h"
#include "ir/SkSLExpressionStatement.h"
#include "ir/SkSLExtension.h"
#include "ir/SkSLIndexExpression.h"
#include "ir/SkSLModifiersDeclaration.h"
#include "ir/SkSLNop.h"
#include "ir/SkSLVariableReference.h"

namespace SkSL {

void MetalCodeGenerator::write(const char* s) {
    if (!s[0]) {
        return;
    }
    if (fAtLineStart) {
        for (int i = 0; i < fIndentation; i++) {
            fOut->writeText("    ");
        }
    }
    fOut->writeText(s);
    fAtLineStart = false;
}

void MetalCodeGenerator::writeLine(const char* s) {
    this->write(s);
    fOut->writeText(fLineEnding);
    fAtLineStart = true;
}

void MetalCodeGenerator::write(const String& s) {
    this->write(s.c_str());
}

void MetalCodeGenerator::writeLine(const String& s) {
    this->writeLine(s.c_str());
}

void MetalCodeGenerator::writeLine() {
    this->writeLine("");
}

void MetalCodeGenerator::writeExtension(const Extension& ext) {
    this->writeLine("#extension " + ext.fName + " : enable");
}

void MetalCodeGenerator::writeType(const Type& type) {
    switch (type.kind()) {
        case Type::kStruct_Kind:
            for (const Type* search : fWrittenStructs) {
                if (*search == type) {
                    // already written
                    this->write(type.name());
                    return;
                }
            }
            fWrittenStructs.push_back(&type);
            this->writeLine("struct " + type.name() + " {");
            fIndentation++;
            for (const auto& f : type.fields()) {
                this->writeModifiers(f.fModifiers, false);
                // sizes (which must be static in structs) are part of the type name here
                this->writeType(*f.fType);
                this->writeLine(" " + f.fName + ";");
            }
            fIndentation--;
            this->write("}");
            break;
        case Type::kVector_Kind:
            this->writeType(type.componentType());
            this->write(to_string(type.columns()));
            break;
        default:
            this->write(type.name());
    }
}

void MetalCodeGenerator::writeExpression(const Expression& expr, Precedence parentPrecedence) {
    switch (expr.fKind) {
        case Expression::kBinary_Kind:
            this->writeBinaryExpression((BinaryExpression&) expr, parentPrecedence);
            break;
        case Expression::kBoolLiteral_Kind:
            this->writeBoolLiteral((BoolLiteral&) expr);
            break;
        case Expression::kConstructor_Kind:
            this->writeConstructor((Constructor&) expr);
            break;
        case Expression::kIntLiteral_Kind:
            this->writeIntLiteral((IntLiteral&) expr);
            break;
        case Expression::kFieldAccess_Kind:
            this->writeFieldAccess(((FieldAccess&) expr));
            break;
        case Expression::kFloatLiteral_Kind:
            this->writeFloatLiteral(((FloatLiteral&) expr));
            break;
        case Expression::kFunctionCall_Kind:
            this->writeFunctionCall((FunctionCall&) expr);
            break;
        case Expression::kPrefix_Kind:
            this->writePrefixExpression((PrefixExpression&) expr, parentPrecedence);
            break;
        case Expression::kPostfix_Kind:
            this->writePostfixExpression((PostfixExpression&) expr, parentPrecedence);
            break;
        case Expression::kSetting_Kind:
            this->writeSetting((Setting&) expr);
            break;
        case Expression::kSwizzle_Kind:
            this->writeSwizzle((Swizzle&) expr);
            break;
        case Expression::kVariableReference_Kind:
            this->writeVariableReference((VariableReference&) expr);
            break;
        case Expression::kTernary_Kind:
            this->writeTernaryExpression((TernaryExpression&) expr, parentPrecedence);
            break;
        case Expression::kIndex_Kind:
            this->writeIndexExpression((IndexExpression&) expr);
            break;
        default:
            ABORT("unsupported expression: %s", expr.description().c_str());
    }
}

void MetalCodeGenerator::writeFunctionCall(const FunctionCall& c) {
    if (c.fFunction.fBuiltin && "atan" == c.fFunction.fName && 2 == c.fArguments.size()) {
        this->write("atan2");
    } else {
        this->write(c.fFunction.fName);
    }
    this->write("(");
    const char* separator = "";
    if (this->requirements(c.fFunction) & kInputs_Requirement) {
        this->write("_in");
        separator = ", ";
    }
    if (this->requirements(c.fFunction) & kOutputs_Requirement) {
        this->write(separator);
        this->write("_out");
        separator = ", ";
    }
    if (this->requirements(c.fFunction) & kUniforms_Requirement) {
        this->write(separator);
        this->write("_uniforms");
        separator = ", ";
    }
    for (size_t i = 0; i < c.fArguments.size(); ++i) {
        const Expression& arg = *c.fArguments[i];
        this->write(separator);
        separator = ", ";
        if (c.fFunction.fParameters[i]->fModifiers.fFlags & Modifiers::kOut_Flag) {
            this->write("&");
        }
        this->writeExpression(arg, kSequence_Precedence);
    }
    this->write(")");
}

void MetalCodeGenerator::writeConstructor(const Constructor& c) {
    this->writeType(c.fType);
    this->write("(");
    const char* separator = "";
    int scalarCount = 0;
    for (const auto& arg : c.fArguments) {
        this->write(separator);
        separator = ", ";
        if (Type::kMatrix_Kind == c.fType.kind() && Type::kScalar_Kind == arg->fType.kind()) {
            // float2x2(float, float, float, float) doesn't work in Metal 1, so we need to merge to
            // float2x2(float2, float2).
            if (!scalarCount) {
                this->writeType(c.fType.componentType());
                this->write(to_string(c.fType.rows()));
                this->write("(");
            }
            ++scalarCount;
        }
        this->writeExpression(*arg, kSequence_Precedence);
        if (scalarCount && scalarCount == c.fType.rows()) {
            this->write(")");
            scalarCount = 0;
        }
    }
    this->write(")");
}

void MetalCodeGenerator::writeFragCoord() {
    this->write("_in.position");
}

void MetalCodeGenerator::writeVariableReference(const VariableReference& ref) {
    switch (ref.fVariable.fModifiers.fLayout.fBuiltin) {
        case SK_FRAGCOLOR_BUILTIN:
            this->write("sk_FragColor");
            break;
        default:
            if (Variable::kGlobal_Storage == ref.fVariable.fStorage) {
                if (ref.fVariable.fModifiers.fFlags & Modifiers::kIn_Flag) {
                    this->write("_in.");
                } else if (ref.fVariable.fModifiers.fFlags & Modifiers::kOut_Flag) {
                    this->write("_out.");
                } else if (ref.fVariable.fModifiers.fFlags & Modifiers::kUniform_Flag) {
                    this->write("_uniforms.");
                } else {
                    fErrors.error(ref.fVariable.fOffset, "Metal backend does not support global "
                                  "variables");
                }
            }
            this->write(ref.fVariable.fName);
    }
}

void MetalCodeGenerator::writeIndexExpression(const IndexExpression& expr) {
    this->writeExpression(*expr.fBase, kPostfix_Precedence);
    this->write("[");
    this->writeExpression(*expr.fIndex, kTopLevel_Precedence);
    this->write("]");
}

void MetalCodeGenerator::writeFieldAccess(const FieldAccess& f) {
    if (FieldAccess::kDefault_OwnerKind == f.fOwnerKind) {
        this->writeExpression(*f.fBase, kPostfix_Precedence);
        this->write(".");
    }
    switch (f.fBase->fType.fields()[f.fFieldIndex].fModifiers.fLayout.fBuiltin) {
        case SK_CLIPDISTANCE_BUILTIN:
            this->write("gl_ClipDistance");
            break;
        case SK_POSITION_BUILTIN:
            this->write("_out.position");
            break;
        default:
            this->write(f.fBase->fType.fields()[f.fFieldIndex].fName);
    }
}

void MetalCodeGenerator::writeSwizzle(const Swizzle& swizzle) {
    this->writeExpression(*swizzle.fBase, kPostfix_Precedence);
    this->write(".");
    for (int c : swizzle.fComponents) {
        this->write(&("x\0y\0z\0w\0"[c * 2]));
    }
}

MetalCodeGenerator::Precedence MetalCodeGenerator::GetBinaryPrecedence(Token::Kind op) {
    switch (op) {
        case Token::STAR:         // fall through
        case Token::SLASH:        // fall through
        case Token::PERCENT:      return MetalCodeGenerator::kMultiplicative_Precedence;
        case Token::PLUS:         // fall through
        case Token::MINUS:        return MetalCodeGenerator::kAdditive_Precedence;
        case Token::SHL:          // fall through
        case Token::SHR:          return MetalCodeGenerator::kShift_Precedence;
        case Token::LT:           // fall through
        case Token::GT:           // fall through
        case Token::LTEQ:         // fall through
        case Token::GTEQ:         return MetalCodeGenerator::kRelational_Precedence;
        case Token::EQEQ:         // fall through
        case Token::NEQ:          return MetalCodeGenerator::kEquality_Precedence;
        case Token::BITWISEAND:   return MetalCodeGenerator::kBitwiseAnd_Precedence;
        case Token::BITWISEXOR:   return MetalCodeGenerator::kBitwiseXor_Precedence;
        case Token::BITWISEOR:    return MetalCodeGenerator::kBitwiseOr_Precedence;
        case Token::LOGICALAND:   return MetalCodeGenerator::kLogicalAnd_Precedence;
        case Token::LOGICALXOR:   return MetalCodeGenerator::kLogicalXor_Precedence;
        case Token::LOGICALOR:    return MetalCodeGenerator::kLogicalOr_Precedence;
        case Token::EQ:           // fall through
        case Token::PLUSEQ:       // fall through
        case Token::MINUSEQ:      // fall through
        case Token::STAREQ:       // fall through
        case Token::SLASHEQ:      // fall through
        case Token::PERCENTEQ:    // fall through
        case Token::SHLEQ:        // fall through
        case Token::SHREQ:        // fall through
        case Token::LOGICALANDEQ: // fall through
        case Token::LOGICALXOREQ: // fall through
        case Token::LOGICALOREQ:  // fall through
        case Token::BITWISEANDEQ: // fall through
        case Token::BITWISEXOREQ: // fall through
        case Token::BITWISEOREQ:  return MetalCodeGenerator::kAssignment_Precedence;
        case Token::COMMA:        return MetalCodeGenerator::kSequence_Precedence;
        default: ABORT("unsupported binary operator");
    }
}

void MetalCodeGenerator::writeBinaryExpression(const BinaryExpression& b,
                                               Precedence parentPrecedence) {
    Precedence precedence = GetBinaryPrecedence(b.fOperator);
    if (precedence >= parentPrecedence) {
        this->write("(");
    }
    if (Compiler::IsAssignment(b.fOperator) &&
        Expression::kVariableReference_Kind == b.fLeft->fKind &&
        Variable::kParameter_Storage == ((VariableReference&) *b.fLeft).fVariable.fStorage &&
        (((VariableReference&) *b.fLeft).fVariable.fModifiers.fFlags & Modifiers::kOut_Flag)) {
        // writing to an out parameter. Since we have to turn those into pointers, we have to
        // dereference it here.
        this->write("*");
    }
    this->writeExpression(*b.fLeft, precedence);
    if (b.fOperator != Token::EQ && Compiler::IsAssignment(b.fOperator) &&
        Expression::kSwizzle_Kind == b.fLeft->fKind && !b.fLeft->hasSideEffects()) {
        // This doesn't compile in Metal:
        // float4 x = float4(1);
        // x.xy *= float2x2(...);
        // with the error message "non-const reference cannot bind to vector element",
        // but switching it to x.xy = x.xy * float2x2(...) fixes it. We perform this tranformation
        // as long as the LHS has no side effects, and hope for the best otherwise.
        this->write(" = ");
        this->writeExpression(*b.fLeft, kAssignment_Precedence);
        this->write(" ");
        String op = Compiler::OperatorName(b.fOperator);
        ASSERT(op.endsWith("="));
        this->write(op.substr(0, op.size() - 1).c_str());
        this->write(" ");
    } else {
        this->write(String(" ") + Compiler::OperatorName(b.fOperator) + " ");
    }
    this->writeExpression(*b.fRight, precedence);
    if (precedence >= parentPrecedence) {
        this->write(")");
    }
}

void MetalCodeGenerator::writeTernaryExpression(const TernaryExpression& t,
                                               Precedence parentPrecedence) {
    if (kTernary_Precedence >= parentPrecedence) {
        this->write("(");
    }
    this->writeExpression(*t.fTest, kTernary_Precedence);
    this->write(" ? ");
    this->writeExpression(*t.fIfTrue, kTernary_Precedence);
    this->write(" : ");
    this->writeExpression(*t.fIfFalse, kTernary_Precedence);
    if (kTernary_Precedence >= parentPrecedence) {
        this->write(")");
    }
}

void MetalCodeGenerator::writePrefixExpression(const PrefixExpression& p,
                                              Precedence parentPrecedence) {
    if (kPrefix_Precedence >= parentPrecedence) {
        this->write("(");
    }
    this->write(Compiler::OperatorName(p.fOperator));
    this->writeExpression(*p.fOperand, kPrefix_Precedence);
    if (kPrefix_Precedence >= parentPrecedence) {
        this->write(")");
    }
}

void MetalCodeGenerator::writePostfixExpression(const PostfixExpression& p,
                                               Precedence parentPrecedence) {
    if (kPostfix_Precedence >= parentPrecedence) {
        this->write("(");
    }
    this->writeExpression(*p.fOperand, kPostfix_Precedence);
    this->write(Compiler::OperatorName(p.fOperator));
    if (kPostfix_Precedence >= parentPrecedence) {
        this->write(")");
    }
}

void MetalCodeGenerator::writeBoolLiteral(const BoolLiteral& b) {
    this->write(b.fValue ? "true" : "false");
}

void MetalCodeGenerator::writeIntLiteral(const IntLiteral& i) {
    if (i.fType == *fContext.fUInt_Type) {
        this->write(to_string(i.fValue & 0xffffffff) + "u");
    } else {
        this->write(to_string((int32_t) i.fValue));
    }
}

void MetalCodeGenerator::writeFloatLiteral(const FloatLiteral& f) {
    this->write(to_string(f.fValue));
}

void MetalCodeGenerator::writeSetting(const Setting& s) {
    ABORT("internal error; setting was not folded to a constant during compilation\n");
}

void MetalCodeGenerator::writeFunction(const FunctionDefinition& f) {
    const char* separator = "";
    if ("main" == f.fDeclaration.fName) {
        switch (fProgram.fKind) {
            case Program::kFragment_Kind:
                this->write("fragment half4 _frag");
                break;
            case Program::kVertex_Kind:
                this->write("vertex Outputs _vert");
                break;
            default:
                ASSERT(false);
        }
        this->write("(Inputs _in [[stage_in]]");
        if (-1 != fUniformBuffer) {
            this->write(", constant Uniforms& _uniforms [[buffer(" +
                        to_string(fUniformBuffer) + ")]]");
        }
        separator = ", ";
    } else {
        this->writeType(f.fDeclaration.fReturnType);
        this->write(" " + f.fDeclaration.fName + "(");
        if (this->requirements(f.fDeclaration) & kInputs_Requirement) {
            this->write("Inputs _in");
            separator = ", ";
        }
        if (this->requirements(f.fDeclaration) & kOutputs_Requirement) {
            this->write(separator);
            this->write("thread Outputs& _out");
            separator = ", ";
        }
        if (this->requirements(f.fDeclaration) & kUniforms_Requirement) {
            this->write(separator);
            this->write("Uniforms _uniforms");
            separator = ", ";
        }
    }
    for (const auto& param : f.fDeclaration.fParameters) {
        this->write(separator);
        separator = ", ";
        this->writeModifiers(param->fModifiers, false);
        std::vector<int> sizes;
        const Type* type = &param->fType;
        while (Type::kArray_Kind == type->kind()) {
            sizes.push_back(type->columns());
            type = &type->componentType();
        }
        this->writeType(*type);
        if (param->fModifiers.fFlags & Modifiers::kOut_Flag) {
            this->write("*");
        }
        this->write(" " + param->fName);
        for (int s : sizes) {
            if (s <= 0) {
                this->write("[]");
            } else {
                this->write("[" + to_string(s) + "]");
            }
        }
    }
    this->writeLine(") {");

    ASSERT(!fProgram.fSettings.fFragColorIsInOut);

    if ("main" == f.fDeclaration.fName) {
        switch (fProgram.fKind) {
            case Program::kFragment_Kind:
                this->writeLine("    half4 sk_FragColor;");
                break;
            case Program::kVertex_Kind:
                this->writeLine("    Outputs _out;");
                break;
            default:
                ASSERT(false);
        }
    }
    fFunctionHeader = "";
    OutputStream* oldOut = fOut;
    StringStream buffer;
    fOut = &buffer;
    fIndentation++;
    this->writeStatements(((Block&) *f.fBody).fStatements);
    if ("main" == f.fDeclaration.fName) {
        switch (fProgram.fKind) {
            case Program::kFragment_Kind:
                this->writeLine("return sk_FragColor;");
                break;
            case Program::kVertex_Kind:
                this->writeLine("return _out;");
                break;
            default:
                ASSERT(false);
        }
    }
    fIndentation--;
    this->writeLine("}");

    fOut = oldOut;
    this->write(fFunctionHeader);
    this->write(buffer.str());
}

void MetalCodeGenerator::writeModifiers(const Modifiers& modifiers,
                                       bool globalContext) {
    if (modifiers.fFlags & Modifiers::kOut_Flag) {
        this->write("thread ");
    }
    if (modifiers.fFlags & Modifiers::kConst_Flag) {
        this->write("const ");
    }
}

void MetalCodeGenerator::writeInterfaceBlock(const InterfaceBlock& intf) {
    if ("sk_PerVertex" == intf.fTypeName) {
        return;
    }
    this->writeModifiers(intf.fVariable.fModifiers, true);
    this->writeLine(intf.fTypeName + " {");
    fIndentation++;
    const Type* structType = &intf.fVariable.fType;
    while (Type::kArray_Kind == structType->kind()) {
        structType = &structType->componentType();
    }
    for (const auto& f : structType->fields()) {
        this->writeModifiers(f.fModifiers, false);
        this->writeType(*f.fType);
        this->writeLine(" " + f.fName + ";");
    }
    fIndentation--;
    this->write("}");
    if (intf.fInstanceName.size()) {
        this->write(" ");
        this->write(intf.fInstanceName);
        for (const auto& size : intf.fSizes) {
            this->write("[");
            if (size) {
                this->writeExpression(*size, kTopLevel_Precedence);
            }
            this->write("]");
        }
    }
    this->writeLine(";");
}

void MetalCodeGenerator::writeVarInitializer(const Variable& var, const Expression& value) {
    this->writeExpression(value, kTopLevel_Precedence);
}

void MetalCodeGenerator::writeVarDeclarations(const VarDeclarations& decl, bool global) {
    ASSERT(decl.fVars.size() > 0);
    bool wroteType = false;
    for (const auto& stmt : decl.fVars) {
        VarDeclaration& var = (VarDeclaration&) *stmt;
        if (var.fVar->fModifiers.fFlags & (Modifiers::kIn_Flag | Modifiers::kOut_Flag |
                                           Modifiers::kUniform_Flag)) {
            ASSERT(global);
            continue;
        }
        if (wroteType) {
            this->write(", ");
        } else {
            this->writeModifiers(var.fVar->fModifiers, global);
            this->writeType(decl.fBaseType);
            this->write(" ");
            wroteType = true;
        }
        this->write(var.fVar->fName);
        for (const auto& size : var.fSizes) {
            this->write("[");
            if (size) {
                this->writeExpression(*size, kTopLevel_Precedence);
            }
            this->write("]");
        }
        if (var.fValue) {
            this->write(" = ");
            this->writeVarInitializer(*var.fVar, *var.fValue);
        }
        if (!fFoundImageDecl && var.fVar->fType == *fContext.fImage2D_Type) {
            if (fProgram.fSettings.fCaps->imageLoadStoreExtensionString()) {
                fHeader.writeText("#extension ");
                fHeader.writeText(fProgram.fSettings.fCaps->imageLoadStoreExtensionString());
                fHeader.writeText(" : require\n");
            }
            fFoundImageDecl = true;
        }
    }
    if (wroteType) {
        this->write(";");
    }
}

void MetalCodeGenerator::writeStatement(const Statement& s) {
    switch (s.fKind) {
        case Statement::kBlock_Kind:
            this->writeBlock((Block&) s);
            break;
        case Statement::kExpression_Kind:
            this->writeExpression(*((ExpressionStatement&) s).fExpression, kTopLevel_Precedence);
            this->write(";");
            break;
        case Statement::kReturn_Kind:
            this->writeReturnStatement((ReturnStatement&) s);
            break;
        case Statement::kVarDeclarations_Kind:
            this->writeVarDeclarations(*((VarDeclarationsStatement&) s).fDeclaration, false);
            break;
        case Statement::kIf_Kind:
            this->writeIfStatement((IfStatement&) s);
            break;
        case Statement::kFor_Kind:
            this->writeForStatement((ForStatement&) s);
            break;
        case Statement::kWhile_Kind:
            this->writeWhileStatement((WhileStatement&) s);
            break;
        case Statement::kDo_Kind:
            this->writeDoStatement((DoStatement&) s);
            break;
        case Statement::kSwitch_Kind:
            this->writeSwitchStatement((SwitchStatement&) s);
            break;
        case Statement::kBreak_Kind:
            this->write("break;");
            break;
        case Statement::kContinue_Kind:
            this->write("continue;");
            break;
        case Statement::kDiscard_Kind:
            this->write("discard;");
            break;
        case Statement::kNop_Kind:
            this->write(";");
            break;
        default:
            ABORT("unsupported statement: %s", s.description().c_str());
    }
}

void MetalCodeGenerator::writeStatements(const std::vector<std::unique_ptr<Statement>>& statements) {
    for (const auto& s : statements) {
        if (!s->isEmpty()) {
            this->writeStatement(*s);
            this->writeLine();
        }
    }
}

void MetalCodeGenerator::writeBlock(const Block& b) {
    this->writeLine("{");
    fIndentation++;
    this->writeStatements(b.fStatements);
    fIndentation--;
    this->write("}");
}

void MetalCodeGenerator::writeIfStatement(const IfStatement& stmt) {
    this->write("if (");
    this->writeExpression(*stmt.fTest, kTopLevel_Precedence);
    this->write(") ");
    this->writeStatement(*stmt.fIfTrue);
    if (stmt.fIfFalse) {
        this->write(" else ");
        this->writeStatement(*stmt.fIfFalse);
    }
}

void MetalCodeGenerator::writeForStatement(const ForStatement& f) {
    this->write("for (");
    if (f.fInitializer && !f.fInitializer->isEmpty()) {
        this->writeStatement(*f.fInitializer);
    } else {
        this->write("; ");
    }
    if (f.fTest) {
        this->writeExpression(*f.fTest, kTopLevel_Precedence);
    }
    this->write("; ");
    if (f.fNext) {
        this->writeExpression(*f.fNext, kTopLevel_Precedence);
    }
    this->write(") ");
    this->writeStatement(*f.fStatement);
}

void MetalCodeGenerator::writeWhileStatement(const WhileStatement& w) {
    this->write("while (");
    this->writeExpression(*w.fTest, kTopLevel_Precedence);
    this->write(") ");
    this->writeStatement(*w.fStatement);
}

void MetalCodeGenerator::writeDoStatement(const DoStatement& d) {
    this->write("do ");
    this->writeStatement(*d.fStatement);
    this->write(" while (");
    this->writeExpression(*d.fTest, kTopLevel_Precedence);
    this->write(");");
}

void MetalCodeGenerator::writeSwitchStatement(const SwitchStatement& s) {
    this->write("switch (");
    this->writeExpression(*s.fValue, kTopLevel_Precedence);
    this->writeLine(") {");
    fIndentation++;
    for (const auto& c : s.fCases) {
        if (c->fValue) {
            this->write("case ");
            this->writeExpression(*c->fValue, kTopLevel_Precedence);
            this->writeLine(":");
        } else {
            this->writeLine("default:");
        }
        fIndentation++;
        for (const auto& stmt : c->fStatements) {
            this->writeStatement(*stmt);
            this->writeLine();
        }
        fIndentation--;
    }
    fIndentation--;
    this->write("}");
}

void MetalCodeGenerator::writeReturnStatement(const ReturnStatement& r) {
    this->write("return");
    if (r.fExpression) {
        this->write(" ");
        this->writeExpression(*r.fExpression, kTopLevel_Precedence);
    }
    this->write(";");
}

void MetalCodeGenerator::writeHeader() {
    this->write("#include <metal_stdlib>\n");
    this->write("#include <simd/simd.h>\n");
    this->write("using namespace metal;\n");
}

void MetalCodeGenerator::writeUniformStruct() {
    for (const auto& e : fProgram.fElements) {
        if (ProgramElement::kVar_Kind == e->fKind) {
            VarDeclarations& decls = (VarDeclarations&) *e;
            if (!decls.fVars.size()) {
                continue;
            }
            const Variable& first = *((VarDeclaration&) *decls.fVars[0]).fVar;
            if (first.fModifiers.fFlags & Modifiers::kUniform_Flag) {
                if (-1 == fUniformBuffer) {
                    this->write("struct Uniforms {\n");
                    fUniformBuffer = first.fModifiers.fLayout.fSet;
                    if (-1 == fUniformBuffer) {
                        fErrors.error(decls.fOffset, "Metal uniforms must have 'layout(set=...)'");
                    }
                } else if (first.fModifiers.fLayout.fSet != fUniformBuffer) {
                    if (-1 == fUniformBuffer) {
                        fErrors.error(decls.fOffset, "Metal backend requires all uniforms to have "
                                    "the same 'layout(set=...)'");
                    }
                }
                this->write("    ");
                this->writeType(first.fType);
                this->write(" ");
                for (const auto& stmt : decls.fVars) {
                    VarDeclaration& var = (VarDeclaration&) *stmt;
                    this->write(var.fVar->fName);
                }
                this->write(";\n");
            }
        }
    }
    if (-1 != fUniformBuffer) {
        this->write("};\n");
    }
}

void MetalCodeGenerator::writeInputStruct() {
    this->write("struct Inputs {\n");
    if (Program::kFragment_Kind == fProgram.fKind) {
        this->write("    float4 position [[position]];\n");
    }
    for (const auto& e : fProgram.fElements) {
        if (ProgramElement::kVar_Kind == e->fKind) {
            VarDeclarations& decls = (VarDeclarations&) *e;
            if (!decls.fVars.size()) {
                continue;
            }
            const Variable& first = *((VarDeclaration&) *decls.fVars[0]).fVar;
            if (first.fModifiers.fFlags & Modifiers::kIn_Flag &&
                -1 == first.fModifiers.fLayout.fBuiltin) {
                this->write("    ");
                this->writeType(first.fType);
                this->write(" ");
                for (const auto& stmt : decls.fVars) {
                    VarDeclaration& var = (VarDeclaration&) *stmt;
                    this->write(var.fVar->fName);
                    if (-1 != var.fVar->fModifiers.fLayout.fLocation) {
                        this->write("  [[attribute(" +
                                    to_string(var.fVar->fModifiers.fLayout.fLocation) + ")]]");
                    }
                }
                this->write(";\n");
            }
        }
    }
    this->write("};\n");
}

void MetalCodeGenerator::writeOutputStruct() {
    this->write("struct Outputs {\n");
    this->write("    float4 position [[position]];\n");
    for (const auto& e : fProgram.fElements) {
        if (ProgramElement::kVar_Kind == e->fKind) {
            VarDeclarations& decls = (VarDeclarations&) *e;
            if (!decls.fVars.size()) {
                continue;
            }
            const Variable& first = *((VarDeclaration&) *decls.fVars[0]).fVar;
            if (first.fModifiers.fFlags & Modifiers::kOut_Flag &&
                -1 == first.fModifiers.fLayout.fBuiltin) {
                this->write("    ");
                this->writeType(first.fType);
                this->write(" ");
                for (const auto& stmt : decls.fVars) {
                    VarDeclaration& var = (VarDeclaration&) *stmt;
                    this->write(var.fVar->fName);
                }
                this->write(";\n");
            }
        }
    }    this->write("};\n");
}

void MetalCodeGenerator::writeProgramElement(const ProgramElement& e) {
    switch (e.fKind) {
        case ProgramElement::kExtension_Kind:
            break;
        case ProgramElement::kVar_Kind: {
            VarDeclarations& decl = (VarDeclarations&) e;
            if (decl.fVars.size() > 0) {
                int builtin = ((VarDeclaration&) *decl.fVars[0]).fVar->fModifiers.fLayout.fBuiltin;
                if (-1 == builtin) {
                    // normal var
                    this->writeVarDeclarations(decl, true);
                    this->writeLine();
                } else if (SK_FRAGCOLOR_BUILTIN == builtin) {
                    // ignore
                }
            }
            break;
        }
        case ProgramElement::kInterfaceBlock_Kind:
            this->writeInterfaceBlock((InterfaceBlock&) e);
            break;
        case ProgramElement::kFunction_Kind:
            this->writeFunction((FunctionDefinition&) e);
            break;
        case ProgramElement::kModifiers_Kind:
            this->writeModifiers(((ModifiersDeclaration&) e).fModifiers, true);
            this->writeLine(";");
            break;
        default:
            printf("%s\n", e.description().c_str());
            ABORT("unsupported program element");
    }
}

MetalCodeGenerator::Requirements MetalCodeGenerator::requirements(const Expression& e) {
    switch (e.fKind) {
        case Expression::kFunctionCall_Kind: {
            const FunctionCall& f = (const FunctionCall&) e;
            Requirements result = this->requirements(f.fFunction);
            for (const auto& e : f.fArguments) {
                result |= this->requirements(*e);
            }
            return result;
        }
        case Expression::kConstructor_Kind: {
            const Constructor& c = (const Constructor&) e;
            Requirements result = kNo_Requirements;
            for (const auto& e : c.fArguments) {
                result |= this->requirements(*e);
            }
            return result;
        }
        case Expression::kFieldAccess_Kind:
            return this->requirements(*((const FieldAccess&) e).fBase);
        case Expression::kSwizzle_Kind:
            return this->requirements(*((const Swizzle&) e).fBase);
        case Expression::kBinary_Kind: {
            const BinaryExpression& b = (const BinaryExpression&) e;
            return this->requirements(*b.fLeft) | this->requirements(*b.fRight);
        }
        case Expression::kIndex_Kind: {
            const IndexExpression& idx = (const IndexExpression&) e;
            return this->requirements(*idx.fBase) | this->requirements(*idx.fIndex);
        }
        case Expression::kPrefix_Kind:
            return this->requirements(*((const PrefixExpression&) e).fOperand);
        case Expression::kPostfix_Kind:
            return this->requirements(*((const PostfixExpression&) e).fOperand);
        case Expression::kTernary_Kind: {
            const TernaryExpression& t = (const TernaryExpression&) e;
            return this->requirements(*t.fTest) | this->requirements(*t.fIfTrue) |
                   this->requirements(*t.fIfFalse);
        }
        case Expression::kVariableReference_Kind: {
            const VariableReference& v = (const VariableReference&) e;
            Requirements result = kNo_Requirements;
            if (v.fVariable.fModifiers.fLayout.fBuiltin == SK_FRAGCOORD_BUILTIN) {
                result = kInputs_Requirement;
            } else if (Variable::kGlobal_Storage == v.fVariable.fStorage) {
                if (v.fVariable.fModifiers.fFlags & Modifiers::kIn_Flag) {
                    result = kInputs_Requirement;
                } else if (v.fVariable.fModifiers.fFlags & Modifiers::kOut_Flag) {
                    result = kOutputs_Requirement;
                } else if (v.fVariable.fModifiers.fFlags & Modifiers::kUniform_Flag) {
                    result = kUniforms_Requirement;
                }
            }
            return result;
        }
        default:
            return kNo_Requirements;
    }
}

MetalCodeGenerator::Requirements MetalCodeGenerator::requirements(const Statement& s) {
    switch (s.fKind) {
        case Statement::kBlock_Kind: {
            Requirements result = kNo_Requirements;
            for (const auto& child : ((const Block&) s).fStatements) {
                result |= this->requirements(*child);
            }
            return result;
        }
        case Statement::kExpression_Kind:
            return this->requirements(*((const ExpressionStatement&) s).fExpression);
        case Statement::kReturn_Kind: {
            const ReturnStatement& r = (const ReturnStatement&) s;
            if (r.fExpression) {
                return this->requirements(*r.fExpression);
            }
            return kNo_Requirements;
        }
        case Statement::kIf_Kind: {
            const IfStatement& i = (const IfStatement&) s;
            return this->requirements(*i.fTest) |
                   this->requirements(*i.fIfTrue) |
                   (i.fIfFalse && this->requirements(*i.fIfFalse));
        }
        case Statement::kFor_Kind: {
            const ForStatement& f = (const ForStatement&) s;
            return this->requirements(*f.fInitializer) |
                   this->requirements(*f.fTest) |
                   this->requirements(*f.fNext) |
                   this->requirements(*f.fStatement);
        }
        case Statement::kWhile_Kind: {
            const WhileStatement& w = (const WhileStatement&) s;
            return this->requirements(*w.fTest) |
                   this->requirements(*w.fStatement);
        }
        case Statement::kDo_Kind: {
            const DoStatement& d = (const DoStatement&) s;
            return this->requirements(*d.fTest) |
                   this->requirements(*d.fStatement);
        }
        case Statement::kSwitch_Kind: {
            const SwitchStatement& sw = (const SwitchStatement&) s;
            Requirements result = this->requirements(*sw.fValue);
            for (const auto& c : sw.fCases) {
                for (const auto& st : c->fStatements) {
                    result |= this->requirements(*st);
                }
            }
            return result;
        }
        default:
            return kNo_Requirements;
    }
}

MetalCodeGenerator::Requirements MetalCodeGenerator::requirements(const FunctionDeclaration& f) {
    if (f.fBuiltin) {
        return kNo_Requirements;
    }
    auto found = fRequirements.find(&f);
    if (found == fRequirements.end()) {
        for (const auto& e : fProgram.fElements) {
            if (ProgramElement::kFunction_Kind == e->fKind) {
                const FunctionDefinition& def = (const FunctionDefinition&) *e;
                if (&def.fDeclaration == &f) {
                    Requirements reqs = this->requirements(*def.fBody);
                    fRequirements[&f] = reqs;
                    return reqs;
                }
            }
        }
    }
    return found->second;
}

bool MetalCodeGenerator::generateCode() {
    OutputStream* rawOut = fOut;
    fOut = &fHeader;
    fProgramKind = fProgram.fKind;
    this->writeHeader();
    this->writeUniformStruct();
    this->writeInputStruct();
    if (Program::kVertex_Kind == fProgram.fKind) {
        this->writeOutputStruct();
    }
    StringStream body;
    fOut = &body;
    for (const auto& e : fProgram.fElements) {
        this->writeProgramElement(*e);
    }
    fOut = rawOut;

    write_stringstream(fHeader, *rawOut);
    write_stringstream(body, *rawOut);
    return true;
}

}
