/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLPipelineStageCodeGenerator.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLOperators.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

namespace SkSL {

class PipelineStageCodeGenerator {
public:
    PipelineStageCodeGenerator(const Program& program, PipelineStage::Args* outArgs)
            : fProgram(program), fArgs(outArgs) {}

    void generateCode();

private:
    using Precedence = Operators::Precedence;

    void write(const char* s);
    void writeLine(const char* s = nullptr);
    void write(const String& s);
    void write(StringFragment s);

    void writeType(const Type& type);

    void writeFunctionDeclaration(const FunctionDeclaration& f);
    void writeFunction(const FunctionDefinition& f);

    void writeModifiers(const Modifiers& modifiers);

    void writeVarDeclaration(const VarDeclaration& var);

    void writeExpression(const Expression& expr, Precedence parentPrecedence);
    void writeFunctionCall(const FunctionCall& c);
    void writeConstructor(const Constructor& c, Precedence parentPrecedence);
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
    void writeForStatement(const ForStatement& f);
    void writeReturnStatement(const ReturnStatement& r);

    void writeProgramElement(const ProgramElement& e);

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

    const Program&       fProgram;
    PipelineStage::Args* fArgs;
    StringStream*        fBuffer;
    bool                 fCastReturnsToHalf = false;
};

void PipelineStageCodeGenerator::write(const char* s) {
    fBuffer->writeText(s);
}

void PipelineStageCodeGenerator::writeLine(const char* s) {
    if (s) {
        fBuffer->writeText(s);
    }
    fBuffer->writeText("\n");
}

void PipelineStageCodeGenerator::write(const String& s) {
    fBuffer->write(s.data(), s.length());
}

void PipelineStageCodeGenerator::write(StringFragment s) {
    fBuffer->write(s.fChars, s.fLength);
}

void PipelineStageCodeGenerator::writeFunctionCall(const FunctionCall& c) {
    const FunctionDeclaration& function = c.function();
    const ExpressionArray& arguments = c.arguments();
    if (function.isBuiltin() && function.name() == "sample" &&
        arguments[0]->type().typeKind() != Type::TypeKind::kSampler) {
        SkASSERT(arguments.size() <= 2);
        SkDEBUGCODE(const Type& arg0Type = arguments[0]->type());
        SkASSERT("fragmentProcessor"  == arg0Type.name());
        SkASSERT(arguments[0]->is<VariableReference>());
        int index = 0;
        bool found = false;
        for (const ProgramElement* p : fProgram.elements()) {
            if (p->is<GlobalVarDeclaration>()) {
                const GlobalVarDeclaration& global = p->as<GlobalVarDeclaration>();
                const VarDeclaration& decl = global.declaration()->as<VarDeclaration>();
                if (&decl.var() == arguments[0]->as<VariableReference>().variable()) {
                    found = true;
                } else if (decl.var().type() == *fProgram.fContext->fTypes.fFragmentProcessor) {
                    ++index;
                }
            }
            if (found) {
                break;
            }
        }
        SkASSERT(found);
        size_t childCallIndex = fArgs->fFormatArgs.size();
        this->write(PipelineStage::kFormatArgPlaceholderStr);
        bool matrixCall = arguments.size() == 2 && arguments[1]->type().isMatrix();
        fArgs->fFormatArgs.push_back(PipelineStage::FormatArg(
                matrixCall ? PipelineStage::FormatArg::Kind::kChildProcessorWithMatrix
                           : PipelineStage::FormatArg::Kind::kChildProcessor,
                index));
        if (arguments.size() > 1) {
            AutoOutputBuffer outputToBuffer(this);
            this->writeExpression(*arguments[1], Precedence::kSequence);
            fArgs->fFormatArgs[childCallIndex].fCoords = outputToBuffer.fBuffer.str();
        }
        return;
    }
    if (function.isBuiltin()) {
        this->write(function.name());
        this->write("(");
        const char* separator = "";
        for (const auto& arg : arguments) {
            this->write(separator);
            separator = ", ";
            this->writeExpression(*arg, Precedence::kSequence);
        }
        this->write(")");
    } else {
        int index = 0;
        for (const ProgramElement* e : fProgram.elements()) {
            if (e->is<FunctionDefinition>()) {
                if (&e->as<FunctionDefinition>().declaration() == &function) {
                    break;
                }
                ++index;
            }
        }
        this->write(PipelineStage::kFormatArgPlaceholderStr);
        fArgs->fFormatArgs.push_back(
                PipelineStage::FormatArg(PipelineStage::FormatArg::Kind::kFunctionName, index));
        this->write("(");
        const char* separator = "";
        for (const std::unique_ptr<Expression>& arg : arguments) {
            this->write(separator);
            separator = ", ";
            this->writeExpression(*arg, Precedence::kSequence);
        }
        this->write(")");
    }
}

void PipelineStageCodeGenerator::writeVariableReference(const VariableReference& ref) {
    switch (ref.variable()->modifiers().fLayout.fBuiltin) {
        case SK_MAIN_COORDS_BUILTIN:
            this->write(PipelineStage::kFormatArgPlaceholderStr);
            fArgs->fFormatArgs.push_back(
                    PipelineStage::FormatArg(PipelineStage::FormatArg::Kind::kCoords));
            break;
        default: {
            auto varIndexByFlag = [this, &ref](uint32_t flag) {
                int index = 0;
                bool found = false;
                for (const ProgramElement* e : fProgram.elements()) {
                    if (found) {
                        break;
                    }
                    if (e->is<GlobalVarDeclaration>()) {
                        const GlobalVarDeclaration& global = e->as<GlobalVarDeclaration>();
                        const Variable& var = global.declaration()->as<VarDeclaration>().var();
                        if (&var == ref.variable()) {
                            found = true;
                            break;
                        }
                        // Skip over fragmentProcessors (shaders).
                        // These are indexed separately from other globals.
                        if (var.modifiers().fFlags & flag &&
                            var.type() != *fProgram.fContext->fTypes.fFragmentProcessor) {
                            ++index;
                        }
                    }
                }
                SkASSERT(found);
                return index;
            };

            if (ref.variable()->modifiers().fFlags & Modifiers::kUniform_Flag) {
                this->write(PipelineStage::kFormatArgPlaceholderStr);
                fArgs->fFormatArgs.push_back(
                        PipelineStage::FormatArg(PipelineStage::FormatArg::Kind::kUniform,
                                                 varIndexByFlag(Modifiers::kUniform_Flag)));
            } else if (ref.variable()->modifiers().fFlags & Modifiers::kVarying_Flag) {
                this->write("_vtx_attr_");
                this->write(to_string(varIndexByFlag(Modifiers::kVarying_Flag)));
            } else {
                this->write(ref.variable()->name());
            }
        }
    }
}

void PipelineStageCodeGenerator::writeIfStatement(const IfStatement& stmt) {
    if (stmt.isStatic()) {
        this->write("@");
    }
    this->write("if (");
    this->writeExpression(*stmt.test(), Precedence::kTopLevel);
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
        this->writeExpression(*r.expression(), Precedence::kTopLevel);
        if (fCastReturnsToHalf) {
            this->write(")");
        }
    }
    this->write(";");
}

