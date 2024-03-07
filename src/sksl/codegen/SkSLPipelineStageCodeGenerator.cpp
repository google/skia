/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/codegen/SkSLPipelineStageCodeGenerator.h"

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkEnumBitMask.h"
#include "src/core/SkTHash.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLContext.h"  // IWYU pragma: keep
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLIntrinsicList.h"
#include "src/sksl/SkSLOperator.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLChildCall.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
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

#include <memory>
#include <string_view>
#include <utility>

using namespace skia_private;

namespace SkSL {
namespace PipelineStage {

class PipelineStageCodeGenerator {
public:
    PipelineStageCodeGenerator(const Program& program,
                               const char* sampleCoords,
                               const char* inputColor,
                               const char* destColor,
                               Callbacks* callbacks)
            : fProgram(program)
            , fSampleCoords(sampleCoords)
            , fInputColor(inputColor)
            , fDestColor(destColor)
            , fCallbacks(callbacks) {}

    void generateCode();

private:
    using Precedence = OperatorPrecedence;

    void write(std::string_view s);
    void writeLine(std::string_view s = std::string_view());

    std::string typeName(const Type& type);
    void writeType(const Type& type);

    std::string functionName(const FunctionDeclaration& decl);
    void writeFunction(const FunctionDefinition& f);
    void writeFunctionDeclaration(const FunctionDeclaration& decl);

    std::string modifierString(ModifierFlags modifiers);
    std::string functionDeclaration(const FunctionDeclaration& decl);

    // Handles arrays correctly, eg: `float x[2]`
    std::string typedVariable(const Type& type, std::string_view name);

    void writeVarDeclaration(const VarDeclaration& var);
    void writeGlobalVarDeclaration(const GlobalVarDeclaration& g);
    void writeStructDefinition(const StructDefinition& s);

    void writeExpression(const Expression& expr, Precedence parentPrecedence);
    void writeChildCall(const ChildCall& c);
    void writeFunctionCall(const FunctionCall& c);
    void writeAnyConstructor(const AnyConstructor& c, Precedence parentPrecedence);
    void writeFieldAccess(const FieldAccess& f);
    void writeSwizzle(const Swizzle& swizzle);
    void writeBinaryExpression(const BinaryExpression& b, Precedence parentPrecedence);
    void writeTernaryExpression(const TernaryExpression& t, Precedence parentPrecedence);
    void writeIndexExpression(const IndexExpression& expr);
    void writePrefixExpression(const PrefixExpression& p, Precedence parentPrecedence);
    void writePostfixExpression(const PostfixExpression& p, Precedence parentPrecedence);
    void writeVariableReference(const VariableReference& ref);

    void writeStatement(const Statement& s);
    void writeBlock(const Block& b);
    void writeIfStatement(const IfStatement& stmt);
    void writeDoStatement(const DoStatement& d);
    void writeForStatement(const ForStatement& f);
    void writeReturnStatement(const ReturnStatement& r);
    void writeSwitchStatement(const SwitchStatement& s);

    void writeProgramElementFirstPass(const ProgramElement& e);
    void writeProgramElementSecondPass(const ProgramElement& e);

    struct AutoOutputBuffer {
        AutoOutputBuffer(PipelineStageCodeGenerator* generator) : fGenerator(generator) {
            fOldBuffer = fGenerator->fBuffer;
            fGenerator->fBuffer = &fBuffer;
        }

        ~AutoOutputBuffer() {
            fGenerator->fBuffer = fOldBuffer;
        }

        PipelineStageCodeGenerator* fGenerator;
        StringStream*               fOldBuffer;
        StringStream                fBuffer;
    };

    const Program& fProgram;
    const char*    fSampleCoords;
    const char*    fInputColor;
    const char*    fDestColor;
    Callbacks*     fCallbacks;

    THashMap<const Variable*, std::string>            fVariableNames;
    THashMap<const FunctionDeclaration*, std::string> fFunctionNames;
    THashMap<const Type*, std::string>                fStructNames;

