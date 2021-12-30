/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/codegen/SkSLMetalCodeGenerator.h"

#include "src/core/SkScopeExit.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLMemoryLayout.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLConstructorArray.h"
#include "src/sksl/ir/SkSLConstructorArrayCast.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLConstructorCompoundCast.h"
#include "src/sksl/ir/SkSLConstructorDiagonalMatrix.h"
#include "src/sksl/ir/SkSLConstructorMatrixResize.h"
#include "src/sksl/ir/SkSLConstructorSplat.h"
#include "src/sksl/ir/SkSLConstructorStruct.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLExtension.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLFunctionPrototype.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLModifiersDeclaration.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSetting.h"
#include "src/sksl/ir/SkSLStructDefinition.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#include <algorithm>

namespace SkSL {

const char* MetalCodeGenerator::OperatorName(Operator op) {
    switch (op.kind()) {
        case Token::Kind::TK_LOGICALXOR:  return "!=";
        default:                          return op.operatorName();
    }
}

class MetalCodeGenerator::GlobalStructVisitor {
public:
    virtual ~GlobalStructVisitor() = default;
    virtual void visitInterfaceBlock(const InterfaceBlock& block, skstd::string_view blockName) = 0;
    virtual void visitTexture(const Type& type, skstd::string_view name) = 0;
    virtual void visitSampler(const Type& type, skstd::string_view name) = 0;
    virtual void visitVariable(const Variable& var, const Expression* value) = 0;
};

void MetalCodeGenerator::write(skstd::string_view s) {
    if (s.empty()) {
        return;
    }
    if (fAtLineStart) {
        for (int i = 0; i < fIndentation; i++) {
            fOut->writeText("    ");
        }
    }
    fOut->writeText(String(s).c_str());
    fAtLineStart = false;
}

void MetalCodeGenerator::writeLine(skstd::string_view s) {
    this->write(s);
    fOut->writeText(fLineEnding);
    fAtLineStart = true;
}

void MetalCodeGenerator::finishLine() {
    if (!fAtLineStart) {
        this->writeLine();
    }
}

void MetalCodeGenerator::writeExtension(const Extension& ext) {
    this->writeLine("#extension " + ext.name() + " : enable");
}

String MetalCodeGenerator::typeName(const Type& type) {
    switch (type.typeKind()) {
        case Type::TypeKind::kArray:
            SkASSERTF(type.columns() > 0, "invalid array size: %s", type.description().c_str());
            return String::printf("array<%s, %d>",
                                  this->typeName(type.componentType()).c_str(), type.columns());

        case Type::TypeKind::kVector:
            return this->typeName(type.componentType()) + to_string(type.columns());

        case Type::TypeKind::kMatrix:
            return this->typeName(type.componentType()) + to_string(type.columns()) + "x" +
                                  to_string(type.rows());

        case Type::TypeKind::kSampler:
            return "texture2d<half>"; // FIXME - support other texture types

        default:
            return String(type.name());
    }
}

void MetalCodeGenerator::writeStructDefinition(const StructDefinition& s) {
    const Type& type = s.type();
    this->writeLine("struct " + type.name() + " {");
    fIndentation++;
    this->writeFields(type.fields(), type.fLine);
    fIndentation--;
    this->writeLine("};");
}

void MetalCodeGenerator::writeType(const Type& type) {
    this->write(this->typeName(type));
}

void MetalCodeGenerator::writeExpression(const Expression& expr, Precedence parentPrecedence) {
    switch (expr.kind()) {
        case Expression::Kind::kBinary:
            this->writeBinaryExpression(expr.as<BinaryExpression>(), parentPrecedence);
            break;
        case Expression::Kind::kConstructorArray:
        case Expression::Kind::kConstructorStruct:
            this->writeAnyConstructor(expr.asAnyConstructor(), "{", "}", parentPrecedence);
            break;
        case Expression::Kind::kConstructorArrayCast:
            this->writeConstructorArrayCast(expr.as<ConstructorArrayCast>(), parentPrecedence);
            break;
        case Expression::Kind::kConstructorCompound:
            this->writeConstructorCompound(expr.as<ConstructorCompound>(), parentPrecedence);
            break;
        case Expression::Kind::kConstructorDiagonalMatrix:
        case Expression::Kind::kConstructorSplat:
            this->writeAnyConstructor(expr.asAnyConstructor(), "(", ")", parentPrecedence);
            break;
        case Expression::Kind::kConstructorMatrixResize:
            this->writeConstructorMatrixResize(expr.as<ConstructorMatrixResize>(),
                                               parentPrecedence);
            break;
        case Expression::Kind::kConstructorScalarCast:
        case Expression::Kind::kConstructorCompoundCast:
            this->writeCastConstructor(expr.asAnyConstructor(), "(", ")", parentPrecedence);
            break;
        case Expression::Kind::kFieldAccess:
            this->writeFieldAccess(expr.as<FieldAccess>());
            break;
        case Expression::Kind::kLiteral:
            this->writeLiteral(expr.as<Literal>());
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

String MetalCodeGenerator::getOutParamHelper(const FunctionCall& call,
                                             const ExpressionArray& arguments,
                                             const SkTArray<VariableReference*>& outVars) {
    AutoOutputStream outputToExtraFunctions(this, &fExtraFunctions, &fIndentation);
    const FunctionDeclaration& function = call.function();

    String name = "_skOutParamHelper" + to_string(fSwizzleHelperCount++) +
                  "_" + function.mangledName();
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
    this->writeType(call.type());
    this->write(" ");
    this->write(name);
    this->write("(");
    this->writeFunctionRequirementParams(function, separator);

    SkASSERT(outVars.size() == arguments.size());
    SkASSERT(outVars.size() == function.parameters().size());

    // We need to detect cases where the caller passes the same variable as an out-param more than
    // once, and avoid reusing the variable name. (In those cases we can actually just ignore the
    // redundant input parameter entirely, and not give it any name.)
    std::unordered_set<const Variable*> writtenVars;

    for (int index = 0; index < arguments.count(); ++index) {
        this->write(separator);
        separator = ", ";

        const Variable* param = function.parameters()[index];
        this->writeModifiers(param->modifiers());

        const Type* type = outVars[index] ? &outVars[index]->type() : &arguments[index]->type();
        this->writeType(*type);

        if (param->modifiers().fFlags & Modifiers::kOut_Flag) {
            this->write("&");
        }
        if (outVars[index]) {
            auto [iter, didInsert] = writtenVars.insert(outVars[index]->variable());
            if (didInsert) {
                this->write(" ");
                fIgnoreVariableReferenceModifiers = true;
                this->writeVariableReference(*outVars[index]);
                fIgnoreVariableReferenceModifiers = false;
            }
        } else {
            this->write(" _var");
            this->write(to_string(index));
        }
    }
    this->writeLine(") {");

    ++fIndentation;
    for (int index = 0; index < outVars.count(); ++index) {
        if (!outVars[index]) {
            continue;
        }
        // float3 _var2[ = outParam.zyx];
        this->writeType(arguments[index]->type());
        this->write(" _var");
        this->write(to_string(index));

        const Variable* param = function.parameters()[index];
        if (param->modifiers().fFlags & Modifiers::kIn_Flag) {
            this->write(" = ");
            fIgnoreVariableReferenceModifiers = true;
            this->writeExpression(*arguments[index], Precedence::kAssignment);
            fIgnoreVariableReferenceModifiers = false;
        }

        this->writeLine(";");
    }

    // [int _skResult = ] myFunction(inputs, outputs, _globals, _var0, _var1, _var2, _var3);
    bool hasResult = (call.type().name() != "void");
    if (hasResult) {
        this->writeType(call.type());
        this->write(" _skResult = ");
    }

    this->writeName(function.mangledName());
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
        this->writeExpression(*arguments[index], Precedence::kAssignment);
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

    // Many intrinsics need to be rewritten in Metal.
    if (function.isIntrinsic()) {
        if (this->writeIntrinsicCall(c, function.intrinsicKind())) {
            return;
        }
    }

    // Determine whether or not we need to emulate GLSL's out-param semantics for Metal using a
    // helper function. (Specifically, out-parameters in GLSL are only written back to the original
    // variable at the end of the function call; also, swizzles are supported, whereas Metal doesn't
    // allow a swizzle to be passed to a `floatN&`.)
    const ExpressionArray& arguments = c.arguments();
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
        this->write(getOutParamHelper(c, arguments, outVars));
    } else {
        this->write(function.mangledName());
    }

    this->write("(");
    const char* separator = "";
    this->writeFunctionRequirementArgs(function, separator);
    for (int i = 0; i < arguments.count(); ++i) {
        this->write(separator);
        separator = ", ";

        if (outVars[i]) {
            this->writeExpression(*outVars[i], Precedence::kSequence);
        } else {
            this->writeExpression(*arguments[i], Precedence::kSequence);
        }
    }
    this->write(")");
}

static constexpr char kInverse2x2[] = R"(
template <typename T>
matrix<T, 2, 2> mat2_inverse(matrix<T, 2, 2> m) {
    return matrix<T, 2, 2>(m[1][1], -m[0][1], -m[1][0], m[0][0]) * (1/determinant(m));
}
)";

static constexpr char kInverse3x3[] = R"(
template <typename T>
matrix<T, 3, 3> mat3_inverse(matrix<T, 3, 3> m) {
    T a00 = m[0][0], a01 = m[0][1], a02 = m[0][2];
    T a10 = m[1][0], a11 = m[1][1], a12 = m[1][2];
    T a20 = m[2][0], a21 = m[2][1], a22 = m[2][2];
    T b01 =  a22*a11 - a12*a21;
    T b11 = -a22*a10 + a12*a20;
    T b21 =  a21*a10 - a11*a20;
    T det = a00*b01 + a01*b11 + a02*b21;
    return matrix<T, 3, 3>(b01, (-a22*a01 + a02*a21), ( a12*a01 - a02*a11),
                           b11, ( a22*a00 - a02*a20), (-a12*a00 + a02*a10),
                           b21, (-a21*a00 + a01*a20), ( a11*a00 - a01*a10)) * (1/det);
}
)";

