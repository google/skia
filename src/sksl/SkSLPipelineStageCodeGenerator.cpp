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

PipelineStageCodeGenerator::PipelineStageCodeGenerator(
                                                  const Context* context,
                                                  const Program* program,
                                                  ErrorReporter* errors,
                                                  OutputStream* out,
                                                  PipelineStageArgs* outArgs)
    : INHERITED(context, program, errors, out)
    , fArgs(outArgs) {}

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
                const VarDeclarations& decls = (const VarDeclarations&) p;
                for (const auto& raw : decls.fVars) {
                    VarDeclaration& decl = (VarDeclaration&) *raw;
                    if (decl.fVar == &((VariableReference&) *c.fArguments[0]).fVariable) {
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
        for (const auto& e : fProgram) {
            if (e.fKind == ProgramElement::kFunction_Kind) {
                if (&((FunctionDefinition&) e).fDeclaration == &c.fFunction) {
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
        for (const auto& arg : c.fArguments) {
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
        case SK_INCOLOR_BUILTIN:
            this->write(Compiler::kFormatArgPlaceholderStr);
            fArgs->fFormatArgs.push_back(Compiler::FormatArg(Compiler::FormatArg::Kind::kInput));
            break;
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
                for (const auto& e : fProgram) {
                    if (found) {
                        break;
                    }
                    if (e.fKind == ProgramElement::Kind::kVar_Kind) {
                        const VarDeclarations& decls = (const VarDeclarations&) e;
                        for (const auto& decl : decls.fVars) {
                            const Variable& var = *((VarDeclaration&) *decl).fVar;
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

static GrSLType glsltype(const Context& context, const Type& type) {
    if (type == *context.fFloat_Type)    { return kFloat_GrSLType;    }
    if (type == *context.fHalf_Type)     { return kHalf_GrSLType;     }
    if (type == *context.fFloat2_Type)   { return kFloat2_GrSLType;   }
    if (type == *context.fHalf2_Type)    { return kHalf2_GrSLType;    }
    if (type == *context.fFloat3_Type)   { return kFloat3_GrSLType;   }
    if (type == *context.fHalf3_Type)    { return kHalf3_GrSLType;    }
    if (type == *context.fFloat4_Type)   { return kFloat4_GrSLType;   }
    if (type == *context.fHalf4_Type)    { return kHalf4_GrSLType;    }
    if (type == *context.fFloat2x2_Type) { return kFloat2x2_GrSLType; }
    if (type == *context.fHalf2x2_Type)  { return kHalf2x2_GrSLType;  }
    if (type == *context.fFloat3x3_Type) { return kFloat3x3_GrSLType; }
    if (type == *context.fHalf3x3_Type)  { return kHalf3x3_GrSLType;  }
    if (type == *context.fFloat4x4_Type) { return kFloat4x4_GrSLType; }
    if (type == *context.fHalf4x4_Type)  { return kHalf4x4_GrSLType;  }
    if (type == *context.fVoid_Type)     { return kVoid_GrSLType;     }
    SK_ABORT("Unsupported type");
}


void PipelineStageCodeGenerator::writeFunction(const FunctionDefinition& f) {
    fFunctionHeader = "";
    OutputStream* oldOut = fOut;
    StringStream buffer;
    fOut = &buffer;
    if (f.fDeclaration.fName == "main") {
        this->write(Compiler::kFormatArgPlaceholderStr);
        this->write(" = ");
        this->write(Compiler::kFormatArgPlaceholderStr);
        this->write(";\n");
        fArgs->fFormatArgs.push_back(Compiler::FormatArg(Compiler::FormatArg::Kind::kOutput));
        fArgs->fFormatArgs.push_back(Compiler::FormatArg(Compiler::FormatArg::Kind::kInput));
        for (const auto& s : ((Block&) *f.fBody).fStatements) {
            this->writeStatement(*s);
            this->writeLine();
        }
        fOut = oldOut;
        this->write(fFunctionHeader);
        this->write(buffer.str());
    } else {
        const FunctionDeclaration& decl = f.fDeclaration;
        Compiler::GLSLFunction result;
        result.fReturnType = glsltype(fContext, decl.fReturnType);
        result.fName = decl.fName;
        for (const Variable* v : decl.fParameters) {
            result.fParameters.emplace_back(v->fName, glsltype(fContext, v->fType));
        }
        for (const auto& s : ((Block&) *f.fBody).fStatements) {
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
        const VarDeclarations& decls = (const VarDeclarations&) p;
        if (!decls.fVars.size()) {
            return;
        }
        const Variable& var = *((VarDeclaration&) *decls.fVars[0]).fVar;
        if (var.fModifiers.fFlags &
                    (Modifiers::kIn_Flag | Modifiers::kUniform_Flag | Modifiers::kVarying_Flag) ||
            -1 != var.fModifiers.fLayout.fBuiltin) {
            return;
        }
    }
    INHERITED::writeProgramElement(p);
}

} // namespace
#endif