    StringStream*              fBuffer = nullptr;
    bool                       fCastReturnsToHalf = false;
    const FunctionDeclaration* fCurrentFunction = nullptr;
};


void PipelineStageCodeGenerator::write(std::string_view s) {
    fBuffer->write(s.data(), s.length());
}

void PipelineStageCodeGenerator::writeLine(std::string_view s) {
    fBuffer->write(s.data(), s.length());
    fBuffer->writeText("\n");
}

void PipelineStageCodeGenerator::writeChildCall(const ChildCall& c) {
    const ExpressionArray& arguments = c.arguments();
    SkASSERT(!arguments.empty());
    int index = 0;
    bool found = false;
    for (const ProgramElement* p : fProgram.elements()) {
        if (p->is<GlobalVarDeclaration>()) {
            const GlobalVarDeclaration& global = p->as<GlobalVarDeclaration>();
            const VarDeclaration& decl = global.varDeclaration();
            if (decl.var() == &c.child()) {
                found = true;
            } else if (decl.var()->type().isEffectChild()) {
                ++index;
            }
        }
        if (found) {
            break;
        }
    }
    SkASSERT(found);

    // Shaders require a coordinate argument. Color filters require a color argument.
    // Blenders require two color arguments.
    std::string sampleOutput;
    {
        AutoOutputBuffer exprBuffer(this);
        this->writeExpression(*arguments[0], Precedence::kSequence);

        switch (c.child().type().typeKind()) {
            case Type::TypeKind::kShader: {
                SkASSERT(arguments.size() == 1);
                SkASSERT(arguments[0]->type().matches(*fProgram.fContext->fTypes.fFloat2));
                sampleOutput = fCallbacks->sampleShader(index, exprBuffer.fBuffer.str());
                break;
            }
            case Type::TypeKind::kColorFilter: {
                SkASSERT(arguments.size() == 1);
                SkASSERT(arguments[0]->type().matches(*fProgram.fContext->fTypes.fHalf4) ||
                         arguments[0]->type().matches(*fProgram.fContext->fTypes.fFloat4));
                sampleOutput = fCallbacks->sampleColorFilter(index, exprBuffer.fBuffer.str());
                break;
            }
            case Type::TypeKind::kBlender: {
                SkASSERT(arguments.size() == 2);
                SkASSERT(arguments[0]->type().matches(*fProgram.fContext->fTypes.fHalf4) ||
                         arguments[0]->type().matches(*fProgram.fContext->fTypes.fFloat4));
                SkASSERT(arguments[1]->type().matches(*fProgram.fContext->fTypes.fHalf4) ||
                         arguments[1]->type().matches(*fProgram.fContext->fTypes.fFloat4));

                AutoOutputBuffer exprBuffer2(this);
                this->writeExpression(*arguments[1], Precedence::kSequence);

                sampleOutput = fCallbacks->sampleBlender(index, exprBuffer.fBuffer.str(),
                                                                exprBuffer2.fBuffer.str());
                break;
            }
            default: {
                SkDEBUGFAILF("cannot sample from type '%s'",
                             c.child().type().description().c_str());
            }
        }
    }
    this->write(sampleOutput);
}

void PipelineStageCodeGenerator::writeFunctionCall(const FunctionCall& c) {
    const FunctionDeclaration& function = c.function();

    if (function.intrinsicKind() == IntrinsicKind::k_toLinearSrgb_IntrinsicKind ||
        function.intrinsicKind() == IntrinsicKind::k_fromLinearSrgb_IntrinsicKind) {
        SkASSERT(c.arguments().size() == 1);
        std::string colorArg;
        {
            AutoOutputBuffer exprBuffer(this);
            this->writeExpression(*c.arguments()[0], Precedence::kSequence);
            colorArg = exprBuffer.fBuffer.str();
        }

        switch (function.intrinsicKind()) {
            case IntrinsicKind::k_toLinearSrgb_IntrinsicKind:
                this->write(fCallbacks->toLinearSrgb(std::move(colorArg)));
                break;
            case IntrinsicKind::k_fromLinearSrgb_IntrinsicKind:
                this->write(fCallbacks->fromLinearSrgb(std::move(colorArg)));
                break;
            default:
                SkUNREACHABLE;
        }

        return;
    }

    if (function.isBuiltin()) {
        this->write(function.name());
    } else {
        this->write(this->functionName(function));
    }

    this->write("(");
    auto separator = SkSL::String::Separator();
    for (const auto& arg : c.arguments()) {
        this->write(separator());
        this->writeExpression(*arg, Precedence::kSequence);
    }
    this->write(")");
}

void PipelineStageCodeGenerator::writeVariableReference(const VariableReference& ref) {
    const Variable* var = ref.variable();

    if (fCurrentFunction && var == fCurrentFunction->getMainCoordsParameter()) {
        this->write(fSampleCoords);
        return;
    }
    if (fCurrentFunction && var == fCurrentFunction->getMainInputColorParameter()) {
        this->write(fInputColor);
        return;
    }
    if (fCurrentFunction && var == fCurrentFunction->getMainDestColorParameter()) {
        this->write(fDestColor);
        return;
    }

    std::string* name = fVariableNames.find(var);
    this->write(name ? *name : var->name());
}

void PipelineStageCodeGenerator::writeIfStatement(const IfStatement& stmt) {
    this->write("if (");
    this->writeExpression(*stmt.test(), Precedence::kExpression);
    this->write(") ");
    this->writeStatement(*stmt.ifTrue());
    if (stmt.ifFalse()) {
        this->write(" else ");
        this->writeStatement(*stmt.ifFalse());
    }
}

void PipelineStageCodeGenerator::writeReturnStatement(const ReturnStatement& r) {
    this->write("return");
    if (r.expression()) {
        this->write(" ");
        if (fCastReturnsToHalf) {
            this->write("half4(");
        }
        this->writeExpression(*r.expression(), Precedence::kExpression);
        if (fCastReturnsToHalf) {
            this->write(")");
        }
    }
    this->write(";");
}

void PipelineStageCodeGenerator::writeSwitchStatement(const SwitchStatement& s) {
    this->write("switch (");
    this->writeExpression(*s.value(), Precedence::kExpression);
    this->writeLine(") {");
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
            this->writeStatement(*c.statement());
            this->writeLine();
        }
    }
    this->writeLine();
    this->write("}");
}