static constexpr char kInverse4x4[] = R"(
template <typename T>
matrix<T, 4, 4> mat4_inverse(matrix<T, 4, 4> m) {
    T a00 = m[0][0], a01 = m[0][1], a02 = m[0][2], a03 = m[0][3];
    T a10 = m[1][0], a11 = m[1][1], a12 = m[1][2], a13 = m[1][3];
    T a20 = m[2][0], a21 = m[2][1], a22 = m[2][2], a23 = m[2][3];
    T a30 = m[3][0], a31 = m[3][1], a32 = m[3][2], a33 = m[3][3];
    T b00 = a00*a11 - a01*a10;
    T b01 = a00*a12 - a02*a10;
    T b02 = a00*a13 - a03*a10;
    T b03 = a01*a12 - a02*a11;
    T b04 = a01*a13 - a03*a11;
    T b05 = a02*a13 - a03*a12;
    T b06 = a20*a31 - a21*a30;
    T b07 = a20*a32 - a22*a30;
    T b08 = a20*a33 - a23*a30;
    T b09 = a21*a32 - a22*a31;
    T b10 = a21*a33 - a23*a31;
    T b11 = a22*a33 - a23*a32;
    T det = b00*b11 - b01*b10 + b02*b09 + b03*b08 - b04*b07 + b05*b06;
    return matrix<T, 4, 4>(a11*b11 - a12*b10 + a13*b09,
                           a02*b10 - a01*b11 - a03*b09,
                           a31*b05 - a32*b04 + a33*b03,
                           a22*b04 - a21*b05 - a23*b03,
                           a12*b08 - a10*b11 - a13*b07,
                           a00*b11 - a02*b08 + a03*b07,
                           a32*b02 - a30*b05 - a33*b01,
                           a20*b05 - a22*b02 + a23*b01,
                           a10*b10 - a11*b08 + a13*b06,
                           a01*b08 - a00*b10 - a03*b06,
                           a30*b04 - a31*b02 + a33*b00,
                           a21*b02 - a20*b04 - a23*b00,
                           a11*b07 - a10*b09 - a12*b06,
                           a00*b09 - a01*b07 + a02*b06,
                           a31*b01 - a30*b03 - a32*b00,
                           a20*b03 - a21*b01 + a22*b00) * (1/det);
}
)";

String MetalCodeGenerator::getInversePolyfill(const ExpressionArray& arguments) {
    // Only use polyfills for a function taking a single-argument square matrix.
    if (arguments.size() == 1) {
        const Type& type = arguments.front()->type();
        if (type.isMatrix() && type.rows() == type.columns()) {
            // Inject the correct polyfill based on the matrix size.
            auto name = String::printf("mat%d_inverse", type.columns());
            auto [iter, didInsert] = fWrittenIntrinsics.insert(name);
            if (didInsert) {
                switch (type.rows()) {
                    case 2:
                        fExtraFunctions.writeText(kInverse2x2);
                        break;
                    case 3:
                        fExtraFunctions.writeText(kInverse3x3);
                        break;
                    case 4:
                        fExtraFunctions.writeText(kInverse4x4);
                        break;
                }
            }
            return name;
        }
    }
    // This isn't the built-in `inverse`. We don't want to polyfill it at all.
    return "inverse";
}

void MetalCodeGenerator::writeMatrixCompMult() {
    static constexpr char kMatrixCompMult[] = R"(
template <typename T, int C, int R>
matrix<T, C, R> matrixCompMult(matrix<T, C, R> a, const matrix<T, C, R> b) {
    for (int c = 0; c < C; ++c) {
        a[c] *= b[c];
    }
    return a;
}
)";

    String name = "matrixCompMult";
    if (fWrittenIntrinsics.find(name) == fWrittenIntrinsics.end()) {
        fWrittenIntrinsics.insert(name);
        fExtraFunctions.writeText(kMatrixCompMult);
    }
}

void MetalCodeGenerator::writeOuterProduct() {
    static constexpr char kOuterProduct[] = R"(
template <typename T, int C, int R>
matrix<T, C, R> outerProduct(const vec<T, R> a, const vec<T, C> b) {
    matrix<T, C, R> result;
    for (int c = 0; c < C; ++c) {
        result[c] = a * b[c];
    }
    return result;
}
)";

    String name = "outerProduct";
    if (fWrittenIntrinsics.find(name) == fWrittenIntrinsics.end()) {
        fWrittenIntrinsics.insert(name);
        fExtraFunctions.writeText(kOuterProduct);
    }
}

String MetalCodeGenerator::getTempVariable(const Type& type) {
    String tempVar = "_skTemp" + to_string(fVarCount++);
    this->fFunctionHeader += "    " + this->typeName(type) + " " + tempVar + ";\n";
    return tempVar;
}

void MetalCodeGenerator::writeSimpleIntrinsic(const FunctionCall& c) {
    // Write out an intrinsic function call exactly as-is. No muss no fuss.
    this->write(c.function().name());
    this->writeArgumentList(c.arguments());
}

void MetalCodeGenerator::writeArgumentList(const ExpressionArray& arguments) {
    this->write("(");
    const char* separator = "";
    for (const std::unique_ptr<Expression>& arg : arguments) {
        this->write(separator);
        separator = ", ";
        this->writeExpression(*arg, Precedence::kSequence);
    }
    this->write(")");
}

