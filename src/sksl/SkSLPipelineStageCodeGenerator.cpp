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
                                                  std::vector<Compiler::FormatArg>* outFormatArgs,
                                                  std::vector<Compiler::GLSLFunction>* outFunctions)
: INHERITED(context, program, errors, out)
, fName("Temp")
, fFullName(String::printf("Gr%s", fName.c_str()))
, fSectionAndParameterHelper(program, *errors)
, fFormatArgs(outFormatArgs)
, fFunctions(outFunctions) {}

void PipelineStageCodeGenerator::writef(const char* s, va_list va) {
    static constexpr int BUFFER_SIZE = 1024;
    va_list copy;
    va_copy(copy, va);
    char buffer[BUFFER_SIZE];
    int length = vsnprintf(buffer, BUFFER_SIZE, s, va);
    if (length < BUFFER_SIZE) {
        fOut->write(buffer, length);
    } else {
        std::unique_ptr<char[]> heap(new char[length + 1]);
        vsprintf(heap.get(), s, copy);
        fOut->write(heap.get(), length);
    }
    va_end(copy);
}

void PipelineStageCodeGenerator::writef(const char* s, ...) {
    va_list va;
    va_start(va, s);
    this->writef(s, va);
    va_end(va);
}

void PipelineStageCodeGenerator::writeHeader() {
}

bool PipelineStageCodeGenerator::usesPrecisionModifiers() const {
    return false;
}

String PipelineStageCodeGenerator::getTypeName(const Type& type) {
    return type.name();
}

void PipelineStageCodeGenerator::writeBinaryExpression(const BinaryExpression& b,
                                                       Precedence parentPrecedence) {
    if (b.fOperator == Token::PERCENT) {
        // need to use "%%" instead of "%" b/c the code will be inside of a printf
        Precedence precedence = GetBinaryPrecedence(b.fOperator);
        if (precedence >= parentPrecedence) {
            this->write("(");
        }
        this->writeExpression(*b.fLeft, precedence);
        this->write(" %% ");
        this->writeExpression(*b.fRight, precedence);
        if (precedence >= parentPrecedence) {
            this->write(")");
        }
    } else {
        INHERITED::writeBinaryExpression(b, parentPrecedence);
    }
}

void PipelineStageCodeGenerator::writeFunctionCall(const FunctionCall& c) {
    if (c.fFunction.fBuiltin && c.fFunction.fName == "sample" &&
        c.fArguments[0]->fType.kind() != Type::Kind::kSampler_Kind) {
        SkASSERT(c.fArguments.size() == 1);
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
        this->write("%s");
        fFormatArgs->push_back(Compiler::FormatArg(Compiler::FormatArg::Kind::kChildProcessor,
                                                   index));
        return;
    }
    if (c.fFunction.fBuiltin) {
        INHERITED::writeFunctionCall(c);
    } else {
        this->write("%s");
        int index = 0;
        for (const auto& e : fProgram) {
            if (e.fKind == ProgramElement::kFunction_Kind) {
                if (&((FunctionDefinition&) e).fDeclaration == &c.fFunction) {
                    break;
                }
                ++index;
            }
        }
        fFormatArgs->push_back(Compiler::FormatArg(Compiler::FormatArg::Kind::kFunctionName,
                                                   index));
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
            this->write("%s");
            fFormatArgs->push_back(Compiler::FormatArg(Compiler::FormatArg::Kind::kInput));
            break;
        case SK_OUTCOLOR_BUILTIN:
            this->write("%s");
            fFormatArgs->push_back(Compiler::FormatArg(Compiler::FormatArg::Kind::kOutput));
            break;
        case SK_MAIN_X_BUILTIN:
            this->write("%s");
            fFormatArgs->push_back(Compiler::FormatArg(Compiler::FormatArg::Kind::kCoordX));
            break;
        case SK_MAIN_Y_BUILTIN:
            this->write("%s");
            fFormatArgs->push_back(Compiler::FormatArg(Compiler::FormatArg::Kind::kCoordY));
            break;
        default:
            if (ref.fVariable.fModifiers.fFlags & Modifiers::kUniform_Flag) {
                this->write("%s");
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
                            if (var.fModifiers.fFlags & Modifiers::kUniform_Flag) {
                                ++index;
                            }
                        }
                    }
                }
                SkASSERT(found);
                fFormatArgs->push_back(Compiler::FormatArg(Compiler::FormatArg::Kind::kUniform,
                                                           index));
            } else {
                this->write(ref.fVariable.fName);
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
    if (type == *context.fFloat_Type) {
        return GrSLType::kFloat_GrSLType;
    } else if (type == *context.fHalf_Type) {
        return GrSLType::kHalf_GrSLType;
    } else if (type == *context.fFloat2_Type) {
        return GrSLType::kFloat2_GrSLType;
    } else if (type == *context.fHalf2_Type) {
        return GrSLType::kHalf2_GrSLType;
    } else if (type == *context.fFloat4_Type) {
        return GrSLType::kFloat4_GrSLType;
    } else if (type == *context.fHalf4_Type) {
        return GrSLType::kHalf4_GrSLType;
    } else if (type == *context.fFloat4x4_Type) {
        return GrSLType::kFloat4x4_GrSLType;
    } else if (type == *context.fHalf4x4_Type) {
        return GrSLType::kHalf4x4_GrSLType;
    } else if (type == *context.fVoid_Type) {
        return GrSLType::kVoid_GrSLType;
    }
    SkASSERT(false);
    return GrSLType::kVoid_GrSLType;
}


void PipelineStageCodeGenerator::writeFunction(const FunctionDefinition& f) {
    fCurrentFunction = &f.fDeclaration;
    fFunctionHeader = "";
    OutputStream* oldOut = fOut;
    StringStream buffer;
    fOut = &buffer;
    if (f.fDeclaration.fName == "main") {
        this->write("%s = %s;\n");
        fFormatArgs->push_back(Compiler::FormatArg(Compiler::FormatArg::Kind::kOutput));
        fFormatArgs->push_back(Compiler::FormatArg(Compiler::FormatArg::Kind::kInput));
        for (const auto& s : ((Block&) *f.fBody).fStatements) {
            this->writeStatement(*s);
            this->writeLine();
        }
        fOut = oldOut;
        this->write(fFunctionHeader);
        this->writef("%s", buffer.str().c_str());
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
        fFunctions->push_back(result);
    }
}

bool PipelineStageCodeGenerator::writeSection(const char* name, const char* prefix) {
    const Section* s = fSectionAndParameterHelper.getSection(name);
    if (s) {
        this->writef("%s%s", prefix, s->fText.c_str());
        return true;
    }
    return false;
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
        if (var.fModifiers.fFlags & (Modifiers::kIn_Flag | Modifiers::kUniform_Flag) ||
            -1 != var.fModifiers.fLayout.fBuiltin) {
            return;
        }
    }
    INHERITED::writeProgramElement(p);
}

} // namespace
#endif