std::string PipelineStageCodeGenerator::functionName(const FunctionDeclaration& decl) {
    if (decl.isMain()) {
        return std::string(fCallbacks->getMainName());
    }

    std::string* name = fFunctionNames.find(&decl);
    if (name) {
        return *name;
    }

    std::string mangledName = fCallbacks->getMangledName(std::string(decl.name()).c_str());
    fFunctionNames.set(&decl, mangledName);
    return mangledName;
}

void PipelineStageCodeGenerator::writeFunction(const FunctionDefinition& f) {
    if (f.declaration().isBuiltin()) {
        // Don't re-emit builtin functions.
        return;
    }

    SkASSERT(!fCurrentFunction);
    fCurrentFunction = &f.declaration();

    AutoOutputBuffer body(this);

    // We allow public SkSL's main() to return half4 -or- float4 (ie vec4). When we emit
    // our code in the processor, the surrounding code is going to expect half4, so we
    // explicitly cast any returns (from main) to half4. This is only strictly necessary
    // if the return type is float4 - injecting it unconditionally reduces the risk of an
    // obscure bug.
    const FunctionDeclaration& decl = f.declaration();
    if (decl.isMain() &&
        fProgram.fConfig->fKind != SkSL::ProgramKind::kMeshVertex &&
        fProgram.fConfig->fKind != SkSL::ProgramKind::kMeshFragment) {
        fCastReturnsToHalf = true;
    }

    for (const std::unique_ptr<Statement>& stmt : f.body()->as<Block>().children()) {
        this->writeStatement(*stmt);
        this->writeLine();
    }

    if (decl.isMain()) {
        fCastReturnsToHalf = false;
    }

    fCallbacks->defineFunction(this->functionDeclaration(decl).c_str(),
                               body.fBuffer.str().c_str(),
                               decl.isMain());

    fCurrentFunction = nullptr;
}

