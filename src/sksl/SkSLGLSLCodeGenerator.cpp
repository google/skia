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
    this->writeLine();
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
    fOut->writeText(fLineEnding);
    fAtLineStart = true;
}

void GLSLCodeGenerator::finishLine() {
    if (!fAtLineStart) {
        this->writeLine();
    }
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
    return this->caps().usesPrecisionModifiers();
}

// Returns the name of the type with array dimensions, e.g. `float[2]`.
String GLSLCodeGenerator::getTypeName(const Type& type) {
    switch (type.typeKind()) {
        case Type::TypeKind::kVector: {
            const Type& component = type.componentType();
            String result;
            if (component == *fContext.fTypes.fFloat || component == *fContext.fTypes.fHalf) {
                result = "vec";
            }
            else if (component.isSigned()) {
                result = "ivec";
            }
            else if (component.isUnsigned()) {
                result = "uvec";
            }
            else if (component == *fContext.fTypes.fBool) {
                result = "bvec";
            }
            else {
                SK_ABORT("unsupported vector type");
            }
            result += to_string(type.columns());
            return result;
        }
        case Type::TypeKind::kMatrix: {
            String result;
            const Type& component = type.componentType();
            if (component == *fContext.fTypes.fFloat || component == *fContext.fTypes.fHalf) {
                result = "mat";
            }
            else {
                SK_ABORT("unsupported matrix type");
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
            if (type == *fContext.fTypes.fHalf) {
                return "float";
            }
            else if (type == *fContext.fTypes.fShort) {
                return "int";
            }
            else if (type == *fContext.fTypes.fUShort) {
                return "uint";
            }
            else if (type == *fContext.fTypes.fByte) {
                return "int";
            }
            else if (type == *fContext.fTypes.fUByte) {
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

void GLSLCodeGenerator::writeStructDefinition(const StructDefinition& s) {
    const Type& type = s.type();
    this->write("struct ");
    this->write(type.name());
    this->writeLine(" {");
    fIndentation++;
    for (const auto& f : type.fields()) {
        this->writeModifiers(f.fModifiers, false);
        this->writeTypePrecision(*f.fType);
        const Type& baseType = f.fType->isArray() ? f.fType->componentType() : *f.fType;
        this->writeType(baseType);
        this->write(" ");
        this->write(f.fName);
        if (f.fType->isArray()) {
            this->write("[" + to_string(f.fType->columns()) + "]");
        }
        this->writeLine(";");
    }
    fIndentation--;
    this->writeLine("};");
}

void GLSLCodeGenerator::writeType(const Type& type) {
    this->write(this->getTypeName(type));
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
            SkDEBUGFAILF("unsupported expression: %s", expr.description().c_str());
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
    SkASSERT(!this->caps().canUseMinAndAbsTogether());
    String tmpVar1 = "minAbsHackVar" + to_string(fVarCount++);
    String tmpVar2 = "minAbsHackVar" + to_string(fVarCount++);
    this->fFunctionHeader += String("    ") + this->getTypePrecision(absExpr.type()) +
                             this->getTypeName(absExpr.type()) + " " + tmpVar1 + ";\n";
    this->fFunctionHeader += String("    ") + this->getTypePrecision(otherExpr.type()) +
                             this->getTypeName(otherExpr.type()) + " " + tmpVar2 + ";\n";
    this->write("((" + tmpVar1 + " = ");
    this->writeExpression(absExpr, Precedence::kTopLevel);
    this->write(") < (" + tmpVar2 + " = ");
    this->writeExpression(otherExpr, Precedence::kAssignment);
    this->write(") ? " + tmpVar1 + " : " + tmpVar2 + ")");
}

void GLSLCodeGenerator::writeInverseSqrtHack(const Expression& x) {
    this->write("(1.0 / sqrt(");
    this->writeExpression(x, Precedence::kTopLevel);
    this->write("))");
}

void GLSLCodeGenerator::writeDeterminantHack(const Expression& mat) {
    String name;
    const Type& type = mat.type();
    if (type == *fContext.fTypes.fFloat2x2 || type == *fContext.fTypes.fHalf2x2) {
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
    else if (type == *fContext.fTypes.fFloat3x3 || type == *fContext.fTypes.fHalf3x3) {
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
    else if (type == *fContext.fTypes.fFloat4x4 || type == *fContext.fTypes.fHalf4x4) {
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
    this->writeExpression(mat, Precedence::kTopLevel);
    this->write(")");
}

void GLSLCodeGenerator::writeInverseHack(const Expression& mat) {
    String name;
    const Type& type = mat.type();
    if (type == *fContext.fTypes.fFloat2x2 || type == *fContext.fTypes.fHalf2x2) {
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
    else if (type == *fContext.fTypes.fFloat3x3 || type == *fContext.fTypes.fHalf3x3) {
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
    else if (type == *fContext.fTypes.fFloat4x4 || type == *fContext.fTypes.fHalf4x4) {
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
    this->writeExpression(mat, Precedence::kTopLevel);
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
    this->writeExpression(mat, Precedence::kTopLevel);
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
    const auto found = (function.isBuiltin() && !function.definition())
                               ? fFunctionClasses->find(function.name())
                               : fFunctionClasses->end();
    bool isTextureFunctionWithBias = false;
    bool nameWritten = false;
    if (found != fFunctionClasses->end()) {
        switch (found->second) {
            case FunctionClass::kAbs: {
                if (!this->caps().emulateAbsIntFunction())
                    break;
                SkASSERT(arguments.size() == 1);
                if (arguments[0]->type() != *fContext.fTypes.fInt) {
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
                if (this->caps().mustForceNegatedAtanParamToFloat() &&
                    arguments.size() == 2 &&
                    arguments[1]->kind() == Expression::Kind::kPrefix) {
                    const PrefixExpression& p = (PrefixExpression&) *arguments[1];
                    if (p.getOperator().kind() == Token::Kind::TK_MINUS) {
                        this->write("atan(");
                        this->writeExpression(*arguments[0], Precedence::kSequence);
                        this->write(", -1.0 * ");
                        this->writeExpression(*p.operand(), Precedence::kMultiplicative);
                        this->write(")");
                        return;
                    }
                }
                break;
            case FunctionClass::kDFdy:
                if (fProgram.fConfig->fSettings.fFlipY) {
                    // Flipping Y also negates the Y derivatives.
                    this->write("-dFdy");
                    nameWritten = true;
                }
                [[fallthrough]];
            case FunctionClass::kDFdx:
            case FunctionClass::kFwidth:
                if (!fFoundDerivatives &&
                    this->caps().shaderDerivativeExtensionString()) {
                    this->writeExtension(this->caps().shaderDerivativeExtensionString());
                    fFoundDerivatives = true;
                }
                break;
            case FunctionClass::kDeterminant:
                if (!this->caps().builtinDeterminantSupport()) {
                    SkASSERT(arguments.size() == 1);
                    this->writeDeterminantHack(*arguments[0]);
                    return;
                }
                break;
            case FunctionClass::kFMA:
                if (!this->caps().builtinFMASupport()) {
                    SkASSERT(arguments.size() == 3);
                    this->write("((");
                    this->writeExpression(*arguments[0], Precedence::kSequence);
                    this->write(") * (");
                    this->writeExpression(*arguments[1], Precedence::kSequence);
                    this->write(") + (");
                    this->writeExpression(*arguments[2], Precedence::kSequence);
                    this->write("))");
                    return;
                }
                break;
            case FunctionClass::kFract:
                if (!this->caps().canUseFractForNegativeValues()) {
                    SkASSERT(arguments.size() == 1);
                    this->write("(0.5 - sign(");
                    this->writeExpression(*arguments[0], Precedence::kSequence);
                    this->write(") * (0.5 - fract(abs(");
                    this->writeExpression(*arguments[0], Precedence::kSequence);
                    this->write("))))");
                    return;
                }
                break;
            case FunctionClass::kInverse:
                if (this->caps().generation() < k140_GrGLSLGeneration) {
                    SkASSERT(arguments.size() == 1);
                    this->writeInverseHack(*arguments[0]);
                    return;
                }
                break;
            case FunctionClass::kInverseSqrt:
                if (this->caps().generation() < k130_GrGLSLGeneration) {
                    SkASSERT(arguments.size() == 1);
                    this->writeInverseSqrtHack(*arguments[0]);
                    return;
                }
                break;
            case FunctionClass::kMin:
                if (!this->caps().canUseMinAndAbsTogether()) {
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
                if (!this->caps().removePowWithConstantExponent()) {
                    break;
                }
                // pow(x, y) on some NVIDIA drivers causes crashes if y is a
                // constant.  It's hard to tell what constitutes "constant" here
                // so just replace in all cases.

                // Change pow(x, y) into exp2(y * log2(x))
                this->write("exp2(");
                this->writeExpression(*arguments[1], Precedence::kMultiplicative);
                this->write(" * log2(");
                this->writeExpression(*arguments[0], Precedence::kSequence);
                this->write("))");
                return;
            case FunctionClass::kSaturate:
                SkASSERT(arguments.size() == 1);
                this->write("clamp(");
                this->writeExpression(*arguments[0], Precedence::kSequence);
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
                        if (arg1Type == *fContext.fTypes.fFloat) {
                            proj = false;
                        } else {
                            SkASSERT(arg1Type == *fContext.fTypes.fFloat2);
                            proj = true;
                        }
                        break;
                    case SpvDim2D:
                        dim = "2D";
                        if (arg0Type != *fContext.fTypes.fSamplerExternalOES) {
                            isTextureFunctionWithBias = true;
                        }
                        if (arg1Type == *fContext.fTypes.fFloat2) {
                            proj = false;
                        } else {
                            SkASSERT(arg1Type == *fContext.fTypes.fFloat3);
                            proj = true;
                        }
                        break;
                    case SpvDim3D:
                        dim = "3D";
                        isTextureFunctionWithBias = true;
                        if (arg1Type == *fContext.fTypes.fFloat3) {
                            proj = false;
                        } else {
                            SkASSERT(arg1Type == *fContext.fTypes.fFloat4);
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
                    if (this->caps().generation() < k130_GrGLSLGeneration) {
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
                if (this->caps().generation() < k130_GrGLSLGeneration) {
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
        this->writeExpression(*arg, Precedence::kSequence);
    }
    if (fProgram.fConfig->fSettings.fSharpenTextures && isTextureFunctionWithBias) {
        this->write(", -0.5");
    }
    this->write(")");
}

void GLSLCodeGenerator::writeConstructor(const Constructor& c, Precedence parentPrecedence) {
    if (c.arguments().size() == 1 &&
        (this->getTypeName(c.type()) == this->getTypeName(c.arguments()[0]->type()) ||
        (c.type().isScalar() &&
         c.arguments()[0]->type() == *fContext.fTypes.fFloatLiteral))) {
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
        this->writeExpression(*arg, Precedence::kSequence);
    }
    this->write(")");
}

void GLSLCodeGenerator::writeFragCoord() {
    if (!this->caps().canUseFragCoord()) {
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
    if (!fProgram.fConfig->fSettings.fFlipY) {
        this->write("gl_FragCoord");
    } else if (const char* extension = this->caps().fragCoordConventionsExtensionString()) {
        if (!fSetupFragPositionGlobal) {
            if (this->caps().generation() < k150_GrGLSLGeneration) {
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
            if (this->caps().mustDeclareFragmentShaderOutput()) {
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
            this->write(fProgram.fConfig->fSettings.fFlipY ? "(!gl_FrontFacing)" : "gl_FrontFacing");
            break;
        case SK_SAMPLEMASK_BUILTIN:
            SkASSERT(this->caps().sampleMaskSupport());
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
            this->write(this->caps().fbFetchColorName());
            break;
        default:
            this->write(ref.variable()->name());
    }
}

void GLSLCodeGenerator::writeIndexExpression(const IndexExpression& expr) {
    this->writeExpression(*expr.base(), Precedence::kPostfix);
    this->write("[");
    this->writeExpression(*expr.index(), Precedence::kTopLevel);
    this->write("]");
}

bool is_sk_position(const FieldAccess& f) {
    return "sk_Position" == f.base()->type().fields()[f.fieldIndex()].fName;
}

void GLSLCodeGenerator::writeFieldAccess(const FieldAccess& f) {
    if (f.ownerKind() == FieldAccess::OwnerKind::kDefault) {
        this->writeExpression(*f.base(), Precedence::kPostfix);
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
    this->writeExpression(*swizzle.base(), Precedence::kPostfix);
    this->write(".");
    for (int c : swizzle.components()) {
        SkASSERT(c >= 0 && c <= 3);
        this->write(&("x\0y\0z\0w\0"[c * 2]));
    }
}

void GLSLCodeGenerator::writeBinaryExpression(const BinaryExpression& b,
                                              Precedence parentPrecedence) {
    const Expression& left = *b.left();
    const Expression& right = *b.right();
    Operator op = b.getOperator();
    if (this->caps().unfoldShortCircuitAsTernary() &&
            (op.kind() == Token::Kind::TK_LOGICALAND || op.kind() == Token::Kind::TK_LOGICALOR)) {
        this->writeShortCircuitWorkaroundExpression(b, parentPrecedence);
        return;
    }

    Precedence precedence = op.getBinaryPrecedence();
    if (precedence >= parentPrecedence) {
        this->write("(");
    }
    bool positionWorkaround = fProgram.fConfig->fKind == ProgramKind::kVertex &&
                              op.isAssignment() &&
                              left.is<FieldAccess>() &&
                              is_sk_position(left.as<FieldAccess>()) &&
                              !right.containsRTAdjust() &&
                              !this->caps().canUseFragCoord();
    if (positionWorkaround) {
        this->write("sk_FragCoord_Workaround = (");
    }
    this->writeExpression(left, precedence);
    this->write(" ");
    this->write(op.operatorName());
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
    if (Precedence::kTernary >= parentPrecedence) {
        this->write("(");
    }

    // Transform:
    // a && b  =>   a ? b : false
    // a || b  =>   a ? true : b
    this->writeExpression(*b.left(), Precedence::kTernary);
    this->write(" ? ");
    if (b.getOperator().kind() == Token::Kind::TK_LOGICALAND) {
        this->writeExpression(*b.right(), Precedence::kTernary);
    } else {
        BoolLiteral boolTrue(/*offset=*/-1, /*value=*/true, fContext.fTypes.fBool.get());
        this->writeBoolLiteral(boolTrue);
    }
    this->write(" : ");
    if (b.getOperator().kind() == Token::Kind::TK_LOGICALAND) {
        BoolLiteral boolFalse(/*offset=*/-1, /*value=*/false, fContext.fTypes.fBool.get());
        this->writeBoolLiteral(boolFalse);
    } else {
        this->writeExpression(*b.right(), Precedence::kTernary);
    }
    if (Precedence::kTernary >= parentPrecedence) {
        this->write(")");
    }
}

void GLSLCodeGenerator::writeTernaryExpression(const TernaryExpression& t,
                                               Precedence parentPrecedence) {
    if (Precedence::kTernary >= parentPrecedence) {
        this->write("(");
    }
    this->writeExpression(*t.test(), Precedence::kTernary);
    this->write(" ? ");
    this->writeExpression(*t.ifTrue(), Precedence::kTernary);
    this->write(" : ");
    this->writeExpression(*t.ifFalse(), Precedence::kTernary);
    if (Precedence::kTernary >= parentPrecedence) {
        this->write(")");
    }
}

void GLSLCodeGenerator::writePrefixExpression(const PrefixExpression& p,
                                              Precedence parentPrecedence) {
    if (Precedence::kPrefix >= parentPrecedence) {
        this->write("(");
    }
    this->write(p.getOperator().operatorName());
    this->writeExpression(*p.operand(), Precedence::kPrefix);
    if (Precedence::kPrefix >= parentPrecedence) {
        this->write(")");
    }
}

void GLSLCodeGenerator::writePostfixExpression(const PostfixExpression& p,
                                               Precedence parentPrecedence) {
    if (Precedence::kPostfix >= parentPrecedence) {
        this->write("(");
    }
    this->writeExpression(*p.operand(), Precedence::kPostfix);
    this->write(p.getOperator().operatorName());
    if (Precedence::kPostfix >= parentPrecedence) {
        this->write(")");
    }
}

void GLSLCodeGenerator::writeBoolLiteral(const BoolLiteral& b) {
    this->write(b.value() ? "true" : "false");
}

void GLSLCodeGenerator::writeIntLiteral(const IntLiteral& i) {
    const Type& type = i.type();
    if (type == *fContext.fTypes.fUInt) {
        this->write(to_string(i.value() & 0xffffffff) + "u");
    } else if (type == *fContext.fTypes.fUShort) {
        this->write(to_string(i.value() & 0xffff) + "u");
    } else if (type == *fContext.fTypes.fUByte) {
        this->write(to_string(i.value() & 0xff) + "u");
    } else {
        this->write(to_string(i.value()));
    }
}

void GLSLCodeGenerator::writeFloatLiteral(const FloatLiteral& f) {
    this->write(to_string(f.value()));
}

void GLSLCodeGenerator::writeSetting(const Setting& s) {
    SK_ABORT("internal error; setting was not folded to a constant during compilation\n");
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
            this->finishLine();
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
    if ((modifiers.fFlags & Modifiers::kIn_Flag) &&
        (modifiers.fFlags & Modifiers::kOut_Flag)) {
        this->write("inout ");
    } else if (modifiers.fFlags & Modifiers::kIn_Flag) {
        if (globalContext &&
            this->caps().generation() < GrGLSLGeneration::k130_GrGLSLGeneration) {
            this->write(fProgram.fConfig->fKind == ProgramKind::kVertex ? "attribute "
                                                                        : "varying ");
        } else {
            this->write("in ");
        }
    } else if (modifiers.fFlags & Modifiers::kOut_Flag) {
        if (globalContext &&
            this->caps().generation() < GrGLSLGeneration::k130_GrGLSLGeneration) {
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
    this->writeExpression(value, Precedence::kTopLevel);
}

const char* GLSLCodeGenerator::getTypePrecision(const Type& type) {
    if (usesPrecisionModifiers()) {
        switch (type.typeKind()) {
            case Type::TypeKind::kScalar:
                if (type == *fContext.fTypes.fShort || type == *fContext.fTypes.fUShort ||
                    type == *fContext.fTypes.fByte || type == *fContext.fTypes.fUByte) {
                    if (fProgram.fConfig->fSettings.fForceHighPrecision ||
                            this->caps().incompleteShortIntPrecision()) {
                        return "highp ";
                    }
                    return "mediump ";
                }
                if (type == *fContext.fTypes.fHalf) {
                    return fProgram.fConfig->fSettings.fForceHighPrecision ? "highp " : "mediump ";
                }
                if (type == *fContext.fTypes.fFloat || type == *fContext.fTypes.fInt ||
                        type == *fContext.fTypes.fUInt) {
                    return "highp ";
                }
                return "";
            case Type::TypeKind::kVector: // fall through
            case Type::TypeKind::kMatrix:
            case Type::TypeKind::kArray:
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
    if (!fFoundExternalSamplerDecl && var.var().type() == *fContext.fTypes.fSamplerExternalOES) {
        if (this->caps().externalTextureExtensionString()) {
            this->writeExtension(this->caps().externalTextureExtensionString());
        }
        if (this->caps().secondExternalTextureExtensionString()) {
            this->writeExtension(this->caps().secondExternalTextureExtensionString());
        }
        fFoundExternalSamplerDecl = true;
    }
    if (!fFoundRectSamplerDecl && var.var().type() == *fContext.fTypes.fSampler2DRect) {
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
            this->writeExpression(*s.as<ExpressionStatement>().expression(), Precedence::kTopLevel);
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
            SkDEBUGFAILF("unsupported statement: %s", s.description().c_str());
            break;
    }
}

void GLSLCodeGenerator::writeBlock(const Block& b) {
    // Write scope markers if this block is a scope, or if the block is empty (since we need to emit
    // something here to make the code valid).
    bool isScope = b.isScope() || b.isEmpty();
    if (isScope) {
        this->writeLine("{");
        fIndentation++;
    }
    for (const std::unique_ptr<Statement>& stmt : b.children()) {
        if (!stmt->isEmpty()) {
            this->writeStatement(*stmt);
            this->finishLine();
        }
    }
    if (isScope) {
        fIndentation--;
        this->write("}");
    }
}

void GLSLCodeGenerator::writeIfStatement(const IfStatement& stmt) {
    this->write("if (");
    this->writeExpression(*stmt.test(), Precedence::kTopLevel);
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
        this->writeExpression(*f.test(), Precedence::kTopLevel);
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
        if (this->caps().addAndTrueToLoopCondition()) {
            std::unique_ptr<Expression> and_true(new BinaryExpression(
                    /*offset=*/-1, f.test()->clone(), Token::Kind::TK_LOGICALAND,
                    BoolLiteral::Make(fContext, /*offset=*/-1, /*value=*/true),
                    fContext.fTypes.fBool.get()));
            this->writeExpression(*and_true, Precedence::kTopLevel);
        } else {
            this->writeExpression(*f.test(), Precedence::kTopLevel);
        }
    }
    this->write("; ");
    if (f.next()) {
        this->writeExpression(*f.next(), Precedence::kTopLevel);
    }
    this->write(") ");
    this->writeStatement(*f.statement());
}

void GLSLCodeGenerator::writeDoStatement(const DoStatement& d) {
    if (!this->caps().rewriteDoWhileLoops()) {
        this->write("do ");
        this->writeStatement(*d.statement());
        this->write(" while (");
        this->writeExpression(*d.test(), Precedence::kTopLevel);
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
    this->writeExpression(*d.test(), Precedence::kPrefix);
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
    this->finishLine();
    fIndentation--;
    this->write("}");
}

void GLSLCodeGenerator::writeSwitchStatement(const SwitchStatement& s) {
    this->write("switch (");
    this->writeExpression(*s.value(), Precedence::kTopLevel);
    this->writeLine(") {");
    fIndentation++;
    for (const std::unique_ptr<Statement>& stmt : s.cases()) {
        const SwitchCase& c = stmt->as<SwitchCase>();
        if (c.value()) {
            this->write("case ");
            this->writeExpression(*c.value(), Precedence::kTopLevel);
            this->writeLine(":");
        } else {
            this->writeLine("default:");
        }
        if (!c.statement()->isEmpty()) {
            fIndentation++;
            this->writeStatement(*c.statement());
            this->finishLine();
            fIndentation--;
        }
    }
    fIndentation--;
    this->finishLine();
    this->write("}");
}

void GLSLCodeGenerator::writeReturnStatement(const ReturnStatement& r) {
    this->write("return");
    if (r.expression()) {
        this->write(" ");
        this->writeExpression(*r.expression(), Precedence::kTopLevel);
    }
    this->write(";");
}

void GLSLCodeGenerator::writeHeader() {
    if (this->caps().versionDeclString()) {
        this->write(this->caps().versionDeclString());
        this->finishLine();
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
                this->finishLine();
            } else if (builtin == SK_FRAGCOLOR_BUILTIN &&
                       this->caps().mustDeclareFragmentShaderOutput()) {
                if (fProgram.fConfig->fSettings.fFragColorIsInOut) {
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
                if (this->caps().gsInvocationsExtensionString()) {
                    this->writeExtension(this->caps().gsInvocationsExtensionString());
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
            this->writeStructDefinition(e.as<StructDefinition>());
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
    if (fProgram.fConfig->fKind == ProgramKind::kGeometry &&
        this->caps().geometryShaderExtensionString()) {
        this->writeExtension(this->caps().geometryShaderExtensionString());
    }
    OutputStream* rawOut = fOut;
    StringStream body;
    fOut = &body;
    // Write all the program elements except for functions.
    for (const ProgramElement* e : fProgram.elements()) {
        if (!e->is<FunctionDefinition>()) {
            this->writeProgramElement(*e);
        }
    }
    // Write the functions last.
    // Why don't we write things in their original order? Because the Inliner likes to move function
    // bodies around. After inlining, code can inadvertently move upwards, above ProgramElements
    // that the code relies on.
    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<FunctionDefinition>()) {
            this->writeProgramElement(*e);
        }
    }
    fOut = rawOut;

    write_stringstream(fExtensions, *rawOut);
    this->writeInputVars();
    write_stringstream(fGlobals, *rawOut);

    if (!this->caps().canUseFragCoord()) {
        Layout layout;
        switch (fProgram.fConfig->fKind) {
            case ProgramKind::kVertex: {
                Modifiers modifiers(layout, Modifiers::kOut_Flag);
                this->writeModifiers(modifiers, true);
                if (this->usesPrecisionModifiers()) {
                    this->write("highp ");
                }
                this->write("vec4 sk_FragCoord_Workaround;\n");
                break;
            }
            case ProgramKind::kFragment: {
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
            !this->caps().noDefaultPrecisionForExternalSamplers()) {
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
