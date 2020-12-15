/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLGLSLCodeGenerator.h"

#include <memory>

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLExtension.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLModifiersDeclaration.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLStructDefinition.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#ifndef SKSL_STANDALONE
#include "include/private/SkOnce.h"
#endif

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

void GLSLCodeGenerator::writeExtension(const String& name) {
    this->writeExtension(name, true);
}

void GLSLCodeGenerator::writeExtension(const String& name, bool require) {
    fExtensions.writeText("#extension ");
    fExtensions.write(name.c_str(), name.length());
    fExtensions.writeText(require ? " : require\n" : " : enable\n");
}

bool GLSLCodeGenerator::usesPrecisionModifiers() const {
    return fProgram.fCaps->usesPrecisionModifiers();
}

// Returns the name of the type with array dimensions, e.g. `float[2][4]`.
String GLSLCodeGenerator::getTypeName(const Type& type) {
    switch (type.typeKind()) {
        case Type::TypeKind::kVector: {
            const Type& component = type.componentType();
            String result;
            if (component == *fContext.fFloat_Type || component == *fContext.fHalf_Type) {
                result = "vec";
            }
            else if (component.isSigned()) {
                result = "ivec";
            }
            else if (component.isUnsigned()) {
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
        case Type::TypeKind::kMatrix: {
            String result;
            const Type& component = type.componentType();
            if (component == *fContext.fFloat_Type || component == *fContext.fHalf_Type) {
                result = "mat";
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
        case Type::TypeKind::kArray: {
            String baseTypeName = this->getTypeName(type.componentType());
            return (type.columns() == Type::kUnsizedArray)
                           ? String::printf("%s[]", baseTypeName.c_str())
                           : String::printf("%s[%d]", baseTypeName.c_str(), type.columns());
        }
        case Type::TypeKind::kScalar: {
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
        case Type::TypeKind::kEnum:
            return "int";
        default:
            return type.name();
    }
}

bool GLSLCodeGenerator::writeStructDefinition(const Type& type) {
    for (const Type* search : fWrittenStructs) {
        if (*search == type) {
            // already written
            return false;
        }
    }
    fWrittenStructs.push_back(&type);
    this->write("struct ");
    this->write(type.name());
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
    return true;
}

void GLSLCodeGenerator::writeType(const Type& type) {
    if (type.isStruct()) {
        if (!this->writeStructDefinition(type)) {
            this->write(type.name());
        }
    } else {
        this->write(this->getTypeName(type));
    }
}

void GLSLCodeGenerator::writeExpression(const Expression& expr, Precedence parentPrecedence) {
    switch (expr.kind()) {
        case Expression::Kind::kBinary:
            this->writeBinaryExpression(expr.as<BinaryExpression>(), parentPrecedence);
            break;
        case Expression::Kind::kBoolLiteral:
            this->writeBoolLiteral(expr.as<BoolLiteral>());
            break;
        case Expression::Kind::kConstructor:
            this->writeConstructor(expr.as<Constructor>(), parentPrecedence);
            break;
        case Expression::Kind::kIntLiteral:
            this->writeIntLiteral(expr.as<IntLiteral>());
            break;
        case Expression::Kind::kFieldAccess:
            this->writeFieldAccess(expr.as<FieldAccess>());
            break;
        case Expression::Kind::kFloatLiteral:
            this->writeFloatLiteral(expr.as<FloatLiteral>());
            break;
        case Expression::Kind::kFunctionCall:
            this->writeFunctionCall(expr.as<FunctionCall>());
            break;
        case Expression::Kind::kPrefix:
            this->writePrefixExpression(expr.as<PrefixExpression>(), parentPrecedence);
            break;
        case Expression::Kind::kPostfix:
            this->writePostfixExpression(expr.as<PostfixExpression>(), parentPrecedence);
            break;
        case Expression::Kind::kSetting:
            this->writeSetting(expr.as<Setting>());
            break;
        case Expression::Kind::kSwizzle:
            this->writeSwizzle(expr.as<Swizzle>());
            break;
        case Expression::Kind::kVariableReference:
            this->writeVariableReference(expr.as<VariableReference>());
            break;
        case Expression::Kind::kTernary:
            this->writeTernaryExpression(expr.as<TernaryExpression>(), parentPrecedence);
            break;
        case Expression::Kind::kIndex:
            this->writeIndexExpression(expr.as<IndexExpression>());
            break;
        default:
#ifdef SK_DEBUG
            ABORT("unsupported expression: %s", expr.description().c_str());
#endif
            break;
    }
}

static bool is_abs(Expression& expr) {
    if (expr.kind() != Expression::Kind::kFunctionCall) {
        return false;
    }
    return expr.as<FunctionCall>().function().name() == "abs";
}

// turns min(abs(x), y) into ((tmpVar1 = abs(x)) < (tmpVar2 = y) ? tmpVar1 : tmpVar2) to avoid a
// Tegra3 compiler bug.
void GLSLCodeGenerator::writeMinAbsHack(Expression& absExpr, Expression& otherExpr) {
    SkASSERT(!fProgram.fCaps->canUseMinAndAbsTogether());
    String tmpVar1 = "minAbsHackVar" + to_string(fVarCount++);
    String tmpVar2 = "minAbsHackVar" + to_string(fVarCount++);
    this->fFunctionHeader += String("    ") + this->getTypePrecision(absExpr.type()) +
                             this->getTypeName(absExpr.type()) + " " + tmpVar1 + ";\n";
    this->fFunctionHeader += String("    ") + this->getTypePrecision(otherExpr.type()) +
                             this->getTypeName(otherExpr.type()) + " " + tmpVar2 + ";\n";
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
    const Type& type = mat.type();
    if (type == *fContext.fFloat2x2_Type || type == *fContext.fHalf2x2_Type) {
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
    else if (type == *fContext.fFloat3x3_Type || type == *fContext.fHalf3x3_Type) {
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
    else if (type == *fContext.fFloat4x4_Type || type == *fContext.fHalf4x4_Type) {
        name = "_determinant4";
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
    const Type& type = mat.type();
    if (type == *fContext.fFloat2x2_Type || type == *fContext.fHalf2x2_Type) {
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
    else if (type == *fContext.fFloat3x3_Type || type == *fContext.fHalf3x3_Type) {
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
    else if (type == *fContext.fFloat4x4_Type || type == *fContext.fHalf4x4_Type) {
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
    const Type& type = mat.type();
    String name = "transpose" + to_string(type.columns()) + to_string(type.rows());
    if (fWrittenIntrinsics.find(name) == fWrittenIntrinsics.end()) {
        fWrittenIntrinsics.insert(name);
        String typeName = this->getTypeName(type);
        const Type& base = type.componentType();
        String transposed =  this->getTypeName(base.toCompound(fContext,
                                                               type.rows(),
                                                               type.columns()));
        fExtraFunctions.writeText((transposed + " " + name + "(" + typeName + " m) {\nreturn " +
                                  transposed + "(").c_str());
        const char* separator = "";
        for (int row = 0; row < type.rows(); ++row) {
            for (int column = 0; column < type.columns(); ++column) {
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

std::unordered_map<StringFragment, GLSLCodeGenerator::FunctionClass>*
                                                      GLSLCodeGenerator::fFunctionClasses = nullptr;

void GLSLCodeGenerator::writeFunctionCall(const FunctionCall& c) {
    const FunctionDeclaration& function = c.function();
    const ExpressionArray& arguments = c.arguments();
#ifdef SKSL_STANDALONE
    if (!fFunctionClasses) {
#else
    static SkOnce once;
    once([] {
#endif
        fFunctionClasses = new std::unordered_map<StringFragment, FunctionClass>();
        (*fFunctionClasses)["abs"]         = FunctionClass::kAbs;
        (*fFunctionClasses)["atan"]        = FunctionClass::kAtan;
        (*fFunctionClasses)["determinant"] = FunctionClass::kDeterminant;
        (*fFunctionClasses)["dFdx"]        = FunctionClass::kDFdx;
        (*fFunctionClasses)["dFdy"]        = FunctionClass::kDFdy;
        (*fFunctionClasses)["fwidth"]      = FunctionClass::kFwidth;
        (*fFunctionClasses)["fma"]         = FunctionClass::kFMA;
        (*fFunctionClasses)["fract"]       = FunctionClass::kFract;
        (*fFunctionClasses)["inverse"]     = FunctionClass::kInverse;
        (*fFunctionClasses)["inverseSqrt"] = FunctionClass::kInverseSqrt;
        (*fFunctionClasses)["min"]         = FunctionClass::kMin;
        (*fFunctionClasses)["pow"]         = FunctionClass::kPow;
        (*fFunctionClasses)["saturate"]    = FunctionClass::kSaturate;
        (*fFunctionClasses)["sample"]      = FunctionClass::kTexture;
        (*fFunctionClasses)["transpose"]   = FunctionClass::kTranspose;
    }
#ifndef SKSL_STANDALONE
    );
#endif
    const auto found = function.isBuiltin() ? fFunctionClasses->find(function.name()) :
                                              fFunctionClasses->end();
    bool isTextureFunctionWithBias = false;
    bool nameWritten = false;
    if (found != fFunctionClasses->end()) {
        switch (found->second) {
            case FunctionClass::kAbs: {
                if (!fProgram.fCaps->emulateAbsIntFunction())
                    break;
                SkASSERT(arguments.size() == 1);
                if (arguments[0]->type() != *fContext.fInt_Type) {
                    break;
                }
                // abs(int) on Intel OSX is incorrect, so emulate it:
                String name = "_absemulation";
                this->write(name);
                nameWritten = true;
                if (fWrittenIntrinsics.find(name) == fWrittenIntrinsics.end()) {
                    fWrittenIntrinsics.insert(name);
                    fExtraFunctions.writeText((
                        "int " + name + "(int x) {\n"
                        "    return x * sign(x);\n"
                        "}\n"
                    ).c_str());
                }
                break;
            }
            case FunctionClass::kAtan:
                if (fProgram.fCaps->mustForceNegatedAtanParamToFloat() &&
                    arguments.size() == 2 &&
                    arguments[1]->kind() == Expression::Kind::kPrefix) {
                    const PrefixExpression& p = (PrefixExpression&) *arguments[1];
                    if (p.getOperator() == Token::Kind::TK_MINUS) {
                        this->write("atan(");
                        this->writeExpression(*arguments[0], kSequence_Precedence);
                        this->write(", -1.0 * ");
                        this->writeExpression(*p.operand(), kMultiplicative_Precedence);
                        this->write(")");
                        return;
                    }
                }
                break;
            case FunctionClass::kDFdy:
                if (fProgram.fSettings.fFlipY) {
                    // Flipping Y also negates the Y derivatives.
                    this->write("-dFdy");
                    nameWritten = true;
                }
                [[fallthrough]];
            case FunctionClass::kDFdx:
            case FunctionClass::kFwidth:
                if (!fFoundDerivatives &&
                    fProgram.fCaps->shaderDerivativeExtensionString()) {
                    this->writeExtension(fProgram.fCaps->shaderDerivativeExtensionString());
                    fFoundDerivatives = true;
                }
                break;
            case FunctionClass::kDeterminant:
                if (!fProgram.fCaps->builtinDeterminantSupport()) {
                    SkASSERT(arguments.size() == 1);
                    this->writeDeterminantHack(*arguments[0]);
                    return;
                }
                break;
            case FunctionClass::kFMA:
                if (!fProgram.fCaps->builtinFMASupport()) {
                    SkASSERT(arguments.size() == 3);
                    this->write("((");
                    this->writeExpression(*arguments[0], kSequence_Precedence);
                    this->write(") * (");
                    this->writeExpression(*arguments[1], kSequence_Precedence);
                    this->write(") + (");
                    this->writeExpression(*arguments[2], kSequence_Precedence);
                    this->write("))");
                    return;
                }
                break;
            case FunctionClass::kFract:
                if (!fProgram.fCaps->canUseFractForNegativeValues()) {
                    SkASSERT(arguments.size() == 1);
                    this->write("(0.5 - sign(");
                    this->writeExpression(*arguments[0], kSequence_Precedence);
                    this->write(") * (0.5 - fract(abs(");
                    this->writeExpression(*arguments[0], kSequence_Precedence);
                    this->write("))))");
                    return;
                }
                break;
            case FunctionClass::kInverse:
                if (fProgram.fCaps->generation() < k140_GrGLSLGeneration) {
                    SkASSERT(arguments.size() == 1);
                    this->writeInverseHack(*arguments[0]);
                    return;
                }
                break;
            case FunctionClass::kInverseSqrt:
                if (fProgram.fCaps->generation() < k130_GrGLSLGeneration) {
                    SkASSERT(arguments.size() == 1);
                    this->writeInverseSqrtHack(*arguments[0]);
                    return;
                }
                break;
            case FunctionClass::kMin:
                if (!fProgram.fCaps->canUseMinAndAbsTogether()) {
                    SkASSERT(arguments.size() == 2);
                    if (is_abs(*arguments[0])) {
                        this->writeMinAbsHack(*arguments[0], *arguments[1]);
                        return;
                    }
                    if (is_abs(*arguments[1])) {
                        // note that this violates the GLSL left-to-right evaluation semantics.
                        // I doubt it will ever end up mattering, but it's worth calling out.
                        this->writeMinAbsHack(*arguments[1], *arguments[0]);
                        return;
                    }
                }
                break;
            case FunctionClass::kPow:
                if (!fProgram.fCaps->removePowWithConstantExponent()) {
                    break;
                }
                // pow(x, y) on some NVIDIA drivers causes crashes if y is a
                // constant.  It's hard to tell what constitutes "constant" here
                // so just replace in all cases.

                // Change pow(x, y) into exp2(y * log2(x))
                this->write("exp2(");
                this->writeExpression(*arguments[1], kMultiplicative_Precedence);
                this->write(" * log2(");
                this->writeExpression(*arguments[0], kSequence_Precedence);
                this->write("))");
                return;
            case FunctionClass::kSaturate:
                SkASSERT(arguments.size() == 1);
                this->write("clamp(");
                this->writeExpression(*arguments[0], kSequence_Precedence);
                this->write(", 0.0, 1.0)");
                return;
            case FunctionClass::kTexture: {
                const char* dim = "";
                bool proj = false;
                const Type& arg0Type = arguments[0]->type();
                const Type& arg1Type = arguments[1]->type();
                switch (arg0Type.dimensions()) {
                    case SpvDim1D:
                        dim = "1D";
                        isTextureFunctionWithBias = true;
                        if (arg1Type == *fContext.fFloat_Type) {
                            proj = false;
                        } else {
                            SkASSERT(arg1Type == *fContext.fFloat2_Type);
                            proj = true;
                        }
                        break;
                    case SpvDim2D:
                        dim = "2D";
                        if (arg0Type != *fContext.fSamplerExternalOES_Type) {
                            isTextureFunctionWithBias = true;
                        }
                        if (arg1Type == *fContext.fFloat2_Type) {
                            proj = false;
                        } else {
                            SkASSERT(arg1Type == *fContext.fFloat3_Type);
                            proj = true;
                        }
                        break;
                    case SpvDim3D:
                        dim = "3D";
                        isTextureFunctionWithBias = true;
                        if (arg1Type == *fContext.fFloat3_Type) {
                            proj = false;
                        } else {
                            SkASSERT(arg1Type == *fContext.fFloat4_Type);
                            proj = true;
                        }
                        break;
                    case SpvDimCube:
                        dim = "Cube";
                        isTextureFunctionWithBias = true;
                        proj = false;
                        break;
                    case SpvDimRect:
                        dim = "2DRect";
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
                if (fTextureFunctionOverride != "") {
                    this->write(fTextureFunctionOverride.c_str());
                } else {
                    this->write("texture");
                    if (fProgram.fCaps->generation() < k130_GrGLSLGeneration) {
                        this->write(dim);
                    }
                    if (proj) {
                        this->write("Proj");
                    }
                }
                nameWritten = true;
                break;
            }
            case FunctionClass::kTranspose:
                if (fProgram.fCaps->generation() < k130_GrGLSLGeneration) {
                    SkASSERT(arguments.size() == 1);
                    this->writeTransposeHack(*arguments[0]);
                    return;
                }
                break;
        }
    }
    if (!nameWritten) {
        this->write(function.name());
    }
    this->write("(");
    const char* separator = "";
    for (const auto& arg : arguments) {
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
    if (c.arguments().size() == 1 &&
        (this->getTypeName(c.type()) == this->getTypeName(c.arguments()[0]->type()) ||
        (c.type().isScalar() &&
         c.arguments()[0]->type() == *fContext.fFloatLiteral_Type))) {
        // in cases like half(float), they're different types as far as SkSL is concerned but the
        // same type as far as GLSL is concerned. We avoid a redundant float(float) by just writing
        // out the inner expression here.
        this->writeExpression(*c.arguments()[0], parentPrecedence);
        return;
    }
    this->writeType(c.type());
    this->write("(");
    const char* separator = "";
    for (const auto& arg : c.arguments()) {
        this->write(separator);
        separator = ", ";
        this->writeExpression(*arg, kSequence_Precedence);
    }
    this->write(")");
}

void GLSLCodeGenerator::writeFragCoord() {
    if (!fProgram.fCaps->canUseFragCoord()) {
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
    } else if (const char* extension = fProgram.fCaps->fragCoordConventionsExtensionString()) {
        if (!fSetupFragPositionGlobal) {
            if (fProgram.fCaps->generation() < k150_GrGLSLGeneration) {
                this->writeExtension(extension);
            }
            fGlobals.writeText("layout(origin_upper_left) in vec4 gl_FragCoord;\n");
            fSetupFragPositionGlobal = true;
        }
        this->write("gl_FragCoord");
    } else {
        if (!fSetupFragPositionLocal) {
            fFunctionHeader += usesPrecisionModifiers() ? "highp " : "";
            fFunctionHeader += "    vec4 sk_FragCoord = vec4(gl_FragCoord.x, " SKSL_RTHEIGHT_NAME
                               " - gl_FragCoord.y, gl_FragCoord.z, gl_FragCoord.w);\n";
            fSetupFragPositionLocal = true;
        }
        this->write("sk_FragCoord");
    }
}

void GLSLCodeGenerator::writeVariableReference(const VariableReference& ref) {
    switch (ref.variable()->modifiers().fLayout.fBuiltin) {
        case SK_FRAGCOLOR_BUILTIN:
            if (fProgram.fCaps->mustDeclareFragmentShaderOutput()) {
                this->write("sk_FragColor");
            } else {
                this->write("gl_FragColor");
            }
            break;
        case SK_FRAGCOORD_BUILTIN:
            this->writeFragCoord();
            break;
        case SK_WIDTH_BUILTIN:
            this->write("u_skRTWidth");
            break;
        case SK_HEIGHT_BUILTIN:
            this->write("u_skRTHeight");
            break;
        case SK_CLOCKWISE_BUILTIN:
            this->write(fProgram.fSettings.fFlipY ? "(!gl_FrontFacing)" : "gl_FrontFacing");
            break;
        case SK_SAMPLEMASK_BUILTIN:
            SkASSERT(fProgram.fCaps->sampleMaskSupport());
            this->write("gl_SampleMask");
            break;
        case SK_VERTEXID_BUILTIN:
            this->write("gl_VertexID");
            break;
        case SK_INSTANCEID_BUILTIN:
            this->write("gl_InstanceID");
            break;
        case SK_IN_BUILTIN:
            this->write("gl_in");
            break;
        case SK_INVOCATIONID_BUILTIN:
            this->write("gl_InvocationID");
            break;
        case SK_LASTFRAGCOLOR_BUILTIN:
            this->write(fProgram.fCaps->fbFetchColorName());
            break;
        default:
            this->write(ref.variable()->name());
    }
}

void GLSLCodeGenerator::writeIndexExpression(const IndexExpression& expr) {
    this->writeExpression(*expr.base(), kPostfix_Precedence);
    this->write("[");
    this->writeExpression(*expr.index(), kTopLevel_Precedence);
    this->write("]");
}

bool is_sk_position(const FieldAccess& f) {
    return "sk_Position" == f.base()->type().fields()[f.fieldIndex()].fName;
}

void GLSLCodeGenerator::writeFieldAccess(const FieldAccess& f) {
    if (f.ownerKind() == FieldAccess::OwnerKind::kDefault) {
        this->writeExpression(*f.base(), kPostfix_Precedence);
        this->write(".");
    }
    const Type& baseType = f.base()->type();
    StringFragment name = baseType.fields()[f.fieldIndex()].fName;
    if (name == "sk_Position") {
        this->write("gl_Position");
    } else if (name == "sk_PointSize") {
        this->write("gl_PointSize");
    } else {
        this->write(baseType.fields()[f.fieldIndex()].fName);
    }
}

void GLSLCodeGenerator::writeSwizzle(const Swizzle& swizzle) {
    this->writeExpression(*swizzle.base(), kPostfix_Precedence);
    this->write(".");
    for (int c : swizzle.components()) {
        SkASSERT(c >= 0 && c <= 3);
        this->write(&("x\0y\0z\0w\0"[c * 2]));
    }
}

GLSLCodeGenerator::Precedence GLSLCodeGenerator::GetBinaryPrecedence(Token::Kind op) {
    switch (op) {
        case Token::Kind::TK_STAR:         // fall through
        case Token::Kind::TK_SLASH:        // fall through
        case Token::Kind::TK_PERCENT:      return GLSLCodeGenerator::kMultiplicative_Precedence;
        case Token::Kind::TK_PLUS:         // fall through
        case Token::Kind::TK_MINUS:        return GLSLCodeGenerator::kAdditive_Precedence;
        case Token::Kind::TK_SHL:          // fall through
        case Token::Kind::TK_SHR:          return GLSLCodeGenerator::kShift_Precedence;
        case Token::Kind::TK_LT:           // fall through
        case Token::Kind::TK_GT:           // fall through
        case Token::Kind::TK_LTEQ:         // fall through
        case Token::Kind::TK_GTEQ:         return GLSLCodeGenerator::kRelational_Precedence;
        case Token::Kind::TK_EQEQ:         // fall through
        case Token::Kind::TK_NEQ:          return GLSLCodeGenerator::kEquality_Precedence;
        case Token::Kind::TK_BITWISEAND:   return GLSLCodeGenerator::kBitwiseAnd_Precedence;
        case Token::Kind::TK_BITWISEXOR:   return GLSLCodeGenerator::kBitwiseXor_Precedence;
        case Token::Kind::TK_BITWISEOR:    return GLSLCodeGenerator::kBitwiseOr_Precedence;
        case Token::Kind::TK_LOGICALAND:   return GLSLCodeGenerator::kLogicalAnd_Precedence;
        case Token::Kind::TK_LOGICALXOR:   return GLSLCodeGenerator::kLogicalXor_Precedence;
        case Token::Kind::TK_LOGICALOR:    return GLSLCodeGenerator::kLogicalOr_Precedence;
        case Token::Kind::TK_EQ:           // fall through
        case Token::Kind::TK_PLUSEQ:       // fall through
        case Token::Kind::TK_MINUSEQ:      // fall through
        case Token::Kind::TK_STAREQ:       // fall through
        case Token::Kind::TK_SLASHEQ:      // fall through
        case Token::Kind::TK_PERCENTEQ:    // fall through
        case Token::Kind::TK_SHLEQ:        // fall through
        case Token::Kind::TK_SHREQ:        // fall through
        case Token::Kind::TK_BITWISEANDEQ: // fall through
        case Token::Kind::TK_BITWISEXOREQ: // fall through
        case Token::Kind::TK_BITWISEOREQ:  return GLSLCodeGenerator::kAssignment_Precedence;
        case Token::Kind::TK_COMMA:        return GLSLCodeGenerator::kSequence_Precedence;
        default: ABORT("unsupported binary operator");
    }
}

void GLSLCodeGenerator::writeBinaryExpression(const BinaryExpression& b,
                                              Precedence parentPrecedence) {
    const Expression& left = *b.left();
    const Expression& right = *b.right();
    Token::Kind op = b.getOperator();
    if (fProgram.fCaps->unfoldShortCircuitAsTernary() &&
            (op == Token::Kind::TK_LOGICALAND || op == Token::Kind::TK_LOGICALOR)) {
        this->writeShortCircuitWorkaroundExpression(b, parentPrecedence);
        return;
    }

    Precedence precedence = GetBinaryPrecedence(op);
    if (precedence >= parentPrecedence) {
        this->write("(");
    }
    bool positionWorkaround = fProgramKind == Program::Kind::kVertex_Kind &&
                              Compiler::IsAssignment(op) &&
                              left.kind() == Expression::Kind::kFieldAccess &&
                              is_sk_position((FieldAccess&) left) &&
                              !right.containsRTAdjust() &&
                              !fProgram.fCaps->canUseFragCoord();
    if (positionWorkaround) {
        this->write("sk_FragCoord_Workaround = (");
    }
    this->writeExpression(left, precedence);
    this->write(" ");
    this->write(Compiler::OperatorName(op));
    this->write(" ");
    this->writeExpression(right, precedence);
    if (positionWorkaround) {
        this->write(")");
    }
    if (precedence >= parentPrecedence) {
        this->write(")");
    }
}

void GLSLCodeGenerator::writeShortCircuitWorkaroundExpression(const BinaryExpression& b,
                                                              Precedence parentPrecedence) {
    if (kTernary_Precedence >= parentPrecedence) {
        this->write("(");
    }

    // Transform:
    // a && b  =>   a ? b : false
    // a || b  =>   a ? true : b
    this->writeExpression(*b.left(), kTernary_Precedence);
    this->write(" ? ");
    if (b.getOperator() == Token::Kind::TK_LOGICALAND) {
        this->writeExpression(*b.right(), kTernary_Precedence);
    } else {
        BoolLiteral boolTrue(fContext, -1, true);
        this->writeBoolLiteral(boolTrue);
    }
    this->write(" : ");
    if (b.getOperator() == Token::Kind::TK_LOGICALAND) {
        BoolLiteral boolFalse(fContext, -1, false);
        this->writeBoolLiteral(boolFalse);
    } else {
        this->writeExpression(*b.right(), kTernary_Precedence);
    }
    if (kTernary_Precedence >= parentPrecedence) {
        this->write(")");
    }
}

void GLSLCodeGenerator::writeTernaryExpression(const TernaryExpression& t,
                                               Precedence parentPrecedence) {
    if (kTernary_Precedence >= parentPrecedence) {
        this->write("(");
    }
    this->writeExpression(*t.test(), kTernary_Precedence);
    this->write(" ? ");
    this->writeExpression(*t.ifTrue(), kTernary_Precedence);
    this->write(" : ");
    this->writeExpression(*t.ifFalse(), kTernary_Precedence);
    if (kTernary_Precedence >= parentPrecedence) {
        this->write(")");
    }
}

void GLSLCodeGenerator::writePrefixExpression(const PrefixExpression& p,
                                              Precedence parentPrecedence) {
    if (kPrefix_Precedence >= parentPrecedence) {
        this->write("(");
    }
    this->write(Compiler::OperatorName(p.getOperator()));
    this->writeExpression(*p.operand(), kPrefix_Precedence);
    if (kPrefix_Precedence >= parentPrecedence) {
        this->write(")");
    }
}

void GLSLCodeGenerator::writePostfixExpression(const PostfixExpression& p,
                                               Precedence parentPrecedence) {
    if (kPostfix_Precedence >= parentPrecedence) {
        this->write("(");
    }
    this->writeExpression(*p.operand(), kPostfix_Precedence);
    this->write(Compiler::OperatorName(p.getOperator()));
    if (kPostfix_Precedence >= parentPrecedence) {
        this->write(")");
    }
}

void GLSLCodeGenerator::writeBoolLiteral(const BoolLiteral& b) {
    this->write(b.value() ? "true" : "false");
}

void GLSLCodeGenerator::writeIntLiteral(const IntLiteral& i) {
    const Type& type = i.type();
    if (type == *fContext.fUInt_Type) {
        this->write(to_string(i.value() & 0xffffffff) + "u");
    } else if (type == *fContext.fUShort_Type) {
        this->write(to_string(i.value() & 0xffff) + "u");
    } else if (type == *fContext.fUByte_Type) {
        this->write(to_string(i.value() & 0xff) + "u");
    } else {
        this->write(to_string((int32_t) i.value()));
    }
}

void GLSLCodeGenerator::writeFloatLiteral(const FloatLiteral& f) {
    this->write(to_string(f.value()));
}

void GLSLCodeGenerator::writeSetting(const Setting& s) {
    ABORT("internal error; setting was not folded to a constant during compilation\n");
}

void GLSLCodeGenerator::writeFunctionDeclaration(const FunctionDeclaration& f) {
    this->writeTypePrecision(f.returnType());
    this->writeType(f.returnType());
    this->write(" " + f.name() + "(");
    const char* separator = "";
    for (const auto& param : f.parameters()) {
        this->write(separator);
        separator = ", ";
        this->writeModifiers(param->modifiers(), false);
        std::vector<int> sizes;
        const Type* type = &param->type();
        if (type->isArray()) {
            sizes.push_back(type->columns());
            type = &type->componentType();
        }
        this->writeTypePrecision(*type);
        this->writeType(*type);
        this->write(" " + param->name());
        for (int s : sizes) {
            if (s == Type::kUnsizedArray) {
                this->write("[]");
            } else {
                this->write("[" + to_string(s) + "]");
            }
        }
    }
    this->write(")");
}

void GLSLCodeGenerator::writeFunction(const FunctionDefinition& f) {
    fSetupFragPositionLocal = false;
    fSetupFragCoordWorkaround = false;

    // The pipeline-stage code generator can't use functions written this way, so make sure we don't
    // accidentally end up here.
    SkASSERT(fProgramKind != Program::kPipelineStage_Kind);

    this->writeFunctionDeclaration(f.declaration());
    this->writeLine(" {");
    fIndentation++;

    fFunctionHeader = "";
    OutputStream* oldOut = fOut;
    StringStream buffer;
    fOut = &buffer;
    for (const std::unique_ptr<Statement>& stmt : f.body()->as<Block>().children()) {
        if (!stmt->isEmpty()) {
            this->writeStatement(*stmt);
            this->writeLine();
        }
    }

    fIndentation--;
    this->writeLine("}");

    fOut = oldOut;
    this->write(fFunctionHeader);
    this->write(buffer.str());
}

void GLSLCodeGenerator::writeFunctionPrototype(const FunctionPrototype& f) {
    this->writeFunctionDeclaration(f.declaration());
    this->writeLine(";");
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
            fProgram.fCaps->generation() < GrGLSLGeneration::k130_GrGLSLGeneration) {
            this->write(fProgramKind == Program::kVertex_Kind ? "attribute "
                                                              : "varying ");
        } else {
            this->write("in ");
        }
    } else if (modifiers.fFlags & Modifiers::kOut_Flag) {
        if (globalContext &&
            fProgram.fCaps->generation() < GrGLSLGeneration::k130_GrGLSLGeneration) {
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
    if (modifiers.fFlags & Modifiers::kPLS_Flag) {
        this->write("__pixel_localEXT ");
    }
    if (modifiers.fFlags & Modifiers::kPLSIn_Flag) {
        this->write("__pixel_local_inEXT ");
    }
    if (modifiers.fFlags & Modifiers::kPLSOut_Flag) {
        this->write("__pixel_local_outEXT ");
    }
    switch (modifiers.fLayout.fFormat) {
        case Layout::Format::kUnspecified:
            break;
        case Layout::Format::kRGBA32F:      // fall through
        case Layout::Format::kR32F:
            this->write("highp ");
            break;
        case Layout::Format::kRGBA16F:      // fall through
        case Layout::Format::kR16F:         // fall through
        case Layout::Format::kLUMINANCE16F: // fall through
        case Layout::Format::kRG16F:
            this->write("mediump ");
            break;
        case Layout::Format::kRGBA8:        // fall through
        case Layout::Format::kR8:           // fall through
        case Layout::Format::kRGBA8I:       // fall through
        case Layout::Format::kR8I:
            this->write("lowp ");
            break;
    }
}

void GLSLCodeGenerator::writeInterfaceBlock(const InterfaceBlock& intf) {
    if (intf.typeName() == "sk_PerVertex") {
        return;
    }
    this->writeModifiers(intf.variable().modifiers(), true);
    this->writeLine(intf.typeName() + " {");
    fIndentation++;
    const Type* structType = &intf.variable().type();
    if (structType->isArray()) {
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
    if (intf.instanceName().size()) {
        this->write(" ");
        this->write(intf.instanceName());
        if (intf.arraySize() > 0) {
            this->write("[");
            this->write(to_string(intf.arraySize()));
            this->write("]");
        } else if (intf.arraySize() == Type::kUnsizedArray){
            this->write("[]");
        }
    }
    this->writeLine(";");
}

void GLSLCodeGenerator::writeVarInitializer(const Variable& var, const Expression& value) {
    this->writeExpression(value, kTopLevel_Precedence);
}

const char* GLSLCodeGenerator::getTypePrecision(const Type& type) {
    if (usesPrecisionModifiers()) {
        switch (type.typeKind()) {
            case Type::TypeKind::kScalar:
                if (type == *fContext.fShort_Type || type == *fContext.fUShort_Type ||
                    type == *fContext.fByte_Type || type == *fContext.fUByte_Type) {
                    if (fProgram.fSettings.fForceHighPrecision ||
                            fProgram.fCaps->incompleteShortIntPrecision()) {
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
            case Type::TypeKind::kVector: // fall through
            case Type::TypeKind::kMatrix:
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

void GLSLCodeGenerator::writeVarDeclaration(const VarDeclaration& var, bool global) {
    this->writeModifiers(var.var().modifiers(), global);
    this->writeTypePrecision(var.baseType());
    this->writeType(var.baseType());
    this->write(" ");
    this->write(var.var().name());
    if (var.arraySize() > 0) {
        this->write("[");
        this->write(to_string(var.arraySize()));
        this->write("]");
    } else if (var.arraySize() == Type::kUnsizedArray){
        this->write("[]");
    }
    if (var.value()) {
        this->write(" = ");
        this->writeVarInitializer(var.var(), *var.value());
    }
    if (!fFoundExternalSamplerDecl && var.var().type() == *fContext.fSamplerExternalOES_Type) {
        if (fProgram.fCaps->externalTextureExtensionString()) {
            this->writeExtension(fProgram.fCaps->externalTextureExtensionString());
        }
        if (fProgram.fCaps->secondExternalTextureExtensionString()) {
            this->writeExtension(fProgram.fCaps->secondExternalTextureExtensionString());
        }
        fFoundExternalSamplerDecl = true;
    }
    if (!fFoundRectSamplerDecl && var.var().type() == *fContext.fSampler2DRect_Type) {
        fFoundRectSamplerDecl = true;
    }
    this->write(";");
}

void GLSLCodeGenerator::writeStatement(const Statement& s) {
    switch (s.kind()) {
        case Statement::Kind::kBlock:
            this->writeBlock(s.as<Block>());
            break;
        case Statement::Kind::kExpression:
            this->writeExpression(*s.as<ExpressionStatement>().expression(), kTopLevel_Precedence);
            this->write(";");
            break;
        case Statement::Kind::kReturn:
            this->writeReturnStatement(s.as<ReturnStatement>());
            break;
        case Statement::Kind::kVarDeclaration:
            this->writeVarDeclaration(s.as<VarDeclaration>(), false);
            break;
        case Statement::Kind::kIf:
            this->writeIfStatement(s.as<IfStatement>());
            break;
        case Statement::Kind::kFor:
            this->writeForStatement(s.as<ForStatement>());
            break;
        case Statement::Kind::kDo:
            this->writeDoStatement(s.as<DoStatement>());
            break;
        case Statement::Kind::kSwitch:
            this->writeSwitchStatement(s.as<SwitchStatement>());
            break;
        case Statement::Kind::kBreak:
            this->write("break;");
            break;
        case Statement::Kind::kContinue:
            this->write("continue;");
            break;
        case Statement::Kind::kDiscard:
            this->write("discard;");
            break;
        case Statement::Kind::kInlineMarker:
        case Statement::Kind::kNop:
            this->write(";");
            break;
        default:
#ifdef SK_DEBUG
            ABORT("unsupported statement: %s", s.description().c_str());
#endif
            break;
    }
}

void GLSLCodeGenerator::writeBlock(const Block& b) {
    bool isScope = b.isScope();
    if (isScope) {
        this->writeLine("{");
        fIndentation++;
    }
    for (const std::unique_ptr<Statement>& stmt : b.children()) {
        if (!stmt->isEmpty()) {
            this->writeStatement(*stmt);
            this->writeLine();
        }
    }
    if (isScope) {
        fIndentation--;
        this->write("}");
    }
}

void GLSLCodeGenerator::writeIfStatement(const IfStatement& stmt) {
    this->write("if (");
    this->writeExpression(*stmt.test(), kTopLevel_Precedence);
    this->write(") ");
    this->writeStatement(*stmt.ifTrue());
    if (stmt.ifFalse()) {
        this->write(" else ");
        this->writeStatement(*stmt.ifFalse());
    }
}

void GLSLCodeGenerator::writeForStatement(const ForStatement& f) {
    // Emit loops of the form 'for(;test;)' as 'while(test)', which is probably how they started
    if (!f.initializer() && f.test() && !f.next()) {
        this->write("while (");
        this->writeExpression(*f.test(), kTopLevel_Precedence);
        this->write(") ");
        this->writeStatement(*f.statement());
        return;
    }

    this->write("for (");
    if (f.initializer() && !f.initializer()->isEmpty()) {
        this->writeStatement(*f.initializer());
    } else {
        this->write("; ");
    }
    if (f.test()) {
        if (fProgram.fCaps->addAndTrueToLoopCondition()) {
            std::unique_ptr<Expression> and_true(new BinaryExpression(
                    -1, f.test()->clone(), Token::Kind::TK_LOGICALAND,
                    std::make_unique<BoolLiteral>(fContext, -1, true),
                    fContext.fBool_Type.get()));
            this->writeExpression(*and_true, kTopLevel_Precedence);
        } else {
            this->writeExpression(*f.test(), kTopLevel_Precedence);
        }
    }
    this->write("; ");
    if (f.next()) {
        this->writeExpression(*f.next(), kTopLevel_Precedence);
    }
    this->write(") ");
    this->writeStatement(*f.statement());
}

void GLSLCodeGenerator::writeDoStatement(const DoStatement& d) {
    if (!fProgram.fCaps->rewriteDoWhileLoops()) {
        this->write("do ");
        this->writeStatement(*d.statement());
        this->write(" while (");
        this->writeExpression(*d.test(), kTopLevel_Precedence);
        this->write(");");
        return;
    }

    // Otherwise, do the do while loop workaround, to rewrite loops of the form:
    //     do {
    //         CODE;
    //     } while (CONDITION)
    //
    // to loops of the form
    //     bool temp = false;
    //     while (true) {
    //         if (temp) {
    //             if (!CONDITION) {
    //                 break;
    //             }
    //         }
    //         temp = true;
    //         CODE;
    //     }
    String tmpVar = "_tmpLoopSeenOnce" + to_string(fVarCount++);
    this->write("bool ");
    this->write(tmpVar);
    this->writeLine(" = false;");
    this->writeLine("while (true) {");
    fIndentation++;
    this->write("if (");
    this->write(tmpVar);
    this->writeLine(") {");
    fIndentation++;
    this->write("if (!");
    this->writeExpression(*d.test(), kPrefix_Precedence);
    this->writeLine(") {");
    fIndentation++;
    this->writeLine("break;");
    fIndentation--;
    this->writeLine("}");
    fIndentation--;
    this->writeLine("}");
    this->write(tmpVar);
    this->writeLine(" = true;");
    this->writeStatement(*d.statement());
    this->writeLine();
    fIndentation--;
    this->write("}");
}

void GLSLCodeGenerator::writeSwitchStatement(const SwitchStatement& s) {
    this->write("switch (");
    this->writeExpression(*s.value(), kTopLevel_Precedence);
    this->writeLine(") {");
    fIndentation++;
    for (const std::unique_ptr<SwitchCase>& c : s.cases()) {
        if (c->value()) {
            this->write("case ");
            this->writeExpression(*c->value(), kTopLevel_Precedence);
            this->writeLine(":");
        } else {
            this->writeLine("default:");
        }
        fIndentation++;
        for (const auto& stmt : c->statements()) {
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
    if (r.expression()) {
        this->write(" ");
        this->writeExpression(*r.expression(), kTopLevel_Precedence);
    }
    this->write(";");
}

void GLSLCodeGenerator::writeHeader() {
    if (fProgram.fCaps->versionDeclString()) {
        this->write(fProgram.fCaps->versionDeclString());
        this->writeLine();
    }
}

void GLSLCodeGenerator::writeProgramElement(const ProgramElement& e) {
    switch (e.kind()) {
        case ProgramElement::Kind::kExtension:
            this->writeExtension(e.as<Extension>().name());
            break;
        case ProgramElement::Kind::kGlobalVar: {
            const VarDeclaration& decl =
                                   e.as<GlobalVarDeclaration>().declaration()->as<VarDeclaration>();
            int builtin = decl.var().modifiers().fLayout.fBuiltin;
            if (builtin == -1) {
                // normal var
                this->writeVarDeclaration(decl, true);
                this->writeLine();
            } else if (builtin == SK_FRAGCOLOR_BUILTIN &&
                       fProgram.fCaps->mustDeclareFragmentShaderOutput()) {
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
            break;
        }
        case ProgramElement::Kind::kInterfaceBlock:
            this->writeInterfaceBlock(e.as<InterfaceBlock>());
            break;
        case ProgramElement::Kind::kFunction:
            this->writeFunction(e.as<FunctionDefinition>());
            break;
        case ProgramElement::Kind::kFunctionPrototype:
            this->writeFunctionPrototype(e.as<FunctionPrototype>());
            break;
        case ProgramElement::Kind::kModifiers: {
            const Modifiers& modifiers = e.as<ModifiersDeclaration>().modifiers();
            if (!fFoundGSInvocations && modifiers.fLayout.fInvocations >= 0) {
                if (fProgram.fCaps->gsInvocationsExtensionString()) {
                    this->writeExtension(fProgram.fCaps->gsInvocationsExtensionString());
                }
                fFoundGSInvocations = true;
            }
            this->writeModifiers(modifiers, true);
            this->writeLine(";");
            break;
        }
        case ProgramElement::Kind::kEnum:
            break;
        case ProgramElement::Kind::kStructDefinition:
            if (this->writeStructDefinition(e.as<StructDefinition>().type())) {
                this->writeLine(";");
            }
            break;
        default:
            SkDEBUGFAILF("unsupported program element %s\n", e.description().c_str());
            break;
    }
}

void GLSLCodeGenerator::writeInputVars() {
    if (fProgram.fInputs.fRTWidth) {
        const char* precision = usesPrecisionModifiers() ? "highp " : "";
        fGlobals.writeText("uniform ");
        fGlobals.writeText(precision);
        fGlobals.writeText("float " SKSL_RTWIDTH_NAME ";\n");
    }
    if (fProgram.fInputs.fRTHeight) {
        const char* precision = usesPrecisionModifiers() ? "highp " : "";
        fGlobals.writeText("uniform ");
        fGlobals.writeText(precision);
        fGlobals.writeText("float " SKSL_RTHEIGHT_NAME ";\n");
    }
}

bool GLSLCodeGenerator::generateCode() {
    this->writeHeader();
    if (Program::kGeometry_Kind == fProgramKind &&
        fProgram.fCaps->geometryShaderExtensionString()) {
        this->writeExtension(fProgram.fCaps->geometryShaderExtensionString());
    }
    OutputStream* rawOut = fOut;
    StringStream body;
    fOut = &body;
    for (const ProgramElement* e : fProgram.elements()) {
        this->writeProgramElement(*e);
    }
    fOut = rawOut;

    write_stringstream(fExtensions, *rawOut);
    this->writeInputVars();
    write_stringstream(fGlobals, *rawOut);

    if (!fProgram.fCaps->canUseFragCoord()) {
        Layout layout;
        switch (fProgram.fKind) {
            case Program::kVertex_Kind: {
                Modifiers modifiers(layout, Modifiers::kOut_Flag);
                this->writeModifiers(modifiers, true);
                if (this->usesPrecisionModifiers()) {
                    this->write("highp ");
                }
                this->write("vec4 sk_FragCoord_Workaround;\n");
                break;
            }
            case Program::kFragment_Kind: {
                Modifiers modifiers(layout, Modifiers::kIn_Flag);
                this->writeModifiers(modifiers, true);
                if (this->usesPrecisionModifiers()) {
                    this->write("highp ");
                }
                this->write("vec4 sk_FragCoord_Workaround;\n");
                break;
            }
            default:
                break;
        }
    }

    if (this->usesPrecisionModifiers()) {
        this->writeLine("precision mediump float;");
        this->writeLine("precision mediump sampler2D;");
        if (fFoundExternalSamplerDecl &&
            !fProgram.fCaps->noDefaultPrecisionForExternalSamplers()) {
            this->writeLine("precision mediump samplerExternalOES;");
        }
        if (fFoundRectSamplerDecl) {
            this->writeLine("precision mediump sampler2DRect;");
        }
    }
    write_stringstream(fExtraFunctions, *rawOut);
    write_stringstream(body, *rawOut);
    return 0 == fErrors.errorCount();
}

}  // namespace SkSL
