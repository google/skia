/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/codegen/SkSLMetalCodeGenerator.h"

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkScopeExit.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLIntrinsicList.h"
#include "src/sksl/SkSLMemoryLayout.h"
#include "src/sksl/SkSLOperator.h"
#include "src/sksl/SkSLOutputStream.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLConstructorArrayCast.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLConstructorMatrixResize.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLExtension.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLFunctionPrototype.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLLayout.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSetting.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLStructDefinition.h"
#include "src/sksl/ir/SkSLSwitchCase.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"
#include "src/sksl/spirv.h"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

using namespace skia_private;

namespace SkSL {

static const char* operator_name(Operator op) {
    switch (op.kind()) {
        case Operator::Kind::LOGICALXOR:  return " != ";
        default:                          return op.operatorName();
    }
}

class MetalCodeGenerator::GlobalStructVisitor {
public:
    virtual ~GlobalStructVisitor() = default;
    virtual void visitInterfaceBlock(const InterfaceBlock& block, std::string_view blockName) {}
    virtual void visitTexture(const Type& type, std::string_view name) {}
    virtual void visitSampler(const Type& type, std::string_view name) {}
    virtual void visitConstantVariable(const VarDeclaration& decl) {}
    virtual void visitNonconstantVariable(const Variable& var, const Expression* value) {}
};

class MetalCodeGenerator::ThreadgroupStructVisitor {
public:
    virtual ~ThreadgroupStructVisitor() = default;
    virtual void visitNonconstantVariable(const Variable& var) = 0;
};

void MetalCodeGenerator::write(std::string_view s) {
    if (s.empty()) {
        return;
    }
    if (fAtLineStart) {
        for (int i = 0; i < fIndentation; i++) {
            fOut->writeText("    ");
        }
    }
    fOut->writeText(std::string(s).c_str());
    fAtLineStart = false;
}

void MetalCodeGenerator::writeLine(std::string_view s) {
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
    this->writeLine("#extension " + std::string(ext.name()) + " : enable");
}

std::string MetalCodeGenerator::typeName(const Type& raw) {
    // we need to know the modifiers for textures
    const Type& type = raw.resolve().scalarTypeForLiteral();
    switch (type.typeKind()) {
        case Type::TypeKind::kArray:
            SkASSERT(!type.isUnsizedArray());
            SkASSERTF(type.columns() > 0, "invalid array size: %s", type.description().c_str());
            return String::printf("array<%s, %d>",
                                  this->typeName(type.componentType()).c_str(), type.columns());

        case Type::TypeKind::kVector:
            return this->typeName(type.componentType()) + std::to_string(type.columns());

        case Type::TypeKind::kMatrix:
            return this->typeName(type.componentType()) + std::to_string(type.columns()) + "x" +
                                  std::to_string(type.rows());

        case Type::TypeKind::kSampler:
            if (type.dimensions() != SpvDim2D) {
                fContext.fErrors->error(Position(), "Unsupported texture dimensions");
            }
            return "sampler2D";

        case Type::TypeKind::kTexture:
            switch (type.textureAccess()) {
                case Type::TextureAccess::kSample:    return "texture2d<half>";
                case Type::TextureAccess::kRead:      return "texture2d<half, access::read>";
                case Type::TextureAccess::kWrite:     return "texture2d<half, access::write>";
                case Type::TextureAccess::kReadWrite: return "texture2d<half, access::read_write>";
                default:                              break;
            }
            SkUNREACHABLE;

        case Type::TypeKind::kAtomic:
            // SkSL currently only supports the atomicUint type.
            SkASSERT(type.matches(*fContext.fTypes.fAtomicUInt));
            return "atomic_uint";

        default:
            return std::string(type.name());
    }
}

void MetalCodeGenerator::writeStructDefinition(const StructDefinition& s) {
    const Type& type = s.type();
    this->writeLine("struct " + type.displayName() + " {");
    fIndentation++;
    this->writeFields(type.fields(), type.fPosition);
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
        case Expression::Kind::kEmpty:
            this->write("false");
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
            this->writeExpression(*expr.as<Setting>().toLiteral(fContext), parentPrecedence);
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

// returns true if we should pass by reference instead of by value
static bool pass_by_reference(const Type& type, ModifierFlags flags) {
    return (flags & ModifierFlag::kOut) && !type.isUnsizedArray();
}

// returns true if we need to specify an address space modifier
static bool needs_address_space(const Type& type, ModifierFlags modifiers) {
    return type.isUnsizedArray() || pass_by_reference(type, modifiers);
}

// returns true if the InterfaceBlock has the `buffer` modifier
static bool is_buffer(const InterfaceBlock& block) {
    return block.var()->modifierFlags().isBuffer();
}

// returns true if the InterfaceBlock has the `readonly` modifier
static bool is_readonly(const InterfaceBlock& block) {
    return block.var()->modifierFlags().isReadOnly();
}

std::string MetalCodeGenerator::getBitcastIntrinsic(const Type& outType) {
    return "as_type<" +  outType.displayName() + ">";
}

void MetalCodeGenerator::writeWithIndexSubstitution(const std::function<void()>& fn) {
    auto oldIndexSubstitutionData = std::make_unique<IndexSubstitutionData>();
    fIndexSubstitutionData.swap(oldIndexSubstitutionData);

    // Invoke our helper function, with output going into our temporary stream.
    {
        AutoOutputStream outputToMainStream(this, &fIndexSubstitutionData->fMainStream);
        fn();
    }

    if (fIndexSubstitutionData->fPrefixStream.bytesWritten() == 0) {
        // Emit the main stream into the program as-is.
        write_stringstream(fIndexSubstitutionData->fMainStream, *fOut);
    } else {
        // Emit the prefix stream and main stream into the program as a sequence-expression.
        // (Each prefix-expression must end with a comma.)
        this->write("(");
        write_stringstream(fIndexSubstitutionData->fPrefixStream, *fOut);
        write_stringstream(fIndexSubstitutionData->fMainStream, *fOut);
        this->write(")");
    }

    fIndexSubstitutionData.swap(oldIndexSubstitutionData);
}

void MetalCodeGenerator::writeFunctionCall(const FunctionCall& c) {
    const FunctionDeclaration& function = c.function();

    // Many intrinsics need to be rewritten in Metal.
    if (function.isIntrinsic()) {
        if (this->writeIntrinsicCall(c, function.intrinsicKind())) {
            return;
        }
    }

    // Look for out parameters. SkSL guarantees GLSL's out-param semantics, and we need to emulate
    // it if an out-param is encountered. (Specifically, out-parameters in GLSL are only written
    // back to the original variable at the end of the function call; also, swizzles are supported,
    // whereas Metal doesn't allow a swizzle to be passed to a `floatN&`.)
    const ExpressionArray& arguments = c.arguments();
    SkSpan<Variable* const> parameters = function.parameters();
    SkASSERT(SkToSizeT(arguments.size()) == parameters.size());

    bool foundOutParam = false;
    STArray<16, std::string> scratchVarName;
    scratchVarName.push_back_n(arguments.size(), std::string());

    for (int index = 0; index < arguments.size(); ++index) {
        // If this is an out parameter...
        if (parameters[index]->modifierFlags() & ModifierFlag::kOut) {
            // Assignability was verified at IRGeneration time, so this should always succeed.
            [[maybe_unused]] Analysis::AssignmentInfo info;
            SkASSERT(Analysis::IsAssignable(*arguments[index], &info));

            scratchVarName[index] = this->getTempVariable(arguments[index]->type());
            foundOutParam = true;
        }
    }

    if (foundOutParam) {
        // Out parameters need to be written back to at the end of the function. To do this, we
        // generate a comma-separated sequence expression that copies the out-param expressions into
        // our temporary variables, calls the original function--storing its result into a scratch
        // variable--and then writes the temp variables back into the original out params using the
        // original out-param expressions. This would look something like:
        //
        // ((_skResult = func((_skTemp = myOutParam.x), 123)), (myOutParam.x = _skTemp), _skResult)
        //       ^                     ^                                     ^                ^
        //   return value       passes copy of argument    copies back into argument    return value
        //
        // While these expressions are complex, they allow us to maintain the proper sequencing that
        // is necessary for out-parameters, as well as allowing us to support things like swizzles
        // and array indices which Metal references cannot natively handle.

        // We will be emitting inout expressions twice, so it's important to enable index
        // substitution in case we encounter any side-effecting indexes.
        this->writeWithIndexSubstitution([&] {
            this->write("((");

            // ((_skResult =
            std::string scratchResultName;
            if (!function.returnType().isVoid()) {
                scratchResultName = this->getTempVariable(c.type());
                this->write(scratchResultName);
                this->write(" = ");
            }

            // ((_skResult = func(
            this->write(function.mangledName());
            this->write("(");

            // ((_skResult = func((_skTemp = myOutParam.x), 123
            const char* separator = "";
            this->writeFunctionRequirementArgs(function, separator);

            for (int i = 0; i < arguments.size(); ++i) {
                this->write(separator);
                separator = ", ";
                if (parameters[i]->modifierFlags() & ModifierFlag::kOut) {
                    SkASSERT(!scratchVarName[i].empty());
                    if (parameters[i]->modifierFlags() & ModifierFlag::kIn) {
                        // `inout` parameters initialize the scratch variable with the passed-in
                        // argument's value.
                        this->write("(");
                        this->write(scratchVarName[i]);
                        this->write(" = ");
                        this->writeExpression(*arguments[i], Precedence::kAssignment);
                        this->write(")");
                    } else {
                        // `out` parameters pass a reference to the uninitialized scratch variable.
                        this->write(scratchVarName[i]);
                    }
                } else {
                    // Regular parameters are passed as-is.
                    this->writeExpression(*arguments[i], Precedence::kSequence);
                }
            }

            // ((_skResult = func((_skTemp = myOutParam.x), 123))
            this->write("))");

            // ((_skResult = func((_skTemp = myOutParam.x), 123)), (myOutParam.x = _skTemp)
            for (int i = 0; i < arguments.size(); ++i) {
                if (!scratchVarName[i].empty()) {
                    this->write(", (");
                    this->writeExpression(*arguments[i], Precedence::kAssignment);
                    this->write(" = ");
                    this->write(scratchVarName[i]);
                    this->write(")");
                }
            }

            // ((_skResult = func((_skTemp = myOutParam.x), 123)), (myOutParam.x = _skTemp),
            //                                                     _skResult
            if (!scratchResultName.empty()) {
                this->write(", ");
                this->write(scratchResultName);
            }

            // ((_skResult = func((_skTemp = myOutParam.x), 123)), (myOutParam.x = _skTemp),
            //                                                     _skResult)
            this->write(")");
        });
    } else {
        // Emit the function call as-is, only prepending the required arguments.
        this->write(function.mangledName());
        this->write("(");
        const char* separator = "";
        this->writeFunctionRequirementArgs(function, separator);
        for (int i = 0; i < arguments.size(); ++i) {
            SkASSERT(scratchVarName[i].empty());
            this->write(separator);
            separator = ", ";
            this->writeExpression(*arguments[i], Precedence::kSequence);
        }
        this->write(")");
    }
}

static constexpr char kInverse2x2[] = R"(
template <typename T>
matrix<T, 2, 2> mat2_inverse(matrix<T, 2, 2> m) {
return matrix<T, 2, 2>(m[1].y, -m[0].y, -m[1].x, m[0].x) * (1/determinant(m));
}
)";

static constexpr char kInverse3x3[] = R"(
template <typename T>
matrix<T, 3, 3> mat3_inverse(matrix<T, 3, 3> m) {
T
 a00 = m[0].x, a01 = m[0].y, a02 = m[0].z,
 a10 = m[1].x, a11 = m[1].y, a12 = m[1].z,
 a20 = m[2].x, a21 = m[2].y, a22 = m[2].z,
 b01 =  a22*a11 - a12*a21,
 b11 = -a22*a10 + a12*a20,
 b21 =  a21*a10 - a11*a20,
 det = a00*b01 + a01*b11 + a02*b21;
return matrix<T, 3, 3>(
 b01, (-a22*a01 + a02*a21), ( a12*a01 - a02*a11),
 b11, ( a22*a00 - a02*a20), (-a12*a00 + a02*a10),
 b21, (-a21*a00 + a01*a20), ( a11*a00 - a01*a10)) * (1/det);
}
)";

