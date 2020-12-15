/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLMetalCodeGenerator.h"

#include "src/core/SkScopeExit.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLMemoryLayout.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLExtension.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLModifiersDeclaration.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLStructDefinition.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#include <algorithm>

namespace SkSL {

const char* MetalCodeGenerator::OperatorName(Token::Kind op) {
    switch (op) {
        case Token::Kind::TK_LOGICALXOR:  return "!=";
        default:                          return Compiler::OperatorName(op);
    }
}

class MetalCodeGenerator::GlobalStructVisitor {
public:
    virtual ~GlobalStructVisitor() = default;
    virtual void visitInterfaceBlock(const InterfaceBlock& block, const String& blockName) = 0;
    virtual void visitTexture(const Type& type, const String& name) = 0;
    virtual void visitSampler(const Type& type, const String& name) = 0;
    virtual void visitVariable(const Variable& var, const Expression* value) = 0;
};

void MetalCodeGenerator::setupIntrinsics() {
#define METAL(x) std::make_pair(kMetal_IntrinsicKind, k ## x ## _MetalIntrinsic)
#define SPECIAL(x) std::make_pair(kSpecial_IntrinsicKind, k ## x ## _SpecialIntrinsic)
    fIntrinsicMap[String("floatBitsToInt")]     = SPECIAL(Bitcast);
    fIntrinsicMap[String("floatBitsToUint")]    = SPECIAL(Bitcast);
    fIntrinsicMap[String("intBitsToFloat")]     = SPECIAL(Bitcast);
    fIntrinsicMap[String("uintBitsToFloat")]    = SPECIAL(Bitcast);
    fIntrinsicMap[String("degrees")]            = SPECIAL(Degrees);
    fIntrinsicMap[String("distance")]           = SPECIAL(Distance);
    fIntrinsicMap[String("dot")]                = SPECIAL(Dot);
    fIntrinsicMap[String("faceforward")]        = SPECIAL(Faceforward);
    fIntrinsicMap[String("findLSB")]            = SPECIAL(FindLSB);
    fIntrinsicMap[String("findMSB")]            = SPECIAL(FindMSB);
    fIntrinsicMap[String("length")]             = SPECIAL(Length);
    fIntrinsicMap[String("mod")]                = SPECIAL(Mod);
    fIntrinsicMap[String("normalize")]          = SPECIAL(Normalize);
    fIntrinsicMap[String("radians")]            = SPECIAL(Radians);
    fIntrinsicMap[String("sample")]             = SPECIAL(Texture);
    fIntrinsicMap[String("equal")]              = METAL(Equal);
    fIntrinsicMap[String("notEqual")]           = METAL(NotEqual);
    fIntrinsicMap[String("lessThan")]           = METAL(LessThan);
    fIntrinsicMap[String("lessThanEqual")]      = METAL(LessThanEqual);
    fIntrinsicMap[String("greaterThan")]        = METAL(GreaterThan);
    fIntrinsicMap[String("greaterThanEqual")]   = METAL(GreaterThanEqual);
}

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
    this->writeLine("#extension " + ext.name() + " : enable");
}

String MetalCodeGenerator::typeName(const Type& type) {
    switch (type.typeKind()) {
        case Type::TypeKind::kVector:
            return this->typeName(type.componentType()) + to_string(type.columns());
        case Type::TypeKind::kMatrix:
            return this->typeName(type.componentType()) + to_string(type.columns()) + "x" +
                                  to_string(type.rows());
        case Type::TypeKind::kSampler:
            return "texture2d<float>"; // FIXME - support other texture types;
        case Type::TypeKind::kEnum:
            return "int";
        default:
            if (type == *fContext.fHalf_Type) {
                // FIXME - Currently only supporting floats in MSL to avoid type coercion issues.
                return fContext.fFloat_Type->name();
            } else if (type == *fContext.fByte_Type) {
                return "char";
            } else if (type == *fContext.fUByte_Type) {
                return "uchar";
            } else {
                return type.name();
            }
    }
}

bool MetalCodeGenerator::writeStructDefinition(const Type& type) {
    for (const Type* search : fWrittenStructs) {
        if (*search == type) {
            // already written
            return false;
        }
    }
    fWrittenStructs.push_back(&type);
    this->writeLine("struct " + type.name() + " {");
    fIndentation++;
    this->writeFields(type.fields(), type.fOffset);
    fIndentation--;
    this->write("}");
    return true;
}

// Flags an error if an array type is found. Meant to be used in places where an array type might
// appear in the SkSL/IR, but can't be represented by Metal.
void MetalCodeGenerator::disallowArrayTypes(const Type& type, int offset) {
    if (type.isArray()) {
        fErrors.error(offset, "Metal does not support array types in this context");
    }
}

// Writes the base type, stripping array suffixes. e.g. `float[2]` will output `float`.
// Call `writeArrayDimensions` to write the type's accompanying array sizes.
void MetalCodeGenerator::writeBaseType(const Type& type) {
    switch (type.typeKind()) {
        case Type::TypeKind::kStruct:
            if (!this->writeStructDefinition(type)) {
                this->write(type.name());
            }
            break;
        case Type::TypeKind::kArray:
            this->writeBaseType(type.componentType());
            break;
        default:
            this->write(this->typeName(type));
            break;
    }
}

// Writes the array suffix of a type, if one exists. e.g. `float[2][4]` will output `[2][4]`.
void MetalCodeGenerator::writeArrayDimensions(const Type& type) {
    if (type.isArray()) {
        this->write("[");
        if (type.columns() != Type::kUnsizedArray) {
            this->write(to_string(type.columns()));
        }
        this->write("]");
    }
}

