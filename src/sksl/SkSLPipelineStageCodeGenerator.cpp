/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLPipelineStageCodeGenerator.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLHCodeGenerator.h"

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

namespace SkSL {

PipelineStageCodeGenerator::PipelineStageCodeGenerator(const Context* context,
                                                       const Program* program,
                                                       ErrorReporter* errors,
                                                       OutputStream* out,
                                                       PipelineStageArgs* outArgs)
        : INHERITED(program, errors, out), fContext(*context), fArgs(outArgs) {}

void PipelineStageCodeGenerator::write(const char* s) {
    fOut->writeText(s);
}

void PipelineStageCodeGenerator::writeLine(const char* s) {
    if (s) {
        fOut->writeText(s);
    }
    fOut->writeText("\n");
}

void PipelineStageCodeGenerator::write(const String& s) {
    fOut->write(s.data(), s.length());
}

void PipelineStageCodeGenerator::write(StringFragment s) {
    fOut->write(s.fChars, s.fLength);
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
                } else if (decl.var().type() == *fContext.fTypes.fFragmentProcessor) {
                    ++index;
                }
            }
            if (found) {
                break;
            }
        }
        SkASSERT(found);
        size_t childCallIndex = fArgs->fFormatArgs.size();
        this->write(Compiler::kFormatArgPlaceholderStr);
        bool matrixCall = arguments.size() == 2 && arguments[1]->type().isMatrix();
        fArgs->fFormatArgs.push_back(Compiler::FormatArg(
                matrixCall ? Compiler::FormatArg::Kind::kChildProcessorWithMatrix
                           : Compiler::FormatArg::Kind::kChildProcessor,
                index));
        if (arguments.size() > 1) {
            StringStream buffer;
            AutoOutputStream outputToBuffer(this, &buffer);
            this->writeExpression(*arguments[1], kSequence_Precedence);
            fArgs->fFormatArgs[childCallIndex].fCoords = buffer.str();
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
            this->writeExpression(*arg, kSequence_Precedence);
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
        this->write(Compiler::kFormatArgPlaceholderStr);
        fArgs->fFormatArgs.push_back(
                Compiler::FormatArg(Compiler::FormatArg::Kind::kFunctionName, index));
        this->write("(");
        const char* separator = "";
        for (const std::unique_ptr<Expression>& arg : arguments) {
            this->write(separator);
            separator = ", ";
            this->writeExpression(*arg, kSequence_Precedence);
        }
        this->write(")");
    }
}