void PipelineStageCodeGenerator::writeFunction(const FunctionDefinition& f) {
    PipelineStage::Function result;
    if (f.declaration().name() == "main") {
        // We allow public SkSL's main() to return half4 -or- float4 (ie vec4). When we emit
        // our code in the processor, the surrounding code is going to expect half4, so we
        // explicitly cast any returns (from main) to half4. This is only strictly necessary
        // if the return type is float4 - injecting it unconditionally reduces the risk of an
        // obscure bug.
        fCastReturnsToHalf = true;
        for (const std::unique_ptr<Statement>& stmt : f.body()->as<Block>().children()) {
            this->writeStatement(*stmt);
            this->writeLine();
        }
        fCastReturnsToHalf = false;
    } else {
        AutoOutputBuffer outputToBuffer(this);
        result.fDecl = &f.declaration();
        for (const std::unique_ptr<Statement>& stmt : f.body()->as<Block>().children()) {
            this->writeStatement(*stmt);
            this->writeLine();
        }

        result.fBody = outputToBuffer.fBuffer.str();
        result.fFormatArgs = std::move(fArgs->fFormatArgs);
        fArgs->fFunctions.push_back(std::move(result));
    }
}

void PipelineStageCodeGenerator::writeProgramElement(const ProgramElement& e) {
    switch (e.kind()) {
        case ProgramElement::Kind::kGlobalVar:
            // All global variables are manually re-declared by the FP
            break;
        case ProgramElement::Kind::kFunction:
            this->writeFunction(e.as<FunctionDefinition>());
            break;
        case ProgramElement::Kind::kFunctionPrototype:
            // Runtime effects don't allow calls to undefined functions, so prototypes are never
            // necessary. If we do support them, they should emit calls to emitFunctionPrototype.
            break;
        case ProgramElement::Kind::kModifiers: {
            const Modifiers& modifiers = e.as<ModifiersDeclaration>().modifiers();
            this->writeModifiers(modifiers);
            this->writeLine(";");
            break;
        }
        case ProgramElement::Kind::kEnum:
        case ProgramElement::Kind::kExtension:
        case ProgramElement::Kind::kInterfaceBlock:
        case ProgramElement::Kind::kSection:
        case ProgramElement::Kind::kStructDefinition:
        default:
            SkDEBUGFAILF("unsupported program element %s\n", e.description().c_str());
            break;
    }
}