std::string PipelineStageCodeGenerator::functionDeclaration(const FunctionDeclaration& decl) {
    // This is similar to decl.description(), but substitutes a mangled name, and handles modifiers
    // on the function (e.g. `inline`) and its parameters (e.g. `inout`).
    std::string declString =
            String::printf("%s%s%s %s(",
                           decl.modifierFlags().isInline() ? "inline " : "",
                           decl.modifierFlags().isNoInline() ? "noinline " : "",
                           this->typeName(decl.returnType()).c_str(),
                           this->functionName(decl).c_str());
    auto separator = SkSL::String::Separator();
    for (const Variable* p : decl.parameters()) {
        declString.append(separator());
        declString.append(this->modifierString(p->modifierFlags()));
        declString.append(this->typedVariable(p->type(), p->name()).c_str());
    }

    return declString + ")";
}

void PipelineStageCodeGenerator::writeFunctionDeclaration(const FunctionDeclaration& decl) {
    if (!decl.isMain() && !decl.isBuiltin()) {
        std::string prototype = this->functionDeclaration(decl) + ';';
        fCallbacks->declareFunction(prototype.c_str());
    }
}

void PipelineStageCodeGenerator::writeGlobalVarDeclaration(const GlobalVarDeclaration& g) {
    const VarDeclaration& decl = g.varDeclaration();
    const Variable& var = *decl.var();

    if (var.isBuiltin() || var.type().isOpaque()) {
        // Don't re-declare these. (eg, sk_FragCoord, or fragmentProcessor children)
    } else if (var.modifierFlags().isUniform()) {
        std::string uniformName = fCallbacks->declareUniform(&decl);
        fVariableNames.set(&var, std::move(uniformName));
    } else {
        std::string mangledName = fCallbacks->getMangledName(std::string(var.name()).c_str());
        std::string declaration = this->modifierString(var.modifierFlags()) +
                                  this->typedVariable(var.type(), mangledName);
        if (decl.value()) {
            AutoOutputBuffer outputToBuffer(this);
            this->writeExpression(*decl.value(), Precedence::kExpression);
            declaration += " = ";
            declaration += outputToBuffer.fBuffer.str();
        }
        declaration += ";\n";
        fCallbacks->declareGlobal(declaration.c_str());
        fVariableNames.set(&var, std::move(mangledName));
    }
}

void PipelineStageCodeGenerator::writeStructDefinition(const StructDefinition& s) {
    const Type& type = s.type();
    std::string mangledName = fCallbacks->getMangledName(type.displayName().c_str());
    std::string definition = "struct " + mangledName + " {\n";
    for (const auto& f : type.fields()) {
        definition += this->typedVariable(*f.fType, f.fName) + ";\n";
    }
    definition += "};\n";
    fStructNames.set(&type, std::move(mangledName));
    fCallbacks->defineStruct(definition.c_str());
}

void PipelineStageCodeGenerator::writeProgramElementFirstPass(const ProgramElement& e) {
    switch (e.kind()) {
        case ProgramElement::Kind::kGlobalVar:
            this->writeGlobalVarDeclaration(e.as<GlobalVarDeclaration>());
            break;
        case ProgramElement::Kind::kFunction:
            this->writeFunctionDeclaration(e.as<FunctionDefinition>().declaration());
            break;
        case ProgramElement::Kind::kFunctionPrototype:
            // Skip this; we're already emitting prototypes for every FunctionDefinition.
            // (See case kFunction, directly above.)
            break;
        case ProgramElement::Kind::kStructDefinition:
            this->writeStructDefinition(e.as<StructDefinition>());
            break;

        case ProgramElement::Kind::kExtension:
        case ProgramElement::Kind::kInterfaceBlock:
        case ProgramElement::Kind::kModifiers:
        default:
            SkDEBUGFAILF("unsupported program element %s\n", e.description().c_str());
            break;
    }
}

void PipelineStageCodeGenerator::writeProgramElementSecondPass(const ProgramElement& e) {
    if (e.is<FunctionDefinition>()) {
        this->writeFunction(e.as<FunctionDefinition>());
    }
}

std::string PipelineStageCodeGenerator::typeName(const Type& raw) {
    const Type& type = raw.resolve().scalarTypeForLiteral();
    if (type.isArray()) {
        // This is necessary so that name mangling on arrays-of-structs works properly.
        std::string arrayName = this->typeName(type.componentType());
        arrayName.push_back('[');
        arrayName += std::to_string(type.columns());
        arrayName.push_back(']');
        return arrayName;
    }

    std::string* name = fStructNames.find(&type);
    return name ? *name : std::string(type.name());
}