void PipelineStageCodeGenerator::writeVariableReference(const VariableReference& ref) {
    switch (ref.variable()->modifiers().fLayout.fBuiltin) {
        case SK_MAIN_COORDS_BUILTIN:
            this->write(Compiler::kFormatArgPlaceholderStr);
            fArgs->fFormatArgs.push_back(Compiler::FormatArg(Compiler::FormatArg::Kind::kCoords));
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
                            var.type() != *fContext.fTypes.fFragmentProcessor) {
                            ++index;
                        }
                    }
                }
                SkASSERT(found);
                return index;
            };

            if (ref.variable()->modifiers().fFlags & Modifiers::kUniform_Flag) {
                this->write(Compiler::kFormatArgPlaceholderStr);
                fArgs->fFormatArgs.push_back(
                        Compiler::FormatArg(Compiler::FormatArg::Kind::kUniform,
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
    this->writeExpression(*stmt.test(), kTopLevel_Precedence);
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
        this->writeExpression(*r.expression(), kTopLevel_Precedence);
        if (fCastReturnsToHalf) {
            this->write(")");
        }
    }
    this->write(";");
}

void PipelineStageCodeGenerator::writeFunction(const FunctionDefinition& f) {
    StringStream buffer;
    Compiler::GLSLFunction result;
    if (f.declaration().name() == "main") {
        {
            AutoOutputStream streamToBuffer(this, &buffer);
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
        }
        this->write(buffer.str());
    } else {
        {
            AutoOutputStream streamToBuffer(this, &buffer);
            result.fDecl = &f.declaration();
            for (const std::unique_ptr<Statement>& stmt : f.body()->as<Block>().children()) {
                this->writeStatement(*stmt);
                this->writeLine();
            }
        }
        result.fBody = buffer.str();
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
        this->writeExpression(*arg, kSequence_Precedence);
    }
    this->write(")");
}

void PipelineStageCodeGenerator::writeIndexExpression(const IndexExpression& expr) {
    this->writeExpression(*expr.base(), kPostfix_Precedence);
    this->write("[");
    this->writeExpression(*expr.index(), kTopLevel_Precedence);
    this->write("]");
}

void PipelineStageCodeGenerator::writeFieldAccess(const FieldAccess& f) {
    if (f.ownerKind() == FieldAccess::OwnerKind::kDefault) {
        this->writeExpression(*f.base(), kPostfix_Precedence);
        this->write(".");
    }
    const Type& baseType = f.base()->type();
    this->write(baseType.fields()[f.fieldIndex()].fName);
}

void PipelineStageCodeGenerator::writeSwizzle(const Swizzle& swizzle) {
    this->writeExpression(*swizzle.base(), kPostfix_Precedence);
    this->write(".");
    for (int c : swizzle.components()) {
        SkASSERT(c >= 0 && c <= 3);
        this->write(&("x\0y\0z\0w\0"[c * 2]));
    }
}

PipelineStageCodeGenerator::Precedence PipelineStageCodeGenerator::GetBinaryPrecedence(Token::Kind op) {
    switch (op) {
        case Token::Kind::TK_STAR:         // fall through
        case Token::Kind::TK_SLASH:        // fall through
        case Token::Kind::TK_PERCENT:      return PipelineStageCodeGenerator::kMultiplicative_Precedence;
        case Token::Kind::TK_PLUS:         // fall through
        case Token::Kind::TK_MINUS:        return PipelineStageCodeGenerator::kAdditive_Precedence;
        case Token::Kind::TK_SHL:          // fall through
        case Token::Kind::TK_SHR:          return PipelineStageCodeGenerator::kShift_Precedence;
        case Token::Kind::TK_LT:           // fall through
        case Token::Kind::TK_GT:           // fall through
        case Token::Kind::TK_LTEQ:         // fall through
        case Token::Kind::TK_GTEQ:         return PipelineStageCodeGenerator::kRelational_Precedence;
        case Token::Kind::TK_EQEQ:         // fall through
        case Token::Kind::TK_NEQ:          return PipelineStageCodeGenerator::kEquality_Precedence;
        case Token::Kind::TK_BITWISEAND:   return PipelineStageCodeGenerator::kBitwiseAnd_Precedence;
        case Token::Kind::TK_BITWISEXOR:   return PipelineStageCodeGenerator::kBitwiseXor_Precedence;
        case Token::Kind::TK_BITWISEOR:    return PipelineStageCodeGenerator::kBitwiseOr_Precedence;
        case Token::Kind::TK_LOGICALAND:   return PipelineStageCodeGenerator::kLogicalAnd_Precedence;
        case Token::Kind::TK_LOGICALXOR:   return PipelineStageCodeGenerator::kLogicalXor_Precedence;
        case Token::Kind::TK_LOGICALOR:    return PipelineStageCodeGenerator::kLogicalOr_Precedence;
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
        case Token::Kind::TK_BITWISEOREQ:  return PipelineStageCodeGenerator::kAssignment_Precedence;
        case Token::Kind::TK_COMMA:        return PipelineStageCodeGenerator::kSequence_Precedence;
        default: SK_ABORT("unsupported binary operator");
    }
}

void PipelineStageCodeGenerator::writeBinaryExpression(const BinaryExpression& b,
                                                       Precedence parentPrecedence) {
    const Expression& left = *b.left();
    const Expression& right = *b.right();
    Token::Kind op = b.getOperator();

    Precedence precedence = GetBinaryPrecedence(op);
    if (precedence >= parentPrecedence) {
        this->write("(");
    }
    this->writeExpression(left, precedence);
    this->write(" ");
    this->write(Compiler::OperatorName(op));
    this->write(" ");
    this->writeExpression(right, precedence);
    if (precedence >= parentPrecedence) {
        this->write(")");
    }
}

void PipelineStageCodeGenerator::writeTernaryExpression(const TernaryExpression& t,
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

void PipelineStageCodeGenerator::writePrefixExpression(const PrefixExpression& p,
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

void PipelineStageCodeGenerator::writePostfixExpression(const PostfixExpression& p,
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
        this->writeExpression(*var.value(), kTopLevel_Precedence);
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
            this->writeExpression(*s.as<ExpressionStatement>().expression(), kTopLevel_Precedence);
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
        this->writeExpression(*f.test(), kTopLevel_Precedence);
    }
    this->write("; ");
    if (f.next()) {
        this->writeExpression(*f.next(), kTopLevel_Precedence);
    }
    this->write(") ");
    this->writeStatement(*f.statement());
}

bool PipelineStageCodeGenerator::generateCode() {
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

    write_stringstream(body, *rawOut);
    return 0 == fErrors.errorCount();
}

}  // namespace SkSL

#endif
