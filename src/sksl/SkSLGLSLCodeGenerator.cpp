/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLGLSLCodeGenerator.h"

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

void GLSLCodeGenerator::write(StringFragment s) {
    if (!s.fLength) {
        return;
    }
    if (fAtLineStart) {
        for (int i = 0; i < fIndentation; i++) {
            fOut->writeText("    ");
        }
    }
    fOut->write(s.fChars, s.fLength);
    fAtLineStart = false;
}

void GLSLCodeGenerator::writeLine(const String& s) {
    this->writeLine(s.c_str());
}

void GLSLCodeGenerator::writeLine() {
    this->writeLine("");
}

void GLSLCodeGenerator::writeExtension(const Extension& ext) {
    this->write("#extension ");
    this->write(ext.fName);
    this->writeLine(" : enable");
}

bool GLSLCodeGenerator::usesPrecisionModifiers() const {
    return fProgram.fSettings.fCaps->usesPrecisionModifiers();
}

String GLSLCodeGenerator::getTypeName(const Type& type) {
    switch (type.kind()) {
        case Type::kVector_Kind: {
            Type component = type.componentType();
            String result;
            if (component == *fContext.fFloat_Type || component == *fContext.fHalf_Type) {
                result = "vec";
            }
            else if (component == *fContext.fDouble_Type) {
                result = "dvec";
            }
            else if (component == *fContext.fInt_Type ||
                     component == *fContext.fShort_Type ||
                     component == *fContext.fByte_Type) {
                result = "ivec";
            }
            else if (component == *fContext.fUInt_Type ||
                     component == *fContext.fUShort_Type ||
                     component == *fContext.fUByte_Type) {
                result = "uvec";
            }
            else if (component == *fContext.fBool_Type) {
                result = "bvec";
            }
            else {
                ABORT("unsupported vector type");
            }
            result += to_string(type.columns());
            return result;
        }
        case Type::kMatrix_Kind: {
            String result;
            Type component = type.componentType();
            if (component == *fContext.fFloat_Type || component == *fContext.fHalf_Type) {
                result = "mat";
            }
            else if (component == *fContext.fDouble_Type) {
                result = "dmat";
            }
            else {
                ABORT("unsupported matrix type");
            }
            result += to_string(type.columns());
            if (type.columns() != type.rows()) {
                result += "x";
                result += to_string(type.rows());
            }
            return result;
        }
        case Type::kArray_Kind: {
            String result = this->getTypeName(type.componentType()) + "[";
            if (type.columns() != -1) {
                result += to_string(type.columns());
            }
            result += "]";
            return result;
        }
        case Type::kScalar_Kind: {
            if (type == *fContext.fHalf_Type) {
                return "float";
            }
            else if (type == *fContext.fShort_Type) {
                return "int";
            }
            else if (type == *fContext.fUShort_Type) {
                return "uint";
            }
            else if (type == *fContext.fByte_Type) {
                return "int";
            }
            else if (type == *fContext.fUByte_Type) {
                return "uint";
            }
            else {
                return type.name();
            }
            break;
        }
        default:
            return type.name();
    }
}

