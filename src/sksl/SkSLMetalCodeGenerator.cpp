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
    if (s[0] == 0) {
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
    if (c.fFunction.fName == "texture" && c.fFunction.fBuiltin) {
        const char* dim = "";
        bool proj = false;
        switch (c.fArguments[0]->fType.dimensions()) {
            case SpvDim1D:
                dim = "1D";
                if (c.fArguments[1]->fType == *fContext.fFloat_Type) {
                    proj = false;
                } else {
                    ASSERT(c.fArguments[1]->fType == *fContext.fFloat2_Type);
                    proj = true;
                }
                break;
            case SpvDim2D:
                dim = "2D";
                if (c.fArguments[1]->fType == *fContext.fFloat2_Type) {
                    proj = false;
                } else {
                    ASSERT(c.fArguments[1]->fType == *fContext.fFloat3_Type);
                    proj = true;
                }
                break;
            case SpvDim3D:
                dim = "3D";
                if (c.fArguments[1]->fType == *fContext.fFloat3_Type) {
                    proj = false;
                } else {
                    ASSERT(c.fArguments[1]->fType == *fContext.fFloat4_Type);
                    proj = true;
                }
                break;
            case SpvDimCube:
                dim = "Cube";
                proj = false;
                break;
            case SpvDimRect:
                dim = "Rect";
                proj = false;
                break;
            case SpvDimBuffer:
                ASSERT(false); // doesn't exist
                dim = "Buffer";
                proj = false;
                break;
            case SpvDimSubpassData:
                ASSERT(false); // doesn't exist
                dim = "SubpassData";
                proj = false;
                break;
        }
        this->write("texture");
        if (proj) {
            this->write("Proj");
        }

    } else {
        this->write(c.fFunction.fName);
    }
    this->write("(");
    const char* separator;
    if (this->requiresInputs(c.fFunction)) {
        this->write("_in");
        separator = ", ";
    } else {
        separator = "";
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
    for (const auto& arg : c.fArguments) {
        this->write(separator);
        separator = ", ";
        this->writeExpression(*arg, kSequence_Precedence);
    }
    this->write(")");
}

void MetalCodeGenerator::writeFragCoord() {
    this->write("_in.position");
}

void MetalCodeGenerator::writeVariableReference(const VariableReference& ref) {
    switch (ref.fVariable.fModifiers.fLayout.fBuiltin) {
        case SK_FRAGCOLOR_BUILTIN:
            if (fProgram.fSettings.fCaps->mustDeclareFragmentShaderOutput()) {
                this->write("sk_FragColor");
            } else {
                this->write("gl_FragColor");
            }
            break;
        case SK_FRAGCOORD_BUILTIN:
            this->writeFragCoord();
            break;
        case SK_VERTEXID_BUILTIN:
            this->write("gl_VertexID");
            break;
        case SK_CLIPDISTANCE_BUILTIN:
            this->write("gl_ClipDistance");
            break;
        case SK_IN_BUILTIN:
            this->write("gl_in");
            break;
        case SK_INVOCATIONID_BUILTIN:
            this->write("gl_InvocationID");
            break;
        default:
            if (ref.fVariable.fModifiers.fFlags & Modifiers::kIn_Flag) {
                this->write("_in.");
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
    if (f.fOwnerKind == FieldAccess::kDefault_OwnerKind) {
        this->writeExpression(*f.fBase, kPostfix_Precedence);
        this->write(".");
    }
    switch (f.fBase->fType.fields()[f.fFieldIndex].fModifiers.fLayout.fBuiltin) {
        case SK_CLIPDISTANCE_BUILTIN:
            this->write("gl_ClipDistance");
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
    this->writeExpression(*b.fLeft, precedence);
    this->write(String(" ") + Compiler::OperatorName(b.fOperator) + " ");
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
    const char* separator;
    if (f.fDeclaration.fName == "main") {
        switch (fProgram.fKind) {
            case Program::kFragment_Kind:
                this->write("fragment half4 _frag");
                break;
            case Program::kVertex_Kind:
                this->write("vertex Output _vert");
                break;
            default:
                ASSERT(false);
        }
        this->write("(Input _in [[stage_in]]");
        separator = ", ";
    } else {
        this->writeType(f.fDeclaration.fReturnType);
        this->write(" " + f.fDeclaration.fName + "(");
        if (this->requiresInputs(f.fDeclaration)) {
            this->write("Input _in");
            separator = ", ";
        } else {
            separator = "";
        }
    }
    for (const auto& param : f.fDeclaration.fParameters) {
        this->write(separator);
        separator = ", ";
        this->writeModifiers(param->fModifiers, false);
        std::vector<int> sizes;
        const Type* type = &param->fType;
        while (type->kind() == Type::kArray_Kind) {
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

    if (fProgram.fKind == Program::kFragment_Kind && f.fDeclaration.fName == "main") {
        this->writeLine("    half4 sk_FragColor;");
    }
    fFunctionHeader = "";
    OutputStream* oldOut = fOut;
    StringStream buffer;
    fOut = &buffer;
    fIndentation++;
    this->writeStatements(((Block&) *f.fBody).fStatements);
    if (fProgram.fKind == Program::kFragment_Kind && f.fDeclaration.fName == "main") {
        this->writeLine("return sk_FragColor;");
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
    if (intf.fTypeName == "sk_PerVertex") {
        return;
    }
    this->writeModifiers(intf.fVariable.fModifiers, true);
    this->writeLine(intf.fTypeName + " {");
    fIndentation++;
    const Type* structType = &intf.fVariable.fType;
    while (structType->kind() == Type::kArray_Kind) {
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
        if (var.fVar->fModifiers.fFlags & (Modifiers::kIn_Flag | Modifiers::kOut_Flag)) {
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

void MetalCodeGenerator::writeInputStruct() {
    this->write("struct Input {\n");
    if (fProgram.fKind == Program::kFragment_Kind) {
        this->write("    float4 position [[position]];\n");
    }
    for (const auto& e : fProgram.fElements) {
        if (e->fKind == ProgramElement::kVar_Kind) {
            VarDeclarations& decls = (VarDeclarations&) *e;
            if (!decls.fVars.size()) {
                continue;
            }
            const Variable& first = *((VarDeclaration&) *decls.fVars[0]).fVar;
            if (first.fModifiers.fFlags & Modifiers::kIn_Flag &&
                first.fModifiers.fLayout.fBuiltin == -1) {
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

void MetalCodeGenerator::writeOutputStruct() {
    this->write("struct Output {\n");
    this->write("    float4 position [[position]];\n");
    for (const auto& e : fProgram.fElements) {
        if (e->fKind == ProgramElement::kVar_Kind) {
            VarDeclarations& decls = (VarDeclarations&) *e;
            if (!decls.fVars.size()) {
                continue;
            }
            const Variable& first = *((VarDeclaration&) *decls.fVars[0]).fVar;
            if (first.fModifiers.fFlags & Modifiers::kOut_Flag &&
                first.fModifiers.fLayout.fBuiltin == -1) {
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
                if (builtin == -1) {
                    // normal var
                    this->writeVarDeclarations(decl, true);
                    this->writeLine();
                } else if (builtin == SK_FRAGCOLOR_BUILTIN) {
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

bool MetalCodeGenerator::requiresInputs(const Expression& e) {
    printf("requiresInputs: %s (%d)\n", e.description().c_str(), e.fKind);
    switch (e.fKind) {
        case Expression::kFunctionCall_Kind: {
            const FunctionCall& f = (const FunctionCall&) e;
            if (this->requiresInputs(f.fFunction)) {
                return true;
            }
            for (const auto& e : f.fArguments) {
                if (this->requiresInputs(*e)) {
                    return true;
                }
            }
            return false;
        }
        case Expression::kConstructor_Kind: {
            const Constructor& c = (const Constructor&) e;
            for (const auto& e : c.fArguments) {
                if (this->requiresInputs(*e)) {
                    return true;
                }
            }
            return false;
        }
        case Expression::kFieldAccess_Kind:
            return this->requiresInputs(*((const FieldAccess&) e).fBase);
        case Expression::kSwizzle_Kind:
            return this->requiresInputs(*((const Swizzle&) e).fBase);
        case Expression::kBinary_Kind: {
            const BinaryExpression& b = (const BinaryExpression&) e;
            return this->requiresInputs(*b.fLeft) || this->requiresInputs(*b.fRight);
        }
        case Expression::kIndex_Kind: {
            const IndexExpression& idx = (const IndexExpression&) e;
            return this->requiresInputs(*idx.fBase) || this->requiresInputs(*idx.fIndex);
        }
        case Expression::kPrefix_Kind:
            return this->requiresInputs(*((const PrefixExpression&) e).fOperand);
        case Expression::kPostfix_Kind:
            return this->requiresInputs(*((const PostfixExpression&) e).fOperand);
        case Expression::kTernary_Kind: {
            const TernaryExpression& t = (const TernaryExpression&) e;
            return this->requiresInputs(*t.fTest) || this->requiresInputs(*t.fIfTrue) ||
                   this->requiresInputs(*t.fIfFalse);
        }
        case Expression::kVariableReference_Kind: {
            const VariableReference& v = (const VariableReference&) e;
            return SK_FRAGCOORD_BUILTIN == v.fVariable.fModifiers.fLayout.fBuiltin ||
                   (v.fVariable.fModifiers.fFlags & Modifiers::kIn_Flag);
        }
        default:
            printf("DEFAULT!\n");
            return false;
    }
}

bool MetalCodeGenerator::requiresInputs(const Statement& s) {
    switch (s.fKind) {
        case Statement::kBlock_Kind: {
            for (const auto& s : ((const Block&) s).fStatements) {
                if (this->requiresInputs(*s)) {
                    return true;
                }
            }
            return false;
        }
        case Statement::kExpression_Kind:
            return this->requiresInputs(*((const ExpressionStatement&) s).fExpression);
        case Statement::kReturn_Kind: {
            const ReturnStatement& r = (const ReturnStatement&) s;
            return r.fExpression && this->requiresInputs(*r.fExpression);
        }
        case Statement::kIf_Kind: {
            const IfStatement& i = (const IfStatement&) s;
            return this->requiresInputs(*i.fTest) ||
                   this->requiresInputs(*i.fIfTrue) ||
                   (i.fIfFalse && this->requiresInputs(*i.fIfFalse));
        }
        case Statement::kFor_Kind: {
            const ForStatement& f = (const ForStatement&) s;
            return this->requiresInputs(*f.fInitializer) ||
                   this->requiresInputs(*f.fTest) ||
                   this->requiresInputs(*f.fNext) ||
                   this->requiresInputs(*f.fStatement);
        }
        case Statement::kWhile_Kind: {
            const WhileStatement& w = (const WhileStatement&) s;
            return this->requiresInputs(*w.fTest) ||
                   this->requiresInputs(*w.fStatement);
        }
        case Statement::kDo_Kind: {
            const DoStatement& d = (const DoStatement&) s;
            return this->requiresInputs(*d.fTest) ||
                   this->requiresInputs(*d.fStatement);
        }
        case Statement::kSwitch_Kind: {
            const SwitchStatement& sw = (const SwitchStatement&) s;
            if (this->requiresInputs(*sw.fValue)) {
                return true;
            }
            for (const auto& c : sw.fCases) {
                for (const auto& st : c->fStatements) {
                    if (this->requiresInputs(*st)) {
                        return true;
                    }
                }
            }
            return false;
        }
        default:
            return false;
    }
}

bool MetalCodeGenerator::requiresInputs(const FunctionDeclaration& f) {
    if (f.fBuiltin) {
        return false;
    }
    auto found = fNeedsInputs.find(&f);
    if (found == fNeedsInputs.end()) {
        for (const auto& e : fProgram.fElements) {
            if (e->fKind == ProgramElement::kFunction_Kind) {
                const FunctionDefinition& def = (const FunctionDefinition&) *e;
                if (&def.fDeclaration == &f) {
                    bool result = this->requiresInputs(*def.fBody);
                    return result;
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
    this->writeInputStruct();
    if (fProgram.fKind == Program::kVertex_Kind) {
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