void PipelineStageCodeGenerator::writeType(const Type& type) {
    this->write(this->typeName(type));
}

void PipelineStageCodeGenerator::writeExpression(const Expression& expr,
                                                 Precedence parentPrecedence) {
    switch (expr.kind()) {
        case Expression::Kind::kBinary:
            this->writeBinaryExpression(expr.as<BinaryExpression>(), parentPrecedence);
            break;
        case Expression::Kind::kLiteral:
        case Expression::Kind::kSetting:
            this->write(expr.description());
            break;
        case Expression::Kind::kChildCall:
            this->writeChildCall(expr.as<ChildCall>());
            break;
        case Expression::Kind::kConstructorArray:
        case Expression::Kind::kConstructorArrayCast:
        case Expression::Kind::kConstructorCompound:
        case Expression::Kind::kConstructorCompoundCast:
        case Expression::Kind::kConstructorDiagonalMatrix:
        case Expression::Kind::kConstructorMatrixResize:
        case Expression::Kind::kConstructorScalarCast:
        case Expression::Kind::kConstructorSplat:
        case Expression::Kind::kConstructorStruct:
            this->writeAnyConstructor(expr.asAnyConstructor(), parentPrecedence);
            break;
        case Expression::Kind::kEmpty:
            this->write("false");
            break;
        case Expression::Kind::kFieldAccess:
            this->writeFieldAccess(expr.as<FieldAccess>());
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

void PipelineStageCodeGenerator::writeAnyConstructor(const AnyConstructor& c,
                                                     Precedence parentPrecedence) {
    this->writeType(c.type());
    this->write("(");
    auto separator = SkSL::String::Separator();
    for (const auto& arg : c.argumentSpan()) {
        this->write(separator());
        this->writeExpression(*arg, Precedence::kSequence);
    }
    this->write(")");
}

void PipelineStageCodeGenerator::writeIndexExpression(const IndexExpression& expr) {
    this->writeExpression(*expr.base(), Precedence::kPostfix);
    this->write("[");
    this->writeExpression(*expr.index(), Precedence::kExpression);
    this->write("]");
}

void PipelineStageCodeGenerator::writeFieldAccess(const FieldAccess& f) {
    if (f.ownerKind() == FieldAccess::OwnerKind::kDefault) {
        this->writeExpression(*f.base(), Precedence::kPostfix);
        this->write(".");
    }
    const Type& baseType = f.base()->type();
    this->write(baseType.fields()[f.fieldIndex()].fName);
}

void PipelineStageCodeGenerator::writeSwizzle(const Swizzle& swizzle) {
    this->writeExpression(*swizzle.base(), Precedence::kPostfix);
    this->write(".");
    this->write(Swizzle::MaskString(swizzle.components()));
}

void PipelineStageCodeGenerator::writeBinaryExpression(const BinaryExpression& b,
                                                       Precedence parentPrecedence) {
    const Expression& left = *b.left();
    const Expression& right = *b.right();
    Operator op = b.getOperator();

    Precedence precedence = op.getBinaryPrecedence();
    if (precedence >= parentPrecedence) {
        this->write("(");
    }
    this->writeExpression(left, precedence);
    this->write(op.operatorName());
    this->writeExpression(right, precedence);
    if (precedence >= parentPrecedence) {
        this->write(")");
    }
}

void PipelineStageCodeGenerator::writeTernaryExpression(const TernaryExpression& t,
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

void PipelineStageCodeGenerator::writePrefixExpression(const PrefixExpression& p,
                                                       Precedence parentPrecedence) {
    if (Precedence::kPrefix >= parentPrecedence) {
        this->write("(");
    }
    this->write(p.getOperator().tightOperatorName());
    this->writeExpression(*p.operand(), Precedence::kPrefix);
    if (Precedence::kPrefix >= parentPrecedence) {
        this->write(")");
    }
}

void PipelineStageCodeGenerator::writePostfixExpression(const PostfixExpression& p,
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

std::string PipelineStageCodeGenerator::modifierString(ModifierFlags flags) {
    std::string result;
    if (flags.isConst()) {
        result.append("const ");
    }
    if ((flags & ModifierFlag::kIn) && (flags & ModifierFlag::kOut)) {
        result.append("inout ");
    } else if (flags & ModifierFlag::kIn) {
        result.append("in ");
    } else if (flags & ModifierFlag::kOut) {
        result.append("out ");
    }

    return result;
}

std::string PipelineStageCodeGenerator::typedVariable(const Type& type, std::string_view name) {
    const Type& baseType = type.isArray() ? type.componentType() : type;

    std::string decl = this->typeName(baseType) + " " + std::string(name);
    if (type.isArray()) {
        decl += "[" + std::to_string(type.columns()) + "]";
    }
    return decl;
}

void PipelineStageCodeGenerator::writeVarDeclaration(const VarDeclaration& var) {
    this->write(this->modifierString(var.var()->modifierFlags()));
    this->write(this->typedVariable(var.var()->type(), var.var()->name()));
    if (var.value()) {
        this->write(" = ");
        this->writeExpression(*var.value(), Precedence::kExpression);
    }
    this->write(";");
}

void PipelineStageCodeGenerator::writeStatement(const Statement& s) {
    switch (s.kind()) {
        case Statement::Kind::kBlock:
            this->writeBlock(s.as<Block>());
            break;
        case Statement::Kind::kBreak:
            this->write("break;");
            break;
        case Statement::Kind::kContinue:
            this->write("continue;");
            break;
        case Statement::Kind::kExpression:
            this->writeExpression(*s.as<ExpressionStatement>().expression(),
                                  Precedence::kStatement);
            this->write(";");
            break;
        case Statement::Kind::kDo:
            this->writeDoStatement(s.as<DoStatement>());
            break;
        case Statement::Kind::kFor:
            this->writeForStatement(s.as<ForStatement>());
            break;
        case Statement::Kind::kIf:
            this->writeIfStatement(s.as<IfStatement>());
            break;
        case Statement::Kind::kReturn:
            this->writeReturnStatement(s.as<ReturnStatement>());
            break;
        case Statement::Kind::kSwitch:
            this->writeSwitchStatement(s.as<SwitchStatement>());
            break;
        case Statement::Kind::kVarDeclaration:
            this->writeVarDeclaration(s.as<VarDeclaration>());
            break;
        case Statement::Kind::kDiscard:
            SkDEBUGFAIL("Unsupported control flow");
            break;
        case Statement::Kind::kNop:
            this->write(";");
            break;
        default:
            SkDEBUGFAILF("unsupported statement: %s", s.description().c_str());
            break;
    }
}

void PipelineStageCodeGenerator::writeBlock(const Block& b) {
    // Write scope markers if this block is a scope, or if the block is empty (since we need to emit
    // something here to make the code valid).
    bool isScope = b.isScope() || b.isEmpty();
    if (isScope) {
        this->writeLine("{");
    }
    for (const std::unique_ptr<Statement>& stmt : b.children()) {
        if (!stmt->isEmpty()) {
            this->writeStatement(*stmt);
            this->writeLine();
        }
    }
    if (isScope) {
        this->write("}");
    }
}

void PipelineStageCodeGenerator::writeDoStatement(const DoStatement& d) {
    this->write("do ");
    this->writeStatement(*d.statement());
    this->write(" while (");
    this->writeExpression(*d.test(), Precedence::kExpression);
    this->write(");");
}

void PipelineStageCodeGenerator::writeForStatement(const ForStatement& f) {
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

void PipelineStageCodeGenerator::generateCode() {
    // Write all the program elements except for functions; prototype all the functions.
    for (const ProgramElement* e : fProgram.elements()) {
        this->writeProgramElementFirstPass(*e);
    }

    // We always place FunctionDefinition elements last, because the inliner likes to move function
    // bodies around. After inlining, code can inadvertently move upwards, above ProgramElements
    // that the code relies on.
    for (const ProgramElement* e : fProgram.elements()) {
        this->writeProgramElementSecondPass(*e);
    }
}

void ConvertProgram(const Program& program,
                    const char* sampleCoords,
                    const char* inputColor,
                    const char* destColor,
                    Callbacks* callbacks) {
    PipelineStageCodeGenerator generator(program, sampleCoords, inputColor, destColor, callbacks);
    generator.generateCode();
}

}  // namespace PipelineStage
}  // namespace SkSL
