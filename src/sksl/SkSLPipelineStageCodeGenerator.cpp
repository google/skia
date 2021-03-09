/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLPipelineStageCodeGenerator.h"

#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
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
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLStructDefinition.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#include <unordered_map>

#if defined(SKSL_STANDALONE) || SK_SUPPORT_GPU

namespace SkSL {
namespace PipelineStage {

class PipelineStageCodeGenerator {
public:
    PipelineStageCodeGenerator(const Program& program,
                               const char* sampleCoords,
                               Callbacks* callbacks)
            : fProgram(program)
            , fSampleCoords(sampleCoords)
            , fCallbacks(callbacks) {}

    void generateCode();

private:
    using Precedence = Operator::Precedence;

    void write(const char* s);
    void writeLine(const char* s = nullptr);
    void write(const String& s);
    void write(StringFragment s);

    String typeName(const Type& type);
    void writeType(const Type& type);

    void writeFunction(const FunctionDefinition& f);

    String modifierString(const Modifiers& modifiers);

    // Handles arrays correctly, eg: `float x[2]`
    String typedVariable(const Type& type, StringFragment name);

    void writeVarDeclaration(const VarDeclaration& var);
    void writeGlobalVarDeclaration(const GlobalVarDeclaration& g);
    void writeStructDefinition(const StructDefinition& s);

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

    const Program& fProgram;
    const char*    fSampleCoords;
    Callbacks*     fCallbacks;

    std::unordered_map<const Variable*, String>            fVariableNames;
    std::unordered_map<const FunctionDeclaration*, String> fFunctionNames;
    std::unordered_map<const Type*, String>                fStructNames;

    StringStream* fBuffer = nullptr;
    bool          fCastReturnsToHalf = false;
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
    if (function.isBuiltin() && function.name() == "sample") {
        SkASSERT(arguments.size() <= 2);
        SkASSERT("fragmentProcessor" == arguments[0]->type().name());
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

        String coordsOrMatrix;
        if (arguments.size() > 1) {
            AutoOutputBuffer outputToBuffer(this);
            this->writeExpression(*arguments[1], Precedence::kSequence);
            coordsOrMatrix = outputToBuffer.fBuffer.str();
        }

        bool matrixCall = arguments.size() == 2 && arguments[1]->type().isMatrix();
        if (matrixCall) {
            this->write(fCallbacks->sampleChildWithMatrix(index, std::move(coordsOrMatrix)));
        } else {
            this->write(fCallbacks->sampleChild(index, std::move(coordsOrMatrix)));
        }
        return;
    }

    if (function.isBuiltin()) {
        this->write(function.name());
    } else {
        auto it = fFunctionNames.find(&function);
        SkASSERT(it != fFunctionNames.end());
        this->write(it->second);
    }