static constexpr char kInverse4x4[] = R"(
template <typename T>
matrix<T, 4, 4> mat4_inverse(matrix<T, 4, 4> m) {
T
 a00 = m[0].x, a01 = m[0].y, a02 = m[0].z, a03 = m[0].w,
 a10 = m[1].x, a11 = m[1].y, a12 = m[1].z, a13 = m[1].w,
 a20 = m[2].x, a21 = m[2].y, a22 = m[2].z, a23 = m[2].w,
 a30 = m[3].x, a31 = m[3].y, a32 = m[3].z, a33 = m[3].w,
 b00 = a00*a11 - a01*a10,
 b01 = a00*a12 - a02*a10,
 b02 = a00*a13 - a03*a10,
 b03 = a01*a12 - a02*a11,
 b04 = a01*a13 - a03*a11,
 b05 = a02*a13 - a03*a12,
 b06 = a20*a31 - a21*a30,
 b07 = a20*a32 - a22*a30,
 b08 = a20*a33 - a23*a30,
 b09 = a21*a32 - a22*a31,
 b10 = a21*a33 - a23*a31,
 b11 = a22*a33 - a23*a32,
 det = b00*b11 - b01*b10 + b02*b09 + b03*b08 - b04*b07 + b05*b06;
return matrix<T, 4, 4>(
 a11*b11 - a12*b10 + a13*b09,
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

std::string MetalCodeGenerator::getInversePolyfill(const ExpressionArray& arguments) {
    // Only use polyfills for a function taking a single-argument square matrix.
    SkASSERT(arguments.size() == 1);
    const Type& type = arguments.front()->type();
    if (type.isMatrix() && type.rows() == type.columns()) {
        switch (type.rows()) {
            case 2:
                if (!fWrittenInverse2) {
                    fWrittenInverse2 = true;
                    fExtraFunctions.writeText(kInverse2x2);
                }
                return "mat2_inverse";
            case 3:
                if (!fWrittenInverse3) {
                    fWrittenInverse3 = true;
                    fExtraFunctions.writeText(kInverse3x3);
                }
                return "mat3_inverse";
            case 4:
                if (!fWrittenInverse4) {
                    fWrittenInverse4 = true;
                    fExtraFunctions.writeText(kInverse4x4);
                }
                return "mat4_inverse";
        }
    }
    SkDEBUGFAILF("no polyfill for inverse(%s)", type.description().c_str());
    return "inverse";
}

void MetalCodeGenerator::writeMatrixCompMult() {
    static constexpr char kMatrixCompMult[] = R"(
template <typename T, int C, int R>
matrix<T, C, R> matrixCompMult(matrix<T, C, R> a, const matrix<T, C, R> b) {
 for (int c = 0; c < C; ++c) { a[c] *= b[c]; }
 return a;
}
)";
    if (!fWrittenMatrixCompMult) {
        fWrittenMatrixCompMult = true;
        fExtraFunctions.writeText(kMatrixCompMult);
    }
}

void MetalCodeGenerator::writeOuterProduct() {
    static constexpr char kOuterProduct[] = R"(
template <typename T, int C, int R>
matrix<T, C, R> outerProduct(const vec<T, R> a, const vec<T, C> b) {
 matrix<T, C, R> m;
 for (int c = 0; c < C; ++c) { m[c] = a * b[c]; }
 return m;
}
)";
    if (!fWrittenOuterProduct) {
        fWrittenOuterProduct = true;
        fExtraFunctions.writeText(kOuterProduct);
    }
}