void GLSLCodeGenerator::writeType(const Type& type) {
    if (type.kind() == Type::kStruct_Kind) {
        for (const Type* search : fWrittenStructs) {
            if (*search == type) {
                // already written
                this->write(type.fName);
                return;
            }
        }
        fWrittenStructs.push_back(&type);
        this->write("struct ");
        this->write(type.fName);
        this->writeLine(" {");
        fIndentation++;
        for (const auto& f : type.fields()) {
            this->writeModifiers(f.fModifiers, false);
            this->writeTypePrecision(*f.fType);
            // sizes (which must be static in structs) are part of the type name here
            this->writeType(*f.fType);
            this->write(" ");
            this->write(f.fName);
            this->writeLine(";");
        }
        fIndentation--;
        this->write("}");
    } else {
        this->write(this->getTypeName(type));
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
            this->writeConstructor((Constructor&) expr, parentPrecedence);
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
    SkASSERT(!fProgram.fSettings.fCaps->canUseMinAndAbsTogether());
    String tmpVar1 = "minAbsHackVar" + to_string(fVarCount++);
    String tmpVar2 = "minAbsHackVar" + to_string(fVarCount++);
    this->fFunctionHeader += String("    ") + this->getTypePrecision(absExpr.fType) +
                             this->getTypeName(absExpr.fType) + " " + tmpVar1 + ";\n";
    this->fFunctionHeader += String("    ") + this->getTypePrecision(otherExpr.fType) +
                             this->getTypeName(otherExpr.fType) + " " + tmpVar2 + ";\n";
    this->write("((" + tmpVar1 + " = ");
    this->writeExpression(absExpr, kTopLevel_Precedence);
    this->write(") < (" + tmpVar2 + " = ");
    this->writeExpression(otherExpr, kAssignment_Precedence);
    this->write(") ? " + tmpVar1 + " : " + tmpVar2 + ")");
}

void GLSLCodeGenerator::writeInverseSqrtHack(const Expression& x) {
    this->write("(1.0 / sqrt(");
    this->writeExpression(x, kTopLevel_Precedence);
    this->write("))");
}

void GLSLCodeGenerator::writeDeterminantHack(const Expression& mat) {
    String name;
    if (mat.fType == *fContext.fFloat2x2_Type || mat.fType == *fContext.fHalf2x2_Type) {
        name = "_determinant2";
        if (fWrittenIntrinsics.find(name) == fWrittenIntrinsics.end()) {
            fWrittenIntrinsics.insert(name);
            fExtraFunctions.writeText((
                "float " + name + "(mat2 m) {"
                "    return m[0][0] * m[1][1] - m[0][1] * m[1][0];"
                "}"
            ).c_str());
        }
    }
    else if (mat.fType == *fContext.fFloat3x3_Type || mat.fType == *fContext.fHalf3x3_Type) {
        name = "_determinant3";
        if (fWrittenIntrinsics.find(name) == fWrittenIntrinsics.end()) {
            fWrittenIntrinsics.insert(name);
            fExtraFunctions.writeText((
                "float " + name + "(mat3 m) {"
                "    float a00 = m[0][0], a01 = m[0][1], a02 = m[0][2];"
                "    float a10 = m[1][0], a11 = m[1][1], a12 = m[1][2];"
                "    float a20 = m[2][0], a21 = m[2][1], a22 = m[2][2];"
                "    float b01 = a22 * a11 - a12 * a21;"
                "    float b11 = -a22 * a10 + a12 * a20;"
                "    float b21 = a21 * a10 - a11 * a20;"
                "    return a00 * b01 + a01 * b11 + a02 * b21;"
                "}"
            ).c_str());
        }
    }
    else if (mat.fType == *fContext.fFloat4x4_Type || mat.fType == *fContext.fHalf4x4_Type) {
        name = "_determinant3";
        if (fWrittenIntrinsics.find(name) == fWrittenIntrinsics.end()) {
            fWrittenIntrinsics.insert(name);
            fExtraFunctions.writeText((
                "mat4 " + name + "(mat4 m) {"
                "    float a00 = m[0][0], a01 = m[0][1], a02 = m[0][2], a03 = m[0][3];"
                "    float a10 = m[1][0], a11 = m[1][1], a12 = m[1][2], a13 = m[1][3];"
                "    float a20 = m[2][0], a21 = m[2][1], a22 = m[2][2], a23 = m[2][3];"
                "    float a30 = m[3][0], a31 = m[3][1], a32 = m[3][2], a33 = m[3][3];"
                "    float b00 = a00 * a11 - a01 * a10;"
                "    float b01 = a00 * a12 - a02 * a10;"
                "    float b02 = a00 * a13 - a03 * a10;"
                "    float b03 = a01 * a12 - a02 * a11;"
                "    float b04 = a01 * a13 - a03 * a11;"
                "    float b05 = a02 * a13 - a03 * a12;"
                "    float b06 = a20 * a31 - a21 * a30;"
                "    float b07 = a20 * a32 - a22 * a30;"
                "    float b08 = a20 * a33 - a23 * a30;"
                "    float b09 = a21 * a32 - a22 * a31;"
                "    float b10 = a21 * a33 - a23 * a31;"
                "    float b11 = a22 * a33 - a23 * a32;"
                "    return b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;"
                "}"
            ).c_str());
        }
    }
    else {
        SkASSERT(false);
    }
    this->write(name + "(");
    this->writeExpression(mat, kTopLevel_Precedence);
    this->write(")");
}

void GLSLCodeGenerator::writeInverseHack(const Expression& mat) {
    String name;
    if (mat.fType == *fContext.fFloat2x2_Type || mat.fType == *fContext.fHalf2x2_Type) {
        name = "_inverse2";
        if (fWrittenIntrinsics.find(name) == fWrittenIntrinsics.end()) {
            fWrittenIntrinsics.insert(name);
            fExtraFunctions.writeText((
                "mat2 " + name + "(mat2 m) {"
                "    return mat2(m[1][1], -m[0][1], -m[1][0], m[0][0]) / "
                               "(m[0][0] * m[1][1] - m[0][1] * m[1][0]);"
                "}"
            ).c_str());
        }
    }
    else if (mat.fType == *fContext.fFloat3x3_Type || mat.fType == *fContext.fHalf3x3_Type) {
        name = "_inverse3";
        if (fWrittenIntrinsics.find(name) == fWrittenIntrinsics.end()) {
            fWrittenIntrinsics.insert(name);
            fExtraFunctions.writeText((
                "mat3 " +  name + "(mat3 m) {"
                "    float a00 = m[0][0], a01 = m[0][1], a02 = m[0][2];"
                "    float a10 = m[1][0], a11 = m[1][1], a12 = m[1][2];"
                "    float a20 = m[2][0], a21 = m[2][1], a22 = m[2][2];"
                "    float b01 = a22 * a11 - a12 * a21;"
                "    float b11 = -a22 * a10 + a12 * a20;"
                "    float b21 = a21 * a10 - a11 * a20;"
                "    float det = a00 * b01 + a01 * b11 + a02 * b21;"
                "    return mat3(b01, (-a22 * a01 + a02 * a21), (a12 * a01 - a02 * a11),"
                "                b11, (a22 * a00 - a02 * a20), (-a12 * a00 + a02 * a10),"
                "                b21, (-a21 * a00 + a01 * a20), (a11 * a00 - a01 * a10)) / det;"
                "}"
            ).c_str());
        }
    }
    else if (mat.fType == *fContext.fFloat4x4_Type || mat.fType == *fContext.fHalf4x4_Type) {
        name = "_inverse4";
        if (fWrittenIntrinsics.find(name) == fWrittenIntrinsics.end()) {
            fWrittenIntrinsics.insert(name);
            fExtraFunctions.writeText((
                "mat4 " + name + "(mat4 m) {"
                "    float a00 = m[0][0], a01 = m[0][1], a02 = m[0][2], a03 = m[0][3];"
                "    float a10 = m[1][0], a11 = m[1][1], a12 = m[1][2], a13 = m[1][3];"
                "    float a20 = m[2][0], a21 = m[2][1], a22 = m[2][2], a23 = m[2][3];"
                "    float a30 = m[3][0], a31 = m[3][1], a32 = m[3][2], a33 = m[3][3];"
                "    float b00 = a00 * a11 - a01 * a10;"
                "    float b01 = a00 * a12 - a02 * a10;"
                "    float b02 = a00 * a13 - a03 * a10;"
                "    float b03 = a01 * a12 - a02 * a11;"
                "    float b04 = a01 * a13 - a03 * a11;"
                "    float b05 = a02 * a13 - a03 * a12;"
                "    float b06 = a20 * a31 - a21 * a30;"
                "    float b07 = a20 * a32 - a22 * a30;"
                "    float b08 = a20 * a33 - a23 * a30;"
                "    float b09 = a21 * a32 - a22 * a31;"
                "    float b10 = a21 * a33 - a23 * a31;"
                "    float b11 = a22 * a33 - a23 * a32;"
                "    float det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - "
                "                b04 * b07 + b05 * b06;"
                "    return mat4("
                "        a11 * b11 - a12 * b10 + a13 * b09,"
                "        a02 * b10 - a01 * b11 - a03 * b09,"
                "        a31 * b05 - a32 * b04 + a33 * b03,"
                "        a22 * b04 - a21 * b05 - a23 * b03,"
                "        a12 * b08 - a10 * b11 - a13 * b07,"
                "        a00 * b11 - a02 * b08 + a03 * b07,"
                "        a32 * b02 - a30 * b05 - a33 * b01,"
                "        a20 * b05 - a22 * b02 + a23 * b01,"
                "        a10 * b10 - a11 * b08 + a13 * b06,"
                "        a01 * b08 - a00 * b10 - a03 * b06,"
                "        a30 * b04 - a31 * b02 + a33 * b00,"
                "        a21 * b02 - a20 * b04 - a23 * b00,"
                "        a11 * b07 - a10 * b09 - a12 * b06,"
                "        a00 * b09 - a01 * b07 + a02 * b06,"
                "        a31 * b01 - a30 * b03 - a32 * b00,"
                "        a20 * b03 - a21 * b01 + a22 * b00) / det;"
                "}"
            ).c_str());
        }
    }
    else {
        SkASSERT(false);
    }
    this->write(name + "(");
    this->writeExpression(mat, kTopLevel_Precedence);
    this->write(")");
}

void GLSLCodeGenerator::writeTransposeHack(const Expression& mat) {
    String name = "transpose" + to_string(mat.fType.columns()) + to_string(mat.fType.rows());
    if (fWrittenIntrinsics.find(name) == fWrittenIntrinsics.end()) {
        fWrittenIntrinsics.insert(name);
        String type = this->getTypeName(mat.fType);
        const Type& base = mat.fType.componentType();
        String transposed =  this->getTypeName(base.toCompound(fContext,
                                                               mat.fType.rows(),
                                                               mat.fType.columns()));
        fExtraFunctions.writeText((transposed + " " + name + "(" + type + " m) {\nreturn " +
                                  transposed + "(").c_str());
        const char* separator = "";
        for (int row = 0; row < mat.fType.rows(); ++row) {
            for (int column = 0; column < mat.fType.columns(); ++column) {
                fExtraFunctions.writeText(separator);
                fExtraFunctions.writeText(("m[" + to_string(column) + "][" + to_string(row) +
                                           "]").c_str());
                separator = ", ";
            }
        }
        fExtraFunctions.writeText("); }");
    }
    this->write(name + "(");
    this->writeExpression(mat, kTopLevel_Precedence);
    this->write(")");
}

void GLSLCodeGenerator::writeFunctionCall(const FunctionCall& c) {
    if (!fProgram.fSettings.fCaps->canUseMinAndAbsTogether() && c.fFunction.fName == "min" &&
        c.fFunction.fBuiltin) {
        SkASSERT(c.fArguments.size() == 2);
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
    if (!fProgram.fSettings.fCaps->canUseFractForNegativeValues() && c.fFunction.fName == "fract" &&
        c.fFunction.fBuiltin) {
        SkASSERT(c.fArguments.size() == 1);

        this->write("(0.5 - sign(");
        this->writeExpression(*c.fArguments[0], kSequence_Precedence);
        this->write(") * (0.5 - fract(abs(");
        this->writeExpression(*c.fArguments[0], kSequence_Precedence);
        this->write("))))");

        return;
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
    if (c.fFunction.fBuiltin && c.fFunction.fName == "determinant" &&
        fProgram.fSettings.fCaps->generation() < k150_GrGLSLGeneration) {
        SkASSERT(c.fArguments.size() == 1);
        this->writeDeterminantHack(*c.fArguments[0]);
        return;
    }
    if (c.fFunction.fBuiltin && c.fFunction.fName == "inverse" &&
        fProgram.fSettings.fCaps->generation() < k140_GrGLSLGeneration) {
        SkASSERT(c.fArguments.size() == 1);
        this->writeInverseHack(*c.fArguments[0]);
        return;
    }
    if (c.fFunction.fBuiltin && c.fFunction.fName == "inverseSqrt" &&
        fProgram.fSettings.fCaps->generation() < k130_GrGLSLGeneration) {
        SkASSERT(c.fArguments.size() == 1);
        this->writeInverseSqrtHack(*c.fArguments[0]);
        return;
    }
    if (c.fFunction.fBuiltin && c.fFunction.fName == "transpose" &&
        fProgram.fSettings.fCaps->generation() < k130_GrGLSLGeneration) {
        SkASSERT(c.fArguments.size() == 1);
        this->writeTransposeHack(*c.fArguments[0]);
        return;
    }
    if (!fFoundDerivatives && (c.fFunction.fName == "dFdx" || c.fFunction.fName == "dFdy") &&
        c.fFunction.fBuiltin && fProgram.fSettings.fCaps->shaderDerivativeExtensionString()) {
        SkASSERT(fProgram.fSettings.fCaps->shaderDerivativeSupport());
        fHeader.writeText("#extension ");
        fHeader.writeText(fProgram.fSettings.fCaps->shaderDerivativeExtensionString());
        fHeader.writeText(" : require\n");
        fFoundDerivatives = true;
    }
    bool isTextureFunctionWithBias = false;
    if (c.fFunction.fName == "texture" && c.fFunction.fBuiltin) {
        const char* dim = "";
        bool proj = false;
        switch (c.fArguments[0]->fType.dimensions()) {
            case SpvDim1D:
                dim = "1D";
                isTextureFunctionWithBias = true;
                if (c.fArguments[1]->fType == *fContext.fFloat_Type) {
                    proj = false;
                } else {
                    SkASSERT(c.fArguments[1]->fType == *fContext.fFloat2_Type);
                    proj = true;
                }
                break;
            case SpvDim2D:
                dim = "2D";
                if (c.fArguments[0]->fType != *fContext.fSamplerExternalOES_Type) {
                    isTextureFunctionWithBias = true;
                }
                if (c.fArguments[1]->fType == *fContext.fFloat2_Type) {
                    proj = false;
                } else {
                    SkASSERT(c.fArguments[1]->fType == *fContext.fFloat3_Type);
                    proj = true;
                }
                break;
            case SpvDim3D:
                dim = "3D";
                isTextureFunctionWithBias = true;
                if (c.fArguments[1]->fType == *fContext.fFloat3_Type) {
                    proj = false;
                } else {
                    SkASSERT(c.fArguments[1]->fType == *fContext.fFloat4_Type);
                    proj = true;
                }
                break;
            case SpvDimCube:
                dim = "Cube";
                isTextureFunctionWithBias = true;
                proj = false;
                break;
            case SpvDimRect:
                dim = "Rect";
                proj = false;
                break;
            case SpvDimBuffer:
                SkASSERT(false); // doesn't exist
                dim = "Buffer";
                proj = false;
                break;
            case SpvDimSubpassData:
                SkASSERT(false); // doesn't exist
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
    if (fProgram.fSettings.fSharpenTextures && isTextureFunctionWithBias) {
        this->write(", -0.5");
    }
    this->write(")");
}

void GLSLCodeGenerator::writeConstructor(const Constructor& c, Precedence parentPrecedence) {
    if (c.fArguments.size() == 1 &&
        this->getTypeName(c.fType) == this->getTypeName(c.fArguments[0]->fType)) {
        // in cases like half(float), they're different types as far as SkSL is concerned but the
        // same type as far as GLSL is concerned. We avoid a redundant float(float) by just writing
        // out the inner expression here.
        this->writeExpression(*c.fArguments[0], parentPrecedence);
        return;
    }
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

void GLSLCodeGenerator::writeFragCoord() {
    if (!fProgram.fSettings.fCaps->canUseFragCoord()) {
        if (!fSetupFragCoordWorkaround) {
            const char* precision = usesPrecisionModifiers() ? "highp " : "";
            fFunctionHeader += precision;
            fFunctionHeader += "    float sk_FragCoord_InvW = 1. / sk_FragCoord_Workaround.w;\n";
            fFunctionHeader += precision;
            fFunctionHeader += "    vec4 sk_FragCoord_Resolved = "
                "vec4(sk_FragCoord_Workaround.xyz * sk_FragCoord_InvW, sk_FragCoord_InvW);\n";
            // Ensure that we get exact .5 values for x and y.
            fFunctionHeader += "    sk_FragCoord_Resolved.xy = floor(sk_FragCoord_Resolved.xy) + "
                               "vec2(.5);\n";
            fSetupFragCoordWorkaround = true;
        }
        this->write("sk_FragCoord_Resolved");
        return;
    }

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
            // do the same thing. Copying gl_FragCoord.xy into a temp float2 beforehand
            // (and only accessing .xy) seems to "fix" things.
            const char* precision = usesPrecisionModifiers() ? "highp " : "";
            fHeader.writeText("uniform ");
            fHeader.writeText(precision);
            fHeader.writeText("float " SKSL_RTHEIGHT_NAME ";\n");
            fSetupFragPositionGlobal = true;
        }
        if (!fSetupFragPositionLocal) {
            const char* precision = usesPrecisionModifiers() ? "highp " : "";
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
        case SK_INSTANCEID_BUILTIN:
            this->write("gl_InstanceID");
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
        case SK_LASTFRAGCOLOR_BUILTIN:
            this->write(fProgram.fSettings.fCaps->fbFetchColorName());
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

bool is_sk_position(const FieldAccess& f) {
    return "sk_Position" == f.fBase->fType.fields()[f.fFieldIndex].fName;
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
            StringFragment name = f.fBase->fType.fields()[f.fFieldIndex].fName;
            if (name == "sk_Position") {
                this->write("gl_Position");
            } else if (name == "sk_PointSize") {
                this->write("gl_PointSize");
            } else {
                this->write(f.fBase->fType.fields()[f.fFieldIndex].fName);
            }
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
    bool positionWorkaround = Compiler::IsAssignment(b.fOperator) &&
                              Expression::kFieldAccess_Kind == b.fLeft->fKind &&
                              is_sk_position((FieldAccess&) *b.fLeft) &&
                              !strstr(b.fRight->description().c_str(), "sk_RTAdjust") &&
                              !fProgram.fSettings.fCaps->canUseFragCoord();
    if (positionWorkaround) {
        this->write("sk_FragCoord_Workaround = (");
    }
    this->writeExpression(*b.fLeft, precedence);
    this->write(" ");
    this->write(Compiler::OperatorName(b.fOperator));
    this->write(" ");
    this->writeExpression(*b.fRight, precedence);
    if (positionWorkaround) {
        this->write(")");
    }
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
    this->write(Compiler::OperatorName(p.fOperator));
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
    this->write(Compiler::OperatorName(p.fOperator));
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
    } else if (i.fType == *fContext.fUShort_Type) {
        this->write(to_string(i.fValue & 0xffff) + "u");
    } else if (i.fType == *fContext.fUByte_Type) {
        this->write(to_string(i.fValue & 0xff) + "u");
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
    this->writeTypePrecision(f.fDeclaration.fReturnType);
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
        this->writeTypePrecision(*type);
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
    if (usesPrecisionModifiers()) {
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
        this->writeTypePrecision(*f.fType);
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

const char* GLSLCodeGenerator::getTypePrecision(const Type& type) {
    if (usesPrecisionModifiers()) {
        switch (type.kind()) {
            case Type::kScalar_Kind:
                if (type == *fContext.fShort_Type || type == *fContext.fUShort_Type ||
                    type == *fContext.fByte_Type || type == *fContext.fUByte_Type) {
                    if (fProgram.fSettings.fForceHighPrecision ||
                            fProgram.fSettings.fCaps->incompleteShortIntPrecision()) {
                        return "highp ";
                    }
                    return "mediump ";
                }
                if (type == *fContext.fHalf_Type) {
                    return fProgram.fSettings.fForceHighPrecision ? "highp " : "mediump ";
                }
                if (type == *fContext.fFloat_Type || type == *fContext.fInt_Type ||
                        type == *fContext.fUInt_Type) {
                    return "highp ";
                }
                return "";
            case Type::kVector_Kind: // fall through
            case Type::kMatrix_Kind:
                return this->getTypePrecision(type.componentType());
            default:
                break;
        }
    }
    return "";
}

void GLSLCodeGenerator::writeTypePrecision(const Type& type) {
    this->write(this->getTypePrecision(type));
}

void GLSLCodeGenerator::writeVarDeclarations(const VarDeclarations& decl, bool global) {
    if (!decl.fVars.size()) {
        return;
    }
    bool wroteType = false;
    for (const auto& stmt : decl.fVars) {
        VarDeclaration& var = (VarDeclaration&) *stmt;
        if (wroteType) {
            this->write(", ");
        } else {
            this->writeModifiers(var.fVar->fModifiers, global);
            this->writeTypePrecision(decl.fBaseType);
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
        if (!fFoundExternalSamplerDecl && var.fVar->fType == *fContext.fSamplerExternalOES_Type) {
            if (fProgram.fSettings.fCaps->externalTextureExtensionString()) {
                fHeader.writeText("#extension ");
                fHeader.writeText(fProgram.fSettings.fCaps->externalTextureExtensionString());
                fHeader.writeText(" : enable\n");
            }
            if (fProgram.fSettings.fCaps->secondExternalTextureExtensionString()) {
                fHeader.writeText("#extension ");
                fHeader.writeText(fProgram.fSettings.fCaps->secondExternalTextureExtensionString());
                fHeader.writeText(" : enable\n");
            }
            fFoundExternalSamplerDecl = true;
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
    for (const auto& e : fProgram) {
        if (e.fKind == ProgramElement::kExtension_Kind) {
            this->writeExtension((Extension&) e);
        }
    }
    if (!fProgram.fSettings.fCaps->canUseFragCoord()) {
        Layout layout;
        switch (fProgram.fKind) {
            case Program::kVertex_Kind: {
                Modifiers modifiers(layout, Modifiers::kOut_Flag | Modifiers::kHighp_Flag);
                this->writeModifiers(modifiers, true);
                this->write("vec4 sk_FragCoord_Workaround;\n");
                break;
            }
            case Program::kFragment_Kind: {
                Modifiers modifiers(layout, Modifiers::kIn_Flag | Modifiers::kHighp_Flag);
                this->writeModifiers(modifiers, true);
                this->write("vec4 sk_FragCoord_Workaround;\n");
                break;
            }
            default:
                break;
        }
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
                    if (fProgram.fSettings.fFragColorIsInOut) {
                        this->write("inout ");
                    } else {
                        this->write("out ");
                    }
                    if (usesPrecisionModifiers()) {
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
        case ProgramElement::kModifiers_Kind: {
            const Modifiers& modifiers = ((ModifiersDeclaration&) e).fModifiers;
            if (!fFoundGSInvocations && modifiers.fLayout.fInvocations >= 0) {
                if (fProgram.fSettings.fCaps->gsInvocationsExtensionString()) {
                    fHeader.writeText("#extension ");
                    fHeader.writeText(fProgram.fSettings.fCaps->gsInvocationsExtensionString());
                    fHeader.writeText(" : require\n");
                }
                fFoundGSInvocations = true;
            }
            this->writeModifiers(modifiers, true);
            this->writeLine(";");
            break;
        }
        case ProgramElement::kEnum_Kind:
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
    if (Program::kGeometry_Kind == fProgramKind &&
        fProgram.fSettings.fCaps->geometryShaderExtensionString()) {
        fHeader.writeText("#extension ");
        fHeader.writeText(fProgram.fSettings.fCaps->geometryShaderExtensionString());
        fHeader.writeText(" : require\n");
    }
    StringStream body;
    fOut = &body;
    for (const auto& e : fProgram) {
        this->writeProgramElement(e);
    }
    fOut = rawOut;

    write_stringstream(fHeader, *rawOut);
    if (this->usesPrecisionModifiers()) {
        this->writeLine("precision mediump float;");
    }
    write_stringstream(fExtraFunctions, *rawOut);
    write_stringstream(body, *rawOut);
    return true;
}

}