bool MetalCodeGenerator::writeIntrinsicCall(const FunctionCall& c, IntrinsicKind kind) {
    const ExpressionArray& arguments = c.arguments();
    switch (kind) {
        case k_sample_IntrinsicKind: {
            this->writeExpression(*arguments[0], Precedence::kSequence);
            this->write(".sample(");
            this->writeExpression(*arguments[0], Precedence::kSequence);
            this->write(SAMPLER_SUFFIX);
            this->write(", ");
            const Type& arg1Type = arguments[1]->type();
            if (arg1Type.columns() == 3) {
                // have to store the vector in a temp variable to avoid double evaluating it
                String tmpVar = this->getTempVariable(arg1Type);
                this->write("(" + tmpVar + " = ");
                this->writeExpression(*arguments[1], Precedence::kSequence);
                this->write(", " + tmpVar + ".xy / " + tmpVar + ".z))");
            } else {
                SkASSERT(arg1Type.columns() == 2);
                this->writeExpression(*arguments[1], Precedence::kSequence);
                this->write(")");
            }
            return true;
        }
        case k_mod_IntrinsicKind: {
            // fmod(x, y) in metal calculates x - y * trunc(x / y) instead of x - y * floor(x / y)
            String tmpX = this->getTempVariable(arguments[0]->type());
            String tmpY = this->getTempVariable(arguments[1]->type());
            this->write("(" + tmpX + " = ");
            this->writeExpression(*arguments[0], Precedence::kSequence);
            this->write(", " + tmpY + " = ");
            this->writeExpression(*arguments[1], Precedence::kSequence);
            this->write(", " + tmpX + " - " + tmpY + " * floor(" + tmpX + " / " + tmpY + "))");
            return true;
        }
        // GLSL declares scalar versions of most geometric intrinsics, but these don't exist in MSL
        case k_distance_IntrinsicKind: {
            if (arguments[0]->type().columns() == 1) {
                this->write("abs(");
                this->writeExpression(*arguments[0], Precedence::kAdditive);
                this->write(" - ");
                this->writeExpression(*arguments[1], Precedence::kAdditive);
                this->write(")");
            } else {
                this->writeSimpleIntrinsic(c);
            }
            return true;
        }
        case k_dot_IntrinsicKind: {
            if (arguments[0]->type().columns() == 1) {
                this->write("(");
                this->writeExpression(*arguments[0], Precedence::kMultiplicative);
                this->write(" * ");
                this->writeExpression(*arguments[1], Precedence::kMultiplicative);
                this->write(")");
            } else {
                this->writeSimpleIntrinsic(c);
            }
            return true;
        }
        case k_faceforward_IntrinsicKind: {
            if (arguments[0]->type().columns() == 1) {
                // ((((Nref) * (I) < 0) ? 1 : -1) * (N))
                this->write("((((");
                this->writeExpression(*arguments[2], Precedence::kSequence);
                this->write(") * (");
                this->writeExpression(*arguments[1], Precedence::kSequence);
                this->write(") < 0) ? 1 : -1) * (");
                this->writeExpression(*arguments[0], Precedence::kSequence);
                this->write("))");
            } else {
                this->writeSimpleIntrinsic(c);
            }
            return true;
        }
        case k_length_IntrinsicKind: {
            this->write(arguments[0]->type().columns() == 1 ? "abs(" : "length(");
            this->writeExpression(*arguments[0], Precedence::kSequence);
            this->write(")");
            return true;
        }
        case k_normalize_IntrinsicKind: {
            this->write(arguments[0]->type().columns() == 1 ? "sign(" : "normalize(");
            this->writeExpression(*arguments[0], Precedence::kSequence);
            this->write(")");
            return true;
        }
        case k_packUnorm2x16_IntrinsicKind: {
            this->write("pack_float_to_unorm2x16(");
            this->writeExpression(*arguments[0], Precedence::kSequence);
            this->write(")");
            return true;
        }
        case k_unpackUnorm2x16_IntrinsicKind: {
            this->write("unpack_unorm2x16_to_float(");
            this->writeExpression(*arguments[0], Precedence::kSequence);
            this->write(")");
            return true;
        }
        case k_packSnorm2x16_IntrinsicKind: {
            this->write("pack_float_to_snorm2x16(");
            this->writeExpression(*arguments[0], Precedence::kSequence);
            this->write(")");
            return true;
        }
        case k_unpackSnorm2x16_IntrinsicKind: {
            this->write("unpack_snorm2x16_to_float(");
            this->writeExpression(*arguments[0], Precedence::kSequence);
            this->write(")");
            return true;
        }
        case k_packUnorm4x8_IntrinsicKind: {
            this->write("pack_float_to_unorm4x8(");
            this->writeExpression(*arguments[0], Precedence::kSequence);
            this->write(")");
            return true;
        }
        case k_unpackUnorm4x8_IntrinsicKind: {
            this->write("unpack_unorm4x8_to_float(");
            this->writeExpression(*arguments[0], Precedence::kSequence);
            this->write(")");
            return true;
        }
        case k_packSnorm4x8_IntrinsicKind: {
            this->write("pack_float_to_snorm4x8(");
            this->writeExpression(*arguments[0], Precedence::kSequence);
            this->write(")");
            return true;
        }
        case k_unpackSnorm4x8_IntrinsicKind: {
            this->write("unpack_snorm4x8_to_float(");
            this->writeExpression(*arguments[0], Precedence::kSequence);
            this->write(")");
            return true;
        }
        case k_packHalf2x16_IntrinsicKind: {
            this->write("as_type<uint>(half2(");
            this->writeExpression(*arguments[0], Precedence::kSequence);
            this->write("))");
            return true;
        }
        case k_unpackHalf2x16_IntrinsicKind: {
            this->write("float2(as_type<half2>(");
            this->writeExpression(*arguments[0], Precedence::kSequence);
            this->write("))");
            return true;
        }
        case k_floatBitsToInt_IntrinsicKind:
        case k_floatBitsToUint_IntrinsicKind:
        case k_intBitsToFloat_IntrinsicKind:
        case k_uintBitsToFloat_IntrinsicKind: {
            this->write(this->getBitcastIntrinsic(c.type()));
            this->write("(");
            this->writeExpression(*arguments[0], Precedence::kSequence);
            this->write(")");
            return true;
        }
        case k_degrees_IntrinsicKind: {
            this->write("((");
            this->writeExpression(*arguments[0], Precedence::kSequence);
            this->write(") * 57.2957795)");
            return true;
        }
        case k_radians_IntrinsicKind: {
            this->write("((");
            this->writeExpression(*arguments[0], Precedence::kSequence);
            this->write(") * 0.0174532925)");
            return true;
        }
        case k_dFdx_IntrinsicKind: {
            this->write("dfdx");
            this->writeArgumentList(c.arguments());
            return true;
        }
        case k_dFdy_IntrinsicKind: {
            this->write("(" + fRTFlipName + ".y * dfdy");
            this->writeArgumentList(c.arguments());
            this->write(")");
            return true;
        }
        case k_inverse_IntrinsicKind: {
            this->write(this->getInversePolyfill(arguments));
            this->writeArgumentList(c.arguments());
            return true;
        }
        case k_inversesqrt_IntrinsicKind: {
            this->write("rsqrt");
            this->writeArgumentList(c.arguments());
            return true;
        }
        case k_atan_IntrinsicKind: {
            this->write(c.arguments().size() == 2 ? "atan2" : "atan");
            this->writeArgumentList(c.arguments());
            return true;
        }
        case k_reflect_IntrinsicKind: {
            if (arguments[0]->type().columns() == 1) {
                // We need to synthesize `I - 2 * N * I * N`.
                String tmpI = this->getTempVariable(arguments[0]->type());
                String tmpN = this->getTempVariable(arguments[1]->type());

                // (_skTempI = ...
                this->write("(" + tmpI + " = ");
                this->writeExpression(*arguments[0], Precedence::kSequence);

                // , _skTempN = ...
                this->write(", " + tmpN + " = ");
                this->writeExpression(*arguments[1], Precedence::kSequence);

                // , _skTempI - 2 * _skTempN * _skTempI * _skTempN)
                this->write(", " + tmpI + " - 2 * " + tmpN + " * " + tmpI + " * " + tmpN + ")");
            } else {
                this->writeSimpleIntrinsic(c);
            }
            return true;
        }
        case k_refract_IntrinsicKind: {
            if (arguments[0]->type().columns() == 1) {
                // Metal does implement refract for vectors; rather than reimplementing refract from
                // scratch, we can replace the call with `refract(float2(I,0), float2(N,0), eta).x`.
                this->write("(refract(float2(");
                this->writeExpression(*arguments[0], Precedence::kSequence);
                this->write(", 0), float2(");
                this->writeExpression(*arguments[1], Precedence::kSequence);
                this->write(", 0), ");
                this->writeExpression(*arguments[2], Precedence::kSequence);
                this->write(").x)");
            } else {
                this->writeSimpleIntrinsic(c);
            }
            return true;
        }
        case k_roundEven_IntrinsicKind: {
            this->write("rint");
            this->writeArgumentList(c.arguments());
            return true;
        }
        case k_bitCount_IntrinsicKind: {
            this->write("popcount(");
            this->writeExpression(*arguments[0], Precedence::kSequence);
            this->write(")");
            return true;
        }
        case k_findLSB_IntrinsicKind: {
            // Create a temp variable to store the expression, to avoid double-evaluating it.
            String skTemp = this->getTempVariable(arguments[0]->type());
            String exprType = this->typeName(arguments[0]->type());

            // ctz returns numbits(type) on zero inputs; GLSL documents it as generating -1 instead.
            // Use select to detect zero inputs and force a -1 result.

            // (_skTemp1 = (.....), select(ctz(_skTemp1), int4(-1), _skTemp1 == int4(0)))
            this->write("(");
            this->write(skTemp);
            this->write(" = (");
            this->writeExpression(*arguments[0], Precedence::kSequence);
            this->write("), select(ctz(");
            this->write(skTemp);
            this->write("), ");
            this->write(exprType);
            this->write("(-1), ");
            this->write(skTemp);
            this->write(" == ");
            this->write(exprType);
            this->write("(0)))");
            return true;
        }
        case k_findMSB_IntrinsicKind: {
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
            this->writeExpression(*arguments[0], Precedence::kSequence);
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
            return true;
        }
        case k_matrixCompMult_IntrinsicKind: {
            this->writeMatrixCompMult();
            this->writeSimpleIntrinsic(c);
            return true;
        }
        case k_outerProduct_IntrinsicKind: {
            this->writeOuterProduct();
            this->writeSimpleIntrinsic(c);
            return true;
        }
        case k_mix_IntrinsicKind: {
            SkASSERT(c.arguments().size() == 3);
            if (arguments[2]->type().componentType().isBoolean()) {
                // The Boolean forms of GLSL mix() use the select() intrinsic in Metal.
                this->write("select");
                this->writeArgumentList(c.arguments());
                return true;
            }
            // The basic form of mix() is supported by Metal as-is.
            this->writeSimpleIntrinsic(c);
            return true;
        }
        case k_equal_IntrinsicKind:
        case k_greaterThan_IntrinsicKind:
        case k_greaterThanEqual_IntrinsicKind:
        case k_lessThan_IntrinsicKind:
        case k_lessThanEqual_IntrinsicKind:
        case k_notEqual_IntrinsicKind: {
            this->write("(");
            this->writeExpression(*c.arguments()[0], Precedence::kRelational);
            switch (kind) {
                case k_equal_IntrinsicKind:
                    this->write(" == ");
                    break;
                case k_notEqual_IntrinsicKind:
                    this->write(" != ");
                    break;
                case k_lessThan_IntrinsicKind:
                    this->write(" < ");
                    break;
                case k_lessThanEqual_IntrinsicKind:
                    this->write(" <= ");
                    break;
                case k_greaterThan_IntrinsicKind:
                    this->write(" > ");
                    break;
                case k_greaterThanEqual_IntrinsicKind:
                    this->write(" >= ");
                    break;
                default:
                    SK_ABORT("unsupported comparison intrinsic kind");
            }
            this->writeExpression(*c.arguments()[1], Precedence::kRelational);
            this->write(")");
            return true;
        }
        default:
            return false;
    }
}

