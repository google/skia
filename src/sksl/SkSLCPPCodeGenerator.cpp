/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLCPPCodeGenerator.h"

#include "SkSLCompiler.h"
#include "SkSLHCodeGenerator.h"

namespace SkSL {

static bool create_uniform_var(const Variable& var) {
    return (var.fModifiers.fFlags & Modifiers::kUniform_Flag) && 
           strcmp(var.fType.fName.c_str(), "GrColorSpaceXform");
}

CPPCodeGenerator::CPPCodeGenerator(const Context* context, const Program* program,
                                   ErrorReporter* errors, String name, OutputStream* out)
: INHERITED(context, program, errors, out)
, fName(std::move(name)) {
    fLineEnding = "\\n";
    for (const auto& p : fProgram.fElements) {
        switch (p->fKind) {
            case ProgramElement::kVar_Kind: {
                const VarDeclarations* decls = (const VarDeclarations*) p.get();
                for (const auto& decl : decls->fVars) {
                    if ((decl->fVar->fModifiers.fFlags & Modifiers::kIn_Flag) &&
                        -1 == decl->fVar->fModifiers.fLayout.fBuiltin) {
                        fParameters.push_back(decl->fVar);
                    }
                }
                break;
            }
            case ProgramElement::kSection_Kind: {
                const Section* s = (const Section*) p.get();
                if (HCodeGenerator::IsSupportedSection(s->fName.c_str())) {
                    if (HCodeGenerator::SectionAcceptsArgument(s->fName.c_str())) {
                        if (!s->fArgument.size()) {
                            fErrors.error(s->fPosition,
                                          ("section '@" + s->fName +
                                           "' requires one parameter").c_str());
                        }
                    } else if (s->fArgument.size()) {
                        fErrors.error(s->fPosition,
                                      ("section '@" + s->fName + "' has no parameters").c_str());
                    }
                } else {
                    fErrors.error(s->fPosition,
                                  ("unsupported section '@" + s->fName + "'").c_str());
                }
                if (fSections.find(s->fName) != fSections.end()) {
                    fErrors.error(s->fPosition,
                                  ("duplicate section '@" + s->fName + "'").c_str());
                }
                fSections[s->fName] = s;
                break;
            }
            default:
                break;
        }
    }
    fFullName = String::printf("Gr%s", fName.c_str());
}

void CPPCodeGenerator::writef(const char* s, va_list va) {
    String formatted;
    formatted.vappendf(s, va);
    fOut->writeString(formatted);
}

void CPPCodeGenerator::writef(const char* s, ...) {
    va_list va;
    va_start(va, s);
    this->writef(s, va);
    va_end(va);
}

void CPPCodeGenerator::writeHeader() {
}

void CPPCodeGenerator::writeBinaryExpression(const BinaryExpression& b,
                                            Precedence parentPrecedence) {
    if (b.fOperator == Token::PERCENT) {
        Precedence precedence = this->getBinaryPrecedence(b.fOperator);
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

void CPPCodeGenerator::writeIndexExpression(const IndexExpression& i) {
    const Expression& base = *i.fBase;
    if (base.fKind == Expression::kVariableReference_Kind) {
        int builtin = ((VariableReference&) base).fVariable.fModifiers.fLayout.fBuiltin;
        if (SK_TRANSFORMEDCOORDS2D_BUILTIN == builtin) {
            this->write("%s");
            if (i.fIndex->fKind != Expression::kIntLiteral_Kind) {
                fErrors.error(i.fIndex->fPosition,
                              "index into sk_TransformedCoords2D must be an integer literal");
            }
            int64_t index = ((IntLiteral&) *i.fIndex).fValue;
            String name = "sk_TransformedCoords2D_" + to_string(index);
            fFormatArgs.push_back(name + ".c_str()");
            fExtraEmitCodeCode += "        SkSL::String " + name +
                                  " = fragBuilder->ensureCoords2D(args.fTransformedCoords[" +
                                  to_string(index) + "]);\n";
            return;
        } else if (SK_TEXTURESAMPLERS_BUILTIN == builtin) {
            this->write("%s");
            if (i.fIndex->fKind != Expression::kIntLiteral_Kind) {
                fErrors.error(i.fIndex->fPosition,
                              "index into sk_TextureSamplers must be an integer literal");
            }
            int64_t index = ((IntLiteral&) *i.fIndex).fValue;
            fFormatArgs.push_back("        fragBuilder->getProgramBuilder()->samplerVariable("
                                            "args.fTexSamplers[" + to_string(index) + "]).c_str()");
            return;
        }
    }
    INHERITED::writeIndexExpression(i);
}

static const char* default_value(const Type& type) {
    const char* name = type.name().c_str();
    if (!strcmp(name, "float")) {
        return "0.0";
    } else if (!strcmp(name, "vec2")) {
        return "vec2(0.0)";
    } else if (!strcmp(name, "vec3")) {
        return "vec3(0.0)";
    } else if (!strcmp(name, "vec4")) {
        return "vec4(0.0)";
    } else if (!strcmp(name, "mat4") || !strcmp(name, "GrColorSpaceXform")) {
        return "mat4(1.0)";
    }
    ABORT("unsupported default_value type\n");
}

static bool is_private(const Variable& var) {
    return !(var.fModifiers.fFlags & Modifiers::kUniform_Flag) &&
           !(var.fModifiers.fFlags & Modifiers::kIn_Flag) &&
           var.fModifiers.fLayout.fBuiltin == -1;
}

void CPPCodeGenerator::writeRuntimeValue(const Type& type, const String& cppCode) {
    if (type == *fContext.fFloat_Type) {
        this->write("%f");
        fFormatArgs.push_back(cppCode);
    } else if (type == *fContext.fInt_Type) {
        this->write("%d");
        fFormatArgs.push_back(cppCode);
    } else if (type == *fContext.fBool_Type) {
        this->write("%s");
        fFormatArgs.push_back(cppCode + " ? \"true\" : \"false\"");
    } else if (type == *fContext.fVec2_Type) {
        this->write("vec2(%f, %f)");
        fFormatArgs.push_back(cppCode + ".fX");
        fFormatArgs.push_back(cppCode + ".fY");
    } else {
        printf("%s\n", type.name().c_str());
        ABORT("unsupported runtime value type\n");
    }
}

void CPPCodeGenerator::writeVarInitializer(const Variable& var, const Expression& value) {
    if (var.fStorage == Variable::kGlobal_Storage && is_private(var)) {
        this->writeRuntimeValue(var.fType, var.fName);
    } else {
        this->writeExpression(value, kTopLevel_Precedence);
    }
}

void CPPCodeGenerator::writeVariableReference(const VariableReference& ref) {
    switch (ref.fVariable.fModifiers.fLayout.fBuiltin) {
        case SK_INCOLOR_BUILTIN:
            this->write("%s");
            fFormatArgs.push_back(String("args.fInputColor ? args.fInputColor : \"vec4(1)\""));
            break;
        case SK_OUTCOLOR_BUILTIN:
            this->write("%s");
            fFormatArgs.push_back(String("args.fOutputColor"));
            break;
        case SK_DISTANCEVECTOR_BUILTIN:
            this->write("%s");
            // if gpImplementsDistanceVector is false, any usage of the distance vector should be in
            // code that gets optimized away. But even though it's going to be optimized away, it
            // still can't reference an undefined variable, so we output vec4(0) when it's not
            // supported.
            fFormatArgs.push_back(String("fGpImplementsDistanceVector ? "
                                           "fragBuilder->distanceVectorName() : \"vec4(0)\""));
            break;
        default:
            if (ref.fVariable.fType.kind() == Type::kSampler_Kind) {
                int samplerCount = 0;
                for (const auto param : fParameters) {
                    if (&ref.fVariable == param) {
                        this->writef("%%s");
                        fFormatArgs.push_back("fragBuilder->getProgramBuilder()->samplerVariable(args.fTexSamplers[" + to_string(samplerCount) + "]).c_str()");
                        return;
                    }
                    if (param->fType.kind() == Type::kSampler_Kind) {
                        ++samplerCount;
                    }
                }
                ABORT("should have found sampler in parameters\n");
            }
            if (ref.fVariable.fModifiers.fFlags & Modifiers::kUniform_Flag) {
                this->write("%s");
                String name = ref.fVariable.fName;
                String var;
                if (ref.fVariable.fType == *fContext.fGrColorSpaceXform_Type) {
                    ASSERT(fNeedColorSpaceHelper);
                    var = String::printf("fColorSpaceHelper.isValid() ? "
                                         "args.fUniformHandler->getUniformCStr("
                                                  "fColorSpaceHelper.gamutXformUniform()) : \"%s\"",
                           default_value(ref.fVariable.fType));
                } else {
                    var = String::printf("args.fUniformHandler->getUniformCStr("
                                         "f%c%sVar)", 
                                         toupper(name.c_str()[0]),
                                         name.c_str() + 1);
                }
                String code;
                if (ref.fVariable.fModifiers.fLayout.fWhen.size()) {
                    code = String::printf("f%c%sVar.isValid() ? %s : \"%s\"",
                                          toupper(name.c_str()[0]),
                                          name.c_str() + 1,
                                          var.c_str(),
                                          default_value(ref.fVariable.fType));
                } else {
                    code = var;
                }
                fFormatArgs.push_back(code);
            } else if (-1 == ref.fVariable.fModifiers.fLayout.fBuiltin && 
                       ref.fVariable.fModifiers.fFlags & Modifiers::kIn_Flag) {
                const char* name = ref.fVariable.fName.c_str();
                this->writeRuntimeValue(ref.fVariable.fType,
                                        String::printf("_outer.%s()", name).c_str());
            } else {
                this->write(ref.fVariable.fName.c_str());
            }
    }
}

void CPPCodeGenerator::writeFunction(const FunctionDefinition& f) {
    if (f.fDeclaration.fName == "main") {
        fFunctionHeader = "";
        OutputStream* oldOut = fOut;
        StringStream buffer;
        fOut = &buffer;
        for (const auto& s : ((Block&) *f.fBody).fStatements) {
            this->writeStatement(*s);
            this->writeLine();
        }

        fOut = oldOut;
        this->write(fFunctionHeader);
        this->write(String((const char*) buffer.data(), buffer.size()));
    } else {
        INHERITED::writeFunction(f);
    }
}

void CPPCodeGenerator::writeSetting(const Setting& s) {
    if (!strncmp(s.fName.c_str(), "sk_Args.", strlen("sk_Args."))) {
        const char* name = s.fName.c_str() + strlen("sk_Args.");
        this->writeRuntimeValue(s.fType,
                                String::printf("f%c%s", toupper(name[0]), name + 1).c_str());
    } else {
        this->write(s.fName.c_str());
    }
}

// FIXME resolve duplication between this & HCodeGenerator
void CPPCodeGenerator::writeSection(const char* name, const char* prefix) {
    const auto found = fSections.find(String(name));
    if (found != fSections.end()) {
        this->write(prefix);
        const char* start = found->second->fText.c_str();
        int remaining = found->second->fText.size();
        // String.appendVAList can only handle 1024 character long chunks
        #define CHUNK_SIZE 1024
        while (remaining) {
            int count = remaining;
            if (CHUNK_SIZE < count) {
                count = CHUNK_SIZE;
            }
            fOut->write(start, count);
            start += count;
            remaining -= count;
        }
    }
}

void CPPCodeGenerator::writeProgramElement(const ProgramElement& p) {
    if (p.fKind == ProgramElement::kSection_Kind) {
        return;
    }
    if (p.fKind == ProgramElement::kVar_Kind) {
        const VarDeclarations& decls = (const VarDeclarations&) p;
        if (!decls.fVars.size()) {
            return;
        }
        const Variable& var = *decls.fVars[0]->fVar;
        if (var.fModifiers.fFlags & (Modifiers::kIn_Flag | Modifiers::kUniform_Flag) ||
            -1 != var.fModifiers.fLayout.fBuiltin) {
            return;
        }
    }
    INHERITED::writeProgramElement(p);
}

void CPPCodeGenerator::addUniform(const Variable& var) {
    if (!create_uniform_var(var)) {
        return;
    }
    const char* precision;
    if (var.fModifiers.fFlags & Modifiers::kHighp_Flag) {
        precision = "kHigh_GrSLPrecision";
    } else if (var.fModifiers.fFlags & Modifiers::kMediump_Flag) {
        precision = "kMedium_GrSLPrecision";
    } else if (var.fModifiers.fFlags & Modifiers::kLowp_Flag) {
        precision = "kLow_GrSLPrecision";
    } else {
        precision = "kDefault_GrSLPrecision";
    }
    const char* type;
    if (var.fType == *fContext.fFloat_Type) {
        type = "kFloat_GrSLType";
    } else if (var.fType == *fContext.fVec2_Type) {
        type = "kVec2f_GrSLType";
    } else if (var.fType == *fContext.fVec4_Type) {
        type = "kVec4f_GrSLType";
    } else if (var.fType == *fContext.fMat4x4_Type ||
               var.fType == *fContext.fGrColorSpaceXform_Type) {
        type = "kMat44f_GrSLType";
    } else {
        ABORT("unsupported uniform type: %s %s;\n", var.fType.name().c_str(), var.fName.c_str());
    }
    if (var.fModifiers.fLayout.fWhen.size()) {
        this->writef("        if (%s) {\n    ", var.fModifiers.fLayout.fWhen.c_str());
    }
    const char* name = var.fName.c_str();
    this->writef("        f%c%sVar = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, %s, "
                 "%s, \"%s\");\n", toupper(name[0]), name + 1, type, precision, name);
    if (var.fModifiers.fLayout.fWhen.size()) {
        this->writef("        }\n");
    }
}

void CPPCodeGenerator::writePrivateVars() {
    for (const auto& p : fProgram.fElements) {
        if (ProgramElement::kVar_Kind == p->fKind) {
            const VarDeclarations* decls = (const VarDeclarations*) p.get();
            for (const auto& decl : decls->fVars) {
                if (is_private(*decl->fVar)) {
                    this->writef("%s %s;\n",
                                 HCodeGenerator::FieldType(decl->fVar->fType).c_str(),
                                 decl->fVar->fName.c_str());
                }
            }
        }
    }
}

void CPPCodeGenerator::writePrivateVarValues() {
    for (const auto& p : fProgram.fElements) {
        if (ProgramElement::kVar_Kind == p->fKind) {
            const VarDeclarations* decls = (const VarDeclarations*) p.get();
            for (const auto& decl : decls->fVars) {
                if (is_private(*decl->fVar) && decl->fValue) {
                    this->writef("%s = %s;\n",
                                 decl->fVar->fName.c_str(),
                                 decl->fValue->description().c_str());
                }
            }
        }
    }
}

bool CPPCodeGenerator::writeEmitCode(std::vector<const Variable*>& uniforms) {
    this->writef("    void emitCode(EmitArgs& args) override {\n"
                 "        fArgsRead = true;\n"
                 "        fGpImplementsDistanceVector = args.fGpImplementsDistanceVector;\n"
                 "        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;\n");
    this->writef("        const %s& _outer = args.fFp.cast<%s>();\n"
                 "        (void) _outer;\n", 
                 fFullName.c_str(), fFullName.c_str());
    this->writePrivateVarValues();
    for (const auto u : uniforms) {
        this->addUniform(*u);
        if (u->fType == *fContext.fGrColorSpaceXform_Type) {
            fNeedColorSpaceHelper = true;
            this->writef("        fColorSpaceHelper.emitCode(args.fUniformHandler, "
                                                            "_outer.%s().get());\n",
                         u->fName.c_str());
        }
    }
    this->writeSection(EMIT_CODE_SECTION);
    OutputStream* old = fOut;
    StringStream mainBuffer;
    fOut = &mainBuffer;
    bool result = INHERITED::generateCode();
    fOut = old;
    this->writef("%s        fragBuilder->codeAppendf(\"%s\"", fExtraEmitCodeCode.c_str(),
                 mainBuffer.data());
    for (const auto& s : fFormatArgs) {
        this->writef(", %s", s.c_str());
    }
    this->write(");\n"
                "    }\n");
    return result;
}

void CPPCodeGenerator::writeSetData(std::vector<const Variable*>& uniforms) {
    const char* fullName = fFullName.c_str();
    auto section = fSections.find(String(SET_DATA_SECTION));
    const char* pdman = section != fSections.end() ? section->second->fArgument.c_str() : "pdman";
    this->writef("    void onSetData(const GrGLSLProgramDataManager& %s, "
                                    "const GrFragmentProcessor& _proc) override {\n",
                 pdman);
    bool wroteProcessor = false;
    for (const auto u : uniforms) {
        if (u->fModifiers.fFlags & Modifiers::kIn_Flag) {
            if (!wroteProcessor) {
                this->writef("        const %s& _outer = _proc.cast<%s>();\n", fullName,
                             fullName);
                wroteProcessor = true;
            }
            const char* name = u->fName.c_str();
            if (u->fType == *fContext.fVec4_Type) {
                this->writef("        const SkRect %sValue = _outer.%s();\n"
                             "        %s.set4fv(f%c%sVar, 4, (float*) &%sValue);\n",
                             name, name, pdman, toupper(name[0]), name + 1, name);
            } else if (u->fType == *fContext.fMat4x4_Type) {
                this->writef("        float %sValue[16];\n"
                             "        _outer.%s().asColMajorf(%sValue);\n"
                             "        %s.setMatrix4f(f%c%sVar, %sValue);\n",
                             name, name, name, pdman, toupper(name[0]), name + 1, name);
            } else if (u->fType == *fContext.fGrColorSpaceXform_Type) {
                fNeedColorSpaceHelper = true;
                this->writef("        if (fColorSpaceHelper.isValid()) {\n"
                             "            fColorSpaceHelper.setData(pdman, _outer.%s().get());\n"
                             "        }\n",
                             name);
            } else {
                this->writef("        %s.set1f(f%c%sVar, _outer.%s());\n", 
                             pdman, toupper(name[0]), name + 1, name);
            }
        }
    }
    if (section != fSections.end()) {
        for (const auto& p : fProgram.fElements) {
            if (ProgramElement::kVar_Kind == p->fKind) {
                const VarDeclarations* decls = (const VarDeclarations*) p.get();
                for (const auto& decl : decls->fVars) {
                    if (create_uniform_var(*decl->fVar)) {
                        const char* name = decl->fVar->fName.c_str();
                        this->writef("        UniformHandle& %s = f%c%sVar;\n"
                                     "        (void) %s;\n",
                                     name,  toupper(name[0]), name + 1, name);
                    } else if (decl->fVar->fModifiers.fFlags & Modifiers::kIn_Flag &&
                        -1 == decl->fVar->fModifiers.fLayout.fBuiltin) {
                        const char* name = decl->fVar->fName.c_str();
                        if (!wroteProcessor) {
                            this->writef("        const %s& _outer = _proc.cast<%s>();\n", fullName,
                                         fullName);
                            wroteProcessor = true;
                        }
                        this->writef("        auto %s = _outer.%s();\n"
                                     "        (void) %s;\n",
                                     name, name, name);
                    }
                }
            }
        }
        this->writeSection(SET_DATA_SECTION);
    }
    this->write("    }\n");
}

void CPPCodeGenerator::writeTest() {
    const auto found = fSections.find(TEST_CODE_SECTION);
    if (found == fSections.end()) {
        return;
    }
    const Section* test = found->second;
    this->writef("GR_DEFINE_FRAGMENT_PROCESSOR_TEST(%s);\n"
                 "#if GR_TEST_UTILS\n"
                 "sk_sp<GrFragmentProcessor> %s::TestCreate(GrProcessorTestData* %s) {\n", 
                 fFullName.c_str(),
                 fFullName.c_str(),
                 test->fArgument.c_str());
    this->writeSection(TEST_CODE_SECTION);
    this->writef("}\n"
                 "#endif\n");
}

void CPPCodeGenerator::writeGetKey() {
    this->writef("void %s::onGetGLSLProcessorKey(const GrShaderCaps& caps, "
                                                "GrProcessorKeyBuilder* b) const {\n",
                 fFullName.c_str());
    for (const auto& param : fParameters) {
        const char* name = param->fName.c_str();
        if (param->fModifiers.fFlags & Modifiers::kIn_Flag &&
            -1 == param->fModifiers.fLayout.fBuiltin) {
            if (param->fType == *fContext.fGrColorSpaceXform_Type) {
                this->writef("    b->add32(GrColorSpaceXform::XformKey(f%c%s.get()));\n",
                             toupper(name[0]), name + 1);
                continue;
            }
            if (param->fModifiers.fLayout.fKey != Layout::kNo_Key &&
                (param->fModifiers.fFlags & Modifiers::kUniform_Flag)) {
                fErrors.error(param->fPosition,
                              "layout(key) may not be specified on uniforms");
            }
            switch (param->fModifiers.fLayout.fKey) {
                case Layout::kKey_Key:
                    if (param->fType == *fContext.fMat4x4_Type) {
                        ABORT("no automatic key handling for mat4\n");
                    } else if (param->fType == *fContext.fVec2_Type) {
                        this->writef("    b->add32(f%c%s.fX);\n",
                                     toupper(name[0]), name + 1);
                        this->writef("    b->add32(f%c%s.fY);\n",
                                     toupper(name[0]), name + 1);
                    } else if (param->fType == *fContext.fVec4_Type) {
                        this->writef("    b->add32(f%c%s.x());\n",
                                     toupper(name[0]), name + 1);
                        this->writef("    b->add32(f%c%s.y());\n",
                                     toupper(name[0]), name + 1);
                        this->writef("    b->add32(f%c%s.width());\n",
                                     toupper(name[0]), name + 1);
                        this->writef("    b->add32(f%c%s.height());\n",
                                     toupper(name[0]), name + 1);
                    } else {
                        this->writef("    b->add32(f%c%s);\n",
                                     toupper(name[0]), name + 1);
                    }
                    break;
                case Layout::kIdentity_Key:
                    if (param->fType.kind() != Type::kMatrix_Kind) {
                        fErrors.error(param->fPosition,
                                      "layout(key=identity) requires matrix type");
                    }
                    this->writef("    b->add32(f%c%s.isIdentity() ? 1 : 0);\n",
                                 toupper(name[0]), name + 1);
                    break;
                case Layout::kNo_Key:
                    break;
            }
        }
    }
    this->write("}\n");
}

bool CPPCodeGenerator::generateCode() {
    // FIXME eliminate the code duplication between this and HCodeGenerator
    std::vector<const Variable*> uniforms;
    for (const auto& p : fProgram.fElements) {
        if (ProgramElement::kVar_Kind == p->fKind) {
            const VarDeclarations* decls = (const VarDeclarations*) p.get();
            for (const auto& decl : decls->fVars) {
                if ((decl->fVar->fModifiers.fFlags & Modifiers::kUniform_Flag) &&
                           decl->fVar->fType.kind() != Type::kSampler_Kind) {
                    uniforms.push_back(decl->fVar);
                }
            }
        }
    }
    const char* baseName = fName.c_str();
    const char* fullName = fFullName.c_str();
    this->writef("/*\n"
                 " * This file was autogenerated from %s.fp.\n"
                 " */\n"
                 "#include \"%s.h\"\n"
                 "#include \"glsl/GrGLSLColorSpaceXformHelper.h\"\n"
                 "#include \"glsl/GrGLSLFragmentProcessor.h\"\n"
                 "#include \"glsl/GrGLSLFragmentShaderBuilder.h\"\n"
                 "#include \"glsl/GrGLSLProgramBuilder.h\"\n"
                 "#include \"glsl/GrGLSLProgramBuilder.h\"\n"
                 "#include \"GrResourceProvider.h\"\n"
                 "#include \"SkSLCPP.h\"\n"
                 "#include \"SkSLUtil.h\"\n"
                 "class GrGLSL%s : public GrGLSLFragmentProcessor {\n"
                 "public:\n"
                 "    GrGLSL%s() {}\n",
                 fullName, fullName, baseName, baseName);
    bool result = this->writeEmitCode(uniforms);
    this->writef("private:\n");
    this->writeSetData(uniforms);
    this->write("    bool fArgsRead = false;\n");
    this->writePrivateVars();
    for (const auto& u : uniforms) {
        const char* name = u->fName.c_str();
        if (create_uniform_var(*u) && !(u->fModifiers.fFlags & Modifiers::kIn_Flag)) {
            this->writef("    UniformHandle f%c%sVar;\n", toupper(name[0]), name + 1);
        }
    }
    for (const auto& param : fParameters) {
        const char* name = param->fName.c_str();
        if (create_uniform_var(*param)) {
            this->writef("    UniformHandle f%c%sVar;\n", toupper(name[0]), name + 1);
        }
    }
    if (fNeedColorSpaceHelper) {
        this->write("    GrGLSLColorSpaceXformHelper fColorSpaceHelper;\n");
    }
    this->writef("    bool fGpImplementsDistanceVector;\n"
                 "};\n"
                 "GrGLSLFragmentProcessor* %s::onCreateGLSLInstance() const {\n"
                 "    return new GrGLSL%s();\n"
                 "}\n",
                 fullName, baseName);
    this->writeGetKey();
    this->writef("bool %s::onIsEqual(const GrFragmentProcessor& other) const {\n"
                 "    const %s& that = other.cast<%s>();\n"
                 "    (void) that;\n",
                 fullName, fullName, fullName);
    for (const auto& param : fParameters) {
        const char* name = param->fName.c_str();
        this->writef("    if (f%c%s != that.f%c%s) return false;\n",
                     toupper(name[0]), name + 1, toupper(name[0]), name + 1);
    }
    this->write("    return true;\n"
                "}\n");
    this->writeSection(BODY_SECTION);
    this->writeTest();
    result &= 0 == fErrors.errorCount();
    return result;
}

} // namespace