void PipelineStageCodeGenerator::writeType(const Type& type) {
    this->write(type.name());
}

void PipelineStageCodeGenerator::writeExpression(const Expression& expr,
                                                 Precedence parentPrecedence) {
    switch (expr.kind()) {
        case Expression::Kind::kBinary:
            this->writeBinaryExpression(expr.as<BinaryExpression>(), parentPrecedence);
            break;
        case Expression::Kind::kBoolLiteral:
        case Expression::Kind::kFloatLiteral:
        case Expression::Kind::kIntLiteral:
            this->write(expr.description());
            break;
        case Expression::Kind::kConstructor:
            this->writeConstructor(expr.as<Constructor>(), parentPrecedence);
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
        case Expression::Kind::kSetting:
        default:
            SkDEBUGFAILF("unsupported expression: %s", expr.description().c_str());
            break;
    }
}

void PipelineStageCodeGenerator::writeConstructor(const Constructor& c,
                                                  Precedence parentPrecedence) {
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

void PipelineStageCodeGenerator::writeIndexExpression(const IndexExpression& expr) {
    this->writeExpression(*expr.base(), Precedence::kPostfix);
    this->write("[");
    this->writeExpression(*expr.index(), Precedence::kTopLevel);
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
    for (int c : swizzle.components()) {
        SkASSERT(c >= 0 && c <= 3);
        this->write(&("x\0y\0z\0w\0"[c * 2]));
    }
}

void PipelineStageCodeGenerator::writeBinaryExpression(const BinaryExpression& b,
                                                       Precedence parentPrecedence) {
    const Expression& left = *b.left();
    const Expression& right = *b.right();
    Token::Kind op = b.getOperator();

    Precedence precedence = Operators::GetBinaryPrecedence(op);
    if (precedence >= parentPrecedence) {
        this->write("(");
    }
    this->writeExpression(left, precedence);
    this->write(" ");
    this->write(Operators::OperatorName(op));
    this->write(" ");
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
    this->write(Operators::OperatorName(p.getOperator()));
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
    this->write(Operators::OperatorName(p.getOperator()));
    if (Precedence::kPostfix >= parentPrecedence) {
        this->write(")");
    }
}

void PipelineStageCodeGenerator::writeFunctionDeclaration(const FunctionDeclaration& f) {
    this->writeType(f.returnType());
    this->write(" " + f.name() + "(");
    const char* separator = "";
    for (const auto& param : f.parameters()) {
        this->write(separator);
        separator = ", ";
        this->writeModifiers(param->modifiers());
        std::vector<int> sizes;
        const Type* type = &param->type();
        if (type->isArray()) {
            sizes.push_back(type->columns());
            type = &type->componentType();
        }
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

void PipelineStageCodeGenerator::writeModifiers(const Modifiers& modifiers) {
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
        this->write("in ");
    } else if (modifiers.fFlags & Modifiers::kOut_Flag) {
        this->write("out ");
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
}

void PipelineStageCodeGenerator::writeVarDeclaration(const VarDeclaration& var) {
    this->writeModifiers(var.var().modifiers());
    this->writeType(var.baseType());
    this->write(" ");
    this->write(var.var().name());
    if (var.arraySize() > 0) {
        this->write("[");
        this->write(to_string(var.arraySize()));
        this->write("]");
    }
    if (var.value()) {
        this->write(" = ");
        this->writeExpression(*var.value(), Precedence::kTopLevel);
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
            this->writeExpression(*s.as<ExpressionStatement>().expression(), Precedence::kTopLevel);
            this->write(";");
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
        case Statement::Kind::kVarDeclaration:
            this->writeVarDeclaration(s.as<VarDeclaration>());
            break;
        case Statement::Kind::kDiscard:
        case Statement::Kind::kDo:
        case Statement::Kind::kSwitch:
            SkDEBUGFAIL("Unsupported control flow");
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

void PipelineStageCodeGenerator::writeForStatement(const ForStatement& f) {
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

void PipelineStageCodeGenerator::generateCode() {
    StringStream mainBody;
    fBuffer = &mainBody;

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

    fArgs->fCode = mainBody.str();
}

void PipelineStage::ConvertProgram(const Program& program, Args* outArgs) {
    PipelineStageCodeGenerator generator(program, outArgs);
    generator.generateCode();
}

}  // namespace SkSL

#endif