std::string MetalCodeGenerator::getTempVariable(const Type& type) {
    std::string tempVar = "_skTemp" + std::to_string(fVarCount++);
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
        case k_textureRead_IntrinsicKind: {
            this->writeExpression(*arguments[0], Precedence::kExpression);
            this->write(".read(");
            this->writeExpression(*arguments[1], Precedence::kSequence);
            this->write(")");
            return true;
        }
        case k_textureWrite_IntrinsicKind: {
            this->writeExpression(*arguments[0], Precedence::kExpression);
            this->write(".write(");
            this->writeExpression(*arguments[2], Precedence::kSequence);
            this->write(", ");
            this->writeExpression(*arguments[1], Precedence::kSequence);
            this->write(")");
            return true;
        }
        case k_textureWidth_IntrinsicKind: {
            this->writeExpression(*arguments[0], Precedence::kExpression);
            this->write(".get_width()");
            return true;
        }
        case k_textureHeight_IntrinsicKind: {
            this->writeExpression(*arguments[0], Precedence::kExpression);
            this->write(".get_height()");
            return true;
        }
        case k_mod_IntrinsicKind: {
            // fmod(x, y) in metal calculates x - y * trunc(x / y) instead of x - y * floor(x / y)
            std::string tmpX = this->getTempVariable(arguments[0]->type());
            std::string tmpY = this->getTempVariable(arguments[1]->type());
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
            if (!fRTFlipName.empty()) {
                this->write("(" + fRTFlipName + ".y * dfdy");
            } else {
                this->write("(dfdy");
            }
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
                std::string tmpI = this->getTempVariable(arguments[0]->type());
                std::string tmpN = this->getTempVariable(arguments[1]->type());

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
            std::string skTemp = this->getTempVariable(arguments[0]->type());
            std::string exprType = this->typeName(arguments[0]->type());

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
            std::string skTemp1 = this->getTempVariable(arguments[0]->type());
            std::string exprType = this->typeName(arguments[0]->type());

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
            std::string skTemp2;
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
        case k_sign_IntrinsicKind: {
            if (arguments[0]->type().componentType().isInteger()) {
                // Create a temp variable to store the expression, to avoid double-evaluating it.
                std::string skTemp = this->getTempVariable(arguments[0]->type());
                std::string exprType = this->typeName(arguments[0]->type());

                // (_skTemp = (.....),
                this->write("(");
                this->write(skTemp);
                this->write(" = (");
                this->writeExpression(*arguments[0], Precedence::kSequence);
                this->write("), ");

                // ... select(select(int4(0), int4(-1), _skTemp < 0), int4(1), _skTemp > 0))
                this->write("select(select(");
                this->write(exprType);
                this->write("(0), ");
                this->write(exprType);
                this->write("(-1), ");
                this->write(skTemp);
                this->write(" < 0), ");
                this->write(exprType);
                this->write("(1), ");
                this->write(skTemp);
                this->write(" > 0))");
            } else {
                this->writeSimpleIntrinsic(c);
            }
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
        case k_storageBarrier_IntrinsicKind:
            this->write("threadgroup_barrier(mem_flags::mem_device)");
            return true;
        case k_workgroupBarrier_IntrinsicKind:
            this->write("threadgroup_barrier(mem_flags::mem_threadgroup)");
            return true;
        case k_atomicAdd_IntrinsicKind:
            this->write("atomic_fetch_add_explicit(&");
            this->writeExpression(*c.arguments()[0], Precedence::kSequence);
            this->write(", ");
            this->writeExpression(*c.arguments()[1], Precedence::kSequence);
            this->write(", memory_order_relaxed)");
            return true;
        case k_atomicLoad_IntrinsicKind:
            this->write("atomic_load_explicit(&");
            this->writeExpression(*c.arguments()[0], Precedence::kSequence);
            this->write(", memory_order_relaxed)");
            return true;
        case k_atomicStore_IntrinsicKind:
            this->write("atomic_store_explicit(&");
            this->writeExpression(*c.arguments()[0], Precedence::kSequence);
            this->write(", ");
            this->writeExpression(*c.arguments()[1], Precedence::kSequence);
            this->write(", memory_order_relaxed)");
            return true;
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
std::string MetalCodeGenerator::getMatrixConstructHelper(const AnyConstructor& c) {
    const Type& type = c.type();
    int columns = type.columns();
    int rows = type.rows();
    auto args = c.argumentSpan();
    std::string typeName = this->typeName(type);

    // Create the helper-method name and use it as our lookup key.
    std::string name = String::printf("%s_from", typeName.c_str());
    for (const std::unique_ptr<Expression>& expr : args) {
        String::appendf(&name, "_%s", this->typeName(expr->type()).c_str());
    }

    // If a helper-method has not been synthesized yet, create it now.
    if (!fHelpers.contains(name)) {
        fHelpers.add(name);

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
    }
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
        fContext.fErrors->error(c.fPosition, "unsupported compound constructor");
    }
}

void MetalCodeGenerator::writeConstructorArrayCast(const ConstructorArrayCast& c,
                                                   Precedence parentPrecedence) {
    const Type& inType = c.argument()->type().componentType();
    const Type& outType = c.type().componentType();
    std::string inTypeName = this->typeName(inType);
    std::string outTypeName = this->typeName(outType);

    std::string name = "array_of_" + outTypeName + "_from_" + inTypeName;
    if (!fHelpers.contains(name)) {
        fHelpers.add(name);
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

std::string MetalCodeGenerator::getVectorFromMat2x2ConstructorHelper(const Type& matrixType) {
    SkASSERT(matrixType.isMatrix());
    SkASSERT(matrixType.rows() == 2);
    SkASSERT(matrixType.columns() == 2);

    std::string baseType = this->typeName(matrixType.componentType());
    std::string name = String::printf("%s4_from_%s2x2", baseType.c_str(), baseType.c_str());
    if (!fHelpers.contains(name)) {
        fHelpers.add(name);

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
    if (!fRTFlipName.empty()) {
        this->write("float4(_fragCoord.x, ");
        this->write(fRTFlipName.c_str());
        this->write(".x + ");
        this->write(fRTFlipName.c_str());
        this->write(".y * _fragCoord.y, 0.0, _fragCoord.w)");
    } else {
        this->write("float4(_fragCoord.x, _fragCoord.y, 0.0, _fragCoord.w)");
    }
}

static bool is_compute_builtin(const Variable& var) {
    switch (var.layout().fBuiltin) {
        case SK_NUMWORKGROUPS_BUILTIN:
        case SK_WORKGROUPID_BUILTIN:
        case SK_LOCALINVOCATIONID_BUILTIN:
        case SK_GLOBALINVOCATIONID_BUILTIN:
        case SK_LOCALINVOCATIONINDEX_BUILTIN:
            return true;
        default:
            break;
    }
    return false;
}

// true if the var is part of the Inputs struct
static bool is_input(const Variable& var) {
    SkASSERT(var.storage() == VariableStorage::kGlobal);
    return var.modifierFlags() & ModifierFlag::kIn &&
           (var.layout().fBuiltin == -1 || is_compute_builtin(var)) &&
           var.type().typeKind() != Type::TypeKind::kTexture;
}

// true if the var is part of the Outputs struct
static bool is_output(const Variable& var) {
    SkASSERT(var.storage() == VariableStorage::kGlobal);
    // inout vars get written into the Inputs struct, so we exclude them from Outputs
    return  (var.modifierFlags() & ModifierFlag::kOut) &&
           !(var.modifierFlags() & ModifierFlag::kIn) &&
             var.layout().fBuiltin == -1 &&
             var.type().typeKind() != Type::TypeKind::kTexture;
}

// true if the var is part of the Uniforms struct
static bool is_uniforms(const Variable& var) {
    SkASSERT(var.storage() == VariableStorage::kGlobal);
    return var.modifierFlags().isUniform() &&
           var.type().typeKind() != Type::TypeKind::kSampler;
}

// true if the var is part of the Threadgroups struct
static bool is_threadgroup(const Variable& var) {
    SkASSERT(var.storage() == VariableStorage::kGlobal);
    return var.modifierFlags().isWorkgroup();
}

// true if the var is part of the Globals struct
static bool is_in_globals(const Variable& var) {
    SkASSERT(var.storage() == VariableStorage::kGlobal);
    return !var.modifierFlags().isConst();
}

void MetalCodeGenerator::writeVariableReference(const VariableReference& ref) {
    switch (ref.variable()->layout().fBuiltin) {
        case SK_FRAGCOLOR_BUILTIN:
            this->write("_out.sk_FragColor");
            break;
        case SK_SAMPLEMASK_BUILTIN:
            this->write("_out.sk_SampleMask");
            break;
        case SK_SECONDARYFRAGCOLOR_BUILTIN:
            this->write("_out.sk_SecondaryFragColor");
            break;
        case SK_FRAGCOORD_BUILTIN:
            this->writeFragCoord();
            break;
        case SK_SAMPLEMASKIN_BUILTIN:
            this->write("sk_SampleMaskIn");
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
            if (!fRTFlipName.empty()) {
                this->write("(" + fRTFlipName + ".y < 0 ? _frontFacing : !_frontFacing)");
            } else {
                this->write("_frontFacing");
            }
            break;
        case SK_LASTFRAGCOLOR_BUILTIN:
            this->write(fContext.fCaps->fFBFetchColorName);
            break;
        default:
            const Variable& var = *ref.variable();
            if (var.storage() == Variable::Storage::kGlobal) {
                if (is_input(var)) {
                    this->write("_in.");
                } else if (is_output(var)) {
                    this->write("_out.");
                } else if (is_uniforms(var)) {
                    this->write("_uniforms.");
                } else if (is_threadgroup(var)) {
                    this->write("_threadgroups.");
                } else if (is_in_globals(var)) {
                    this->write("_globals.");
                }
            }
            this->writeName(var.mangledName());
    }
}

void MetalCodeGenerator::writeIndexInnerExpression(const Expression& expr) {
    if (fIndexSubstitutionData) {
        // If this expression already exists in the index-substitution map, use the substitute.
        if (const std::string* existing = fIndexSubstitutionData->fMap.find(&expr)) {
            this->write(*existing);
            return;
        }

        // If this expression is non-trivial, we will need to create a scratch variable and store
        // its value there.
        if (fIndexSubstitutionData->fCreateSubstitutes && !Analysis::IsTrivialExpression(expr)) {
            // Create a substitute variable and emit it into the main stream.
            std::string scratchVar = this->getTempVariable(expr.type());
            this->write(scratchVar);

            // Initialize the substitute variable in the prefix-stream.
            AutoOutputStream outputToPrefixStream(this, &fIndexSubstitutionData->fPrefixStream);
            this->write(scratchVar);
            this->write(" = ");
            this->writeExpression(expr, Precedence::kAssignment);
            this->write(", ");

            // Remember the substitute variable in our map.
            fIndexSubstitutionData->fMap.set(&expr, std::move(scratchVar));
            return;
        }
    }

    // We don't require index-substitution; just emit the expression normally.
    this->writeExpression(expr, Precedence::kExpression);
}

void MetalCodeGenerator::writeIndexExpression(const IndexExpression& expr) {
    // Metal does not seem to handle assignment into `vec.zyx[i]` properly--it compiles, but the
    // results are wrong. We rewrite the expression as `vec[uint3(2,1,0)[i]]` instead. (Filed with
    // Apple as FB12055941.)
    if (expr.base()->is<Swizzle>() && expr.base()->as<Swizzle>().components().size() > 1) {
        const Swizzle& swizzle = expr.base()->as<Swizzle>();
        this->writeExpression(*swizzle.base(), Precedence::kPostfix);
        this->write("[uint" + std::to_string(swizzle.components().size()) + "(");
        auto separator = SkSL::String::Separator();
        for (int8_t component : swizzle.components()) {
            this->write(separator());
            this->write(std::to_string(component));
        }
        this->write(")[");
        this->writeIndexInnerExpression(*expr.index());
        this->write("]]");
    } else {
        this->writeExpression(*expr.base(), Precedence::kPostfix);
        this->write("[");
        this->writeIndexInnerExpression(*expr.index());
        this->write("]");
    }
}

void MetalCodeGenerator::writeFieldAccess(const FieldAccess& f) {
    const Field* field = &f.base()->type().fields()[f.fieldIndex()];
    if (FieldAccess::OwnerKind::kDefault == f.ownerKind()) {
        this->writeExpression(*f.base(), Precedence::kPostfix);
        this->write(".");
    }
    switch (field->fLayout.fBuiltin) {
        case SK_POSITION_BUILTIN:
            this->write("_out.sk_Position");
            break;
        case SK_POINTSIZE_BUILTIN:
            this->write("_out.sk_PointSize");
            break;
        default:
            if (FieldAccess::OwnerKind::kAnonymousInterfaceBlock == f.ownerKind()) {
                this->write("_globals.");
                this->write(fInterfaceBlockNameMap[&f.base()->type()]);
                this->write("->");
            }
            this->writeName(field->fName);
    }
}

void MetalCodeGenerator::writeSwizzle(const Swizzle& swizzle) {
    this->writeExpression(*swizzle.base(), Precedence::kPostfix);
    this->write(".");
    this->write(Swizzle::MaskString(swizzle.components()));
}

void MetalCodeGenerator::writeMatrixTimesEqualHelper(const Type& left, const Type& right,
                                                     const Type& result) {
    SkASSERT(left.isMatrix());
    SkASSERT(right.isMatrix());
    SkASSERT(result.isMatrix());

    std::string key = "Matrix *= " + this->typeName(left) + ":" + this->typeName(right);

    if (!fHelpers.contains(key)) {
        fHelpers.add(key);
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

    std::string key = "Matrix == " + this->typeName(left) + ":" + this->typeName(right);

    if (!fHelpers.contains(key)) {
        fHelpers.add(key);
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

    std::string key = "Matrix / " + this->typeName(type);

    if (!fHelpers.contains(key)) {
        fHelpers.add(key);
        std::string typeName = this->typeName(type);

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

    std::string key = "ArrayEquality []";
    if (!fHelpers.contains(key)) {
        fHelpers.add(key);
        fExtraFunctionPrototypes.writeText(R"(
template <typename T1, typename T2>
bool operator==(const array_ref<T1> left, const array_ref<T2> right);
template <typename T1, typename T2>
bool operator!=(const array_ref<T1> left, const array_ref<T2> right);
)");
        fExtraFunctions.writeText(R"(
template <typename T1, typename T2>
bool operator==(const array_ref<T1> left, const array_ref<T2> right) {
    if (left.size() != right.size()) {
        return false;
    }
    for (size_t index = 0; index < left.size(); ++index) {
        if (!all(left[index] == right[index])) {
            return false;
        }
    }
    return true;
}

template <typename T1, typename T2>
bool operator!=(const array_ref<T1> left, const array_ref<T2> right) {
    return !(left == right);
}
)");
    }
}

void MetalCodeGenerator::writeStructEqualityHelpers(const Type& type) {
    SkASSERT(type.isStruct());
    std::string key = "StructEquality " + this->typeName(type);

    if (!fHelpers.contains(key)) {
        fHelpers.add(key);
        // If one of the struct's fields needs a helper as well, we need to emit that one first.
        for (const Field& field : type.fields()) {
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
        for (const Field& field : type.fields()) {
            if (field.fType->isArray()) {
                fExtraFunctions.printf(
                        "%s(make_array_ref(left.%.*s) == make_array_ref(right.%.*s))",
                        separator,
                        (int)field.fName.size(), field.fName.data(),
                        (int)field.fName.size(), field.fName.data());
            } else {
                fExtraFunctions.printf("%sall(left.%.*s == right.%.*s)",
                                       separator,
                                       (int)field.fName.size(), field.fName.data(),
                                       (int)field.fName.size(), field.fName.data());
            }
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

void MetalCodeGenerator::writeBinaryExpressionElement(const Expression& expr,
                                                      Operator op,
                                                      const Expression& other,
                                                      Precedence precedence) {
    bool needMatrixSplatOnScalar = other.type().isMatrix() && expr.type().isNumber() &&
                                   op.isValidForMatrixOrVector() &&
                                   op.removeAssignment().kind() != Operator::Kind::STAR;
    if (needMatrixSplatOnScalar) {
        this->writeNumberAsMatrix(expr, other.type());
    } else if (op.isEquality() && expr.type().isArray()) {
        this->write("make_array_ref(");
        this->writeExpression(expr, precedence);
        this->write(")");
    } else {
        this->writeExpression(expr, precedence);
    }
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
        case Operator::Kind::EQEQ:
            this->writeEqualityHelpers(leftType, rightType);
            if (leftType.isVector()) {
                this->write("all");
                needParens = true;
            }
            break;
        case Operator::Kind::NEQ:
            this->writeEqualityHelpers(leftType, rightType);
            if (leftType.isVector()) {
                this->write("any");
                needParens = true;
            }
            break;
        default:
            break;
    }
    if (leftType.isMatrix() && rightType.isMatrix() && op.kind() == Operator::Kind::STAREQ) {
        this->writeMatrixTimesEqualHelper(leftType, rightType, b.type());
    }
    if (op.removeAssignment().kind() == Operator::Kind::SLASH &&
        ((leftType.isMatrix() && rightType.isMatrix()) ||
         (leftType.isScalar() && rightType.isMatrix()) ||
         (leftType.isMatrix() && rightType.isScalar()))) {
        this->writeMatrixDivisionHelpers(leftType.isMatrix() ? leftType : rightType);
    }

    if (needParens) {
        this->write("(");
    }

    // Some expressions need to be rewritten from `lhs *= rhs` to `lhs = lhs * rhs`, e.g.:
    //     float4 x = float4(1);
    //     x.xy *= float2x2(...);
    // will report the error "non-const reference cannot bind to vector element."
    if (op.isCompoundAssignment() && left.kind() == Expression::Kind::kSwizzle) {
        // We need to do the rewrite. This could be dangerous if the lhs contains an index
        // expression with a side effect (such as `array[Func()]`), so we enable index-substitution
        // here for the LHS; any index-expression with side effects will be evaluated into a scratch
        // variable.
        this->writeWithIndexSubstitution([&] {
            this->writeExpression(left, precedence);
            this->write(" = ");
            this->writeExpression(left, Precedence::kAssignment);
            this->write(operator_name(op.removeAssignment()));

            // We never want to create index-expression substitutes on the RHS of the expression;
            // the RHS is only emitted one time.
            fIndexSubstitutionData->fCreateSubstitutes = false;

            this->writeBinaryExpressionElement(right, op, left,
                                               op.removeAssignment().getBinaryPrecedence());
        });
    } else {
        // We don't need any rewrite; emit the binary expression as-is.
        this->writeBinaryExpressionElement(left, op, right, precedence);
        this->write(operator_name(op));
        this->writeBinaryExpressionElement(right, op, left, precedence);
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
    // According to the MSL specification, the arithmetic unary operators (+ and –) do not act
    // upon matrix type operands. We treat the unary "+" as a no-op for all operands.
    const Operator op = p.getOperator();
    if (op.kind() == Operator::Kind::PLUS) {
        this->writeExpression(*p.operand(), Precedence::kPrefix);
        return;
    }

    if (op.kind() == Operator::Kind::MINUS && p.operand()->type().isMatrix()) {
        // Transform the unary "-" on a matrix type to a multiplication by -1.
        this->write(p.type().componentType().highPrecision() ? "(-1.0 * "
                                                             : "(-1.0h * ");
        this->writeExpression(*p.operand(), Precedence::kMultiplicative);
        this->write(")");
        return;
    }

    if (Precedence::kPrefix >= parentPrecedence) {
        this->write("(");
    }

    this->write(p.getOperator().tightOperatorName());
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
    this->write(p.getOperator().tightOperatorName());
    if (Precedence::kPostfix >= parentPrecedence) {
        this->write(")");
    }
}

void MetalCodeGenerator::writeLiteral(const Literal& l) {
    const Type& type = l.type();
    if (type.isFloat()) {
        this->write(l.description(OperatorPrecedence::kExpression));
        if (!l.type().highPrecision()) {
            this->write("h");
        }
        return;
    }
    if (type.isInteger()) {
        if (type.matches(*fContext.fTypes.fUInt)) {
            this->write(std::to_string(l.intValue() & 0xffffffff));
            this->write("u");
        } else if (type.matches(*fContext.fTypes.fUShort)) {
            this->write(std::to_string(l.intValue() & 0xffff));
            this->write("u");
        } else {
            this->write(std::to_string(l.intValue()));
        }
        return;
    }
    SkASSERT(type.isBoolean());
    this->write(l.description(OperatorPrecedence::kExpression));
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
    if (requirements & kSampleMaskIn_Requirement) {
        this->write(separator);
        this->write("sk_SampleMaskIn");
        separator = ", ";
    }
    if (requirements & kVertexID_Requirement) {
        this->write(separator);
        this->write("sk_VertexID");
        separator = ", ";
    }
    if (requirements & kInstanceID_Requirement) {
        this->write(separator);
        this->write("sk_InstanceID");
        separator = ", ";
    }
    if (requirements & kThreadgroups_Requirement) {
        this->write(separator);
        this->write("_threadgroups");
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
    if (requirements & kSampleMaskIn_Requirement) {
        this->write(separator);
        this->write("uint sk_SampleMaskIn");
        separator = ", ";
    }
    if (requirements & kVertexID_Requirement) {
        this->write(separator);
        this->write("uint sk_VertexID");
        separator = ", ";
    }
    if (requirements & kInstanceID_Requirement) {
        this->write(separator);
        this->write("uint sk_InstanceID");
        separator = ", ";
    }
    if (requirements & kThreadgroups_Requirement) {
        this->write(separator);
        this->write("threadgroup Threadgroups& _threadgroups");
        separator = ", ";
    }
}

int MetalCodeGenerator::getUniformBinding(const Layout& layout) {
    return (layout.fBinding >= 0) ? layout.fBinding
                                  : fProgram.fConfig->fSettings.fDefaultUniformBinding;
}

int MetalCodeGenerator::getUniformSet(const Layout& layout) {
    return (layout.fSet >= 0) ? layout.fSet
                              : fProgram.fConfig->fSettings.fDefaultUniformSet;
}

bool MetalCodeGenerator::writeFunctionDeclaration(const FunctionDeclaration& f) {
    fRTFlipName = fProgram.fInterface.fUseFlipRTUniform
                          ? "_globals._anonInterface0->" SKSL_RTFLIP_NAME
                          : "";
    const char* separator = "";
    if (f.isMain()) {
        if (ProgramConfig::IsFragment(fProgram.fConfig->fKind)) {
            this->write("fragment Outputs fragmentMain(");
        } else if (ProgramConfig::IsVertex(fProgram.fConfig->fKind)) {
            this->write("vertex Outputs vertexMain(");
        } else if (ProgramConfig::IsCompute(fProgram.fConfig->fKind)) {
            this->write("kernel void computeMain(");
        } else {
            fContext.fErrors->error(Position(), "unsupported kind of program");
            return false;
        }
        if (!ProgramConfig::IsCompute(fProgram.fConfig->fKind)) {
            this->write("Inputs _in [[stage_in]]");
            separator = ", ";
        }
        if (-1 != fUniformBuffer) {
            this->write(separator);
            this->write("constant Uniforms& _uniforms [[buffer(" +
                        std::to_string(fUniformBuffer) + ")]]");
            separator = ", ";
        }
        for (const ProgramElement* e : fProgram.elements()) {
            if (e->is<GlobalVarDeclaration>()) {
                const GlobalVarDeclaration& decls = e->as<GlobalVarDeclaration>();
                const VarDeclaration& decl = decls.varDeclaration();
                const Variable* var = decl.var();
                const SkSL::Type::TypeKind varKind = var->type().typeKind();

                if (varKind == Type::TypeKind::kSampler || varKind == Type::TypeKind::kTexture) {
                    if (var->type().dimensions() != SpvDim2D) {
                        // Not yet implemented--Skia currently only uses 2D textures.
                        fContext.fErrors->error(decls.fPosition, "Unsupported texture dimensions");
                        return false;
                    }

                    int binding = getUniformBinding(var->layout());
                    this->write(separator);
                    separator = ", ";

                    if (varKind == Type::TypeKind::kSampler) {
                        this->writeType(var->type().textureType());
                        this->write(" ");
                        this->writeName(var->mangledName());
                        this->write(kTextureSuffix);
                        this->write(" [[texture(");
                        this->write(std::to_string(binding));
                        this->write(")]], sampler ");
                        this->writeName(var->mangledName());
                        this->write(kSamplerSuffix);
                        this->write(" [[sampler(");
                        this->write(std::to_string(binding));
                        this->write(")]]");
                    } else {
                        SkASSERT(varKind == Type::TypeKind::kTexture);
                        this->writeType(var->type());
                        this->write(" ");
                        this->writeName(var->mangledName());
                        this->write(" [[texture(");
                        this->write(std::to_string(binding));
                        this->write(")]]");
                    }
                } else if (ProgramConfig::IsCompute(fProgram.fConfig->fKind)) {
                    std::string_view attr;
                    switch (var->layout().fBuiltin) {
                        case SK_NUMWORKGROUPS_BUILTIN:
                            attr = " [[threadgroups_per_grid]]";
                            break;
                        case SK_WORKGROUPID_BUILTIN:
                            attr = " [[threadgroup_position_in_grid]]";
                            break;
                        case SK_LOCALINVOCATIONID_BUILTIN:
                            attr = " [[thread_position_in_threadgroup]]";
                            break;
                        case SK_GLOBALINVOCATIONID_BUILTIN:
                            attr = " [[thread_position_in_grid]]";
                            break;
                        case SK_LOCALINVOCATIONINDEX_BUILTIN:
                            attr = " [[thread_index_in_threadgroup]]";
                            break;
                        default:
                            break;
                    }
                    if (!attr.empty()) {
                        this->write(separator);
                        this->writeType(var->type());
                        this->write(" ");
                        this->write(var->name());
                        this->write(attr);
                        separator = ", ";
                    }
                }
            } else if (e->is<InterfaceBlock>()) {
                const InterfaceBlock& intf = e->as<InterfaceBlock>();
                if (intf.typeName() == "sk_PerVertex") {
                    continue;
                }
                this->write(separator);
                if (is_readonly(intf)) {
                    this->write("const ");
                }
                this->write(is_buffer(intf) ? "device " : "constant ");
                this->writeType(intf.var()->type());
                this->write("& " );
                this->write(fInterfaceBlockNameMap[&intf.var()->type()]);
                this->write(" [[buffer(");
                this->write(std::to_string(this->getUniformBinding(intf.var()->layout())));
                this->write(")]]");
                separator = ", ";
            }
        }
        if (ProgramConfig::IsFragment(fProgram.fConfig->fKind)) {
            if (fProgram.fInterface.fUseFlipRTUniform && fInterfaceBlockNameMap.empty()) {
                this->write(separator);
                this->write("constant sksl_synthetic_uniforms& _anonInterface0 [[buffer(1)]]");
                fRTFlipName = "_anonInterface0." SKSL_RTFLIP_NAME;
                separator = ", ";
            }
            this->write(separator);
            this->write("bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]");
            if (this->requirements(f) & kSampleMaskIn_Requirement) {
                this->write(", uint sk_SampleMaskIn [[sample_mask]]");
            }
            if (fProgram.fInterface.fUseLastFragColor) {
                this->write(", half4 " + std::string(fContext.fCaps->fFBFetchColorName) +
                            " [[color(0)]]\n");
            }
            separator = ", ";
        } else if (ProgramConfig::IsVertex(fProgram.fConfig->fKind)) {
            this->write(separator);
            this->write("uint sk_VertexID [[vertex_id]], uint sk_InstanceID [[instance_id]]");
            separator = ", ";
        }
    } else {
        this->writeType(f.returnType());
        this->write(" ");
        this->writeName(f.mangledName());
        this->write("(");
        this->writeFunctionRequirementParams(f, separator);
    }
    for (const Variable* param : f.parameters()) {
        // This is a workaround for our test files. They use the runtime effect signature, so main
        // takes a coords parameter. We detect these at IR generation time, and we omit them from
        // the declaration here, so the function is valid Metal. (Well, valid as long as the
        // coordinates aren't actually referenced.)
        if (f.isMain() && param == f.getMainCoordsParameter()) {
            continue;
        }
        this->write(separator);
        separator = ", ";
        this->writeModifiers(param->modifierFlags());
        this->writeType(param->type());
        if (pass_by_reference(param->type(), param->modifierFlags())) {
            this->write("&");
        }
        this->write(" ");
        this->writeName(param->mangledName());
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
    for (int index = block.size(); index--; ) {
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

void MetalCodeGenerator::writeComputeMainInputs() {
    // Compute shaders only have input variables (e.g. sk_GlobalInvocationID) and access program
    // inputs/outputs via the Globals and Uniforms structs. We collect the allowed "in" parameters
    // into an Input struct here, since the rest of the code expects the normal _in / _out pattern.
    this->write("Inputs _in = { ");
    const char* separator = "";
    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const GlobalVarDeclaration& decls = e->as<GlobalVarDeclaration>();
            const Variable* var = decls.varDeclaration().var();
            if (is_input(*var)) {
                this->write(separator);
                separator = ", ";
                this->writeName(var->mangledName());
            }
        }
    }
    this->writeLine(" };");
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
        fIndentation++;
        this->writeGlobalInit();
        if (ProgramConfig::IsCompute(fProgram.fConfig->fKind)) {
            this->writeThreadgroupInit();
            this->writeComputeMainInputs();
        }
        else {
            this->writeLine("Outputs _out;");
            this->writeLine("(void)_out;");
        }
        fIndentation--;
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

void MetalCodeGenerator::writeModifiers(ModifierFlags flags) {
    if (ProgramConfig::IsCompute(fProgram.fConfig->fKind) &&
        (flags & (ModifierFlag::kIn | ModifierFlag::kOut))) {
        this->write("device ");
    } else if (flags & ModifierFlag::kOut) {
        this->write("thread ");
    }
    if (flags.isConst()) {
        this->write("const ");
    }
}

void MetalCodeGenerator::writeInterfaceBlock(const InterfaceBlock& intf) {
    if (intf.typeName() == "sk_PerVertex") {
        return;
    }
    const Type* structType = &intf.var()->type().componentType();
    this->writeModifiers(intf.var()->modifierFlags());
    this->write("struct ");
    this->writeType(*structType);
    this->writeLine(" {");
    fIndentation++;
    this->writeFields(structType->fields(), structType->fPosition);
    if (fProgram.fInterface.fUseFlipRTUniform) {
        this->writeLine("float2 " SKSL_RTFLIP_NAME ";");
    }
    fIndentation--;
    this->write("}");
    if (intf.instanceName().size()) {
        this->write(" ");
        this->write(intf.instanceName());
        if (intf.arraySize() > 0) {
            this->write("[");
            this->write(std::to_string(intf.arraySize()));
            this->write("]");
        }
        fInterfaceBlockNameMap.set(&intf.var()->type(), std::string(intf.instanceName()));
    } else {
        fInterfaceBlockNameMap.set(&intf.var()->type(),
                                   "_anonInterface" + std::to_string(fAnonInterfaceCount++));
    }
    this->writeLine(";");
}

void MetalCodeGenerator::writeFields(SkSpan<const Field> fields, Position parentPos) {
    MemoryLayout memoryLayout(MemoryLayout::Standard::kMetal);
    int currentOffset = 0;
    for (const Field& field : fields) {
        int fieldOffset = field.fLayout.fOffset;
        const Type* fieldType = field.fType;
        if (!memoryLayout.isSupported(*fieldType)) {
            fContext.fErrors->error(parentPos, "type '" + std::string(fieldType->name()) +
                                                "' is not permitted here");
            return;
        }
        if (fieldOffset != -1) {
            if (currentOffset > fieldOffset) {
                fContext.fErrors->error(field.fPosition,
                                        "offset of field '" + std::string(field.fName) +
                                        "' must be at least " + std::to_string(currentOffset));
                return;
            } else if (currentOffset < fieldOffset) {
                this->write("char pad");
                this->write(std::to_string(fPaddingCount++));
                this->write("[");
                this->write(std::to_string(fieldOffset - currentOffset));
                this->writeLine("];");
                currentOffset = fieldOffset;
            }
            int alignment = memoryLayout.alignment(*fieldType);
            if (fieldOffset % alignment) {
                fContext.fErrors->error(field.fPosition,
                                        "offset of field '" + std::string(field.fName) +
                                        "' must be a multiple of " + std::to_string(alignment));
                return;
            }
        }
        if (fieldType->isUnsizedArray()) {
            // An unsized array always appears as the last member of a storage block. We declare
            // it as a one-element array and allow dereferencing past the capacity.
            // TODO(armansito): This is because C++ does not support flexible array members like C99
            // does. This generally works but it can lead to UB as compilers are free to insert
            // padding past the first element of the array. An alternative approach is to declare
            // the struct without the unsized array member and replace variable references with a
            // buffer offset calculation based on sizeof().
            this->writeModifiers(field.fModifierFlags);
            this->writeType(fieldType->componentType());
            this->write(" ");
            this->writeName(field.fName);
            this->write("[1]");
        } else {
            size_t fieldSize = memoryLayout.size(*fieldType);
            if (fieldSize > static_cast<size_t>(std::numeric_limits<int>::max() - currentOffset)) {
                fContext.fErrors->error(parentPos, "field offset overflow");
                return;
            }
            currentOffset += fieldSize;
            this->writeModifiers(field.fModifierFlags);
            this->writeType(*fieldType);
            this->write(" ");
            this->writeName(field.fName);
        }
        this->writeLine(";");
    }
}

void MetalCodeGenerator::writeVarInitializer(const Variable& var, const Expression& value) {
    this->writeExpression(value, Precedence::kExpression);
}

void MetalCodeGenerator::writeName(std::string_view name) {
    if (fReservedWords.contains(name)) {
        this->write("_"); // adding underscore before name to avoid conflict with reserved words
    }
    this->write(name);
}

void MetalCodeGenerator::writeVarDeclaration(const VarDeclaration& varDecl) {
    this->writeModifiers(varDecl.var()->modifierFlags());
    this->writeType(varDecl.var()->type());
    this->write(" ");
    this->writeName(varDecl.var()->mangledName());
    if (varDecl.value()) {
        this->write(" = ");
        this->writeVarInitializer(*varDecl.var(), *varDecl.value());
    }
    this->write(";");
}

void MetalCodeGenerator::writeStatement(const Statement& s) {
    switch (s.kind()) {
        case Statement::Kind::kBlock:
            this->writeBlock(s.as<Block>());
            break;
        case Statement::Kind::kExpression:
            this->writeExpressionStatement(s.as<ExpressionStatement>());
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
    this->writeExpression(*stmt.test(), Precedence::kExpression);
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
        this->writeExpression(*f.test(), Precedence::kExpression);
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
        this->writeExpression(*f.test(), Precedence::kExpression);
    }
    this->write("; ");
    if (f.next()) {
        this->writeExpression(*f.next(), Precedence::kExpression);
    }
    this->write(") ");
    this->writeStatement(*f.statement());
}

void MetalCodeGenerator::writeDoStatement(const DoStatement& d) {
    this->write("do ");
    this->writeStatement(*d.statement());
    this->write(" while (");
    this->writeExpression(*d.test(), Precedence::kExpression);
    this->write(");");
}

void MetalCodeGenerator::writeExpressionStatement(const ExpressionStatement& s) {
    if (fProgram.fConfig->fSettings.fOptimize && !Analysis::HasSideEffects(*s.expression())) {
        // Don't emit dead expressions.
        return;
    }
    this->writeExpression(*s.expression(), Precedence::kStatement);
    this->write(";");
}

void MetalCodeGenerator::writeSwitchStatement(const SwitchStatement& s) {
    this->write("switch (");
    this->writeExpression(*s.value(), Precedence::kExpression);
    this->writeLine(") {");
    fIndentation++;
    for (const std::unique_ptr<Statement>& stmt : s.cases()) {
        const SwitchCase& c = stmt->as<SwitchCase>();
        if (c.isDefault()) {
            this->writeLine("default:");
        } else {
            this->write("case ");
            this->write(std::to_string(c.value()));
            this->writeLine(":");
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
    if (ProgramConfig::IsVertex(fProgram.fConfig->fKind) ||
        ProgramConfig::IsFragment(fProgram.fConfig->fKind)) {
        this->write("return _out;");
    } else if (ProgramConfig::IsCompute(fProgram.fConfig->fKind)) {
        this->write("return;");
    } else {
        SkDEBUGFAIL("unsupported kind of program");
    }
}

void MetalCodeGenerator::writeReturnStatement(const ReturnStatement& r) {
    if (fCurrentFunction && fCurrentFunction->isMain()) {
        if (r.expression()) {
            if (r.expression()->type().matches(*fContext.fTypes.fHalf4)) {
                this->write("_out.sk_FragColor = ");
                this->writeExpression(*r.expression(), Precedence::kExpression);
                this->writeLine(";");
            } else {
                fContext.fErrors->error(r.fPosition,
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
        this->writeExpression(*r.expression(), Precedence::kExpression);
    }
    this->write(";");
}

void MetalCodeGenerator::writeHeader() {
    this->writeLine("#include <metal_stdlib>");
    this->writeLine("#include <simd/simd.h>");
    this->writeLine("#ifdef __clang__");
    this->writeLine("#pragma clang diagnostic ignored \"-Wall\"");
    this->writeLine("#endif");
    this->writeLine("using namespace metal;");
}

void MetalCodeGenerator::writeSampler2DPolyfill() {
    class : public GlobalStructVisitor {
    public:
        void visitSampler(const Type&, std::string_view) override {
            if (fWrotePolyfill) {
                return;
            }
            fWrotePolyfill = true;

            std::string polyfill = SkSL::String::printf(R"(
struct sampler2D {
    texture2d<half> tex;
    sampler smp;
};
half4 sample(sampler2D i, float2 p, float b=%g) { return i.tex.sample(i.smp, p, bias(b)); }
half4 sample(sampler2D i, float3 p, float b=%g) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }
half4 sampleLod(sampler2D i, float2 p, float lod) { return i.tex.sample(i.smp, p, level(lod)); }
half4 sampleLod(sampler2D i, float3 p, float lod) {
    return i.tex.sample(i.smp, p.xy / p.z, level(lod));
}
half4 sampleGrad(sampler2D i, float2 p, float2 dPdx, float2 dPdy) {
    return i.tex.sample(i.smp, p, gradient2d(dPdx, dPdy));
}

)",
                                                        fTextureBias,
                                                        fTextureBias);
            fCodeGen->write(polyfill.c_str());
        }

        MetalCodeGenerator* fCodeGen = nullptr;
        float fTextureBias = 0.0f;
        bool fWrotePolyfill = false;
    } visitor;

    visitor.fCodeGen = this;
    visitor.fTextureBias = fProgram.fConfig->fSettings.fSharpenTextures ? kSharpenTexturesBias
                                                                        : 0.0f;
    this->visitGlobalStruct(&visitor);
}

void MetalCodeGenerator::writeUniformStruct() {
    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const GlobalVarDeclaration& decls = e->as<GlobalVarDeclaration>();
            const Variable& var = *decls.varDeclaration().var();
            if (var.modifierFlags().isUniform()) {
                SkASSERT(var.type().typeKind() != Type::TypeKind::kSampler &&
                         var.type().typeKind() != Type::TypeKind::kTexture);
                int uniformSet = this->getUniformSet(var.layout());
                // Make sure that the program's uniform-set value is consistent throughout.
                if (-1 == fUniformBuffer) {
                    this->write("struct Uniforms {\n");
                    fUniformBuffer = uniformSet;
                } else if (uniformSet != fUniformBuffer) {
                    fContext.fErrors->error(decls.fPosition,
                            "Metal backend requires all uniforms to have the same "
                            "'layout(set=...)'");
                }
                this->write("    ");
                this->writeType(var.type());
                this->write(" ");
                this->writeName(var.mangledName());
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
            const Variable& var = *decls.varDeclaration().var();
            if (is_input(var)) {
                this->write("    ");
                if (ProgramConfig::IsCompute(fProgram.fConfig->fKind) &&
                    needs_address_space(var.type(), var.modifierFlags())) {
                    // TODO: address space support
                    this->write("device ");
                }
                this->writeType(var.type());
                if (pass_by_reference(var.type(), var.modifierFlags())) {
                    this->write("&");
                }
                this->write(" ");
                this->writeName(var.mangledName());
                if (-1 != var.layout().fLocation) {
                    if (ProgramConfig::IsVertex(fProgram.fConfig->fKind)) {
                        this->write("  [[attribute(" + std::to_string(var.layout().fLocation) +
                                    ")]]");
                    } else if (ProgramConfig::IsFragment(fProgram.fConfig->fKind)) {
                        this->write("  [[user(locn" + std::to_string(var.layout().fLocation) +
                                    ")]]");
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
    if (ProgramConfig::IsVertex(fProgram.fConfig->fKind)) {
        this->write("    float4 sk_Position [[position]];\n");
    } else if (ProgramConfig::IsFragment(fProgram.fConfig->fKind)) {
        this->write("    half4 sk_FragColor [[color(0)]];\n");
        if (fProgram.fInterface.fOutputSecondaryColor) {
            this->write("    half4 sk_SecondaryFragColor [[color(0), index(1)]];\n");
        }
    }
    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const GlobalVarDeclaration& decls = e->as<GlobalVarDeclaration>();
            const Variable& var = *decls.varDeclaration().var();
            if (var.layout().fBuiltin == SK_SAMPLEMASK_BUILTIN) {
                this->write("    uint sk_SampleMask [[sample_mask]];\n");
                continue;
            }
            if (is_output(var)) {
                this->write("    ");
                if (ProgramConfig::IsCompute(fProgram.fConfig->fKind) &&
                    needs_address_space(var.type(), var.modifierFlags())) {
                    // TODO: address space support
                    this->write("device ");
                }
                this->writeType(var.type());
                if (ProgramConfig::IsCompute(fProgram.fConfig->fKind) &&
                    pass_by_reference(var.type(), var.modifierFlags())) {
                    this->write("&");
                }
                this->write(" ");
                this->writeName(var.mangledName());

                int location = var.layout().fLocation;
                if (!ProgramConfig::IsCompute(fProgram.fConfig->fKind) && location < 0 &&
                        var.type().typeKind() != Type::TypeKind::kTexture) {
                    fContext.fErrors->error(var.fPosition,
                                            "Metal out variables must have 'layout(location=...)'");
                } else if (ProgramConfig::IsVertex(fProgram.fConfig->fKind)) {
                    this->write(" [[user(locn" + std::to_string(location) + ")]]");
                } else if (ProgramConfig::IsFragment(fProgram.fConfig->fKind)) {
                    this->write(" [[color(" + std::to_string(location) + ")");
                    int colorIndex = var.layout().fIndex;
                    if (colorIndex) {
                        this->write(", index(" + std::to_string(colorIndex) + ")");
                    }
                    this->write("]]");
                }
                this->write(";\n");
            }
        }
    }
    if (ProgramConfig::IsVertex(fProgram.fConfig->fKind)) {
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
    if (!wroteInterfaceBlock && fProgram.fInterface.fUseFlipRTUniform) {
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

void MetalCodeGenerator::writeConstantVariables() {
    class : public GlobalStructVisitor {
    public:
        void visitConstantVariable(const VarDeclaration& decl) override {
            fCodeGen->write("constant ");
            fCodeGen->writeVarDeclaration(decl);
            fCodeGen->finishLine();
        }

        MetalCodeGenerator* fCodeGen = nullptr;
    } visitor;

    visitor.fCodeGen = this;
    this->visitGlobalStruct(&visitor);
}

void MetalCodeGenerator::visitGlobalStruct(GlobalStructVisitor* visitor) {
    for (const ProgramElement* element : fProgram.elements()) {
        if (element->is<InterfaceBlock>()) {
            const auto* ib = &element->as<InterfaceBlock>();
            if (ib->typeName() != "sk_PerVertex") {
                visitor->visitInterfaceBlock(*ib, fInterfaceBlockNameMap[&ib->var()->type()]);
            }
            continue;
        }
        if (!element->is<GlobalVarDeclaration>()) {
            continue;
        }
        const GlobalVarDeclaration& global = element->as<GlobalVarDeclaration>();
        const VarDeclaration& decl = global.varDeclaration();
        const Variable& var = *decl.var();
        if (decl.baseType().typeKind() == Type::TypeKind::kSampler) {
            visitor->visitSampler(var.type(), var.mangledName());
            continue;
        }
        if (decl.baseType().typeKind() == Type::TypeKind::kTexture) {
            visitor->visitTexture(var.type(), var.mangledName());
            continue;
        }
        if (!(var.modifierFlags() & ~ModifierFlag::kConst) && var.layout().fBuiltin == -1) {
            if (is_in_globals(var)) {
                // Visit a regular global variable.
                visitor->visitNonconstantVariable(var, decl.value().get());
            } else {
                // Visit a constant-expression variable.
                SkASSERT(var.modifierFlags().isConst());
                visitor->visitConstantVariable(decl);
            }
        }
    }
}

void MetalCodeGenerator::writeGlobalStruct() {
    class : public GlobalStructVisitor {
    public:
        void visitInterfaceBlock(const InterfaceBlock& block,
                                 std::string_view blockName) override {
            this->addElement();
            fCodeGen->write("    ");
            if (is_readonly(block)) {
                fCodeGen->write("const ");
            }
            fCodeGen->write(is_buffer(block) ? "device " : "constant ");
            fCodeGen->write(block.typeName());
            fCodeGen->write("* ");
            fCodeGen->writeName(blockName);
            fCodeGen->write(";\n");
        }
        void visitTexture(const Type& type, std::string_view name) override {
            this->addElement();
            fCodeGen->write("    ");
            fCodeGen->writeType(type);
            fCodeGen->write(" ");
            fCodeGen->writeName(name);
            fCodeGen->write(";\n");
        }
        void visitSampler(const Type&, std::string_view name) override {
            this->addElement();
            fCodeGen->write("    sampler2D ");
            fCodeGen->writeName(name);
            fCodeGen->write(";\n");
        }
        void visitConstantVariable(const VarDeclaration& decl) override {
            // Constants aren't added to the global struct.
        }
        void visitNonconstantVariable(const Variable& var, const Expression* value) override {
            this->addElement();
            fCodeGen->write("    ");
            fCodeGen->writeModifiers(var.modifierFlags());
            fCodeGen->writeType(var.type());
            fCodeGen->write(" ");
            fCodeGen->writeName(var.mangledName());
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
                                 std::string_view blockName) override {
            this->addElement();
            fCodeGen->write("&");
            fCodeGen->writeName(blockName);
        }
        void visitTexture(const Type&, std::string_view name) override {
            this->addElement();
            fCodeGen->writeName(name);
        }
        void visitSampler(const Type&, std::string_view name) override {
            this->addElement();
            fCodeGen->write("{");
            fCodeGen->writeName(name);
            fCodeGen->write(kTextureSuffix);
            fCodeGen->write(", ");
            fCodeGen->writeName(name);
            fCodeGen->write(kSamplerSuffix);
            fCodeGen->write("}");
        }
        void visitConstantVariable(const VarDeclaration& decl) override {
            // Constant-expression variables aren't put in the global struct.
        }
        void visitNonconstantVariable(const Variable& var, const Expression* value) override {
            this->addElement();
            if (value) {
                fCodeGen->writeVarInitializer(var, *value);
            } else {
                fCodeGen->write("{}");
            }
        }
        void addElement() {
            if (fFirst) {
                fCodeGen->write("Globals _globals{");
                fFirst = false;
            } else {
                fCodeGen->write(", ");
            }
        }
        void finish() {
            if (!fFirst) {
                fCodeGen->writeLine("};");
                fCodeGen->writeLine("(void)_globals;");
            }
        }
        MetalCodeGenerator* fCodeGen = nullptr;
        bool fFirst = true;
    } visitor;

    visitor.fCodeGen = this;
    this->visitGlobalStruct(&visitor);
    visitor.finish();
}

void MetalCodeGenerator::visitThreadgroupStruct(ThreadgroupStructVisitor* visitor) {
    for (const ProgramElement* element : fProgram.elements()) {
        if (!element->is<GlobalVarDeclaration>()) {
            continue;
        }
        const GlobalVarDeclaration& global = element->as<GlobalVarDeclaration>();
        const VarDeclaration& decl = global.varDeclaration();
        const Variable& var = *decl.var();
        if (var.modifierFlags().isWorkgroup()) {
            SkASSERT(!decl.value());
            SkASSERT(!var.modifierFlags().isConst());
            visitor->visitNonconstantVariable(var);
        }
    }
}

void MetalCodeGenerator::writeThreadgroupStruct() {
    class : public ThreadgroupStructVisitor {
    public:
        void visitNonconstantVariable(const Variable& var) override {
            this->addElement();
            fCodeGen->write("    ");
            fCodeGen->writeModifiers(var.modifierFlags());
            fCodeGen->writeType(var.type());
            fCodeGen->write(" ");
            fCodeGen->writeName(var.mangledName());
            fCodeGen->write(";\n");
        }
        void addElement() {
            if (fFirst) {
                fCodeGen->write("struct Threadgroups {\n");
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
    this->visitThreadgroupStruct(&visitor);
    visitor.finish();
}

void MetalCodeGenerator::writeThreadgroupInit() {
    class : public ThreadgroupStructVisitor {
    public:
        void visitNonconstantVariable(const Variable& var) override {
            this->addElement();
            fCodeGen->write("{}");
        }
        void addElement() {
            if (fFirst) {
                fCodeGen->write("threadgroup Threadgroups _threadgroups{");
                fFirst = false;
            } else {
                fCodeGen->write(", ");
            }
        }
        void finish() {
            if (!fFirst) {
                fCodeGen->writeLine("};");
                fCodeGen->writeLine("(void)_threadgroups;");
            }
        }
        MetalCodeGenerator* fCodeGen = nullptr;
        bool fFirst = true;
    } visitor;

    visitor.fCodeGen = this;
    this->visitThreadgroupStruct(&visitor);
    visitor.finish();
}

void MetalCodeGenerator::writeProgramElement(const ProgramElement& e) {
    switch (e.kind()) {
        case ProgramElement::Kind::kExtension:
            break;
        case ProgramElement::Kind::kGlobalVar:
            break;
        case ProgramElement::Kind::kInterfaceBlock:
            // Handled in writeInterfaceBlocks; do nothing.
            break;
        case ProgramElement::Kind::kStructDefinition:
            // Handled in writeStructDefinitions; do nothing.
            break;
        case ProgramElement::Kind::kFunction:
            this->writeFunction(e.as<FunctionDefinition>());
            break;
        case ProgramElement::Kind::kFunctionPrototype:
            this->writeFunctionPrototype(e.as<FunctionPrototype>());
            break;
        case ProgramElement::Kind::kModifiers:
            // Not necessary in Metal; do nothing.
            break;
        default:
            SkDEBUGFAILF("unsupported program element: %s\n", e.description().c_str());
            break;
    }
}

MetalCodeGenerator::Requirements MetalCodeGenerator::requirements(const Statement* s) {
    class RequirementsVisitor : public ProgramVisitor {
    public:
        using ProgramVisitor::visitStatement;

        bool visitExpression(const Expression& e) override {
            switch (e.kind()) {
                case Expression::Kind::kFunctionCall: {
                    const FunctionCall& f = e.as<FunctionCall>();
                    fRequirements |= fCodeGen->requirements(f.function());
                    break;
                }
                case Expression::Kind::kFieldAccess: {
                    const FieldAccess& f = e.as<FieldAccess>();
                    if (f.ownerKind() == FieldAccess::OwnerKind::kAnonymousInterfaceBlock) {
                        fRequirements |= kGlobals_Requirement;
                        return false;  // don't recurse into the base variable
                    }
                    break;
                }
                case Expression::Kind::kVariableReference: {
                    const Variable& var = *e.as<VariableReference>().variable();

                    if (var.layout().fBuiltin == SK_FRAGCOORD_BUILTIN) {
                        fRequirements |= kGlobals_Requirement | kFragCoord_Requirement;
                    } else if (var.layout().fBuiltin == SK_SAMPLEMASKIN_BUILTIN) {
                        fRequirements |= kSampleMaskIn_Requirement;
                    } else if (var.layout().fBuiltin == SK_SAMPLEMASK_BUILTIN) {
                        fRequirements |= kOutputs_Requirement;
                    } else if (var.layout().fBuiltin == SK_VERTEXID_BUILTIN) {
                        fRequirements |= kVertexID_Requirement;
                    } else if (var.layout().fBuiltin == SK_INSTANCEID_BUILTIN) {
                        fRequirements |= kInstanceID_Requirement;
                    } else if (var.storage() == Variable::Storage::kGlobal) {
                        if (is_input(var)) {
                            fRequirements |= kInputs_Requirement;
                        } else if (is_output(var)) {
                            fRequirements |= kOutputs_Requirement;
                        } else if (is_uniforms(var)) {
                            fRequirements |= kUniforms_Requirement;
                        } else if (is_threadgroup(var)) {
                            fRequirements |= kThreadgroups_Requirement;
                        } else if (is_in_globals(var)) {
                            fRequirements |= kGlobals_Requirement;
                        }
                    }
                    break;
                }
                default:
                    break;
            }
            return INHERITED::visitExpression(e);
        }

        MetalCodeGenerator* fCodeGen;
        Requirements fRequirements = kNo_Requirements;
        using INHERITED = ProgramVisitor;
    };

    RequirementsVisitor visitor;
    if (s) {
        visitor.fCodeGen = this;
        visitor.visitStatement(*s);
    }
    return visitor.fRequirements;
}

MetalCodeGenerator::Requirements MetalCodeGenerator::requirements(const FunctionDeclaration& f) {
    Requirements* found = fRequirements.find(&f);
    if (!found) {
        fRequirements.set(&f, kNo_Requirements);
        for (const ProgramElement* e : fProgram.elements()) {
            if (e->is<FunctionDefinition>()) {
                const FunctionDefinition& def = e->as<FunctionDefinition>();
                if (&def.declaration() == &f) {
                    Requirements reqs = this->requirements(def.body().get());
                    fRequirements.set(&f, reqs);
                    return reqs;
                }
            }
        }
        // We never found a definition for this declared function, but it's legal to prototype a
        // function without ever giving a definition, as long as you don't call it.
        return kNo_Requirements;
    }
    return *found;
}

bool MetalCodeGenerator::generateCode() {
    StringStream header;
    {
        AutoOutputStream outputToHeader(this, &header, &fIndentation);
        this->writeHeader();
        this->writeConstantVariables();
        this->writeSampler2DPolyfill();
        this->writeStructDefinitions();
        this->writeUniformStruct();
        this->writeInputStruct();
        if (!ProgramConfig::IsCompute(fProgram.fConfig->fKind)) {
            this->writeOutputStruct();
        }
        this->writeInterfaceBlocks();
        this->writeGlobalStruct();
        this->writeThreadgroupStruct();

        // Emit prototypes for every built-in function; these aren't always added in perfect order.
        for (const ProgramElement* e : fProgram.fSharedElements) {
            if (e->is<FunctionDefinition>()) {
                this->writeFunctionDeclaration(e->as<FunctionDefinition>().declaration());
                this->writeLine(";");
            }
        }
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
    return fContext.fErrors->errorCount() == 0;
}

}  // namespace SkSL