    this->write("(");
    const char* separator = "";
    for (const auto& arg : arguments) {
        this->write(separator);
        separator = ", ";
        this->writeExpression(*arg, Precedence::kSequence);
    }
    this->write(")");
}

void PipelineStageCodeGenerator::writeVariableReference(const VariableReference& ref) {
    const Variable* var = ref.variable();
    const Modifiers& modifiers = var->modifiers();

    if (modifiers.fLayout.fBuiltin == SK_MAIN_COORDS_BUILTIN) {
        this->write(fSampleCoords);
        return;
    }

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

    if (modifiers.fFlags & Modifiers::kVarying_Flag) {
        this->write("_vtx_attr_");
        this->write(to_string(varIndexByFlag(Modifiers::kVarying_Flag)));
    } else {
        auto it = fVariableNames.find(var);
        if (it != fVariableNames.end()) {
            this->write(it->second);
        } else {
            this->write(var->name());
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
    AutoOutputBuffer body(this);

    // We allow public SkSL's main() to return half4 -or- float4 (ie vec4). When we emit
    // our code in the processor, the surrounding code is going to expect half4, so we
    // explicitly cast any returns (from main) to half4. This is only strictly necessary
    // if the return type is float4 - injecting it unconditionally reduces the risk of an
    // obscure bug.
    bool isMain = f.declaration().name() == "main";

    if (isMain) {
        fCastReturnsToHalf = true;
    }

    for (const std::unique_ptr<Statement>& stmt : f.body()->as<Block>().children()) {
        this->writeStatement(*stmt);
        this->writeLine();
    }

    if (isMain) {
        fCastReturnsToHalf = false;
    }

    String fnName =
            isMain ? "main" : fCallbacks->getMangledName(String(f.declaration().name()).c_str());

    // This is similar to decl->description(), but substitutes a mangled name, and handles
    // modifiers on parameters (eg inout).
    const FunctionDeclaration& decl = f.declaration();
    String declString =
            String::printf("%s %s(", this->typeName(decl.returnType()).c_str(), fnName.c_str());
    const char* separator = "";
    for (auto p : decl.parameters()) {
        // TODO: Handle arrays
        declString.appendf("%s%s%s %s",
                           separator,
                           this->modifierString(p->modifiers()).c_str(),
                           this->typeName(p->type()).c_str(),
                           String(p->name()).c_str());
        separator = ", ";
    }
    declString.append(")");

    fFunctionNames.insert({&f.declaration(), std::move(fnName)});
    fCallbacks->defineFunction(declString.c_str(), body.fBuffer.str().c_str(), isMain);
}

void PipelineStageCodeGenerator::writeGlobalVarDeclaration(const GlobalVarDeclaration& g) {
    const VarDeclaration& decl = g.declaration()->as<VarDeclaration>();
    const Variable& var = decl.var();

    if (var.isBuiltin() || var.type().isOpaque()) {
        // Don't re-declare these. (eg, sk_FragCoord, or fragmentProcessor children)
    } else if (var.modifiers().fFlags & Modifiers::kUniform_Flag) {
        String uniformName = fCallbacks->declareUniform(&decl);
        fVariableNames.insert({&var, std::move(uniformName)});
    } else {
        String mangledName = fCallbacks->getMangledName(String(var.name()).c_str());
        String declaration = this->modifierString(var.modifiers()) +
                             this->typedVariable(var.type(), StringFragment(mangledName.c_str()));
        if (decl.value()) {
            AutoOutputBuffer outputToBuffer(this);
            this->writeExpression(*decl.value(), Precedence::kTopLevel);
            declaration += " = ";
            declaration += outputToBuffer.fBuffer.str();
        }
        declaration += ";\n";
        fCallbacks->declareGlobal(declaration.c_str());
        fVariableNames.insert({&var, std::move(mangledName)});
    }
}

void PipelineStageCodeGenerator::writeStructDefinition(const StructDefinition& s) {
    const Type& type = s.type();
    String mangledName = fCallbacks->getMangledName(String(type.name()).c_str());
    String definition = "struct " + mangledName + " {\n";
    for (const auto& f : type.fields()) {
        definition += this->typedVariable(*f.fType, f.fName) + ";\n";
    }
    definition += "};\n";
    fStructNames.insert({&type, std::move(mangledName)});
    fCallbacks->defineStruct(definition.c_str());
}

void PipelineStageCodeGenerator::writeProgramElement(const ProgramElement& e) {
    switch (e.kind()) {
        case ProgramElement::Kind::kGlobalVar:
            this->writeGlobalVarDeclaration(e.as<GlobalVarDeclaration>());
            break;
        case ProgramElement::Kind::kFunction:
            this->writeFunction(e.as<FunctionDefinition>());
            break;
        case ProgramElement::Kind::kFunctionPrototype:
            // Runtime effects don't allow calls to undefined functions, so prototypes are never
            // necessary. If we do support them, they should emit calls to emitFunctionPrototype.
            break;
        case ProgramElement::Kind::kStructDefinition:
            this->writeStructDefinition(e.as<StructDefinition>());
            break;
        // Enums are ignored (so they don't yet work in runtime effects).
        // We need to emit their declarations (via callback), with name mangling support.
        case ProgramElement::Kind::kEnum:              // skbug.com/11296

        case ProgramElement::Kind::kExtension:
        case ProgramElement::Kind::kInterfaceBlock:
        case ProgramElement::Kind::kModifiers:
        case ProgramElement::Kind::kSection:
        default:
            SkDEBUGFAILF("unsupported program element %s\n", e.description().c_str());
            break;
    }
}

String PipelineStageCodeGenerator::typeName(const Type& type) {
    auto it = fStructNames.find(&type);
    return it != fStructNames.end() ? it->second : type.name();
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
    Operator op = b.getOperator();

    Precedence precedence = op.getBinaryPrecedence();
    if (precedence >= parentPrecedence) {
        this->write("(");
    }
    this->writeExpression(left, precedence);
    this->write(" ");
    this->write(op.operatorName());
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
    this->write(p.getOperator().operatorName());
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
    this->write(p.getOperator().operatorName());
    if (Precedence::kPostfix >= parentPrecedence) {
        this->write(")");
    }
}

String PipelineStageCodeGenerator::modifierString(const Modifiers& modifiers) {
    String result;
    if (modifiers.fFlags & Modifiers::kConst_Flag) {
        result.append("const ");
    }

    if ((modifiers.fFlags & Modifiers::kIn_Flag) && (modifiers.fFlags & Modifiers::kOut_Flag)) {
        result.append("inout ");
    } else if (modifiers.fFlags & Modifiers::kIn_Flag) {
        result.append("in ");
    } else if (modifiers.fFlags & Modifiers::kOut_Flag) {
        result.append("out ");
    }

    return result;
}

String PipelineStageCodeGenerator::typedVariable(const Type& type, StringFragment name) {
    const Type& baseType = type.isArray() ? type.componentType() : type;

    String decl = this->typeName(baseType) + " " + name;
    if (type.isArray()) {
        decl += "[" + to_string(type.columns()) + "]";
    }
    return decl;
}

void PipelineStageCodeGenerator::writeVarDeclaration(const VarDeclaration& var) {
    this->write(this->modifierString(var.var().modifiers()));
    this->write(this->typedVariable(var.var().type(), var.var().name()));
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
    // Write all the program elements except for functions.
    for (const ProgramElement* e : fProgram.elements()) {
        if (!e->is<FunctionDefinition>()) {
            this->writeProgramElement(*e);
        }
    }

    // We always place FunctionDefinition elements last, because the inliner likes to move function
    // bodies around. After inlining, code can inadvertently move upwards, above ProgramElements
    // that the code relies on.
    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<FunctionDefinition>()) {
            this->writeProgramElement(*e);
        }
    }
}

void ConvertProgram(const Program& program,
                    const char* sampleCoords,
                    Callbacks* callbacks) {
    PipelineStageCodeGenerator generator(program, sampleCoords, callbacks);
    generator.generateCode();
}

}  // namespace PipelineStage
}  // namespace SkSL

#endif
