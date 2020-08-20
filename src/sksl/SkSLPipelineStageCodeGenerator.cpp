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
    if (c.fFunction.fBuiltin && c.fFunction.fName == "sample" &&
        c.fArguments[0]->fType.kind() != Type::Kind::kSampler_Kind) {
        SkASSERT(c.fArguments.size() <= 2);
        SkASSERT("fragmentProcessor"  == c.fArguments[0]->fType.name() ||
                 "fragmentProcessor?" == c.fArguments[0]->fType.name());
        SkASSERT(Expression::kVariableReference_Kind == c.fArguments[0]->fKind);
        int index = 0;
        bool found = false;
        for (const auto& p : fProgram) {
            if (ProgramElement::kVar_Kind == p.fKind) {
                const VarDeclarations& decls = p.as<VarDeclarations>();
                for (const std::unique_ptr<Statement>& raw : decls.fVars) {
                    VarDeclaration& decl = raw->as<VarDeclaration>();
                    if (decl.fVar == &c.fArguments[0]->as<VariableReference>().fVariable) {
                        found = true;
                    } else if (decl.fVar->fType == *fContext.fFragmentProcessor_Type) {
                        ++index;
                    }
                }
            }
            if (found) {
                break;
            }
        }
        SkASSERT(found);
        size_t childCallIndex = fArgs->fFormatArgs.size();
        this->write(Compiler::kFormatArgPlaceholderStr);
        bool matrixCall =
                c.fArguments.size() == 2 && c.fArguments[1]->fType.kind() == Type::kMatrix_Kind;
        fArgs->fFormatArgs.push_back(Compiler::FormatArg(
                matrixCall ? Compiler::FormatArg::Kind::kChildProcessorWithMatrix
                           : Compiler::FormatArg::Kind::kChildProcessor,
                index));
        if (c.fArguments.size() > 1) {
            OutputStream* oldOut = fOut;
            StringStream buffer;
            fOut = &buffer;
            this->writeExpression(*c.fArguments[1], kSequence_Precedence);
            fOut = oldOut;
            fArgs->fFormatArgs[childCallIndex].fCoords = buffer.str();
        }
        return;
    }
    if (c.fFunction.fBuiltin) {
        INHERITED::writeFunctionCall(c);
    } else {
        int index = 0;
        for (const ProgramElement& e : fProgram) {
            if (e.fKind == ProgramElement::kFunction_Kind) {
                if (&e.as<FunctionDefinition>().fDeclaration == &c.fFunction) {
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
        for (const std::unique_ptr<Expression>& arg : c.fArguments) {
            this->write(separator);
            separator = ", ";
            this->writeExpression(*arg, kSequence_Precedence);
        }
        this->write(")");
    }
}

void PipelineStageCodeGenerator::writeIntLiteral(const IntLiteral& i) {
    this->write(to_string((int32_t) i.fValue));
}

void PipelineStageCodeGenerator::writeVariableReference(const VariableReference& ref) {
    switch (ref.fVariable.fModifiers.fLayout.fBuiltin) {
        case SK_OUTCOLOR_BUILTIN:
            this->write(Compiler::kFormatArgPlaceholderStr);
            fArgs->fFormatArgs.push_back(Compiler::FormatArg(Compiler::FormatArg::Kind::kOutput));
            break;
        case SK_MAIN_COORDS_BUILTIN:
            this->write(Compiler::kFormatArgPlaceholderStr);
            fArgs->fFormatArgs.push_back(Compiler::FormatArg(Compiler::FormatArg::Kind::kCoords));
            break;
        default: {
            auto varIndexByFlag = [this, &ref](uint32_t flag) {
                int index = 0;
                bool found = false;
                for (const ProgramElement& e : fProgram) {
                    if (found) {
                        break;
                    }
                    if (e.fKind == ProgramElement::Kind::kVar_Kind) {
                        const VarDeclarations& decls = e.as<VarDeclarations>();
                        for (const auto& decl : decls.fVars) {
                            const Variable& var = *decl->as<VarDeclaration>().fVar;
                            if (&var == &ref.fVariable) {
                                found = true;
                                break;
                            }
                            if (var.fModifiers.fFlags & flag) {
                                ++index;
                            }
                        }
                    }
                }
                SkASSERT(found);
                return index;
            };

            if (ref.fVariable.fModifiers.fFlags & Modifiers::kUniform_Flag) {
                this->write(Compiler::kFormatArgPlaceholderStr);
                fArgs->fFormatArgs.push_back(
                        Compiler::FormatArg(Compiler::FormatArg::Kind::kUniform,
                                            varIndexByFlag(Modifiers::kUniform_Flag)));
            } else if (ref.fVariable.fModifiers.fFlags & Modifiers::kVarying_Flag) {
                this->write("_vtx_attr_");
                this->write(to_string(varIndexByFlag(Modifiers::kVarying_Flag)));
            } else {
                this->write(ref.fVariable.fName);
            }
        }
    }
}

void PipelineStageCodeGenerator::writeIfStatement(const IfStatement& s) {
    if (s.fIsStatic) {
        this->write("@");
    }
    INHERITED::writeIfStatement(s);
}

void PipelineStageCodeGenerator::writeSwitchStatement(const SwitchStatement& s) {
    if (s.fIsStatic) {
        this->write("@");
    }
    INHERITED::writeSwitchStatement(s);
}

void PipelineStageCodeGenerator::writeFunction(const FunctionDefinition& f) {
    fFunctionHeader = "";
    OutputStream* oldOut = fOut;
    StringStream buffer;
    fOut = &buffer;
    if (f.fDeclaration.fName == "main") {
        for (const std::unique_ptr<Statement>& s : f.fBody->as<Block>().fStatements) {
            this->writeStatement(*s);
            this->writeLine();
        }
        fOut = oldOut;
        this->write(fFunctionHeader);
        this->write(buffer.str());
    } else {
        const FunctionDeclaration& decl = f.fDeclaration;
        Compiler::GLSLFunction result;
        if (!type_to_grsltype(fContext, decl.fReturnType, &result.fReturnType)) {
            fErrors.error(f.fOffset, "unsupported return type");
            return;
        }
        result.fName = decl.fName;
        for (const Variable* v : decl.fParameters) {
            GrSLType paramSLType;
            if (!type_to_grsltype(fContext, v->fType, &paramSLType)) {
                fErrors.error(v->fOffset, "unsupported parameter type");
                return;
            }
            result.fParameters.emplace_back(v->fName, paramSLType);
        }
        for (const std::unique_ptr<Statement>& s : f.fBody->as<Block>().fStatements) {
            this->writeStatement(*s);
            this->writeLine();
        }
        fOut = oldOut;
        result.fBody = buffer.str();
        result.fFormatArgs = std::move(fArgs->fFormatArgs);
        fArgs->fFunctions.push_back(result);
    }
}

void PipelineStageCodeGenerator::writeProgramElement(const ProgramElement& p) {
    if (p.fKind == ProgramElement::kSection_Kind) {
        return;
    }
    if (p.fKind == ProgramElement::kVar_Kind) {
        const VarDeclarations& decls = p.as<VarDeclarations>();
        if (!decls.fVars.size()) {
            return;
        }
        const Variable& var = *decls.fVars[0]->as<VarDeclaration>().fVar;
        if (var.fModifiers.fFlags &
                    (Modifiers::kIn_Flag | Modifiers::kUniform_Flag | Modifiers::kVarying_Flag) ||
            -1 != var.fModifiers.fLayout.fBuiltin) {
            return;
        }
    }
    INHERITED::writeProgramElement(p);
}

}  // namespace SkSL
#endif
