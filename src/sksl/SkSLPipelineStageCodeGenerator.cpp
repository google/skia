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
        : INHERITED(context, program, errors, out), fArgs(outArgs) {}

void PipelineStageCodeGenerator::writeHeader() {
}

bool PipelineStageCodeGenerator::usesPrecisionModifiers() const {
    return false;
}

String PipelineStageCodeGenerator::getTypeName(const Type& type) {
    return type.name();
}

void PipelineStageCodeGenerator::writeFunctionCall(const FunctionCall& c) {
    const FunctionDeclaration& function = c.function();
    const ExpressionArray& arguments = c.arguments();
    if (function.isBuiltin() && function.name() == "sample" &&
        arguments[0]->type().typeKind() != Type::TypeKind::kSampler) {
        SkASSERT(arguments.size() <= 2);
        SkDEBUGCODE(const Type& arg0Type = arguments[0]->type());
        SkASSERT("fragmentProcessor"  == arg0Type.name() ||
                 "fragmentProcessor?" == arg0Type.name());
        SkASSERT(arguments[0]->is<VariableReference>());
        int index = 0;
        bool found = false;
        for (const ProgramElement* p : fProgram.elements()) {
            if (p->is<GlobalVarDeclaration>()) {
                const GlobalVarDeclaration& global = p->as<GlobalVarDeclaration>();
                const VarDeclaration& decl = global.declaration()->as<VarDeclaration>();
                if (&decl.var() == arguments[0]->as<VariableReference>().variable()) {
                    found = true;
                } else if (decl.var().type() == *fContext.fFragmentProcessor_Type) {
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
        INHERITED::writeFunctionCall(c);
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

void PipelineStageCodeGenerator::writeIntLiteral(const IntLiteral& i) {
    this->write(to_string((int32_t) i.value()));
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
                            var.type().nonnullable() != *fContext.fFragmentProcessor_Type) {
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

void PipelineStageCodeGenerator::writeIfStatement(const IfStatement& s) {
    if (s.isStatic()) {
        this->write("@");
    }
    INHERITED::writeIfStatement(s);
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

void PipelineStageCodeGenerator::writeSwitchStatement(const SwitchStatement& s) {
    if (s.isStatic()) {
        this->write("@");
    }
    INHERITED::writeSwitchStatement(s);
}

void PipelineStageCodeGenerator::writeFunction(const FunctionDefinition& f) {
    fFunctionHeader = "";
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
        this->write(fFunctionHeader);
        this->write(buffer.str());
    } else {
        {
            AutoOutputStream streamToBuffer(this, &buffer);
            const FunctionDeclaration& decl = f.declaration();
            if (!type_to_grsltype(fContext, decl.returnType(), &result.fReturnType)) {
                fErrors.error(f.fOffset, "unsupported return type");
                return;
            }
            result.fName = decl.name();
            for (const Variable* v : decl.parameters()) {
                GrSLType paramSLType;
                if (!type_to_grsltype(fContext, v->type(), &paramSLType)) {
                    fErrors.error(v->fOffset, "unsupported parameter type");
                    return;
                }
                result.fParameters.emplace_back(v->name(), paramSLType);
            }
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

void PipelineStageCodeGenerator::writeProgramElement(const ProgramElement& p) {
    if (p.is<Section>()) {
        return;
    }
    if (p.is<GlobalVarDeclaration>()) {
        const GlobalVarDeclaration& global = p.as<GlobalVarDeclaration>();
        const Variable& var = global.declaration()->as<VarDeclaration>().var();
        if (var.modifiers().fFlags &
                    (Modifiers::kIn_Flag | Modifiers::kUniform_Flag | Modifiers::kVarying_Flag) ||
            var.modifiers().fLayout.fBuiltin == -1) {
            return;
        }
    }
    INHERITED::writeProgramElement(p);
}

}  // namespace SkSL
#endif