// Assembles a matrix of type floatRxC by resizing another matrix named `x0`.
// Cells that don't exist in the source matrix will be populated with identity-matrix values.
void MetalCodeGenerator::assembleMatrixFromMatrix(const Type& sourceMatrix, int rows, int columns) {
    SkASSERT(rows <= 4);
    SkASSERT(columns <= 4);

    std::string matrixType = this->typeName(sourceMatrix.componentType());

    const char* separator = "";
    for (int c = 0; c < columns; ++c) {
        fExtraFunctions.printf("%s%s%d(", separator, matrixType.c_str(), rows);
        separator = "), ";

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

// Assembles a matrix of type floatCxR by concatenating an arbitrary mix of values, named `x0`,
// `x1`, etc. An error is written if the expression list don't contain exactly C*R scalars.
void MetalCodeGenerator::assembleMatrixFromExpressions(const AnyConstructor& ctor,
                                                       int columns, int rows) {
    SkASSERT(rows <= 4);
    SkASSERT(columns <= 4);

    std::string matrixType = this->typeName(ctor.type().componentType());
    size_t argIndex = 0;
    int argPosition = 0;
    auto args = ctor.argumentSpan();

    static constexpr char kSwizzle[] = "xyzw";
    const char* separator = "";
    for (int c = 0; c < columns; ++c) {
        fExtraFunctions.printf("%s%s%d(", separator, matrixType.c_str(), rows);
        separator = "), ";

        const char* columnSeparator = "";
        for (int r = 0; r < rows;) {
            fExtraFunctions.writeText(columnSeparator);
            columnSeparator = ", ";

            if (argIndex < args.size()) {
                const Type& argType = args[argIndex]->type();
                switch (argType.typeKind()) {
                    case Type::TypeKind::kScalar: {
                        fExtraFunctions.printf("x%zu", argIndex);
                        ++r;
                        ++argPosition;
                        break;
                    }
                    case Type::TypeKind::kVector: {
                        fExtraFunctions.printf("x%zu.", argIndex);
                        do {
                            fExtraFunctions.write8(kSwizzle[argPosition]);
                            ++r;
                            ++argPosition;
                        } while (r < rows && argPosition < argType.columns());
                        break;
                    }
                    case Type::TypeKind::kMatrix: {
                        fExtraFunctions.printf("x%zu[%d].", argIndex, argPosition / argType.rows());
                        do {
                            fExtraFunctions.write8(kSwizzle[argPosition]);
                            ++r;
                            ++argPosition;
                        } while (r < rows && (argPosition % argType.rows()) != 0);
                        break;
                    }
                    default: {
                        SkDEBUGFAIL("incorrect type of argument for matrix constructor");
                        fExtraFunctions.writeText("<error>");
                        break;
                    }
                }

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
String MetalCodeGenerator::getMatrixConstructHelper(const AnyConstructor& c) {
    const Type& type = c.type();
    int columns = type.columns();
    int rows = type.rows();
    auto args = c.argumentSpan();
    String typeName = this->typeName(type);

    // Create the helper-method name and use it as our lookup key.
    String name;
    name.appendf("%s_from", typeName.c_str());
    for (const std::unique_ptr<Expression>& expr : args) {
        name.appendf("_%s", this->typeName(expr->type()).c_str());
    }

    // If a helper-method has already been synthesized, we don't need to synthesize it again.
    auto [iter, newlyCreated] = fHelpers.insert(name);
    if (!newlyCreated) {
        return name;
    }

    // Unlike GLSL, Metal requires that matrices are initialized with exactly R vectors of C
    // components apiece. (In Metal 2.0, you can also supply R*C scalars, but you still cannot
    // supply a mixture of scalars and vectors.)
    fExtraFunctions.printf("%s %s(", typeName.c_str(), name.c_str());

    size_t argIndex = 0;
    const char* argSeparator = "";
    for (const std::unique_ptr<Expression>& expr : args) {
        fExtraFunctions.printf("%s%s x%zu", argSeparator,
                               this->typeName(expr->type()).c_str(), argIndex++);
        argSeparator = ", ";
    }

    fExtraFunctions.printf(") {\n    return %s(", typeName.c_str());

    if (args.size() == 1 && args.front()->type().isMatrix()) {
        this->assembleMatrixFromMatrix(args.front()->type(), rows, columns);
    } else {
        this->assembleMatrixFromExpressions(c, columns, rows);
    }

    fExtraFunctions.writeText(");\n}\n");
    return name;
}

bool MetalCodeGenerator::matrixConstructHelperIsNeeded(const ConstructorCompound& c) {
    SkASSERT(c.type().isMatrix());

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

void MetalCodeGenerator::writeConstructorMatrixResize(const ConstructorMatrixResize& c,
                                                      Precedence parentPrecedence) {
    // Matrix-resize via casting doesn't natively exist in Metal at all, so we always need to use a
    // matrix-construct helper here.
    this->write(this->getMatrixConstructHelper(c));
    this->write("(");
    this->writeExpression(*c.argument(), Precedence::kSequence);
    this->write(")");
}

void MetalCodeGenerator::writeConstructorCompound(const ConstructorCompound& c,
                                                  Precedence parentPrecedence) {
    if (c.type().isVector()) {
        this->writeConstructorCompoundVector(c, parentPrecedence);
    } else if (c.type().isMatrix()) {
        this->writeConstructorCompoundMatrix(c, parentPrecedence);
    } else {
        fContext.fErrors->error(c.fLine, "unsupported compound constructor");
    }
}

void MetalCodeGenerator::writeConstructorArrayCast(const ConstructorArrayCast& c,
                                                   Precedence parentPrecedence) {
    const Type& inType = c.argument()->type().componentType();
    const Type& outType = c.type().componentType();
    String inTypeName = this->typeName(inType);
    String outTypeName = this->typeName(outType);

    String name = "array_of_" + outTypeName + "_from_" + inTypeName;
    auto [iter, didInsert] = fHelpers.insert(name);
    if (didInsert) {
        fExtraFunctions.printf(R"(
template <size_t N>
array<%s, N> %s(thread const array<%s, N>& x) {
    array<%s, N> result;
    for (int i = 0; i < N; ++i) {
        result[i] = %s(x[i]);
    }
    return result;
}
)",
                               outTypeName.c_str(), name.c_str(), inTypeName.c_str(),
                               outTypeName.c_str(),
                               outTypeName.c_str());
    }

    this->write(name);
    this->write("(");
    this->writeExpression(*c.argument(), Precedence::kSequence);
    this->write(")");
}

String MetalCodeGenerator::getVectorFromMat2x2ConstructorHelper(const Type& matrixType) {
    SkASSERT(matrixType.isMatrix());
    SkASSERT(matrixType.rows() == 2);
    SkASSERT(matrixType.columns() == 2);

    String baseType = this->typeName(matrixType.componentType());
    String name = String::printf("%s4_from_%s2x2", baseType.c_str(), baseType.c_str());
    if (fHelpers.find(name) == fHelpers.end()) {
        fHelpers.insert(name);

        fExtraFunctions.printf(R"(
%s4 %s(%s2x2 x) {
    return %s4(x[0].xy, x[1].xy);
}
)", baseType.c_str(), name.c_str(), baseType.c_str(), baseType.c_str());
    }

    return name;
}

void MetalCodeGenerator::writeConstructorCompoundVector(const ConstructorCompound& c,
                                                        Precedence parentPrecedence) {
    SkASSERT(c.type().isVector());

    // Metal supports constructing vectors from a mix of scalars and vectors, but not matrices.
    // GLSL supports vec4(mat2x2), so we detect that case here and emit a helper function.
    if (c.type().columns() == 4 && c.argumentSpan().size() == 1) {
        const Expression& expr = *c.argumentSpan().front();
        if (expr.type().isMatrix()) {
            this->write(this->getVectorFromMat2x2ConstructorHelper(expr.type()));
            this->write("(");
            this->writeExpression(expr, Precedence::kSequence);
            this->write(")");
            return;
        }
    }

    this->writeAnyConstructor(c, "(", ")", parentPrecedence);
}

void MetalCodeGenerator::writeConstructorCompoundMatrix(const ConstructorCompound& c,
                                                        Precedence parentPrecedence) {
    SkASSERT(c.type().isMatrix());

    // Emit and invoke a matrix-constructor helper method if one is necessary.
    if (this->matrixConstructHelperIsNeeded(c)) {
        this->write(this->getMatrixConstructHelper(c));
        this->write("(");
        const char* separator = "";
        for (const std::unique_ptr<Expression>& expr : c.arguments()) {
            this->write(separator);
            separator = ", ";
            this->writeExpression(*expr, Precedence::kSequence);
        }
        this->write(")");
        return;
    }

    // Metal doesn't allow creating matrices by passing in scalars and vectors in a jumble; it
    // requires your scalars to be grouped up into columns. Because `matrixConstructHelperIsNeeded`
    // returned false, we know that none of our scalars/vectors "wrap" across across a column, so we
    // can group our inputs up and synthesize a constructor for each column.
    const Type& matrixType = c.type();
    const Type& columnType = matrixType.componentType().toCompound(
            fContext, /*columns=*/matrixType.rows(), /*rows=*/1);

    this->writeType(matrixType);
    this->write("(");
    const char* separator = "";
    int scalarCount = 0;
    for (const std::unique_ptr<Expression>& arg : c.arguments()) {
        this->write(separator);
        separator = ", ";
        if (arg->type().columns() < matrixType.rows()) {
            // Write a `floatN(` constructor to group scalars and smaller vectors together.
            if (!scalarCount) {
                this->writeType(columnType);
                this->write("(");
            }
            scalarCount += arg->type().columns();
        }
        this->writeExpression(*arg, Precedence::kSequence);
        if (scalarCount && scalarCount == matrixType.rows()) {
            // Close our `floatN(...` constructor block from above.
            this->write(")");
            scalarCount = 0;
        }
    }
    this->write(")");
}

void MetalCodeGenerator::writeAnyConstructor(const AnyConstructor& c,
                                             const char* leftBracket,
                                             const char* rightBracket,
                                             Precedence parentPrecedence) {
    this->writeType(c.type());
    this->write(leftBracket);
    const char* separator = "";
    for (const std::unique_ptr<Expression>& arg : c.argumentSpan()) {
        this->write(separator);
        separator = ", ";
        this->writeExpression(*arg, Precedence::kSequence);
    }
    this->write(rightBracket);
}

void MetalCodeGenerator::writeCastConstructor(const AnyConstructor& c,
                                              const char* leftBracket,
                                              const char* rightBracket,
                                              Precedence parentPrecedence) {
    return this->writeAnyConstructor(c, leftBracket, rightBracket, parentPrecedence);
}

void MetalCodeGenerator::writeFragCoord() {
    SkASSERT(fRTFlipName.length());
    this->write("float4(_fragCoord.x, ");
    this->write(fRTFlipName.c_str());
    this->write(".x + ");
    this->write(fRTFlipName.c_str());
    this->write(".y * _fragCoord.y, 0.0, _fragCoord.w)");
}

void MetalCodeGenerator::writeVariableReference(const VariableReference& ref) {
    // When assembling out-param helper functions, we copy variables into local clones with matching
    // names. We never want to prepend "_in." or "_globals." when writing these variables since
    // we're actually targeting the clones.
    if (fIgnoreVariableReferenceModifiers) {
        this->writeName(ref.variable()->name());
        return;
    }

    switch (ref.variable()->modifiers().fLayout.fBuiltin) {
        case SK_FRAGCOLOR_BUILTIN:
            this->write("_out.sk_FragColor");
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
            this->write("(" + fRTFlipName + ".y < 0 ? _frontFacing : !_frontFacing)");
            break;
        default:
            const Variable& var = *ref.variable();
            if (var.storage() == Variable::Storage::kGlobal) {
                if (var.modifiers().fFlags & Modifiers::kIn_Flag) {
                    this->write("_in.");
                } else if (var.modifiers().fFlags & Modifiers::kOut_Flag) {
                    this->write("_out.");
                } else if (var.modifiers().fFlags & Modifiers::kUniform_Flag &&
                           var.type().typeKind() != Type::TypeKind::kSampler) {
                    this->write("_uniforms.");
                } else {
                    this->write("_globals.");
                }
            }
            this->writeName(var.name());
    }
}

void MetalCodeGenerator::writeIndexExpression(const IndexExpression& expr) {
    this->writeExpression(*expr.base(), Precedence::kPostfix);
    this->write("[");
    this->writeExpression(*expr.index(), Precedence::kTopLevel);
    this->write("]");
}

void MetalCodeGenerator::writeFieldAccess(const FieldAccess& f) {
    const Type::Field* field = &f.base()->type().fields()[f.fieldIndex()];
    if (FieldAccess::OwnerKind::kDefault == f.ownerKind()) {
        this->writeExpression(*f.base(), Precedence::kPostfix);
        this->write(".");
    }
    switch (field->fModifiers.fLayout.fBuiltin) {
        case SK_POSITION_BUILTIN:
            this->write("_out.sk_Position");
            break;
        default:
            if (field->fName == "sk_PointSize") {
                this->write("_out.sk_PointSize");
            } else {
                if (FieldAccess::OwnerKind::kAnonymousInterfaceBlock == f.ownerKind()) {
                    this->write("_globals.");
                    this->write(fInterfaceBlockNameMap[fInterfaceBlockMap[field]]);
                    this->write("->");
                }
                this->writeName(field->fName);
            }
    }
}

void MetalCodeGenerator::writeSwizzle(const Swizzle& swizzle) {
    this->writeExpression(*swizzle.base(), Precedence::kPostfix);
    this->write(".");
    for (int c : swizzle.components()) {
        SkASSERT(c >= 0 && c <= 3);
        this->write(&("x\0y\0z\0w\0"[c * 2]));
    }
}

void MetalCodeGenerator::writeMatrixTimesEqualHelper(const Type& left, const Type& right,
                                                     const Type& result) {
    SkASSERT(left.isMatrix());
    SkASSERT(right.isMatrix());
    SkASSERT(result.isMatrix());
    SkASSERT(left.rows() == right.rows());
    SkASSERT(left.columns() == right.columns());
    SkASSERT(left.rows() == result.rows());
    SkASSERT(left.columns() == result.columns());

    String key = "Matrix *= " + this->typeName(left) + ":" + this->typeName(right);

    auto [iter, wasInserted] = fHelpers.insert(key);
    if (wasInserted) {
        fExtraFunctions.printf("thread %s& operator*=(thread %s& left, thread const %s& right) {\n"
                               "    left = left * right;\n"
                               "    return left;\n"
                               "}\n",
                               this->typeName(result).c_str(), this->typeName(left).c_str(),
                               this->typeName(right).c_str());
    }
}

void MetalCodeGenerator::writeMatrixEqualityHelpers(const Type& left, const Type& right) {
    SkASSERT(left.isMatrix());
    SkASSERT(right.isMatrix());
    SkASSERT(left.rows() == right.rows());
    SkASSERT(left.columns() == right.columns());

    String key = "Matrix == " + this->typeName(left) + ":" + this->typeName(right);

    auto [iter, wasInserted] = fHelpers.insert(key);
    if (wasInserted) {
        fExtraFunctionPrototypes.printf(R"(
thread bool operator==(const %s left, const %s right);
thread bool operator!=(const %s left, const %s right);
)",
                                        this->typeName(left).c_str(),
                                        this->typeName(right).c_str(),
                                        this->typeName(left).c_str(),
                                        this->typeName(right).c_str());

        fExtraFunctions.printf(
                "thread bool operator==(const %s left, const %s right) {\n"
                "    return ",
                this->typeName(left).c_str(), this->typeName(right).c_str());

        const char* separator = "";
        for (int index=0; index<left.columns(); ++index) {
            fExtraFunctions.printf("%sall(left[%d] == right[%d])", separator, index, index);
            separator = " &&\n           ";
        }

        fExtraFunctions.printf(
                ";\n"
                "}\n"
                "thread bool operator!=(const %s left, const %s right) {\n"
                "    return !(left == right);\n"
                "}\n",
                this->typeName(left).c_str(), this->typeName(right).c_str());
    }
}

void MetalCodeGenerator::writeMatrixDivisionHelpers(const Type& type) {
    SkASSERT(type.isMatrix());

    String key = "Matrix / " + this->typeName(type);

    auto [iter, wasInserted] = fHelpers.insert(key);
    if (wasInserted) {
        String typeName = this->typeName(type);

        fExtraFunctions.printf(
                "thread %s operator/(const %s left, const %s right) {\n"
                "    return %s(",
                typeName.c_str(), typeName.c_str(), typeName.c_str(), typeName.c_str());

        const char* separator = "";
        for (int index=0; index<type.columns(); ++index) {
            fExtraFunctions.printf("%sleft[%d] / right[%d]", separator, index, index);
            separator = ", ";
        }

        fExtraFunctions.printf(");\n"
                               "}\n"
                               "thread %s& operator/=(thread %s& left, thread const %s& right) {\n"
                               "    left = left / right;\n"
                               "    return left;\n"
                               "}\n",
                               typeName.c_str(), typeName.c_str(), typeName.c_str());
    }
}

void MetalCodeGenerator::writeArrayEqualityHelpers(const Type& type) {
    SkASSERT(type.isArray());

    // If the array's component type needs a helper as well, we need to emit that one first.
    this->writeEqualityHelpers(type.componentType(), type.componentType());

    auto [iter, wasInserted] = fHelpers.insert("ArrayEquality []");
    if (wasInserted) {
        fExtraFunctionPrototypes.writeText(R"(
template <typename T1, typename T2, size_t N>
bool operator==(thread const array<T1, N>& left, thread const array<T2, N>& right);
template <typename T1, typename T2, size_t N>
bool operator!=(thread const array<T1, N>& left, thread const array<T2, N>& right);
)");
        fExtraFunctions.writeText(R"(
template <typename T1, typename T2, size_t N>
bool operator==(thread const array<T1, N>& left, thread const array<T2, N>& right) {
    for (size_t index = 0; index < N; ++index) {
        if (!all(left[index] == right[index])) {
            return false;
        }
    }
    return true;
}

template <typename T1, typename T2, size_t N>
bool operator!=(thread const array<T1, N>& left, thread const array<T2, N>& right) {
    return !(left == right);
}
)");
    }
}

void MetalCodeGenerator::writeStructEqualityHelpers(const Type& type) {
    SkASSERT(type.isStruct());
    String key = "StructEquality " + this->typeName(type);

    auto [iter, wasInserted] = fHelpers.insert(key);
    if (wasInserted) {
        // If one of the struct's fields needs a helper as well, we need to emit that one first.
        for (const Type::Field& field : type.fields()) {
            this->writeEqualityHelpers(*field.fType, *field.fType);
        }

        // Write operator== and operator!= for this struct, since those are assumed to exist in SkSL
        // and GLSL but do not exist by default in Metal.
        fExtraFunctionPrototypes.printf(R"(
thread bool operator==(thread const %s& left, thread const %s& right);
thread bool operator!=(thread const %s& left, thread const %s& right);
)",
                                        this->typeName(type).c_str(),
                                        this->typeName(type).c_str(),
                                        this->typeName(type).c_str(),
                                        this->typeName(type).c_str());

        fExtraFunctions.printf(
                "thread bool operator==(thread const %s& left, thread const %s& right) {\n"
                "    return ",
                this->typeName(type).c_str(),
                this->typeName(type).c_str());

        const char* separator = "";
        for (const Type::Field& field : type.fields()) {
            fExtraFunctions.printf("%sall(left.%.*s == right.%.*s)",
                                   separator,
                                   (int)field.fName.size(), field.fName.data(),
                                   (int)field.fName.size(), field.fName.data());
            separator = " &&\n           ";
        }
        fExtraFunctions.printf(
                ";\n"
                "}\n"
                "thread bool operator!=(thread const %s& left, thread const %s& right) {\n"
                "    return !(left == right);\n"
                "}\n",
                this->typeName(type).c_str(),
                this->typeName(type).c_str());
    }
}

void MetalCodeGenerator::writeEqualityHelpers(const Type& leftType, const Type& rightType) {
    if (leftType.isArray() && rightType.isArray()) {
        this->writeArrayEqualityHelpers(leftType);
        return;
    }
    if (leftType.isStruct() && rightType.isStruct()) {
        this->writeStructEqualityHelpers(leftType);
        return;
    }
    if (leftType.isMatrix() && rightType.isMatrix()) {
        this->writeMatrixEqualityHelpers(leftType, rightType);
        return;
    }
}

void MetalCodeGenerator::writeNumberAsMatrix(const Expression& expr, const Type& matrixType) {
    SkASSERT(expr.type().isNumber());
    SkASSERT(matrixType.isMatrix());

    // Componentwise multiply the scalar against a matrix of the desired size which contains all 1s.
    this->write("(");
    this->writeType(matrixType);
    this->write("(");

    const char* separator = "";
    for (int index = matrixType.slotCount(); index--;) {
        this->write(separator);
        this->write("1.0");
        separator = ", ";
    }

    this->write(") * ");
    this->writeExpression(expr, Precedence::kMultiplicative);
    this->write(")");
}

void MetalCodeGenerator::writeBinaryExpression(const BinaryExpression& b,
                                               Precedence parentPrecedence) {
    const Expression& left = *b.left();
    const Expression& right = *b.right();
    const Type& leftType = left.type();
    const Type& rightType = right.type();
    Operator op = b.getOperator();
    Precedence precedence = op.getBinaryPrecedence();
    bool needParens = precedence >= parentPrecedence;
    switch (op.kind()) {
        case Token::Kind::TK_EQEQ:
            this->writeEqualityHelpers(leftType, rightType);
            if (leftType.isVector()) {
                this->write("all");
                needParens = true;
            }
            break;
        case Token::Kind::TK_NEQ:
            this->writeEqualityHelpers(leftType, rightType);
            if (leftType.isVector()) {
                this->write("any");
                needParens = true;
            }
            break;
        default:
            break;
    }
    if (leftType.isMatrix() && rightType.isMatrix() && op.kind() == Token::Kind::TK_STAREQ) {
        this->writeMatrixTimesEqualHelper(leftType, rightType, b.type());
    }
    if (op.removeAssignment().kind() == Token::Kind::TK_SLASH &&
        ((leftType.isMatrix() && rightType.isMatrix()) ||
         (leftType.isScalar() && rightType.isMatrix()) ||
         (leftType.isMatrix() && rightType.isScalar()))) {
        this->writeMatrixDivisionHelpers(leftType.isMatrix() ? leftType : rightType);
    }
    if (needParens) {
        this->write("(");
    }
    bool needMatrixSplatOnScalar = rightType.isMatrix() && leftType.isNumber() &&
                                   op.isValidForMatrixOrVector() &&
                                   op.removeAssignment().kind() != Token::Kind::TK_STAR;
    if (needMatrixSplatOnScalar) {
        this->writeNumberAsMatrix(left, rightType);
    } else {
        this->writeExpression(left, precedence);
    }
    if (op.kind() != Token::Kind::TK_EQ && op.isAssignment() &&
        left.kind() == Expression::Kind::kSwizzle && !left.hasSideEffects()) {
        // This doesn't compile in Metal:
        // float4 x = float4(1);
        // x.xy *= float2x2(...);
        // with the error message "non-const reference cannot bind to vector element",
        // but switching it to x.xy = x.xy * float2x2(...) fixes it. We perform this tranformation
        // as long as the LHS has no side effects, and hope for the best otherwise.
        this->write(" = ");
        this->writeExpression(left, Precedence::kAssignment);
        this->write(" ");
        this->write(OperatorName(op.removeAssignment()));
        this->write(" ");
    } else {
        this->write(String(" ") + OperatorName(op) + " ");
    }

    needMatrixSplatOnScalar = leftType.isMatrix() && rightType.isNumber() &&
                              op.isValidForMatrixOrVector() &&
                              op.removeAssignment().kind() != Token::Kind::TK_STAR;
    if (needMatrixSplatOnScalar) {
        this->writeNumberAsMatrix(right, leftType);
    } else {
        this->writeExpression(right, precedence);
    }
    if (needParens) {
        this->write(")");
    }
}

void MetalCodeGenerator::writeTernaryExpression(const TernaryExpression& t,
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

void MetalCodeGenerator::writePrefixExpression(const PrefixExpression& p,
                                              Precedence parentPrecedence) {
    if (Precedence::kPrefix >= parentPrecedence) {
        this->write("(");
    }
    this->write(OperatorName(p.getOperator()));
    this->writeExpression(*p.operand(), Precedence::kPrefix);
    if (Precedence::kPrefix >= parentPrecedence) {
        this->write(")");
    }
}

void MetalCodeGenerator::writePostfixExpression(const PostfixExpression& p,
                                               Precedence parentPrecedence) {
    if (Precedence::kPostfix >= parentPrecedence) {
        this->write("(");
    }
    this->writeExpression(*p.operand(), Precedence::kPostfix);
    this->write(OperatorName(p.getOperator()));
    if (Precedence::kPostfix >= parentPrecedence) {
        this->write(")");
    }
}

void MetalCodeGenerator::writeLiteral(const Literal& l) {
    const Type& type = l.type();
    if (type.isFloat()) {
        this->write(to_string(l.floatValue()));
        if (!l.type().highPrecision()) {
            this->write("h");
        }
        return;
    }
    if (type.isInteger()) {
        if (type.matches(*fContext.fTypes.fUInt)) {
            this->write(to_string(l.intValue() & 0xffffffff));
            this->write("u");
        } else if (type.matches(*fContext.fTypes.fUShort)) {
            this->write(to_string(l.intValue() & 0xffff));
            this->write("u");
        } else {
            this->write(to_string(l.intValue()));
        }
        return;
    }
    SkASSERT(type.isBoolean());
    this->write(l.boolValue() ? "true" : "false");
}

void MetalCodeGenerator::writeSetting(const Setting& s) {
    SK_ABORT("internal error; setting was not folded to a constant during compilation\n");
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
        this->write("thread Outputs& _out");
        separator = ", ";
    }
    if (requirements & kUniforms_Requirement) {
        this->write(separator);
        this->write("Uniforms _uniforms");
        separator = ", ";
    }
    if (requirements & kGlobals_Requirement) {
        this->write(separator);
        this->write("thread Globals& _globals");
        separator = ", ";
    }
    if (requirements & kFragCoord_Requirement) {
        this->write(separator);
        this->write("float4 _fragCoord");
        separator = ", ";
    }
}

int MetalCodeGenerator::getUniformBinding(const Modifiers& m) {
    return (m.fLayout.fBinding >= 0) ? m.fLayout.fBinding
                                     : fProgram.fConfig->fSettings.fDefaultUniformBinding;
}

int MetalCodeGenerator::getUniformSet(const Modifiers& m) {
    return (m.fLayout.fSet >= 0) ? m.fLayout.fSet
                                 : fProgram.fConfig->fSettings.fDefaultUniformSet;
}

bool MetalCodeGenerator::writeFunctionDeclaration(const FunctionDeclaration& f) {
    fRTFlipName = fProgram.fInputs.fUseFlipRTUniform
                          ? "_globals._anonInterface0->" SKSL_RTFLIP_NAME
                          : "";
    const char* separator = "";
    if (f.isMain()) {
        switch (fProgram.fConfig->fKind) {
            case ProgramKind::kFragment:
                this->write("fragment Outputs fragmentMain");
                break;
            case ProgramKind::kVertex:
                this->write("vertex Outputs vertexMain");
                break;
            default:
                fContext.fErrors->error(-1, "unsupported kind of program");
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
                    if (var.var().type().dimensions() != SpvDim2D) {
                        // Not yet implemented--Skia currently only uses 2D textures.
                        fContext.fErrors->error(decls.fLine, "Unsupported texture dimensions");
                        return false;
                    }
                    int binding = getUniformBinding(var.var().modifiers());
                    this->write(", texture2d<half> ");
                    this->writeName(var.var().name());
                    this->write("[[texture(");
                    this->write(to_string(binding));
                    this->write(")]]");
                    this->write(", sampler ");
                    this->writeName(var.var().name());
                    this->write(SAMPLER_SUFFIX);
                    this->write("[[sampler(");
                    this->write(to_string(binding));
                    this->write(")]]");
                }
            } else if (e->is<InterfaceBlock>()) {
                const InterfaceBlock& intf = e->as<InterfaceBlock>();
                if (intf.typeName() == "sk_PerVertex") {
                    continue;
                }
                this->write(", constant ");
                this->writeType(intf.variable().type());
                this->write("& " );
                this->write(fInterfaceBlockNameMap[&intf]);
                this->write(" [[buffer(");
                this->write(to_string(this->getUniformBinding(intf.variable().modifiers())));
                this->write(")]]");
            }
        }
        if (fProgram.fConfig->fKind == ProgramKind::kFragment) {
            if (fProgram.fInputs.fUseFlipRTUniform && fInterfaceBlockNameMap.empty()) {
                this->write(", constant sksl_synthetic_uniforms& _anonInterface0 [[buffer(1)]]");
                fRTFlipName = "_anonInterface0." SKSL_RTFLIP_NAME;
            }
            this->write(", bool _frontFacing [[front_facing]]");
            this->write(", float4 _fragCoord [[position]]");
        } else if (fProgram.fConfig->fKind == ProgramKind::kVertex) {
            this->write(", uint sk_VertexID [[vertex_id]], uint sk_InstanceID [[instance_id]]");
        }
        separator = ", ";
    } else {
        this->writeType(f.returnType());
        this->write(" ");
        this->writeName(f.mangledName());
        this->write("(");
        this->writeFunctionRequirementParams(f, separator);
    }
    for (const auto& param : f.parameters()) {
        if (f.isMain() && param->modifiers().fLayout.fBuiltin != -1) {
            continue;
        }
        this->write(separator);
        separator = ", ";
        this->writeModifiers(param->modifiers());
        const Type* type = &param->type();
        this->writeType(*type);
        if (param->modifiers().fFlags & Modifiers::kOut_Flag) {
            this->write("&");
        }
        this->write(" ");
        this->writeName(param->name());
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
        stmt = block[index].get();
        if (stmt->is<ReturnStatement>()) {
            return true;
        }
        if (stmt->is<Block>()) {
            return is_block_ending_with_return(stmt);
        }
        if (!stmt->is<Nop>()) {
            break;
        }
    }
    return false;
}

void MetalCodeGenerator::writeFunction(const FunctionDefinition& f) {
    SkASSERT(!fProgram.fConfig->fSettings.fFragColorIsInOut);

    if (!this->writeFunctionDeclaration(f.declaration())) {
        return;
    }

    fCurrentFunction = &f.declaration();
    SkScopeExit clearCurrentFunction([&] { fCurrentFunction = nullptr; });

    this->writeLine(" {");

    if (f.declaration().isMain()) {
        this->writeGlobalInit();
        this->writeLine("    Outputs _out;");
        this->writeLine("    (void)_out;");
    }

    fFunctionHeader.clear();
    StringStream buffer;
    {
        AutoOutputStream outputToBuffer(this, &buffer);
        fIndentation++;
        for (const std::unique_ptr<Statement>& stmt : f.body()->as<Block>().children()) {
            if (!stmt->isEmpty()) {
                this->writeStatement(*stmt);
                this->finishLine();
            }
        }
        if (f.declaration().isMain()) {
            // If the main function doesn't end with a return, we need to synthesize one here.
            if (!is_block_ending_with_return(f.body().get())) {
                this->writeReturnStatementFromMain();
                this->finishLine();
            }
        }
        fIndentation--;
        this->writeLine("}");
    }
    this->write(fFunctionHeader);
    this->write(buffer.str());
}

void MetalCodeGenerator::writeModifiers(const Modifiers& modifiers) {
    if (modifiers.fFlags & Modifiers::kOut_Flag) {
        this->write("thread ");
    }
    if (modifiers.fFlags & Modifiers::kConst_Flag) {
        this->write("const ");
    }
}

void MetalCodeGenerator::writeInterfaceBlock(const InterfaceBlock& intf) {
    if ("sk_PerVertex" == intf.typeName()) {
        return;
    }
    this->writeModifiers(intf.variable().modifiers());
    this->write("struct ");
    this->writeLine(intf.typeName() + " {");
    const Type* structType = &intf.variable().type();
    if (structType->isArray()) {
        structType = &structType->componentType();
    }
    fIndentation++;
    this->writeFields(structType->fields(), structType->fLine, &intf);
    if (fProgram.fInputs.fUseFlipRTUniform) {
        this->writeLine("float2 " SKSL_RTFLIP_NAME ";");
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
        }
        fInterfaceBlockNameMap[&intf] = intf.instanceName();
    } else {
        fInterfaceBlockNameMap[&intf] = *fProgram.fSymbols->takeOwnershipOfString("_anonInterface" +
                to_string(fAnonInterfaceCount++));
    }
    this->writeLine(";");
}

void MetalCodeGenerator::writeFields(const std::vector<Type::Field>& fields, int parentLine,
                                     const InterfaceBlock* parentIntf) {
    MemoryLayout memoryLayout(MemoryLayout::kMetal_Standard);
    int currentOffset = 0;
    for (const Type::Field& field : fields) {
        int fieldOffset = field.fModifiers.fLayout.fOffset;
        const Type* fieldType = field.fType;
        if (!MemoryLayout::LayoutIsSupported(*fieldType)) {
            fContext.fErrors->error(parentLine, "type '" + fieldType->name() +
                                                "' is not permitted here");
            return;
        }
        if (fieldOffset != -1) {
            if (currentOffset > fieldOffset) {
                fContext.fErrors->error(parentLine,
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
                fContext.fErrors->error(parentLine,
                        "offset of field '" + field.fName + "' must be a multiple of " +
                        to_string((int) alignment));
                return;
            }
        }
        size_t fieldSize = memoryLayout.size(*fieldType);
        if (fieldSize > static_cast<size_t>(std::numeric_limits<int>::max() - currentOffset)) {
            fContext.fErrors->error(parentLine, "field offset overflow");
            return;
        }
        currentOffset += fieldSize;
        this->writeModifiers(field.fModifiers);
        this->writeType(*fieldType);
        this->write(" ");
        this->writeName(field.fName);
        this->writeLine(";");
        if (parentIntf) {
            fInterfaceBlockMap[&field] = parentIntf;
        }
    }
}

void MetalCodeGenerator::writeVarInitializer(const Variable& var, const Expression& value) {
    this->writeExpression(value, Precedence::kTopLevel);
}

void MetalCodeGenerator::writeName(skstd::string_view name) {
    if (fReservedWords.find(name) != fReservedWords.end()) {
        this->write("_"); // adding underscore before name to avoid conflict with reserved words
    }
    this->write(name);
}

void MetalCodeGenerator::writeVarDeclaration(const VarDeclaration& varDecl) {
    this->writeModifiers(varDecl.var().modifiers());
    this->writeType(varDecl.var().type());
    this->write(" ");
    this->writeName(varDecl.var().name());
    if (varDecl.value()) {
        this->write(" = ");
        this->writeVarInitializer(varDecl.var(), *varDecl.value());
    }
    this->write(";");
}

void MetalCodeGenerator::writeStatement(const Statement& s) {
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
            this->writeVarDeclaration(s.as<VarDeclaration>());
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
            SkDEBUGFAILF("unsupported statement: %s", s.description().c_str());
            break;
    }
}

void MetalCodeGenerator::writeBlock(const Block& b) {
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

void MetalCodeGenerator::writeIfStatement(const IfStatement& stmt) {
    this->write("if (");
    this->writeExpression(*stmt.test(), Precedence::kTopLevel);
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
        this->writeExpression(*f.test(), Precedence::kTopLevel);
    }
    this->write("; ");
    if (f.next()) {
        this->writeExpression(*f.next(), Precedence::kTopLevel);
    }
    this->write(") ");
    this->writeStatement(*f.statement());
}

void MetalCodeGenerator::writeDoStatement(const DoStatement& d) {
    this->write("do ");
    this->writeStatement(*d.statement());
    this->write(" while (");
    this->writeExpression(*d.test(), Precedence::kTopLevel);
    this->write(");");
}

void MetalCodeGenerator::writeSwitchStatement(const SwitchStatement& s) {
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
    this->write("}");
}

void MetalCodeGenerator::writeReturnStatementFromMain() {
    // main functions in Metal return a magic _out parameter that doesn't exist in SkSL.
    switch (fProgram.fConfig->fKind) {
        case ProgramKind::kVertex:
        case ProgramKind::kFragment:
            this->write("return _out;");
            break;
        default:
            SkDEBUGFAIL("unsupported kind of program");
    }
}

void MetalCodeGenerator::writeReturnStatement(const ReturnStatement& r) {
    if (fCurrentFunction && fCurrentFunction->isMain()) {
        if (r.expression()) {
            if (r.expression()->type().matches(*fContext.fTypes.fHalf4)) {
                this->write("_out.sk_FragColor = ");
                this->writeExpression(*r.expression(), Precedence::kTopLevel);
                this->writeLine(";");
            } else {
                fContext.fErrors->error(r.fLine,
                        "Metal does not support returning '" +
                        r.expression()->type().description() + "' from main()");
            }
        }
        this->writeReturnStatementFromMain();
        return;
    }

    this->write("return");
    if (r.expression()) {
        this->write(" ");
        this->writeExpression(*r.expression(), Precedence::kTopLevel);
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
                int uniformSet = this->getUniformSet(var.modifiers());
                // Make sure that the program's uniform-set value is consistent throughout.
                if (-1 == fUniformBuffer) {
                    this->write("struct Uniforms {\n");
                    fUniformBuffer = uniformSet;
                } else if (uniformSet != fUniformBuffer) {
                    fContext.fErrors->error(decls.fLine,
                            "Metal backend requires all uniforms to have the same "
                            "'layout(set=...)'");
                }
                this->write("    ");
                this->writeType(var.type());
                this->write(" ");
                this->writeName(var.name());
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
                this->writeType(var.type());
                this->write(" ");
                this->writeName(var.name());
                if (-1 != var.modifiers().fLayout.fLocation) {
                    if (fProgram.fConfig->fKind == ProgramKind::kVertex) {
                        this->write("  [[attribute(" +
                                    to_string(var.modifiers().fLayout.fLocation) + ")]]");
                    } else if (fProgram.fConfig->fKind == ProgramKind::kFragment) {
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
    if (fProgram.fConfig->fKind == ProgramKind::kVertex) {
        this->write("    float4 sk_Position [[position]];\n");
    } else if (fProgram.fConfig->fKind == ProgramKind::kFragment) {
        this->write("    half4 sk_FragColor [[color(0)]];\n");
    }
    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const GlobalVarDeclaration& decls = e->as<GlobalVarDeclaration>();
            const Variable& var = decls.declaration()->as<VarDeclaration>().var();
            if (var.modifiers().fFlags & Modifiers::kOut_Flag &&
                -1 == var.modifiers().fLayout.fBuiltin) {
                this->write("    ");
                this->writeType(var.type());
                this->write(" ");
                this->writeName(var.name());

                int location = var.modifiers().fLayout.fLocation;
                if (location < 0) {
                    fContext.fErrors->error(var.fLine,
                            "Metal out variables must have 'layout(location=...)'");
                } else if (fProgram.fConfig->fKind == ProgramKind::kVertex) {
                    this->write(" [[user(locn" + to_string(location) + ")]]");
                } else if (fProgram.fConfig->fKind == ProgramKind::kFragment) {
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
    if (fProgram.fConfig->fKind == ProgramKind::kVertex) {
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
    if (!wroteInterfaceBlock && fProgram.fInputs.fUseFlipRTUniform) {
        this->writeLine("struct sksl_synthetic_uniforms {");
        this->writeLine("    float2 " SKSL_RTFLIP_NAME ";");
        this->writeLine("};");
    }
}

void MetalCodeGenerator::writeStructDefinitions() {
    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<StructDefinition>()) {
            this->writeStructDefinition(e->as<StructDefinition>());
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
        if (var.type().typeKind() == Type::TypeKind::kSampler) {
            // Samplers are represented as a "texture/sampler" duo in the global struct.
            visitor->visitTexture(var.type(), var.name());
            visitor->visitSampler(var.type(), var.name() + SAMPLER_SUFFIX);
            continue;
        }

        if (!(var.modifiers().fFlags & ~Modifiers::kConst_Flag) &&
            -1 == var.modifiers().fLayout.fBuiltin) {
            // Visit a regular variable.
            visitor->visitVariable(var, decl.value().get());
        }
    }
}

void MetalCodeGenerator::writeGlobalStruct() {
    class : public GlobalStructVisitor {
    public:
        void visitInterfaceBlock(const InterfaceBlock& block,
                                 skstd::string_view blockName) override {
            this->addElement();
            fCodeGen->write("    constant ");
            fCodeGen->write(block.typeName());
            fCodeGen->write("* ");
            fCodeGen->writeName(blockName);
            fCodeGen->write(";\n");
        }
        void visitTexture(const Type& type, skstd::string_view name) override {
            this->addElement();
            fCodeGen->write("    ");
            fCodeGen->writeType(type);
            fCodeGen->write(" ");
            fCodeGen->writeName(name);
            fCodeGen->write(";\n");
        }
        void visitSampler(const Type&, skstd::string_view name) override {
            this->addElement();
            fCodeGen->write("    sampler ");
            fCodeGen->writeName(name);
            fCodeGen->write(";\n");
        }
        void visitVariable(const Variable& var, const Expression* value) override {
            this->addElement();
            fCodeGen->write("    ");
            fCodeGen->writeModifiers(var.modifiers());
            fCodeGen->writeType(var.type());
            fCodeGen->write(" ");
            fCodeGen->writeName(var.name());
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
                                 skstd::string_view blockName) override {
            this->addElement();
            fCodeGen->write("&");
            fCodeGen->writeName(blockName);
        }
        void visitTexture(const Type&, skstd::string_view name) override {
            this->addElement();
            fCodeGen->writeName(name);
        }
        void visitSampler(const Type&, skstd::string_view name) override {
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
                fCodeGen->write("    Globals _globals{");
                fFirst = false;
            } else {
                fCodeGen->write(", ");
            }
        }
        void finish() {
            if (!fFirst) {
                fCodeGen->writeLine("};");
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
        case ProgramElement::Kind::kGlobalVar:
            break;
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
            this->writeModifiers(e.as<ModifiersDeclaration>().modifiers());
            this->writeLine(";");
            break;
        default:
            SkDEBUGFAILF("unsupported program element: %s\n", e.description().c_str());
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
        case Expression::Kind::kConstructorCompound:
        case Expression::Kind::kConstructorCompoundCast:
        case Expression::Kind::kConstructorArray:
        case Expression::Kind::kConstructorArrayCast:
        case Expression::Kind::kConstructorDiagonalMatrix:
        case Expression::Kind::kConstructorScalarCast:
        case Expression::Kind::kConstructorSplat:
        case Expression::Kind::kConstructorStruct: {
            const AnyConstructor& c = e->asAnyConstructor();
            Requirements result = kNo_Requirements;
            for (const auto& arg : c.argumentSpan()) {
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
            for (const std::unique_ptr<Statement>& sc : sw.cases()) {
                result |= this->requirements(sc->as<SwitchCase>().statement().get());
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
    write_stringstream(fExtraFunctionPrototypes, *fOut);
    write_stringstream(fExtraFunctions, *fOut);
    write_stringstream(body, *fOut);
    fContext.fErrors->reportPendingErrors(PositionInfo());
    return fContext.fErrors->errorCount() == 0;
}

}  // namespace SkSL