void MetalCodeGenerator::writeExpression(const Expression& expr, Precedence parentPrecedence) {
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

void MetalCodeGenerator::writeIntrinsicCall(const FunctionCall& c) {
    auto i = fIntrinsicMap.find(c.function().name());
    SkASSERT(i != fIntrinsicMap.end());
    Intrinsic intrinsic = i->second;
    int32_t intrinsicId = intrinsic.second;
    switch (intrinsic.first) {
        case kSpecial_IntrinsicKind:
            return this->writeSpecialIntrinsic(c, (SpecialIntrinsic) intrinsicId);
            break;
        case kMetal_IntrinsicKind:
            this->write("(");
            this->writeExpression(*c.arguments()[0], kRelational_Precedence);
            switch ((MetalIntrinsic) intrinsicId) {
                case kEqual_MetalIntrinsic:
                    this->write(" == ");
                    break;
                case kNotEqual_MetalIntrinsic:
                    this->write(" != ");
                    break;
                case kLessThan_MetalIntrinsic:
                    this->write(" < ");
                    break;
                case kLessThanEqual_MetalIntrinsic:
                    this->write(" <= ");
                    break;
                case kGreaterThan_MetalIntrinsic:
                    this->write(" > ");
                    break;
                case kGreaterThanEqual_MetalIntrinsic:
                    this->write(" >= ");
                    break;
                default:
                    ABORT("unsupported Metal intrinsic kind");
            }
            this->writeExpression(*c.arguments()[1], kRelational_Precedence);
            this->write(")");
            break;
        default:
            ABORT("unsupported intrinsic kind");
    }
}

String MetalCodeGenerator::getOutParamHelper(const FunctionCall& call,
                                             const ExpressionArray& arguments,
                                             const SkTArray<VariableReference*>& outVars) {
    AutoOutputStream outputToExtraFunctions(this, &fExtraFunctions, &fIndentation);
    const FunctionDeclaration& function = call.function();

    String name = "_skOutParamHelper" + to_string(fSwizzleHelperCount++) + "_" + function.name();
    const char* separator = "";

    // Emit a prototype for the function we'll be calling through to in our helper.
    if (!function.isBuiltin()) {
        this->writeFunctionDeclaration(function);
        this->writeLine(";");
    }

    // Synthesize a helper function that takes the same inputs as `function`, except in places where
    // `outVars` is non-null; in those places, we take the type of the VariableReference.
    //
    // float _skOutParamHelper0_originalFuncName(float _var0, float _var1, float& outParam) {
    this->writeBaseType(call.type());
    this->write(" ");
    this->write(name);
    this->write("(");
    this->writeFunctionRequirementParams(function, separator);

    SkASSERT(outVars.size() == arguments.size());
    SkASSERT(outVars.size() == function.parameters().size());

    for (int index = 0; index < arguments.count(); ++index) {
        this->write(separator);
        separator = ", ";

        const Variable* param = function.parameters()[index];
        this->writeModifiers(param->modifiers(), /*globalContext=*/false);

        const Type* type = outVars[index] ? &outVars[index]->type() : &arguments[index]->type();
        this->writeBaseType(*type);

        if (param->modifiers().fFlags & Modifiers::kOut_Flag) {
            this->write("&");
        }
        if (outVars[index]) {
            this->write(" ");
            fIgnoreVariableReferenceModifiers = true;
            this->writeVariableReference(*outVars[index]);
            fIgnoreVariableReferenceModifiers = false;
        } else {
            this->write(" _var");
            this->write(to_string(index));
        }
        this->writeArrayDimensions(*type);
    }
    this->writeLine(") {");

    ++fIndentation;
    for (int index = 0; index < outVars.count(); ++index) {
        if (!outVars[index]) {
            continue;
        }
        // float3 _var2[ = outParam.zyx];
        this->writeBaseType(arguments[index]->type());
        this->write(" _var");
        this->write(to_string(index));

        const Variable* param = function.parameters()[index];
        if (param->modifiers().fFlags & Modifiers::kIn_Flag) {
            this->write(" = ");
            fIgnoreVariableReferenceModifiers = true;
            this->writeExpression(*arguments[index], kAssignment_Precedence);
            fIgnoreVariableReferenceModifiers = false;
        }

        this->writeLine(";");
    }

    // [int _skResult = ] myFunction(inputs, outputs, globals, _var0, _var1, _var2, _var3);
    bool hasResult = (call.type().name() != "void");
    if (hasResult) {
        this->writeBaseType(call.type());
        this->write(" _skResult = ");
    }

    this->writeName(function.name());
    this->write("(");
    separator = "";
    this->writeFunctionRequirementArgs(function, separator);

    for (int index = 0; index < arguments.count(); ++index) {
        this->write(separator);
        separator = ", ";

        this->write("_var");
        this->write(to_string(index));
    }
    this->writeLine(");");

    for (int index = 0; index < outVars.count(); ++index) {
        if (!outVars[index]) {
            continue;
        }
        // outParam.zyx = _var2;
        fIgnoreVariableReferenceModifiers = true;
        this->writeExpression(*arguments[index], kAssignment_Precedence);
        fIgnoreVariableReferenceModifiers = false;
        this->write(" = _var");
        this->write(to_string(index));
        this->writeLine(";");
    }

    if (hasResult) {
        this->writeLine("return _skResult;");
    }

    --fIndentation;
    this->writeLine("}");

    return name;
}

String MetalCodeGenerator::getBitcastIntrinsic(const Type& outType) {
    return "as_type<" +  outType.displayName() + ">";
}

void MetalCodeGenerator::writeFunctionCall(const FunctionCall& c) {
    const FunctionDeclaration& function = c.function();
    const ExpressionArray& arguments = c.arguments();
    const auto& entry = fIntrinsicMap.find(function.name());
    if (entry != fIntrinsicMap.end()) {
        this->writeIntrinsicCall(c);
        return;
    }
    String name = function.name();

    if (function.isBuiltin()) {
        if (name == "atan" && arguments.size() == 2) {
            name = "atan2";
        } else if (name == "inversesqrt") {
            name = "rsqrt";
        } else if (name == "inverse") {
            SkASSERT(arguments.size() == 1);
            name = this->getInverseHack(*arguments[0]);
        } else if (name == "dFdx") {
            name = "dfdx";
        } else if (name == "dFdy") {
            // Flipping Y also negates the Y derivatives.
            if (fProgram.fSettings.fFlipY) {
                this->write("-");
            }
            name = "dfdy";
        }
    }

    // We emulate GLSL's out-param semantics for Metal using a helper function. (Specifically,
    // results are only written back to the original variable at the end of the function call; also,
    // swizzles are supported, whereas Metal doesn't allow a swizzle to be passed to a `floatN&`.)
    const std::vector<const Variable*>& parameters = function.parameters();
    SkASSERT(arguments.size() == parameters.size());

    bool foundOutParam = false;
    SkSTArray<16, VariableReference*> outVars;
    outVars.push_back_n(arguments.count(), (VariableReference*)nullptr);

    for (int index = 0; index < arguments.count(); ++index) {
        // If this is an out parameter...
        if (parameters[index]->modifiers().fFlags & Modifiers::kOut_Flag) {
            // Find the expression's inner variable being written to.
            Analysis::AssignmentInfo info;
            // Assignability was verified at IRGeneration time, so this should always succeed.
            SkAssertResult(Analysis::IsAssignable(*arguments[index], &info));
            outVars[index] = info.fAssignedVar;
            foundOutParam = true;
        }
    }

    if (foundOutParam) {
        // Out parameters need to be written back to at the end of the function. To do this, we
        // synthesize a helper function which evaluates the out-param expression into a temporary
        // variable, calls the original function, then writes the temp var back into the out param
        // using the original out-param expression. (This lets us support things like swizzles and
        // array indices.)
        name = getOutParamHelper(c, arguments, outVars);
    }

    this->write(name);
    this->write("(");
    const char* separator = "";
    this->writeFunctionRequirementArgs(function, separator);
    for (int i = 0; i < arguments.count(); ++i) {
        this->write(separator);
        separator = ", ";

        if (outVars[i]) {
            this->writeExpression(*outVars[i], kSequence_Precedence);
        } else {
            this->writeExpression(*arguments[i], kSequence_Precedence);
        }
    }
    this->write(")");
}

String MetalCodeGenerator::getInverseHack(const Expression& mat) {
    const Type& type = mat.type();
    const String& typeName = type.name();
    String name = typeName + "_inverse";
    if (type == *fContext.fFloat2x2_Type || type == *fContext.fHalf2x2_Type) {
        if (fWrittenIntrinsics.find(name) == fWrittenIntrinsics.end()) {
            fWrittenIntrinsics.insert(name);
            fExtraFunctions.writeText((
                typeName + " " + name + "(" + typeName + " m) {"
                "    return float2x2(m[1][1], -m[0][1], -m[1][0], m[0][0]) * (1/determinant(m));"
                "}"
            ).c_str());
        }
    }
    else if (type == *fContext.fFloat3x3_Type || type == *fContext.fHalf3x3_Type) {
        if (fWrittenIntrinsics.find(name) == fWrittenIntrinsics.end()) {
            fWrittenIntrinsics.insert(name);
            fExtraFunctions.writeText((
                typeName + " " +  name + "(" + typeName + " m) {"
                "    float a00 = m[0][0], a01 = m[0][1], a02 = m[0][2];"
                "    float a10 = m[1][0], a11 = m[1][1], a12 = m[1][2];"
                "    float a20 = m[2][0], a21 = m[2][1], a22 = m[2][2];"
                "    float b01 = a22 * a11 - a12 * a21;"
                "    float b11 = -a22 * a10 + a12 * a20;"
                "    float b21 = a21 * a10 - a11 * a20;"
                "    float det = a00 * b01 + a01 * b11 + a02 * b21;"
                "    return " + typeName +
                "                   (b01, (-a22 * a01 + a02 * a21), (a12 * a01 - a02 * a11),"
                "                    b11, (a22 * a00 - a02 * a20), (-a12 * a00 + a02 * a10),"
                "                    b21, (-a21 * a00 + a01 * a20), (a11 * a00 - a01 * a10)) * "
                "                   (1/det);"
                "}"
            ).c_str());
        }
    }
    else if (type == *fContext.fFloat4x4_Type || type == *fContext.fHalf4x4_Type) {
        if (fWrittenIntrinsics.find(name) == fWrittenIntrinsics.end()) {
            fWrittenIntrinsics.insert(name);
            fExtraFunctions.writeText((
                typeName + " " +  name + "(" + typeName + " m) {"
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
                "    return " + typeName + "(a11 * b11 - a12 * b10 + a13 * b09,"
                "                            a02 * b10 - a01 * b11 - a03 * b09,"
                "                            a31 * b05 - a32 * b04 + a33 * b03,"
                "                            a22 * b04 - a21 * b05 - a23 * b03,"
                "                            a12 * b08 - a10 * b11 - a13 * b07,"
                "                            a00 * b11 - a02 * b08 + a03 * b07,"
                "                            a32 * b02 - a30 * b05 - a33 * b01,"
                "                            a20 * b05 - a22 * b02 + a23 * b01,"
                "                            a10 * b10 - a11 * b08 + a13 * b06,"
                "                            a01 * b08 - a00 * b10 - a03 * b06,"
                "                            a30 * b04 - a31 * b02 + a33 * b00,"
                "                            a21 * b02 - a20 * b04 - a23 * b00,"
                "                            a11 * b07 - a10 * b09 - a12 * b06,"
                "                            a00 * b09 - a01 * b07 + a02 * b06,"
                "                            a31 * b01 - a30 * b03 - a32 * b00,"
                "                            a20 * b03 - a21 * b01 + a22 * b00) / det;"
                "}"
            ).c_str());
        }
    }
    return name;
}

String MetalCodeGenerator::getTempVariable(const Type& type) {
    String tempVar = "_skTemp" + to_string(fVarCount++);
    this->fFunctionHeader += "    " + this->typeName(type) + " " + tempVar + ";\n";
    return tempVar;
}

void MetalCodeGenerator::writeSpecialIntrinsic(const FunctionCall & c, SpecialIntrinsic kind) {
    const ExpressionArray& arguments = c.arguments();
    switch (kind) {
        case kTexture_SpecialIntrinsic: {
            this->writeExpression(*arguments[0], kSequence_Precedence);
            this->write(".sample(");
            this->writeExpression(*arguments[0], kSequence_Precedence);
            this->write(SAMPLER_SUFFIX);
            this->write(", ");
            const Type& arg1Type = arguments[1]->type();
            if (arg1Type == *fContext.fFloat3_Type) {
                // have to store the vector in a temp variable to avoid double evaluating it
                String tmpVar = this->getTempVariable(arg1Type);
                this->write("(" + tmpVar + " = ");
                this->writeExpression(*arguments[1], kSequence_Precedence);
                this->write(", " + tmpVar + ".xy / " + tmpVar + ".z))");
            } else {
                SkASSERT(arg1Type == *fContext.fFloat2_Type);
                this->writeExpression(*arguments[1], kSequence_Precedence);
                this->write(")");
            }
            break;
        }
        case kMod_SpecialIntrinsic: {
            // fmod(x, y) in metal calculates x - y * trunc(x / y) instead of x - y * floor(x / y)
            String tmpX = this->getTempVariable(arguments[0]->type());
            String tmpY = this->getTempVariable(arguments[0]->type());
            this->write("(" + tmpX + " = ");
            this->writeExpression(*arguments[0], kSequence_Precedence);
            this->write(", " + tmpY + " = ");
            this->writeExpression(*arguments[1], kSequence_Precedence);
            this->write(", " + tmpX + " - " + tmpY + " * floor(" + tmpX + " / " + tmpY + "))");
            break;
        }
        // GLSL declares scalar versions of most geometric intrinsics, but these don't exist in MSL
        case kDistance_SpecialIntrinsic: {
            if (arguments[0]->type().columns() == 1) {
                this->write("abs(");
                this->writeExpression(*arguments[0], kAdditive_Precedence);
                this->write(" - ");
                this->writeExpression(*arguments[1], kAdditive_Precedence);
                this->write(")");
            } else {
                this->write("distance(");
                this->writeExpression(*arguments[0], kSequence_Precedence);
                this->write(", ");
                this->writeExpression(*arguments[1], kSequence_Precedence);
                this->write(")");
            }
            break;
        }
        case kDot_SpecialIntrinsic: {
            if (arguments[0]->type().columns() == 1) {
                this->write("(");
                this->writeExpression(*arguments[0], kMultiplicative_Precedence);
                this->write(" * ");
                this->writeExpression(*arguments[1], kMultiplicative_Precedence);
                this->write(")");
            } else {
                this->write("dot(");
                this->writeExpression(*arguments[0], kSequence_Precedence);
                this->write(", ");
                this->writeExpression(*arguments[1], kSequence_Precedence);
                this->write(")");
            }
            break;
        }
        case kFaceforward_SpecialIntrinsic: {
            if (arguments[0]->type().columns() == 1) {
                // ((((Nref) * (I) < 0) ? 1 : -1) * (N))
                this->write("((((");
                this->writeExpression(*arguments[2], kSequence_Precedence);
                this->write(") * (");
                this->writeExpression(*arguments[1], kSequence_Precedence);
                this->write(") < 0) ? 1 : -1) * (");
                this->writeExpression(*arguments[0], kSequence_Precedence);
                this->write("))");
            } else {
                this->write("faceforward(");
                this->writeExpression(*arguments[0], kSequence_Precedence);
                this->write(", ");
                this->writeExpression(*arguments[1], kSequence_Precedence);
                this->write(", ");
                this->writeExpression(*arguments[2], kSequence_Precedence);
                this->write(")");
            }
            break;
        }
        case kLength_SpecialIntrinsic: {
            this->write(arguments[0]->type().columns() == 1 ? "abs(" : "length(");
            this->writeExpression(*arguments[0], kSequence_Precedence);
            this->write(")");
            break;
        }
        case kNormalize_SpecialIntrinsic: {
            this->write(arguments[0]->type().columns() == 1 ? "sign(" : "normalize(");
            this->writeExpression(*arguments[0], kSequence_Precedence);
            this->write(")");
            break;
        }
        case kBitcast_SpecialIntrinsic: {
            this->write(this->getBitcastIntrinsic(c.type()));
            this->write("(");
            this->writeExpression(*arguments[0], kSequence_Precedence);
            this->write(")");
            break;
        }
        case kDegrees_SpecialIntrinsic: {
            this->write("((");
            this->writeExpression(*arguments[0], kSequence_Precedence);
            this->write(") * 57.2957795)");
            break;
        }
        case kRadians_SpecialIntrinsic: {
            this->write("((");
            this->writeExpression(*arguments[0], kSequence_Precedence);
            this->write(") * 0.0174532925)");
            break;
        }
        case kFindLSB_SpecialIntrinsic: {
            // Create a temp variable to store the expression, to avoid double-evaluating it.
            String skTemp = this->getTempVariable(arguments[0]->type());
            String exprType = this->typeName(arguments[0]->type());

            // ctz returns numbits(type) on zero inputs; GLSL documents it as generating -1 instead.
            // Use select to detect zero inputs and force a -1 result.

            // (_skTemp1 = (.....), select(ctz(_skTemp1), int4(-1), _skTemp1 == int4(0)))
            this->write("(");
            this->write(skTemp);
            this->write(" = (");
            this->writeExpression(*arguments[0], kSequence_Precedence);
            this->write("), select(ctz(");
            this->write(skTemp);
            this->write("), ");
            this->write(exprType);
            this->write("(-1), ");
            this->write(skTemp);
            this->write(" == ");
            this->write(exprType);
            this->write("(0)))");
            break;
        }
        case kFindMSB_SpecialIntrinsic: {
            // Create a temp variable to store the expression, to avoid double-evaluating it.
            String skTemp1 = this->getTempVariable(arguments[0]->type());
            String exprType = this->typeName(arguments[0]->type());

            // GLSL findMSB is actually quite different from Metal's clz:
            // - For signed negative numbers, it returns the first zero bit, not the first one bit!
            // - For an empty input (0/~0 depending on sign), findMSB gives -1; clz is numbits(type)

            // (_skTemp1 = (.....),
            this->write("(");
            this->write(skTemp1);
            this->write(" = (");
            this->writeExpression(*arguments[0], kSequence_Precedence);
            this->write("), ");

            // Signed input types might be negative; we need another helper variable to negate the
            // input (since we can only find one bits, not zero bits).
            String skTemp2;
            if (arguments[0]->type().isSigned()) {
                // ... _skTemp2 = (select(_skTemp1, ~_skTemp1, _skTemp1 < 0)),
                skTemp2 = this->getTempVariable(arguments[0]->type());
                this->write(skTemp2);
                this->write(" = (select(");
                this->write(skTemp1);
                this->write(", ~");
                this->write(skTemp1);
                this->write(", ");
                this->write(skTemp1);
                this->write(" < 0)), ");
            } else {
                skTemp2 = skTemp1;
            }

            // ... select(int4(clz(_skTemp2)), int4(-1), _skTemp2 == int4(0)))
            this->write("select(");
            this->write(this->typeName(c.type()));
            this->write("(clz(");
            this->write(skTemp2);
            this->write(")), ");
            this->write(this->typeName(c.type()));
            this->write("(-1), ");
            this->write(skTemp2);
            this->write(" == ");
            this->write(exprType);
            this->write("(0)))");
            break;
        }
        default:
            ABORT("unsupported special intrinsic kind");
    }
}

// Assembles a matrix of type floatRxC by resizing another matrix named `x0`.
// Cells that don't exist in the source matrix will be populated with identity-matrix values.
void MetalCodeGenerator::assembleMatrixFromMatrix(const Type& sourceMatrix, int rows, int columns) {
    SkASSERT(rows <= 4);
    SkASSERT(columns <= 4);

    const char* columnSeparator = "";
    for (int c = 0; c < columns; ++c) {
        fExtraFunctions.printf("%sfloat%d(", columnSeparator, rows);
        columnSeparator = "), ";

        // Determine how many values to take from the source matrix for this row.
        int swizzleLength = 0;
        if (c < sourceMatrix.columns()) {
            swizzleLength = std::min<>(rows, sourceMatrix.rows());
        }

        // Emit all the values from the source matrix row.
        bool firstItem;
        switch (swizzleLength) {
            case 0:  firstItem = true;                                            break;
            case 1:  firstItem = false; fExtraFunctions.printf("x0[%d].x", c);    break;
            case 2:  firstItem = false; fExtraFunctions.printf("x0[%d].xy", c);   break;
            case 3:  firstItem = false; fExtraFunctions.printf("x0[%d].xyz", c);  break;
            case 4:  firstItem = false; fExtraFunctions.printf("x0[%d].xyzw", c); break;
            default: SkUNREACHABLE;
        }

        // Emit the placeholder identity-matrix cells.
        for (int r = swizzleLength; r < rows; ++r) {
            fExtraFunctions.printf("%s%s", firstItem ? "" : ", ", (r == c) ? "1.0" : "0.0");
            firstItem = false;
        }
    }

    fExtraFunctions.writeText(")");
}

// Assembles a matrix of type floatRxC by concatenating an arbitrary mix of values, named `x0`,
// `x1`, etc. An error is written if the expression list don't contain exactly R*C scalars.
void MetalCodeGenerator::assembleMatrixFromExpressions(const ExpressionArray& args,
                                                       int rows, int columns) {
    size_t argIndex = 0;
    int argPosition = 0;

    const char* columnSeparator = "";
    for (int c = 0; c < columns; ++c) {
        fExtraFunctions.printf("%sfloat%d(", columnSeparator, rows);
        columnSeparator = "), ";

        const char* rowSeparator = "";
        for (int r = 0; r < rows; ++r) {
            fExtraFunctions.writeText(rowSeparator);
            rowSeparator = ", ";

            if (argIndex < args.size()) {
                const Type& argType = args[argIndex]->type();
                switch (argType.typeKind()) {
                    case Type::TypeKind::kScalar: {
                        fExtraFunctions.printf("x%zu", argIndex);
                        break;
                    }
                    case Type::TypeKind::kVector: {
                        fExtraFunctions.printf("x%zu[%d]", argIndex, argPosition);
                        break;
                    }
                    case Type::TypeKind::kMatrix: {
                        fExtraFunctions.printf("x%zu[%d][%d]", argIndex,
                                               argPosition / argType.rows(),
                                               argPosition % argType.rows());
                        break;
                    }
                    default: {
                        SkDEBUGFAIL("incorrect type of argument for matrix constructor");
                        fExtraFunctions.writeText("<error>");
                        break;
                    }
                }

                ++argPosition;
                if (argPosition >= argType.columns() * argType.rows()) {
                    ++argIndex;
                    argPosition = 0;
                }
            } else {
                SkDEBUGFAIL("not enough arguments for matrix constructor");
                fExtraFunctions.writeText("<error>");
            }
        }
    }

    if (argPosition != 0 || argIndex != args.size()) {
        SkDEBUGFAIL("incorrect number of arguments for matrix constructor");
        fExtraFunctions.writeText(", <error>");
    }

    fExtraFunctions.writeText(")");
}

// Generates a constructor for 'matrix' which reorganizes the input arguments into the proper shape.
// Keeps track of previously generated constructors so that we won't generate more than one
// constructor for any given permutation of input argument types. Returns the name of the
// generated constructor method.
String MetalCodeGenerator::getMatrixConstructHelper(const Constructor& c) {
    const Type& matrix = c.type();
    int columns = matrix.columns();
    int rows = matrix.rows();
    const ExpressionArray& args = c.arguments();

    // Create the helper-method name and use it as our lookup key.
    String name;
    name.appendf("float%dx%d_from", columns, rows);
    for (const std::unique_ptr<Expression>& expr : args) {
        name.appendf("_%s", expr->type().displayName().c_str());
    }

    // If a helper-method has already been synthesized, we don't need to synthesize it again.
    auto [iter, newlyCreated] = fHelpers.insert(name);
    if (!newlyCreated) {
        return name;
    }

    // Unlike GLSL, Metal requires that matrices are initialized with exactly R vectors of C
    // components apiece. (In Metal 2.0, you can also supply R*C scalars, but you still cannot
    // supply a mixture of scalars and vectors.)
    fExtraFunctions.printf("float%dx%d %s(", columns, rows, name.c_str());

    size_t argIndex = 0;
    const char* argSeparator = "";
    for (const std::unique_ptr<Expression>& expr : args) {
        fExtraFunctions.printf("%s%s x%zu", argSeparator,
                               expr->type().displayName().c_str(), argIndex++);
        argSeparator = ", ";
    }

    fExtraFunctions.printf(") {\n    return float%dx%d(", columns, rows);

    if (args.size() == 1 && args.front()->type().isMatrix()) {
        this->assembleMatrixFromMatrix(args.front()->type(), rows, columns);
    } else {
        this->assembleMatrixFromExpressions(args, rows, columns);
    }

    fExtraFunctions.writeText(");\n}\n");
    return name;
}

bool MetalCodeGenerator::canCoerce(const Type& t1, const Type& t2) {
    if (t1.columns() != t2.columns() || t1.rows() != t2.rows()) {
        return false;
    }
    if (t1.columns() > 1) {
        return this->canCoerce(t1.componentType(), t2.componentType());
    }
    return t1.isFloat() && t2.isFloat();
}

bool MetalCodeGenerator::matrixConstructHelperIsNeeded(const Constructor& c) {
    // A matrix construct helper is only necessary if we are, in fact, constructing a matrix.
    if (!c.type().isMatrix()) {
        return false;
    }

    // GLSL is fairly free-form about inputs to its matrix constructors, but Metal is not; it
    // expects exactly R vectors of C components apiece. (Metal 2.0 also allows a list of R*C
    // scalars.) Some cases are simple to translate and so we handle those inline--e.g. a list of
    // scalars can be constructed trivially. In more complex cases, we generate a helper function
    // that converts our inputs into a properly-shaped matrix.
    // A matrix construct helper method is always used if any input argument is a matrix.
    // Helper methods are also necessary when any argument would span multiple rows. For instance:
    //
    // float2 x = (1, 2);
    // float3x2(x, 3, 4, 5, 6) = | 1 3 5 | = no helper needed; conversion can be done inline
    //                           | 2 4 6 |
    //
    // float2 x = (2, 3);
    // float3x2(1, x, 4, 5, 6) = | 1 3 5 | = x spans multiple rows; a helper method will be used
    //                           | 2 4 6 |
    //
    // float4 x = (1, 2, 3, 4);
    // float2x2(x) = | 1 3 | = x spans multiple rows; a helper method will be used
    //               | 2 4 |
    //

    int position = 0;
    for (const std::unique_ptr<Expression>& expr : c.arguments()) {
        // If an input argument is a matrix, we need a helper function.
        if (expr->type().isMatrix()) {
            return true;
        }
        position += expr->type().columns();
        if (position > c.type().rows()) {
            // An input argument would span multiple rows; a helper function is required.
            return true;
        }
        if (position == c.type().rows()) {
            // We've advanced to the end of a row. Wrap to the start of the next row.
            position = 0;
        }
    }

    return false;
}

void MetalCodeGenerator::writeConstructor(const Constructor& c, Precedence parentPrecedence) {
    const Type& constructorType = c.type();
    // Handle special cases for single-argument constructors.
    if (c.arguments().size() == 1) {
        // If the type is coercible, emit it directly.
        const Expression& arg = *c.arguments().front();
        const Type& argType = arg.type();
        if (this->canCoerce(constructorType, argType)) {
            this->writeExpression(arg, parentPrecedence);
            return;
        }

        // Metal supports creating matrices with a scalar on the diagonal via the single-argument
        // matrix constructor.
        if (constructorType.isMatrix() && argType.isNumber()) {
            const Type& matrix = constructorType;
            this->write("float");
            this->write(to_string(matrix.columns()));
            this->write("x");
            this->write(to_string(matrix.rows()));
            this->write("(");
            this->writeExpression(arg, parentPrecedence);
            this->write(")");
            return;
        }
    }

    // Emit and invoke a matrix-constructor helper method if one is necessary.
    if (this->matrixConstructHelperIsNeeded(c)) {
        this->write(this->getMatrixConstructHelper(c));
        this->write("(");
        const char* separator = "";
        for (const std::unique_ptr<Expression>& expr : c.arguments()) {
            this->write(separator);
            separator = ", ";
            this->writeExpression(*expr, kSequence_Precedence);
        }
        this->write(")");
        return;
    }

    // Explicitly invoke the constructor, passing in the necessary arguments.
    this->writeBaseType(constructorType);
    this->disallowArrayTypes(constructorType, c.fOffset);
    this->write("(");
    const char* separator = "";
    int scalarCount = 0;
    for (const std::unique_ptr<Expression>& arg : c.arguments()) {
        const Type& argType = arg->type();
        this->write(separator);
        separator = ", ";
        if (constructorType.isMatrix() &&
            argType.columns() < constructorType.rows()) {
            // Merge scalars and smaller vectors together.
            if (!scalarCount) {
                this->writeBaseType(constructorType.componentType());
                this->write(to_string(constructorType.rows()));
                this->write("(");
            }
            scalarCount += argType.columns();
        }
        this->writeExpression(*arg, kSequence_Precedence);
        if (scalarCount && scalarCount == constructorType.rows()) {
            this->write(")");
            scalarCount = 0;
        }
    }
    this->write(")");
}

void MetalCodeGenerator::writeFragCoord() {
    if (fRTHeightName.length()) {
        this->write("float4(_fragCoord.x, ");
        this->write(fRTHeightName.c_str());
        this->write(" - _fragCoord.y, 0.0, _fragCoord.w)");
    } else {
        this->write("float4(_fragCoord.x, _fragCoord.y, 0.0, _fragCoord.w)");
    }
}

void MetalCodeGenerator::writeVariableReference(const VariableReference& ref) {
    // When assembling out-param helper functions, we copy variables into local clones with matching
    // names. We never want to prepend "_in." or "_globals->" when writing these variables since
    // we're actually targeting the clones.
    if (fIgnoreVariableReferenceModifiers) {
        this->writeName(ref.variable()->name());
        return;
    }

    switch (ref.variable()->modifiers().fLayout.fBuiltin) {
        case SK_FRAGCOLOR_BUILTIN:
            this->write("_out->sk_FragColor");
            break;
        case SK_FRAGCOORD_BUILTIN:
            this->writeFragCoord();
            break;
        case SK_VERTEXID_BUILTIN:
            this->write("sk_VertexID");
            break;
        case SK_INSTANCEID_BUILTIN:
            this->write("sk_InstanceID");
            break;
        case SK_CLOCKWISE_BUILTIN:
            // We'd set the front facing winding in the MTLRenderCommandEncoder to be counter
            // clockwise to match Skia convention.
            this->write(fProgram.fSettings.fFlipY ? "_frontFacing" : "(!_frontFacing)");
            break;
        default:
            const Variable& var = *ref.variable();
            if (var.storage() == Variable::Storage::kGlobal) {
                if (var.modifiers().fFlags & Modifiers::kIn_Flag) {
                    this->write("_in.");
                } else if (var.modifiers().fFlags & Modifiers::kOut_Flag) {
                    this->write("_out->");
                } else if (var.modifiers().fFlags & Modifiers::kUniform_Flag &&
                           var.type().typeKind() != Type::TypeKind::kSampler) {
                    this->write("_uniforms.");
                } else {
                    this->write("_globals->");
                }
            }
            this->writeName(var.name());
    }
}

void MetalCodeGenerator::writeIndexExpression(const IndexExpression& expr) {
    this->writeExpression(*expr.base(), kPostfix_Precedence);
    this->write("[");
    this->writeExpression(*expr.index(), kTopLevel_Precedence);
    this->write("]");
}

void MetalCodeGenerator::writeFieldAccess(const FieldAccess& f) {
    const Type::Field* field = &f.base()->type().fields()[f.fieldIndex()];
    if (FieldAccess::OwnerKind::kDefault == f.ownerKind()) {
        this->writeExpression(*f.base(), kPostfix_Precedence);
        this->write(".");
    }
    switch (field->fModifiers.fLayout.fBuiltin) {
        case SK_POSITION_BUILTIN:
            this->write("_out->sk_Position");
            break;
        default:
            if (field->fName == "sk_PointSize") {
                this->write("_out->sk_PointSize");
            } else {
                if (FieldAccess::OwnerKind::kAnonymousInterfaceBlock == f.ownerKind()) {
                    this->write("_globals->");
                    this->write(fInterfaceBlockNameMap[fInterfaceBlockMap[field]]);
                    this->write("->");
                }
                this->writeName(field->fName);
            }
    }
}

void MetalCodeGenerator::writeSwizzle(const Swizzle& swizzle) {
    this->writeExpression(*swizzle.base(), kPostfix_Precedence);
    this->write(".");
    for (int c : swizzle.components()) {
        SkASSERT(c >= 0 && c <= 3);
        this->write(&("x\0y\0z\0w\0"[c * 2]));
    }
}

MetalCodeGenerator::Precedence MetalCodeGenerator::GetBinaryPrecedence(Token::Kind op) {
    switch (op) {
        case Token::Kind::TK_STAR:         // fall through
        case Token::Kind::TK_SLASH:        // fall through
        case Token::Kind::TK_PERCENT:      return MetalCodeGenerator::kMultiplicative_Precedence;
        case Token::Kind::TK_PLUS:         // fall through
        case Token::Kind::TK_MINUS:        return MetalCodeGenerator::kAdditive_Precedence;
        case Token::Kind::TK_SHL:          // fall through
        case Token::Kind::TK_SHR:          return MetalCodeGenerator::kShift_Precedence;
        case Token::Kind::TK_LT:           // fall through
        case Token::Kind::TK_GT:           // fall through
        case Token::Kind::TK_LTEQ:         // fall through
        case Token::Kind::TK_GTEQ:         return MetalCodeGenerator::kRelational_Precedence;
        case Token::Kind::TK_EQEQ:         // fall through
        case Token::Kind::TK_NEQ:          return MetalCodeGenerator::kEquality_Precedence;
        case Token::Kind::TK_BITWISEAND:   return MetalCodeGenerator::kBitwiseAnd_Precedence;
        case Token::Kind::TK_BITWISEXOR:   return MetalCodeGenerator::kBitwiseXor_Precedence;
        case Token::Kind::TK_BITWISEOR:    return MetalCodeGenerator::kBitwiseOr_Precedence;
        case Token::Kind::TK_LOGICALAND:   return MetalCodeGenerator::kLogicalAnd_Precedence;
        case Token::Kind::TK_LOGICALXOR:   return MetalCodeGenerator::kLogicalXor_Precedence;
        case Token::Kind::TK_LOGICALOR:    return MetalCodeGenerator::kLogicalOr_Precedence;
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
        case Token::Kind::TK_BITWISEOREQ:  return MetalCodeGenerator::kAssignment_Precedence;
        case Token::Kind::TK_COMMA:        return MetalCodeGenerator::kSequence_Precedence;
        default: ABORT("unsupported binary operator");
    }
}

void MetalCodeGenerator::writeMatrixTimesEqualHelper(const Type& left, const Type& right,
                                                     const Type& result) {
    String key = "TimesEqual" + left.name() + right.name();
    if (fHelpers.find(key) == fHelpers.end()) {
        fExtraFunctions.printf("%s operator*=(thread %s& left, thread const %s& right) {\n"
                               "    left = left * right;\n"
                               "    return left;\n"
                               "}", String(result.name()).c_str(), String(left.name()).c_str(),
                                    String(right.name()).c_str());
    }
}

void MetalCodeGenerator::writeBinaryExpression(const BinaryExpression& b,
                                               Precedence parentPrecedence) {
    const Expression& left = *b.left();
    const Expression& right = *b.right();
    const Type& leftType = left.type();
    const Type& rightType = right.type();
    Token::Kind op = b.getOperator();
    Precedence precedence = GetBinaryPrecedence(b.getOperator());
    bool needParens = precedence >= parentPrecedence;
    switch (op) {
        case Token::Kind::TK_EQEQ:
            if (leftType.isVector()) {
                this->write("all");
                needParens = true;
            }
            break;
        case Token::Kind::TK_NEQ:
            if (leftType.isVector()) {
                this->write("any");
                needParens = true;
            }
            break;
        default:
            break;
    }
    if (needParens) {
        this->write("(");
    }
    if (op == Token::Kind::TK_STAREQ && leftType.isMatrix() && rightType.isMatrix()) {
        this->writeMatrixTimesEqualHelper(leftType, rightType, b.type());
    }
    this->writeExpression(left, precedence);
    if (op != Token::Kind::TK_EQ && Compiler::IsAssignment(op) &&
        left.kind() == Expression::Kind::kSwizzle && !left.hasSideEffects()) {
        // This doesn't compile in Metal:
        // float4 x = float4(1);
        // x.xy *= float2x2(...);
        // with the error message "non-const reference cannot bind to vector element",
        // but switching it to x.xy = x.xy * float2x2(...) fixes it. We perform this tranformation
        // as long as the LHS has no side effects, and hope for the best otherwise.
        this->write(" = ");
        this->writeExpression(left, kAssignment_Precedence);
        this->write(" ");
        String opName = OperatorName(op);
        SkASSERT(opName.endsWith("="));
        this->write(opName.substr(0, opName.size() - 1).c_str());
        this->write(" ");
    } else {
        this->write(String(" ") + OperatorName(op) + " ");
    }
    this->writeExpression(right, precedence);
    if (needParens) {
        this->write(")");
    }
}

void MetalCodeGenerator::writeTernaryExpression(const TernaryExpression& t,
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

void MetalCodeGenerator::writePrefixExpression(const PrefixExpression& p,
                                              Precedence parentPrecedence) {
    if (kPrefix_Precedence >= parentPrecedence) {
        this->write("(");
    }
    this->write(OperatorName(p.getOperator()));
    this->writeExpression(*p.operand(), kPrefix_Precedence);
    if (kPrefix_Precedence >= parentPrecedence) {
        this->write(")");
    }
}

void MetalCodeGenerator::writePostfixExpression(const PostfixExpression& p,
                                               Precedence parentPrecedence) {
    if (kPostfix_Precedence >= parentPrecedence) {
        this->write("(");
    }
    this->writeExpression(*p.operand(), kPostfix_Precedence);
    this->write(OperatorName(p.getOperator()));
    if (kPostfix_Precedence >= parentPrecedence) {
        this->write(")");
    }
}

void MetalCodeGenerator::writeBoolLiteral(const BoolLiteral& b) {
    this->write(b.value() ? "true" : "false");
}

void MetalCodeGenerator::writeIntLiteral(const IntLiteral& i) {
    if (i.type() == *fContext.fUInt_Type) {
        this->write(to_string(i.value() & 0xffffffff) + "u");
    } else {
        this->write(to_string((int32_t) i.value()));
    }
}

void MetalCodeGenerator::writeFloatLiteral(const FloatLiteral& f) {
    this->write(to_string(f.value()));
}

void MetalCodeGenerator::writeSetting(const Setting& s) {
    ABORT("internal error; setting was not folded to a constant during compilation\n");
}

void MetalCodeGenerator::writeFunctionRequirementArgs(const FunctionDeclaration& f,
                                                      const char*& separator) {
    Requirements requirements = this->requirements(f);
    if (requirements & kInputs_Requirement) {
        this->write(separator);
        this->write("_in");
        separator = ", ";
    }
    if (requirements & kOutputs_Requirement) {
        this->write(separator);
        this->write("_out");
        separator = ", ";
    }
    if (requirements & kUniforms_Requirement) {
        this->write(separator);
        this->write("_uniforms");
        separator = ", ";
    }
    if (requirements & kGlobals_Requirement) {
        this->write(separator);
        this->write("_globals");
        separator = ", ";
    }
    if (requirements & kFragCoord_Requirement) {
        this->write(separator);
        this->write("_fragCoord");
        separator = ", ";
    }
}

void MetalCodeGenerator::writeFunctionRequirementParams(const FunctionDeclaration& f,
                                                        const char*& separator) {
    Requirements requirements = this->requirements(f);
    if (requirements & kInputs_Requirement) {
        this->write(separator);
        this->write("Inputs _in");
        separator = ", ";
    }
    if (requirements & kOutputs_Requirement) {
        this->write(separator);
        this->write("thread Outputs* _out");
        separator = ", ";
    }
    if (requirements & kUniforms_Requirement) {
        this->write(separator);
        this->write("Uniforms _uniforms");
        separator = ", ";
    }
    if (requirements & kGlobals_Requirement) {
        this->write(separator);
        this->write("thread Globals* _globals");
        separator = ", ";
    }
    if (requirements & kFragCoord_Requirement) {
        this->write(separator);
        this->write("float4 _fragCoord");
        separator = ", ";
    }
}

bool MetalCodeGenerator::writeFunctionDeclaration(const FunctionDeclaration& f) {
    fRTHeightName = fProgram.fInputs.fRTHeight ? "_globals->_anonInterface0->u_skRTHeight" : "";
    const char* separator = "";
    if ("main" == f.name()) {
        switch (fProgram.fKind) {
            case Program::kFragment_Kind:
                this->write("fragment Outputs fragmentMain");
                break;
            case Program::kVertex_Kind:
                this->write("vertex Outputs vertexMain");
                break;
            default:
                fErrors.error(-1, "unsupported kind of program");
                return false;
        }
        this->write("(Inputs _in [[stage_in]]");
        if (-1 != fUniformBuffer) {
            this->write(", constant Uniforms& _uniforms [[buffer(" +
                        to_string(fUniformBuffer) + ")]]");
        }
        for (const ProgramElement* e : fProgram.elements()) {
            if (e->is<GlobalVarDeclaration>()) {
                const GlobalVarDeclaration& decls = e->as<GlobalVarDeclaration>();
                const VarDeclaration& var = decls.declaration()->as<VarDeclaration>();
                if (var.var().type().typeKind() == Type::TypeKind::kSampler) {
                    if (var.var().modifiers().fLayout.fBinding < 0) {
                        fErrors.error(decls.fOffset,
                                        "Metal samplers must have 'layout(binding=...)'");
                        return false;
                    }
                    if (var.var().type().dimensions() != SpvDim2D) {
                        // Not yet implemented--Skia currently only uses 2D textures.
                        fErrors.error(decls.fOffset, "Unsupported texture dimensions");
                        return false;
                    }
                    this->write(", texture2d<float> ");
                    this->writeName(var.var().name());
                    this->write("[[texture(");
                    this->write(to_string(var.var().modifiers().fLayout.fBinding));
                    this->write(")]]");
                    this->write(", sampler ");
                    this->writeName(var.var().name());
                    this->write(SAMPLER_SUFFIX);
                    this->write("[[sampler(");
                    this->write(to_string(var.var().modifiers().fLayout.fBinding));
                    this->write(")]]");
                }
            } else if (e->is<InterfaceBlock>()) {
                const InterfaceBlock& intf = e->as<InterfaceBlock>();
                if (intf.typeName() == "sk_PerVertex") {
                    continue;
                }
                if (intf.variable().modifiers().fLayout.fBinding < 0) {
                    fErrors.error(intf.fOffset,
                                  "Metal interface blocks must have 'layout(binding=...)'");
                    return false;
                }
                this->write(", constant ");
                this->writeBaseType(intf.variable().type());
                this->write("& " );
                this->write(fInterfaceBlockNameMap[&intf]);
                this->write(" [[buffer(");
                this->write(to_string(intf.variable().modifiers().fLayout.fBinding));
                this->write(")]]");
            }
        }
        if (fProgram.fKind == Program::kFragment_Kind) {
            if (fProgram.fInputs.fRTHeight && fInterfaceBlockNameMap.empty()) {
                this->write(", constant sksl_synthetic_uniforms& _anonInterface0 [[buffer(1)]]");
                fRTHeightName = "_anonInterface0.u_skRTHeight";
            }
            this->write(", bool _frontFacing [[front_facing]]");
            this->write(", float4 _fragCoord [[position]]");
        } else if (fProgram.fKind == Program::kVertex_Kind) {
            this->write(", uint sk_VertexID [[vertex_id]], uint sk_InstanceID [[instance_id]]");
        }
        separator = ", ";
    } else {
        this->writeBaseType(f.returnType());
        this->disallowArrayTypes(f.returnType(), f.fOffset);
        this->write(" ");
        this->writeName(f.name());
        this->write("(");
        this->writeFunctionRequirementParams(f, separator);
    }
    for (const auto& param : f.parameters()) {
        this->write(separator);
        separator = ", ";
        this->writeModifiers(param->modifiers(), /*globalContext=*/false);
        const Type* type = &param->type();
        this->writeBaseType(*type);
        if (param->modifiers().fFlags & Modifiers::kOut_Flag) {
            this->write("&");
        }
        this->write(" ");
        this->writeName(param->name());
        this->writeArrayDimensions(*type);
    }
    this->write(")");
    return true;
}

void MetalCodeGenerator::writeFunctionPrototype(const FunctionPrototype& f) {
    this->writeFunctionDeclaration(f.declaration());
    this->writeLine(";");
}

static bool is_block_ending_with_return(const Statement* stmt) {
    // This function detects (potentially nested) blocks that end in a return statement.
    if (!stmt->is<Block>()) {
        return false;
    }
    const StatementArray& block = stmt->as<Block>().children();
    for (int index = block.count(); index--; ) {
        const Statement& stmt = *block[index];
        if (stmt.is<ReturnStatement>()) {
            return true;
        }
        if (stmt.is<Block>()) {
            return is_block_ending_with_return(&stmt);
        }
        if (!stmt.is<Nop>()) {
            break;
        }
    }
    return false;
}

void MetalCodeGenerator::writeFunction(const FunctionDefinition& f) {
    SkASSERT(!fProgram.fSettings.fFragColorIsInOut);

    if (!this->writeFunctionDeclaration(f.declaration())) {
        return;
    }

    fCurrentFunction = &f.declaration();
    SkScopeExit clearCurrentFunction([&] { fCurrentFunction = nullptr; });

    this->writeLine(" {");

    if (f.declaration().name() == "main") {
        this->writeGlobalInit();
        this->writeLine("    Outputs _outputStruct;");
        this->writeLine("    thread Outputs* _out = &_outputStruct;");
    }

    fFunctionHeader = "";
    StringStream buffer;
    {
        AutoOutputStream outputToBuffer(this, &buffer);
        fIndentation++;
        for (const std::unique_ptr<Statement>& stmt : f.body()->as<Block>().children()) {
            if (!stmt->isEmpty()) {
                this->writeStatement(*stmt);
                this->writeLine();
            }
        }
        if (f.declaration().name() == "main") {
            // If the main function doesn't end with a return, we need to synthesize one here.
            if (!is_block_ending_with_return(f.body().get())) {
                this->writeReturnStatementFromMain();
                this->writeLine("");
            }
        }
        fIndentation--;
        this->writeLine("}");
    }
    this->write(fFunctionHeader);
    this->write(buffer.str());
}

void MetalCodeGenerator::writeModifiers(const Modifiers& modifiers,
                                        bool globalContext) {
    if (modifiers.fFlags & Modifiers::kOut_Flag) {
        this->write("thread ");
    }
    if (modifiers.fFlags & Modifiers::kConst_Flag) {
        this->write("constant ");
    }
}

void MetalCodeGenerator::writeInterfaceBlock(const InterfaceBlock& intf) {
    if ("sk_PerVertex" == intf.typeName()) {
        return;
    }
    this->writeModifiers(intf.variable().modifiers(), /*globalContext=*/true);
    this->write("struct ");
    this->writeLine(intf.typeName() + " {");
    const Type* structType = &intf.variable().type();
    if (structType->isArray()) {
        structType = &structType->componentType();
    }
    fWrittenStructs.push_back(structType);
    fIndentation++;
    this->writeFields(structType->fields(), structType->fOffset, &intf);
    if (fProgram.fInputs.fRTHeight) {
        this->writeLine("float u_skRTHeight;");
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
        fInterfaceBlockNameMap[&intf] = intf.instanceName();
    } else {
        fInterfaceBlockNameMap[&intf] = "_anonInterface" +  to_string(fAnonInterfaceCount++);
    }
    this->writeLine(";");
}

void MetalCodeGenerator::writeFields(const std::vector<Type::Field>& fields, int parentOffset,
                                     const InterfaceBlock* parentIntf) {
    MemoryLayout memoryLayout(MemoryLayout::kMetal_Standard);
    int currentOffset = 0;
    for (const auto& field: fields) {
        int fieldOffset = field.fModifiers.fLayout.fOffset;
        const Type* fieldType = field.fType;
        if (!MemoryLayout::LayoutIsSupported(*fieldType)) {
            fErrors.error(parentOffset, "type '" + fieldType->name() + "' is not permitted here");
            return;
        }
        if (fieldOffset != -1) {
            if (currentOffset > fieldOffset) {
                fErrors.error(parentOffset,
                                "offset of field '" + field.fName + "' must be at least " +
                                to_string((int) currentOffset));
                return;
            } else if (currentOffset < fieldOffset) {
                this->write("char pad");
                this->write(to_string(fPaddingCount++));
                this->write("[");
                this->write(to_string(fieldOffset - currentOffset));
                this->writeLine("];");
                currentOffset = fieldOffset;
            }
            int alignment = memoryLayout.alignment(*fieldType);
            if (fieldOffset % alignment) {
                fErrors.error(parentOffset,
                              "offset of field '" + field.fName + "' must be a multiple of " +
                              to_string((int) alignment));
                return;
            }
        }
        size_t fieldSize = memoryLayout.size(*fieldType);
        if (fieldSize > static_cast<size_t>(std::numeric_limits<int>::max() - currentOffset)) {
            fErrors.error(parentOffset, "field offset overflow");
            return;
        }
        currentOffset += fieldSize;
        this->writeModifiers(field.fModifiers, /*globalContext=*/false);
        this->writeBaseType(*fieldType);
        this->write(" ");
        this->writeName(field.fName);
        this->writeArrayDimensions(*fieldType);
        this->writeLine(";");
        if (parentIntf) {
            fInterfaceBlockMap[&field] = parentIntf;
        }
    }
}

void MetalCodeGenerator::writeVarInitializer(const Variable& var, const Expression& value) {
    this->writeExpression(value, kTopLevel_Precedence);
}

void MetalCodeGenerator::writeName(const String& name) {
    if (fReservedWords.find(name) != fReservedWords.end()) {
        this->write("_"); // adding underscore before name to avoid conflict with reserved words
    }
    this->write(name);
}

void MetalCodeGenerator::writeVarDeclaration(const VarDeclaration& var, bool global) {
    if (global && !(var.var().modifiers().fFlags & Modifiers::kConst_Flag)) {
        return;
    }
    this->writeModifiers(var.var().modifiers(), global);
    this->writeBaseType(var.baseType());
    this->disallowArrayTypes(var.baseType(), var.fOffset);
    this->write(" ");
    this->writeName(var.var().name());
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
    this->write(";");
}

void MetalCodeGenerator::writeStatement(const Statement& s) {
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
            this->write("discard_fragment();");
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

void MetalCodeGenerator::writeBlock(const Block& b) {
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

void MetalCodeGenerator::writeIfStatement(const IfStatement& stmt) {
    this->write("if (");
    this->writeExpression(*stmt.test(), kTopLevel_Precedence);
    this->write(") ");
    this->writeStatement(*stmt.ifTrue());
    if (stmt.ifFalse()) {
        this->write(" else ");
        this->writeStatement(*stmt.ifFalse());
    }
}

void MetalCodeGenerator::writeForStatement(const ForStatement& f) {
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
        this->writeExpression(*f.test(), kTopLevel_Precedence);
    }
    this->write("; ");
    if (f.next()) {
        this->writeExpression(*f.next(), kTopLevel_Precedence);
    }
    this->write(") ");
    this->writeStatement(*f.statement());
}

void MetalCodeGenerator::writeDoStatement(const DoStatement& d) {
    this->write("do ");
    this->writeStatement(*d.statement());
    this->write(" while (");
    this->writeExpression(*d.test(), kTopLevel_Precedence);
    this->write(");");
}

void MetalCodeGenerator::writeSwitchStatement(const SwitchStatement& s) {
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

void MetalCodeGenerator::writeReturnStatementFromMain() {
    // main functions in Metal return a magic _out parameter that doesn't exist in SkSL.
    switch (fProgram.fKind) {
        case Program::kFragment_Kind:
            this->write("return *_out;");
            break;
        case Program::kVertex_Kind:
            this->write("return (_out->sk_Position.y = -_out->sk_Position.y, *_out);");
            break;
        default:
            SkDEBUGFAIL("unsupported kind of program");
    }
}

void MetalCodeGenerator::writeReturnStatement(const ReturnStatement& r) {
    if (fCurrentFunction && fCurrentFunction->name() == "main") {
        if (r.expression()) {
            fErrors.error(r.fOffset, "Metal does not support returning values from main()");
        }
        this->writeReturnStatementFromMain();
        return;
    }

    this->write("return");
    if (r.expression()) {
        this->write(" ");
        this->writeExpression(*r.expression(), kTopLevel_Precedence);
    }
    this->write(";");
}

void MetalCodeGenerator::writeHeader() {
    this->write("#include <metal_stdlib>\n");
    this->write("#include <simd/simd.h>\n");
    this->write("using namespace metal;\n");
}

void MetalCodeGenerator::writeUniformStruct() {
    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const GlobalVarDeclaration& decls = e->as<GlobalVarDeclaration>();
            const Variable& var = decls.declaration()->as<VarDeclaration>().var();
            if (var.modifiers().fFlags & Modifiers::kUniform_Flag &&
                var.type().typeKind() != Type::TypeKind::kSampler) {
                if (-1 == fUniformBuffer) {
                    this->write("struct Uniforms {\n");
                    fUniformBuffer = var.modifiers().fLayout.fSet;
                    if (-1 == fUniformBuffer) {
                        fErrors.error(decls.fOffset, "Metal uniforms must have 'layout(set=...)'");
                    }
                } else if (var.modifiers().fLayout.fSet != fUniformBuffer) {
                    if (-1 == fUniformBuffer) {
                        fErrors.error(decls.fOffset, "Metal backend requires all uniforms to have "
                                    "the same 'layout(set=...)'");
                    }
                }
                this->write("    ");
                this->writeBaseType(var.type());
                this->write(" ");
                this->writeName(var.name());
                this->writeArrayDimensions(var.type());
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
    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const GlobalVarDeclaration& decls = e->as<GlobalVarDeclaration>();
            const Variable& var = decls.declaration()->as<VarDeclaration>().var();
            if (var.modifiers().fFlags & Modifiers::kIn_Flag &&
                -1 == var.modifiers().fLayout.fBuiltin) {
                this->write("    ");
                this->writeBaseType(var.type());
                this->write(" ");
                this->writeName(var.name());
                this->writeArrayDimensions(var.type());
                if (-1 != var.modifiers().fLayout.fLocation) {
                    if (fProgram.fKind == Program::kVertex_Kind) {
                        this->write("  [[attribute(" +
                                    to_string(var.modifiers().fLayout.fLocation) + ")]]");
                    } else if (fProgram.fKind == Program::kFragment_Kind) {
                        this->write("  [[user(locn" +
                                    to_string(var.modifiers().fLayout.fLocation) + ")]]");
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
    if (fProgram.fKind == Program::kVertex_Kind) {
        this->write("    float4 sk_Position [[position]];\n");
    } else if (fProgram.fKind == Program::kFragment_Kind) {
        this->write("    float4 sk_FragColor [[color(0)]];\n");
    }
    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const GlobalVarDeclaration& decls = e->as<GlobalVarDeclaration>();
            const Variable& var = decls.declaration()->as<VarDeclaration>().var();
            if (var.modifiers().fFlags & Modifiers::kOut_Flag &&
                -1 == var.modifiers().fLayout.fBuiltin) {
                this->write("    ");
                this->writeBaseType(var.type());
                this->write(" ");
                this->writeName(var.name());
                this->writeArrayDimensions(var.type());

                int location = var.modifiers().fLayout.fLocation;
                if (location < 0) {
                    fErrors.error(var.fOffset,
                                  "Metal out variables must have 'layout(location=...)'");
                } else if (fProgram.fKind == Program::kVertex_Kind) {
                    this->write(" [[user(locn" + to_string(location) + ")]]");
                } else if (fProgram.fKind == Program::kFragment_Kind) {
                    this->write(" [[color(" + to_string(location) + ")");
                    int colorIndex = var.modifiers().fLayout.fIndex;
                    if (colorIndex) {
                        this->write(", index(" + to_string(colorIndex) + ")");
                    }
                    this->write("]]");
                }
                this->write(";\n");
            }
        }
    }
    if (fProgram.fKind == Program::kVertex_Kind) {
        this->write("    float sk_PointSize [[point_size]];\n");
    }
    this->write("};\n");
}

void MetalCodeGenerator::writeInterfaceBlocks() {
    bool wroteInterfaceBlock = false;
    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<InterfaceBlock>()) {
            this->writeInterfaceBlock(e->as<InterfaceBlock>());
            wroteInterfaceBlock = true;
        }
    }
    if (!wroteInterfaceBlock && fProgram.fInputs.fRTHeight) {
        this->writeLine("struct sksl_synthetic_uniforms {");
        this->writeLine("    float u_skRTHeight;");
        this->writeLine("};");
    }
}

void MetalCodeGenerator::writeStructDefinitions() {
    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<StructDefinition>()) {
            if (this->writeStructDefinition(e->as<StructDefinition>().type())) {
                this->writeLine(";");
            }
        } else if (e->is<GlobalVarDeclaration>()) {
            // If a global var declaration introduces a struct type, we need to write that type
            // here, since globals are all embedded in a sub-struct.
            const Type* type = &e->as<GlobalVarDeclaration>().declaration()
                                 ->as<VarDeclaration>().baseType();
            if (type->isStruct()) {
                if (this->writeStructDefinition(*type)) {
                    this->writeLine(";");
                }
            }
        }
    }
}

void MetalCodeGenerator::visitGlobalStruct(GlobalStructVisitor* visitor) {
    // Visit the interface blocks.
    for (const auto& [interfaceType, interfaceName] : fInterfaceBlockNameMap) {
        visitor->visitInterfaceBlock(*interfaceType, interfaceName);
    }
    for (const ProgramElement* element : fProgram.elements()) {
        if (!element->is<GlobalVarDeclaration>()) {
            continue;
        }
        const GlobalVarDeclaration& global = element->as<GlobalVarDeclaration>();
        const VarDeclaration& decl = global.declaration()->as<VarDeclaration>();
        const Variable& var = decl.var();
        if ((!var.modifiers().fFlags && -1 == var.modifiers().fLayout.fBuiltin) ||
            var.type().typeKind() == Type::TypeKind::kSampler) {
            if (var.type().typeKind() == Type::TypeKind::kSampler) {
                // Samplers are represented as a "texture/sampler" duo in the global struct.
                visitor->visitTexture(var.type(), var.name());
                visitor->visitSampler(var.type(), String(var.name()) + SAMPLER_SUFFIX);
            } else {
                // Visit a regular variable.
                visitor->visitVariable(var, decl.value().get());
            }
        }
    }
}

void MetalCodeGenerator::writeGlobalStruct() {
    class : public GlobalStructVisitor {
    public:
        void visitInterfaceBlock(const InterfaceBlock& block, const String& blockName) override {
            this->addElement();
            fCodeGen->write("    constant ");
            fCodeGen->write(block.typeName());
            fCodeGen->write("* ");
            fCodeGen->writeName(blockName);
            fCodeGen->write(";\n");
        }
        void visitTexture(const Type& type, const String& name) override {
            this->addElement();
            fCodeGen->write("    ");
            fCodeGen->writeBaseType(type);
            fCodeGen->write(" ");
            fCodeGen->writeName(name);
            fCodeGen->writeArrayDimensions(type);
            fCodeGen->write(";\n");
        }
        void visitSampler(const Type&, const String& name) override {
            this->addElement();
            fCodeGen->write("    sampler ");
            fCodeGen->writeName(name);
            fCodeGen->write(";\n");
        }
        void visitVariable(const Variable& var, const Expression* value) override {
            this->addElement();
            fCodeGen->write("    ");
            fCodeGen->writeBaseType(var.type());
            fCodeGen->write(" ");
            fCodeGen->writeName(var.name());
            fCodeGen->writeArrayDimensions(var.type());
            fCodeGen->write(";\n");
        }
        void addElement() {
            if (fFirst) {
                fCodeGen->write("struct Globals {\n");
                fFirst = false;
            }
        }
        void finish() {
            if (!fFirst) {
                fCodeGen->writeLine("};");
                fFirst = true;
            }
        }

        MetalCodeGenerator* fCodeGen = nullptr;
        bool fFirst = true;
    } visitor;

    visitor.fCodeGen = this;
    this->visitGlobalStruct(&visitor);
    visitor.finish();
}

void MetalCodeGenerator::writeGlobalInit() {
    class : public GlobalStructVisitor {
    public:
        void visitInterfaceBlock(const InterfaceBlock& blockType,
                                 const String& blockName) override {
            this->addElement();
            fCodeGen->write("&");
            fCodeGen->writeName(blockName);
        }
        void visitTexture(const Type&, const String& name) override {
            this->addElement();
            fCodeGen->writeName(name);
        }
        void visitSampler(const Type&, const String& name) override {
            this->addElement();
            fCodeGen->writeName(name);
        }
        void visitVariable(const Variable& var, const Expression* value) override {
            this->addElement();
            if (value) {
                fCodeGen->writeVarInitializer(var, *value);
            } else {
                fCodeGen->write("{}");
            }
        }
        void addElement() {
            if (fFirst) {
                fCodeGen->write("    Globals globalStruct{");
                fFirst = false;
            } else {
                fCodeGen->write(", ");
            }
        }
        void finish() {
            if (!fFirst) {
                fCodeGen->writeLine("};");
                fCodeGen->writeLine("    thread Globals* _globals = &globalStruct;");
                fCodeGen->writeLine("    (void)_globals;");
            }
        }
        MetalCodeGenerator* fCodeGen = nullptr;
        bool fFirst = true;
    } visitor;

    visitor.fCodeGen = this;
    this->visitGlobalStruct(&visitor);
    visitor.finish();
}

void MetalCodeGenerator::writeProgramElement(const ProgramElement& e) {
    switch (e.kind()) {
        case ProgramElement::Kind::kExtension:
            break;
        case ProgramElement::Kind::kGlobalVar: {
            const GlobalVarDeclaration& global = e.as<GlobalVarDeclaration>();
            const VarDeclaration& decl = global.declaration()->as<VarDeclaration>();
            int builtin = decl.var().modifiers().fLayout.fBuiltin;
            if (-1 == builtin) {
                // normal var
                this->writeVarDeclaration(decl, true);
                this->writeLine();
            } else if (SK_FRAGCOLOR_BUILTIN == builtin) {
                // ignore
            }
            break;
        }
        case ProgramElement::Kind::kInterfaceBlock:
            // handled in writeInterfaceBlocks, do nothing
            break;
        case ProgramElement::Kind::kStructDefinition:
            // Handled in writeStructDefinitions. Do nothing.
            break;
        case ProgramElement::Kind::kFunction:
            this->writeFunction(e.as<FunctionDefinition>());
            break;
        case ProgramElement::Kind::kFunctionPrototype:
            this->writeFunctionPrototype(e.as<FunctionPrototype>());
            break;
        case ProgramElement::Kind::kModifiers:
            this->writeModifiers(e.as<ModifiersDeclaration>().modifiers(),
                                 /*globalContext=*/true);
            this->writeLine(";");
            break;
        case ProgramElement::Kind::kEnum:
            break;
        default:
#ifdef SK_DEBUG
            ABORT("unsupported program element: %s\n", e.description().c_str());
#endif
            break;
    }
}

MetalCodeGenerator::Requirements MetalCodeGenerator::requirements(const Expression* e) {
    if (!e) {
        return kNo_Requirements;
    }
    switch (e->kind()) {
        case Expression::Kind::kFunctionCall: {
            const FunctionCall& f = e->as<FunctionCall>();
            Requirements result = this->requirements(f.function());
            for (const auto& arg : f.arguments()) {
                result |= this->requirements(arg.get());
            }
            return result;
        }
        case Expression::Kind::kConstructor: {
            const Constructor& c = e->as<Constructor>();
            Requirements result = kNo_Requirements;
            for (const auto& arg : c.arguments()) {
                result |= this->requirements(arg.get());
            }
            return result;
        }
        case Expression::Kind::kFieldAccess: {
            const FieldAccess& f = e->as<FieldAccess>();
            if (FieldAccess::OwnerKind::kAnonymousInterfaceBlock == f.ownerKind()) {
                return kGlobals_Requirement;
            }
            return this->requirements(f.base().get());
        }
        case Expression::Kind::kSwizzle:
            return this->requirements(e->as<Swizzle>().base().get());
        case Expression::Kind::kBinary: {
            const BinaryExpression& bin = e->as<BinaryExpression>();
            return this->requirements(bin.left().get()) |
                   this->requirements(bin.right().get());
        }
        case Expression::Kind::kIndex: {
            const IndexExpression& idx = e->as<IndexExpression>();
            return this->requirements(idx.base().get()) | this->requirements(idx.index().get());
        }
        case Expression::Kind::kPrefix:
            return this->requirements(e->as<PrefixExpression>().operand().get());
        case Expression::Kind::kPostfix:
            return this->requirements(e->as<PostfixExpression>().operand().get());
        case Expression::Kind::kTernary: {
            const TernaryExpression& t = e->as<TernaryExpression>();
            return this->requirements(t.test().get()) | this->requirements(t.ifTrue().get()) |
                   this->requirements(t.ifFalse().get());
        }
        case Expression::Kind::kVariableReference: {
            const VariableReference& v = e->as<VariableReference>();
            const Modifiers& modifiers = v.variable()->modifiers();
            Requirements result = kNo_Requirements;
            if (modifiers.fLayout.fBuiltin == SK_FRAGCOORD_BUILTIN) {
                result = kGlobals_Requirement | kFragCoord_Requirement;
            } else if (Variable::Storage::kGlobal == v.variable()->storage()) {
                if (modifiers.fFlags & Modifiers::kIn_Flag) {
                    result = kInputs_Requirement;
                } else if (modifiers.fFlags & Modifiers::kOut_Flag) {
                    result = kOutputs_Requirement;
                } else if (modifiers.fFlags & Modifiers::kUniform_Flag &&
                           v.variable()->type().typeKind() != Type::TypeKind::kSampler) {
                    result = kUniforms_Requirement;
                } else {
                    result = kGlobals_Requirement;
                }
            }
            return result;
        }
        default:
            return kNo_Requirements;
    }
}

MetalCodeGenerator::Requirements MetalCodeGenerator::requirements(const Statement* s) {
    if (!s) {
        return kNo_Requirements;
    }
    switch (s->kind()) {
        case Statement::Kind::kBlock: {
            Requirements result = kNo_Requirements;
            for (const std::unique_ptr<Statement>& child : s->as<Block>().children()) {
                result |= this->requirements(child.get());
            }
            return result;
        }
        case Statement::Kind::kVarDeclaration: {
            const VarDeclaration& var = s->as<VarDeclaration>();
            return this->requirements(var.value().get());
        }
        case Statement::Kind::kExpression:
            return this->requirements(s->as<ExpressionStatement>().expression().get());
        case Statement::Kind::kReturn: {
            const ReturnStatement& r = s->as<ReturnStatement>();
            return this->requirements(r.expression().get());
        }
        case Statement::Kind::kIf: {
            const IfStatement& i = s->as<IfStatement>();
            return this->requirements(i.test().get()) |
                   this->requirements(i.ifTrue().get()) |
                   this->requirements(i.ifFalse().get());
        }
        case Statement::Kind::kFor: {
            const ForStatement& f = s->as<ForStatement>();
            return this->requirements(f.initializer().get()) |
                   this->requirements(f.test().get()) |
                   this->requirements(f.next().get()) |
                   this->requirements(f.statement().get());
        }
        case Statement::Kind::kDo: {
            const DoStatement& d = s->as<DoStatement>();
            return this->requirements(d.test().get()) |
                   this->requirements(d.statement().get());
        }
        case Statement::Kind::kSwitch: {
            const SwitchStatement& sw = s->as<SwitchStatement>();
            Requirements result = this->requirements(sw.value().get());
            for (const std::unique_ptr<SwitchCase>& sc : sw.cases()) {
                for (const auto& st : sc->statements()) {
                    result |= this->requirements(st.get());
                }
            }
            return result;
        }
        default:
            return kNo_Requirements;
    }
}

MetalCodeGenerator::Requirements MetalCodeGenerator::requirements(const FunctionDeclaration& f) {
    if (f.isBuiltin()) {
        return kNo_Requirements;
    }
    auto found = fRequirements.find(&f);
    if (found == fRequirements.end()) {
        fRequirements[&f] = kNo_Requirements;
        for (const ProgramElement* e : fProgram.elements()) {
            if (e->is<FunctionDefinition>()) {
                const FunctionDefinition& def = e->as<FunctionDefinition>();
                if (&def.declaration() == &f) {
                    Requirements reqs = this->requirements(def.body().get());
                    fRequirements[&f] = reqs;
                    return reqs;
                }
            }
        }
        // We never found a definition for this declared function, but it's legal to prototype a
        // function without ever giving a definition, as long as you don't call it.
        return kNo_Requirements;
    }
    return found->second;
}

bool MetalCodeGenerator::generateCode() {
    fProgramKind = fProgram.fKind;

    StringStream header;
    {
        AutoOutputStream outputToHeader(this, &header, &fIndentation);
        this->writeHeader();
        this->writeStructDefinitions();
        this->writeUniformStruct();
        this->writeInputStruct();
        this->writeOutputStruct();
        this->writeInterfaceBlocks();
        this->writeGlobalStruct();
    }
    StringStream body;
    {
        AutoOutputStream outputToBody(this, &body, &fIndentation);
        for (const ProgramElement* e : fProgram.elements()) {
            this->writeProgramElement(*e);
        }
    }
    write_stringstream(header, *fOut);
    write_stringstream(fExtraFunctions, *fOut);
    write_stringstream(body, *fOut);
    return 0 == fErrors.errorCount();
}

}  // namespace SkSL
