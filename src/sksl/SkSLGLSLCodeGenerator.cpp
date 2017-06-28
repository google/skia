/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLGLSLCodeGenerator.h"

#include "GLSL.std.450.h"

#include "SkSLCompiler.h"
#include "ir/SkSLExpressionStatement.h"
#include "ir/SkSLExtension.h"
#include "ir/SkSLIndexExpression.h"
#include "ir/SkSLModifiersDeclaration.h"
#include "ir/SkSLNop.h"
#include "ir/SkSLVariableReference.h"

namespace SkSL {

void GLSLCodeGenerator::write(const char* s) {
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

void GLSLCodeGenerator::writeLine(const char* s) {
    this->write(s);
    fOut->writeText(fLineEnding);
    fAtLineStart = true;
}

void GLSLCodeGenerator::write(const String& s) {
    this->write(s.c_str());
}

void GLSLCodeGenerator::writeLine(const String& s) {
    this->writeLine(s.c_str());
}

void GLSLCodeGenerator::writeLine() {
    this->writeLine("");
}

void GLSLCodeGenerator::writeExtension(const Extension& ext) {
    this->writeLine("#extension " + ext.fName + " : enable");
}

void GLSLCodeGenerator::writeType(const Type& type) {
    if (type.kind() == Type::kStruct_Kind) {
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
    } else {
        this->write(type.name());
    }
}

void GLSLCodeGenerator::writeExpression(const Expression& expr, Precedence parentPrecedence) {
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

static bool is_abs(Expression& expr) {
    if (expr.fKind != Expression::kFunctionCall_Kind) {
        return false;
    }
    return ((FunctionCall&) expr).fFunction.fName == "abs";
}

// turns min(abs(x), y) into ((tmpVar1 = abs(x)) < (tmpVar2 = y) ? tmpVar1 : tmpVar2) to avoid a
// Tegra3 compiler bug.
void GLSLCodeGenerator::writeMinAbsHack(Expression& absExpr, Expression& otherExpr) {
    ASSERT(!fProgram.fSettings.fCaps->canUseMinAndAbsTogether());
    String tmpVar1 = "minAbsHackVar" + to_string(fVarCount++);
    String tmpVar2 = "minAbsHackVar" + to_string(fVarCount++);
    this->fFunctionHeader += "    " + absExpr.fType.name() + " " + tmpVar1 + ";\n";
    this->fFunctionHeader += "    " + otherExpr.fType.name() + " " + tmpVar2 + ";\n";
    this->write("((" + tmpVar1 + " = ");
    this->writeExpression(absExpr, kTopLevel_Precedence);
    this->write(") < (" + tmpVar2 + " = ");
    this->writeExpression(otherExpr, kAssignment_Precedence);
    this->write(") ? " + tmpVar1 + " : " + tmpVar2 + ")");
}

void GLSLCodeGenerator::writeFunctionCall(const FunctionCall& c) {
    if (!fProgram.fSettings.fCaps->canUseMinAndAbsTogether() && c.fFunction.fName == "min" &&
        c.fFunction.fBuiltin) {
        ASSERT(c.fArguments.size() == 2);
        if (is_abs(*c.fArguments[0])) {
            this->writeMinAbsHack(*c.fArguments[0], *c.fArguments[1]);
            return;
        }
        if (is_abs(*c.fArguments[1])) {
            // note that this violates the GLSL left-to-right evaluation semantics. I doubt it will
            // ever end up mattering, but it's worth calling out.
            this->writeMinAbsHack(*c.fArguments[1], *c.fArguments[0]);
            return;
        }
    }
    if (fProgram.fSettings.fCaps->mustForceNegatedAtanParamToFloat() &&
        c.fFunction.fName == "atan" &&
        c.fFunction.fBuiltin && c.fArguments.size() == 2 &&
        c.fArguments[1]->fKind == Expression::kPrefix_Kind) {
        const PrefixExpression& p = (PrefixExpression&) *c.fArguments[1];
        if (p.fOperator == Token::MINUS) {
            this->write("atan(");
            this->writeExpression(*c.fArguments[0], kSequence_Precedence);
            this->write(", -1.0 * ");
            this->writeExpression(*p.fOperand, kMultiplicative_Precedence);
            this->write(")");
            return;
        }
    }
    if (!fFoundDerivatives && (c.fFunction.fName == "dFdx" || c.fFunction.fName == "dFdy") &&
        c.fFunction.fBuiltin && fProgram.fSettings.fCaps->shaderDerivativeExtensionString()) {
        ASSERT(fProgram.fSettings.fCaps->shaderDerivativeSupport());
        fHeader.writeText("#extension ");
        fHeader.writeText(fProgram.fSettings.fCaps->shaderDerivativeExtensionString());
        fHeader.writeText(" : require\n");
        fFoundDerivatives = true;
    }
    if (c.fFunction.fName == "texture" && c.fFunction.fBuiltin) {
        const char* dim = "";
        bool proj = false;
        switch (c.fArguments[0]->fType.dimensions()) {
            case SpvDim1D:
                dim = "1D";
                if (c.fArguments[1]->fType == *fContext.fFloat_Type) {
                    proj = false;
                } else {
                    ASSERT(c.fArguments[1]->fType == *fContext.fVec2_Type);
                    proj = true;
                }
                break;
            case SpvDim2D:
                dim = "2D";
                if (c.fArguments[1]->fType == *fContext.fVec2_Type) {
                    proj = false;
                } else {
                    ASSERT(c.fArguments[1]->fType == *fContext.fVec3_Type);
                    proj = true;
                }
                break;
            case SpvDim3D:
                dim = "3D";
                if (c.fArguments[1]->fType == *fContext.fVec3_Type) {
                    proj = false;
                } else {
                    ASSERT(c.fArguments[1]->fType == *fContext.fVec4_Type);
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
        if (fProgram.fSettings.fCaps->generation() < k130_GrGLSLGeneration) {
            this->write(dim);
        }
        if (proj) {
            this->write("Proj");
        }

    } else {
        this->write(c.fFunction.fName);
    }
    this->write("(");
    const char* separator = "";
    for (const auto& arg : c.fArguments) {
        this->write(separator);
        separator = ", ";
        this->writeExpression(*arg, kSequence_Precedence);
    }
    this->write(")");
}

void GLSLCodeGenerator::writeConstructor(const Constructor& c) {
    this->write(c.fType.name() + "(");
    const char* separator = "";
    for (const auto& arg : c.fArguments) {
        this->write(separator);
        separator = ", ";
        this->writeExpression(*arg, kSequence_Precedence);
    }
    this->write(")");
}

void GLSLCodeGenerator::writeFragCoord() {
    // We only declare "gl_FragCoord" when we're in the case where we want to use layout qualifiers
    // to reverse y. Otherwise it isn't necessary and whether the "in" qualifier appears in the
    // declaration varies in earlier GLSL specs. So it is simpler to omit it.
    if (!fProgram.fSettings.fFlipY) {
        this->write("gl_FragCoord");
    } else if (const char* extension =
               fProgram.fSettings.fCaps->fragCoordConventionsExtensionString()) {
        if (!fSetupFragPositionGlobal) {
            if (fProgram.fSettings.fCaps->generation() < k150_GrGLSLGeneration) {
                fHeader.writeText("#extension ");
                fHeader.writeText(extension);
                fHeader.writeText(" : require\n");
            }
            fHeader.writeText("layout(origin_upper_left) in vec4 gl_FragCoord;\n");
            fSetupFragPositionGlobal = true;
        }
        this->write("gl_FragCoord");
    } else {
        if (!fSetupFragPositionGlobal) {
            // The Adreno compiler seems to be very touchy about access to "gl_FragCoord".
            // Accessing glFragCoord.zw can cause a program to fail to link. Additionally,
            // depending on the surrounding code, accessing .xy with a uniform involved can
            // do the same thing. Copying gl_FragCoord.xy into a temp vec2 beforehand
            // (and only accessing .xy) seems to "fix" things.
            const char* precision = fProgram.fSettings.fCaps->usesPrecisionModifiers() ? "highp "
                                                                                       : "";
            fHeader.writeText("uniform ");
            fHeader.writeText(precision);
            fHeader.writeText("float " SKSL_RTHEIGHT_NAME ";\n");
            fSetupFragPositionGlobal = true;
        }
        if (!fSetupFragPositionLocal) {
            const char* precision = fProgram.fSettings.fCaps->usesPrecisionModifiers() ? "highp "
                                                                                       : "";
            fFunctionHeader += precision;
            fFunctionHeader += "    vec2 _sktmpCoord = gl_FragCoord.xy;\n";
            fFunctionHeader += precision;
            fFunctionHeader += "    vec4 sk_FragCoord = vec4(_sktmpCoord.x, " SKSL_RTHEIGHT_NAME
                               " - _sktmpCoord.y, 1.0, 1.0);\n";
            fSetupFragPositionLocal = true;
        }
        this->write("sk_FragCoord");
    }
}


void GLSLCodeGenerator::writeVariableReference(const VariableReference& ref) {
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
            this->write(ref.fVariable.fName);
    }
}

void GLSLCodeGenerator::writeIndexExpression(const IndexExpression& expr) {
    this->writeExpression(*expr.fBase, kPostfix_Precedence);
    this->write("[");
    this->writeExpression(*expr.fIndex, kTopLevel_Precedence);
    this->write("]");
}

void GLSLCodeGenerator::writeFieldAccess(const FieldAccess& f) {
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

void GLSLCodeGenerator::writeSwizzle(const Swizzle& swizzle) {
    this->writeExpression(*swizzle.fBase, kPostfix_Precedence);
    this->write(".");
    for (int c : swizzle.fComponents) {
        this->write(&("x\0y\0z\0w\0"[c * 2]));
    }
}

GLSLCodeGenerator::Precedence GLSLCodeGenerator::GetBinaryPrecedence(Token::Kind op) {
    switch (op) {
        case Token::STAR:         // fall through
        case Token::SLASH:        // fall through
        case Token::PERCENT:      return GLSLCodeGenerator::kMultiplicative_Precedence;
        case Token::PLUS:         // fall through
        case Token::MINUS:        return GLSLCodeGenerator::kAdditive_Precedence;
        case Token::SHL:          // fall through
        case Token::SHR:          return GLSLCodeGenerator::kShift_Precedence;
        case Token::LT:           // fall through
        case Token::GT:           // fall through
        case Token::LTEQ:         // fall through
        case Token::GTEQ:         return GLSLCodeGenerator::kRelational_Precedence;
        case Token::EQEQ:         // fall through
        case Token::NEQ:          return GLSLCodeGenerator::kEquality_Precedence;
        case Token::BITWISEAND:   return GLSLCodeGenerator::kBitwiseAnd_Precedence;
        case Token::BITWISEXOR:   return GLSLCodeGenerator::kBitwiseXor_Precedence;
        case Token::BITWISEOR:    return GLSLCodeGenerator::kBitwiseOr_Precedence;
        case Token::LOGICALAND:   return GLSLCodeGenerator::kLogicalAnd_Precedence;
        case Token::LOGICALXOR:   return GLSLCodeGenerator::kLogicalXor_Precedence;
        case Token::LOGICALOR:    return GLSLCodeGenerator::kLogicalOr_Precedence;
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
        case Token::BITWISEOREQ:  return GLSLCodeGenerator::kAssignment_Precedence;
        case Token::COMMA:        return GLSLCodeGenerator::kSequence_Precedence;
        default: ABORT("unsupported binary operator");
    }
}

void GLSLCodeGenerator::writeBinaryExpression(const BinaryExpression& b,
                                              Precedence parentPrecedence) {
    Precedence precedence = GetBinaryPrecedence(b.fOperator);
    if (precedence >= parentPrecedence) {
        this->write("(");
    }
    this->writeExpression(*b.fLeft, precedence);
    this->write(" " + Token::OperatorName(b.fOperator) + " ");
    this->writeExpression(*b.fRight, precedence);
    if (precedence >= parentPrecedence) {
        this->write(")");
    }
}

void GLSLCodeGenerator::writeTernaryExpression(const TernaryExpression& t,
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

void GLSLCodeGenerator::writePrefixExpression(const PrefixExpression& p,
                                              Precedence parentPrecedence) {
    if (kPrefix_Precedence >= parentPrecedence) {
        this->write("(");
    }
    this->write(Token::OperatorName(p.fOperator));
    this->writeExpression(*p.fOperand, kPrefix_Precedence);
    if (kPrefix_Precedence >= parentPrecedence) {
        this->write(")");
    }
}

void GLSLCodeGenerator::writePostfixExpression(const PostfixExpression& p,
                                               Precedence parentPrecedence) {
    if (kPostfix_Precedence >= parentPrecedence) {
        this->write("(");
    }
    this->writeExpression(*p.fOperand, kPostfix_Precedence);
    this->write(Token::OperatorName(p.fOperator));
    if (kPostfix_Precedence >= parentPrecedence) {
        this->write(")");
    }
}

void GLSLCodeGenerator::writeBoolLiteral(const BoolLiteral& b) {
    this->write(b.fValue ? "true" : "false");
}

void GLSLCodeGenerator::writeIntLiteral(const IntLiteral& i) {
    if (i.fType == *fContext.fUInt_Type) {
        this->write(to_string(i.fValue & 0xffffffff) + "u");
    } else {
        this->write(to_string((int32_t) i.fValue));
    }
}

void GLSLCodeGenerator::writeFloatLiteral(const FloatLiteral& f) {
    this->write(to_string(f.fValue));
}

void GLSLCodeGenerator::writeSetting(const Setting& s) {
    ABORT("internal error; setting was not folded to a constant during compilation\n");
}

void GLSLCodeGenerator::writeFunction(const FunctionDefinition& f) {
    this->writeType(f.fDeclaration.fReturnType);
    this->write(" " + f.fDeclaration.fName + "(");
    const char* separator = "";
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

    fFunctionHeader = "";
    OutputStream* oldOut = fOut;
    StringStream buffer;
    fOut = &buffer;
    fIndentation++;
    this->writeStatements(((Block&) *f.fBody).fStatements);
    fIndentation--;
    this->writeLine("}");

    fOut = oldOut;
    this->write(fFunctionHeader);
    this->write(buffer.str());
}

void GLSLCodeGenerator::writeModifiers(const Modifiers& modifiers,
                                       bool globalContext) {
    if (modifiers.fFlags & Modifiers::kFlat_Flag) {
        this->write("flat ");
    }
    if (modifiers.fFlags & Modifiers::kNoPerspective_Flag) {
        this->write("noperspective ");
    }
    String layout = modifiers.fLayout.description();
    if (layout.size()) {
        this->write(layout + " ");
    }
    if (modifiers.fFlags & Modifiers::kReadOnly_Flag) {
        this->write("readonly ");
    }
    if (modifiers.fFlags & Modifiers::kWriteOnly_Flag) {
        this->write("writeonly ");
    }
    if (modifiers.fFlags & Modifiers::kCoherent_Flag) {
        this->write("coherent ");
    }
    if (modifiers.fFlags & Modifiers::kVolatile_Flag) {
        this->write("volatile ");
    }
    if (modifiers.fFlags & Modifiers::kRestrict_Flag) {
        this->write("restrict ");
    }
    if ((modifiers.fFlags & Modifiers::kIn_Flag) &&
        (modifiers.fFlags & Modifiers::kOut_Flag)) {
        this->write("inout ");
    } else if (modifiers.fFlags & Modifiers::kIn_Flag) {
        if (globalContext &&
            fProgram.fSettings.fCaps->generation() < GrGLSLGeneration::k130_GrGLSLGeneration) {
            this->write(fProgramKind == Program::kVertex_Kind ? "attribute "
                                                              : "varying ");
        } else {
            this->write("in ");
        }
    } else if (modifiers.fFlags & Modifiers::kOut_Flag) {
        if (globalContext &&
            fProgram.fSettings.fCaps->generation() < GrGLSLGeneration::k130_GrGLSLGeneration) {
            this->write("varying ");
        } else {
            this->write("out ");
        }
    }
    if (modifiers.fFlags & Modifiers::kUniform_Flag) {
        this->write("uniform ");
    }
    if (modifiers.fFlags & Modifiers::kConst_Flag) {
        this->write("const ");
    }
    if (fProgram.fSettings.fCaps->usesPrecisionModifiers()) {
        if (modifiers.fFlags & Modifiers::kLowp_Flag) {
            this->write("lowp ");
        }
        if (modifiers.fFlags & Modifiers::kMediump_Flag) {
            this->write("mediump ");
        }
        if (modifiers.fFlags & Modifiers::kHighp_Flag) {
            this->write("highp ");
        }
    }
}

void GLSLCodeGenerator::writeInterfaceBlock(const InterfaceBlock& intf) {
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

void GLSLCodeGenerator::writeVarInitializer(const Variable& var, const Expression& value) {
    this->writeExpression(value, kTopLevel_Precedence);
}

void GLSLCodeGenerator::writeVarDeclarations(const VarDeclarations& decl, bool global) {
    ASSERT(decl.fVars.size() > 0);
    bool wroteType = false;
    for (const auto& stmt : decl.fVars) {
        VarDeclaration& var = (VarDeclaration&) *stmt;
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

void GLSLCodeGenerator::writeStatement(const Statement& s) {
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

void GLSLCodeGenerator::writeStatements(const std::vector<std::unique_ptr<Statement>>& statements) {
    for (const auto& s : statements) {
        if (!s->isEmpty()) {
            this->writeStatement(*s);
            this->writeLine();
        }
    }
}

void GLSLCodeGenerator::writeBlock(const Block& b) {
    this->writeLine("{");
    fIndentation++;
    this->writeStatements(b.fStatements);
    fIndentation--;
    this->write("}");
}

void GLSLCodeGenerator::writeIfStatement(const IfStatement& stmt) {
    this->write("if (");
    this->writeExpression(*stmt.fTest, kTopLevel_Precedence);
    this->write(") ");
    this->writeStatement(*stmt.fIfTrue);
    if (stmt.fIfFalse) {
        this->write(" else ");
        this->writeStatement(*stmt.fIfFalse);
    }
}

void GLSLCodeGenerator::writeForStatement(const ForStatement& f) {
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

void GLSLCodeGenerator::writeWhileStatement(const WhileStatement& w) {
    this->write("while (");
    this->writeExpression(*w.fTest, kTopLevel_Precedence);
    this->write(") ");
    this->writeStatement(*w.fStatement);
}

void GLSLCodeGenerator::writeDoStatement(const DoStatement& d) {
    this->write("do ");
    this->writeStatement(*d.fStatement);
    this->write(" while (");
    this->writeExpression(*d.fTest, kTopLevel_Precedence);
    this->write(");");
}

void GLSLCodeGenerator::writeSwitchStatement(const SwitchStatement& s) {
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

void GLSLCodeGenerator::writeReturnStatement(const ReturnStatement& r) {
    this->write("return");
    if (r.fExpression) {
        this->write(" ");
        this->writeExpression(*r.fExpression, kTopLevel_Precedence);
    }
    this->write(";");
}

void GLSLCodeGenerator::writeHeader() {
    this->write(fProgram.fSettings.fCaps->versionDeclString());
    this->writeLine();
    for (const auto& e : fProgram.fElements) {
        if (e->fKind == ProgramElement::kExtension_Kind) {
            this->writeExtension((Extension&) *e);
        }
    }
}

void GLSLCodeGenerator::writePrecisionModifier() {
    if (fProgram.fSettings.fCaps->usesPrecisionModifiers()) {
        this->write("precision ");
        switch (fProgram.fDefaultPrecision) {
            case Modifiers::kLowp_Flag:
                this->write("lowp");
                break;
            case Modifiers::kMediump_Flag:
                this->write("mediump");
                break;
            case Modifiers::kHighp_Flag:
                this->write("highp");
                break;
            default:
                ASSERT(false);
                this->write("<error>");
        }
        this->writeLine(" float;");
    }
}

void GLSLCodeGenerator::writeProgramElement(const ProgramElement& e) {
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
                } else if (builtin == SK_FRAGCOLOR_BUILTIN &&
                           fProgram.fSettings.fCaps->mustDeclareFragmentShaderOutput()) {
                    this->write("out ");
                    if (fProgram.fSettings.fCaps->usesPrecisionModifiers()) {
                        this->write("mediump ");
                    }
                    this->writeLine("vec4 sk_FragColor;");
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

bool GLSLCodeGenerator::generateCode() {
    OutputStream* rawOut = fOut;
    fOut = &fHeader;
    fProgramKind = fProgram.fKind;
    this->writeHeader();
    StringStream body;
    fOut = &body;
    this->writePrecisionModifier();
    for (const auto& e : fProgram.fElements) {
        this->writeProgramElement(*e);
    }
    fOut = rawOut;

    write_stringstream(fHeader, *rawOut);
    write_stringstream(body, *rawOut);
    return true;
}

}
