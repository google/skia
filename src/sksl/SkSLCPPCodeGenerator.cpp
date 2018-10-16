/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLCPPCodeGenerator.h"

#include "SkSLCompiler.h"
#include "SkSLCPPUniformCTypes.h"
#include "SkSLHCodeGenerator.h"

#include <algorithm>

namespace SkSL {

static bool needs_uniform_var(const Variable& var) {
    return (var.fModifiers.fFlags & Modifiers::kUniform_Flag) &&
           var.fType.kind() != Type::kSampler_Kind;
}

CPPCodeGenerator::CPPCodeGenerator(const Context* context, const Program* program,
                                   ErrorReporter* errors, String name, OutputStream* out)
: INHERITED(context, program, errors, out)
, fName(std::move(name))
, fFullName(String::printf("Gr%s", fName.c_str()))
, fSectionAndParameterHelper(*program, *errors) {
    fLineEnding = "\\n";
}

void CPPCodeGenerator::writef(const char* s, va_list va) {
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

void CPPCodeGenerator::writef(const char* s, ...) {
    va_list va;
    va_start(va, s);
    this->writef(s, va);
    va_end(va);
}

void CPPCodeGenerator::writeHeader() {
}

bool CPPCodeGenerator::usesPrecisionModifiers() const {
    return false;
}

String CPPCodeGenerator::getTypeName(const Type& type) {
    return type.name();
}

void CPPCodeGenerator::writeBinaryExpression(const BinaryExpression& b,
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

void CPPCodeGenerator::writeIndexExpression(const IndexExpression& i) {
    const Expression& base = *i.fBase;
    if (base.fKind == Expression::kVariableReference_Kind) {
        int builtin = ((VariableReference&) base).fVariable.fModifiers.fLayout.fBuiltin;
        if (SK_TRANSFORMEDCOORDS2D_BUILTIN == builtin) {
            this->write("%s");
            if (i.fIndex->fKind != Expression::kIntLiteral_Kind) {
                fErrors.error(i.fIndex->fOffset,
                              "index into sk_TransformedCoords2D must be an integer literal");
                return;
            }
            int64_t index = ((IntLiteral&) *i.fIndex).fValue;
            String name = "sk_TransformedCoords2D_" + to_string(index);
            fFormatArgs.push_back(name + ".c_str()");
            if (fWrittenTransformedCoords.find(index) == fWrittenTransformedCoords.end()) {
                addExtraEmitCodeLine("SkString " + name +
                                     " = fragBuilder->ensureCoords2D(args.fTransformedCoords[" +
                                     to_string(index) + "]);");
                fWrittenTransformedCoords.insert(index);
            }
            return;
        } else if (SK_TEXTURESAMPLERS_BUILTIN == builtin) {
            this->write("%s");
            if (i.fIndex->fKind != Expression::kIntLiteral_Kind) {
                fErrors.error(i.fIndex->fOffset,
                              "index into sk_TextureSamplers must be an integer literal");
                return;
            }
            int64_t index = ((IntLiteral&) *i.fIndex).fValue;
            fFormatArgs.push_back("        fragBuilder->getProgramBuilder()->samplerVariable("
                                            "args.fTexSamplers[" + to_string(index) + "]).c_str()");
            return;
        }
    }
    INHERITED::writeIndexExpression(i);
}

static String default_value(const Type& type) {
    if (type.fName == "bool") {
        return "false";
    }
    switch (type.kind()) {
        case Type::kScalar_Kind: return "0";
        case Type::kVector_Kind: return type.name() + "(0)";
        case Type::kMatrix_Kind: return type.name() + "(1)";
        default: ABORT("unsupported default_value type\n");
    }
}

static String default_value(const Variable& var) {
    if (var.fModifiers.fLayout.fCType == SkSL::Layout::CType::kSkPMColor4f) {
        return "{SK_FloatNaN, SK_FloatNaN, SK_FloatNaN, SK_FloatNaN}";
    }
    return default_value(var.fType);
}

static bool is_private(const Variable& var) {
    return !(var.fModifiers.fFlags & Modifiers::kUniform_Flag) &&
           !(var.fModifiers.fFlags & Modifiers::kIn_Flag) &&
           var.fStorage == Variable::kGlobal_Storage &&
           var.fModifiers.fLayout.fBuiltin == -1;
}

static bool is_uniform_in(const Variable& var) {
    return (var.fModifiers.fFlags & Modifiers::kUniform_Flag) &&
           (var.fModifiers.fFlags & Modifiers::kIn_Flag) &&
           var.fType.kind() != Type::kSampler_Kind;
}

void CPPCodeGenerator::writeRuntimeValue(const Type& type, const Layout& layout,
                                         const String& cppCode) {
    if (type.isFloat()) {
        this->write("%f");
        fFormatArgs.push_back(cppCode);
    } else if (type == *fContext.fInt_Type) {
        this->write("%d");
        fFormatArgs.push_back(cppCode);
    } else if (type == *fContext.fBool_Type) {
        this->write("%s");
        fFormatArgs.push_back("(" + cppCode + " ? \"true\" : \"false\")");
    } else if (type == *fContext.fFloat2_Type || type == *fContext.fHalf2_Type) {
        this->write(type.name() + "(%f, %f)");
        fFormatArgs.push_back(cppCode + ".fX");
        fFormatArgs.push_back(cppCode + ".fY");
    } else if (type == *fContext.fFloat4_Type || type == *fContext.fHalf4_Type) {
        this->write(type.name() + "(%f, %f, %f, %f)");
        switch (layout.fCType) {
            case Layout::CType::kSkPMColor:
                fFormatArgs.push_back("SkGetPackedR32(" + cppCode + ") / 255.0");
                fFormatArgs.push_back("SkGetPackedG32(" + cppCode + ") / 255.0");
                fFormatArgs.push_back("SkGetPackedB32(" + cppCode + ") / 255.0");
                fFormatArgs.push_back("SkGetPackedA32(" + cppCode + ") / 255.0");
                break;
            case Layout::CType::kSkPMColor4f:
                fFormatArgs.push_back(cppCode + ".fR");
                fFormatArgs.push_back(cppCode + ".fG");
                fFormatArgs.push_back(cppCode + ".fB");
                fFormatArgs.push_back(cppCode + ".fA");
                break;
            case Layout::CType::kSkRect: // fall through
            case Layout::CType::kDefault:
                fFormatArgs.push_back(cppCode + ".left()");
                fFormatArgs.push_back(cppCode + ".top()");
                fFormatArgs.push_back(cppCode + ".right()");
                fFormatArgs.push_back(cppCode + ".bottom()");
                break;
            default:
                SkASSERT(false);
        }
    } else if (type.kind() == Type::kEnum_Kind) {
        this->write("%d");
        fFormatArgs.push_back("(int) " + cppCode);
    } else if (type == *fContext.fInt4_Type ||
               type == *fContext.fShort4_Type ||
               type == *fContext.fByte4_Type) {
        this->write(type.name() + "(%d, %d, %d, %d)");
        fFormatArgs.push_back(cppCode + ".left()");
        fFormatArgs.push_back(cppCode + ".top()");
        fFormatArgs.push_back(cppCode + ".right()");
        fFormatArgs.push_back(cppCode + ".bottom()");
    } else {
        printf("unsupported runtime value type '%s'\n", String(type.fName).c_str());
        SkASSERT(false);
    }
}

void CPPCodeGenerator::writeVarInitializer(const Variable& var, const Expression& value) {
    if (is_private(var)) {
        this->writeRuntimeValue(var.fType, var.fModifiers.fLayout, var.fName);
    } else {
        this->writeExpression(value, kTopLevel_Precedence);
    }
}

String CPPCodeGenerator::getSamplerHandle(const Variable& var) {
    int samplerCount = 0;
    for (const auto param : fSectionAndParameterHelper.getParameters()) {
        if (&var == param) {
            return "args.fTexSamplers[" + to_string(samplerCount) + "]";
        }
        if (param->fType.kind() == Type::kSampler_Kind) {
            ++samplerCount;
        }
    }
    ABORT("should have found sampler in parameters\n");
}

void CPPCodeGenerator::writeIntLiteral(const IntLiteral& i) {
    this->write(to_string((int32_t) i.fValue));
}

void CPPCodeGenerator::writeSwizzle(const Swizzle& swizzle) {
    if (fCPPMode) {
        SkASSERT(swizzle.fComponents.size() == 1); // no support for multiple swizzle components yet
        this->writeExpression(*swizzle.fBase, kPostfix_Precedence);
        switch (swizzle.fComponents[0]) {
            case 0: this->write(".left()");   break;
            case 1: this->write(".top()");    break;
            case 2: this->write(".right()");  break;
            case 3: this->write(".bottom()"); break;
        }
    } else {
        INHERITED::writeSwizzle(swizzle);
    }
}

void CPPCodeGenerator::writeVariableReference(const VariableReference& ref) {
    if (fCPPMode) {
        this->write(ref.fVariable.fName);
        return;
    }
    switch (ref.fVariable.fModifiers.fLayout.fBuiltin) {
        case SK_INCOLOR_BUILTIN:
            this->write("%s");
            // EmitArgs.fInputColor is automatically set to half4(1) if
            // no input was specified
            fFormatArgs.push_back(String("args.fInputColor"));
            break;
        case SK_OUTCOLOR_BUILTIN:
            this->write("%s");
            fFormatArgs.push_back(String("args.fOutputColor"));
            break;
        case SK_WIDTH_BUILTIN:
            this->write("sk_Width");
            break;
        case SK_HEIGHT_BUILTIN:
            this->write("sk_Height");
            break;
        default:
            if (ref.fVariable.fType.kind() == Type::kSampler_Kind) {
                this->write("%s");
                fFormatArgs.push_back("fragBuilder->getProgramBuilder()->samplerVariable(" +
                                      this->getSamplerHandle(ref.fVariable) + ").c_str()");
                return;
            }
            if (ref.fVariable.fModifiers.fFlags & Modifiers::kUniform_Flag) {
                this->write("%s");
                String name = ref.fVariable.fName;
                String var = String::printf("args.fUniformHandler->getUniformCStr(%sVar)",
                                            HCodeGenerator::FieldName(name.c_str()).c_str());
                String code;
                if (ref.fVariable.fModifiers.fLayout.fWhen.size()) {
                    code = String::printf("%sVar.isValid() ? %s : \"%s\"",
                                          HCodeGenerator::FieldName(name.c_str()).c_str(),
                                          var.c_str(),
                                          default_value(ref.fVariable.fType).c_str());
                } else {
                    code = var;
                }
                fFormatArgs.push_back(code);
            } else if (SectionAndParameterHelper::IsParameter(ref.fVariable)) {
                String name(ref.fVariable.fName);
                this->writeRuntimeValue(ref.fVariable.fType, ref.fVariable.fModifiers.fLayout,
                                        String::printf("_outer.%s()", name.c_str()).c_str());
            } else {
                this->write(ref.fVariable.fName);
            }
    }
}

void CPPCodeGenerator::writeIfStatement(const IfStatement& s) {
    if (s.fIsStatic) {
        this->write("@");
    }
    INHERITED::writeIfStatement(s);
}

void CPPCodeGenerator::writeReturnStatement(const ReturnStatement& s) {
    if (fInMain) {
        fErrors.error(s.fOffset, "fragmentProcessor main() may not contain return statements");
    }
    INHERITED::writeReturnStatement(s);
}

void CPPCodeGenerator::writeSwitchStatement(const SwitchStatement& s) {
    if (s.fIsStatic) {
        this->write("@");
    }
    INHERITED::writeSwitchStatement(s);
}

void CPPCodeGenerator::writeFieldAccess(const FieldAccess& access) {
    if (access.fBase->fType.name() == "fragmentProcessor") {
        // Special field access on fragment processors are converted into function calls on
        // GrFragmentProcessor's getters.
        if (access.fBase->fKind != Expression::kVariableReference_Kind) {
            fErrors.error(access.fBase->fOffset, "fragmentProcessor must be a reference\n");
            return;
        }

        const Type::Field& field = fContext.fFragmentProcessor_Type->fields()[access.fFieldIndex];
        int index = getChildFPIndex((const VariableReference&) *access.fBase);
        String cppAccess = String::printf("_outer.childProcessor(%s).%s()",
                                          to_string(index).c_str(), String(field.fName).c_str());

        if (fCPPMode) {
            this->write(cppAccess.c_str());
        } else {
            writeRuntimeValue(*field.fType, Layout(), cppAccess);
        }
        return;
    }
    INHERITED::writeFieldAccess(access);
}

int CPPCodeGenerator::getChildFPIndex(const VariableReference& reference) const {
    int index = 0;
    bool found = false;
    for (const auto& p : fProgram) {
        if (ProgramElement::kVar_Kind == p.fKind) {
            const VarDeclarations& decls = (const VarDeclarations&) p;
            for (const auto& raw : decls.fVars) {
                const VarDeclaration& decl = (VarDeclaration&) *raw;
                if (decl.fVar == &reference.fVariable) {
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
    return index;
}

void CPPCodeGenerator::writeFunctionCall(const FunctionCall& c) {
    if (c.fFunction.fBuiltin && c.fFunction.fName == "process") {
        // Sanity checks that are detected by function definition in sksl_fp.inc
        SkASSERT(c.fArguments.size() == 1 || c.fArguments.size() == 2);
        SkASSERT("fragmentProcessor" == c.fArguments[0]->fType.name());

        // Actually fail during compilation if arguments with valid types are
        // provided that are not variable references, since process() is a
        // special function that impacts code emission.
        if (c.fArguments[0]->fKind != Expression::kVariableReference_Kind) {
            fErrors.error(c.fArguments[0]->fOffset,
                    "process()'s fragmentProcessor argument must be a variable reference\n");
            return;
        }
        if (c.fArguments.size() > 1) {
            // Second argument must also be a half4 expression
            SkASSERT("half4" == c.fArguments[1]->fType.name());
        }
        int index = getChildFPIndex((const VariableReference&) *c.fArguments[0]);

        // Start a new extra emit code section so that the emitted child processor can depend on
        // sksl variables defined in earlier sksl code.
        this->newExtraEmitCodeBlock();

        // Set to the empty string when no input color parameter should be emitted, which means this
        // must be properly formatted with a prefixed comma when the parameter should be inserted
        // into the emitChild() parameter list.
        String inputArg;
        if (c.fArguments.size() > 1) {
            SkASSERT(c.fArguments.size() == 2);
            // Use the emitChild() variant that accepts an input color, so convert the 2nd
            // argument's expression into C++ code that produces sksl stored in an SkString.
            String inputName = "_input" + to_string(index);
            addExtraEmitCodeLine(convertSKSLExpressionToCPP(*c.fArguments[1], inputName));

            // emitChild() needs a char*
            inputArg = ", " + inputName + ".c_str()";
        }

        // Write the output handling after the possible input handling
        String childName = "_child" + to_string(index);
        addExtraEmitCodeLine("SkString " + childName + "(\"" + childName + "\");");
        addExtraEmitCodeLine("this->emitChild(" + to_string(index) + inputArg +
                             ", &" + childName + ", args);");

        this->write("%s");
        fFormatArgs.push_back(childName + ".c_str()");
        return;
    }
    INHERITED::writeFunctionCall(c);
    if (c.fFunction.fBuiltin && c.fFunction.fName == "texture") {
        this->write(".%s");
        SkASSERT(c.fArguments.size() >= 1);
        SkASSERT(c.fArguments[0]->fKind == Expression::kVariableReference_Kind);
        String sampler = this->getSamplerHandle(((VariableReference&) *c.fArguments[0]).fVariable);
        fFormatArgs.push_back("fragBuilder->getProgramBuilder()->samplerSwizzle(" + sampler +
                              ").c_str()");
    }
}

void CPPCodeGenerator::writeFunction(const FunctionDefinition& f) {
    if (f.fDeclaration.fName == "main") {
        fFunctionHeader = "";
        OutputStream* oldOut = fOut;
        StringStream buffer;
        fOut = &buffer;
        fInMain = true;
        for (const auto& s : ((Block&) *f.fBody).fStatements) {
            this->writeStatement(*s);
            this->writeLine();
        }
        fInMain = false;

        fOut = oldOut;
        this->write(fFunctionHeader);
        this->write(buffer.str());
    } else {
        INHERITED::writeFunction(f);
    }
}

void CPPCodeGenerator::writeSetting(const Setting& s) {
    static constexpr const char* kPrefix = "sk_Args.";
    if (!strncmp(s.fName.c_str(), kPrefix, strlen(kPrefix))) {
        const char* name = s.fName.c_str() + strlen(kPrefix);
        this->writeRuntimeValue(s.fType, Layout(), HCodeGenerator::FieldName(name).c_str());
    } else {
        this->write(s.fName.c_str());
    }
}

bool CPPCodeGenerator::writeSection(const char* name, const char* prefix) {
    const Section* s = fSectionAndParameterHelper.getSection(name);
    if (s) {
        this->writef("%s%s", prefix, s->fText.c_str());
        return true;
    }
    return false;
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
        const Variable& var = *((VarDeclaration&) *decls.fVars[0]).fVar;
        if (var.fModifiers.fFlags & (Modifiers::kIn_Flag | Modifiers::kUniform_Flag) ||
            -1 != var.fModifiers.fLayout.fBuiltin) {
            return;
        }
    }
    INHERITED::writeProgramElement(p);
}

void CPPCodeGenerator::addUniform(const Variable& var) {
    if (!needs_uniform_var(var)) {
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
    } else if (var.fType == *fContext.fHalf_Type) {
        type = "kHalf_GrSLType";
    } else if (var.fType == *fContext.fFloat2_Type) {
        type = "kFloat2_GrSLType";
    } else if (var.fType == *fContext.fHalf2_Type) {
        type = "kHalf2_GrSLType";
    } else if (var.fType == *fContext.fFloat4_Type) {
        type = "kFloat4_GrSLType";
    } else if (var.fType == *fContext.fHalf4_Type) {
        type = "kHalf4_GrSLType";
    } else if (var.fType == *fContext.fFloat4x4_Type) {
        type = "kFloat4x4_GrSLType";
    } else if (var.fType == *fContext.fHalf4x4_Type) {
        type = "kHalf4x4_GrSLType";
    } else {
        ABORT("unsupported uniform type: %s %s;\n", String(var.fType.fName).c_str(),
              String(var.fName).c_str());
    }
    if (var.fModifiers.fLayout.fWhen.size()) {
        this->writef("        if (%s) {\n    ", var.fModifiers.fLayout.fWhen.c_str());
    }
    String name(var.fName);
    this->writef("        %sVar = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, %s, "
                 "%s, \"%s\");\n", HCodeGenerator::FieldName(name.c_str()).c_str(), type, precision,
                 name.c_str());
    if (var.fModifiers.fLayout.fWhen.size()) {
        this->write("        }\n");
    }
}

void CPPCodeGenerator::writeInputVars() {
}

void CPPCodeGenerator::writePrivateVars() {
    for (const auto& p : fProgram) {
        if (ProgramElement::kVar_Kind == p.fKind) {
            const VarDeclarations& decls = (const VarDeclarations&) p;
            for (const auto& raw : decls.fVars) {
                VarDeclaration& decl = (VarDeclaration&) *raw;
                if (is_private(*decl.fVar)) {
                    if (decl.fVar->fType == *fContext.fFragmentProcessor_Type) {
                        fErrors.error(decl.fOffset,
                                      "fragmentProcessor variables must be declared 'in'");
                        return;
                    }
                    this->writef("%s %s = %s;\n",
                                 HCodeGenerator::FieldType(fContext, decl.fVar->fType,
                                                           decl.fVar->fModifiers.fLayout).c_str(),
                                 String(decl.fVar->fName).c_str(),
                                 default_value(*decl.fVar).c_str());
                } else if (decl.fVar->fModifiers.fLayout.fFlags & Layout::kTracked_Flag) {
                    // An auto-tracked uniform in variable, so add a field to hold onto the prior
                    // state. Note that tracked variables must be uniform in's and that is validated
                    // before writePrivateVars() is called.
                    const UniformCTypeMapper* mapper = UniformCTypeMapper::Get(fContext, *decl.fVar);
                    SkASSERT(mapper && mapper->supportsTracking());

                    String name = HCodeGenerator::FieldName(String(decl.fVar->fName).c_str());
                    // The member statement is different if the mapper reports a default value
                    if (mapper->defaultValue().size() > 0) {
                        this->writef("%s %sPrev = %s;\n",
                                     Layout::CTypeToStr(mapper->ctype()), name.c_str(),
                                     mapper->defaultValue().c_str());
                    } else {
                        this->writef("%s %sPrev;\n",
                                     Layout::CTypeToStr(mapper->ctype()), name.c_str());
                    }
                }
            }
        }
    }
}

void CPPCodeGenerator::writePrivateVarValues() {
    for (const auto& p : fProgram) {
        if (ProgramElement::kVar_Kind == p.fKind) {
            const VarDeclarations& decls = (const VarDeclarations&) p;
            for (const auto& raw : decls.fVars) {
                VarDeclaration& decl = (VarDeclaration&) *raw;
                if (is_private(*decl.fVar) && decl.fValue) {
                    this->writef("%s = ", String(decl.fVar->fName).c_str());
                    fCPPMode = true;
                    this->writeExpression(*decl.fValue, kAssignment_Precedence);
                    fCPPMode = false;
                    this->write(";\n");
                }
            }
        }
    }
}

static bool is_accessible(const Variable& var) {
    return Type::kSampler_Kind != var.fType.kind() &&
           Type::kOther_Kind != var.fType.kind();
}

void CPPCodeGenerator::newExtraEmitCodeBlock() {
    // This should only be called when emitting SKSL for emitCode(), which can be detected if the
    // cpp buffer is not null, and the cpp buffer is not the current output.
    SkASSERT(fCPPBuffer && fCPPBuffer != fOut);

    // Start a new block as an empty string
    fExtraEmitCodeBlocks.push_back("");
    // Mark its location in the output buffer, uses ${\d} for the token since ${} will not occur in
    // valid sksl and makes detection trivial.
    this->writef("${%zu}", fExtraEmitCodeBlocks.size() - 1);
}

void CPPCodeGenerator::addExtraEmitCodeLine(const String& toAppend) {
    SkASSERT(fExtraEmitCodeBlocks.size() > 0);
    String& currentBlock = fExtraEmitCodeBlocks[fExtraEmitCodeBlocks.size() - 1];
    // Automatically add indentation and newline
    currentBlock += "        " + toAppend + "\n";
}

void CPPCodeGenerator::flushEmittedCode() {
    if (fCPPBuffer == nullptr) {
        // Not actually within writeEmitCode() so nothing to flush
        return;
    }

    StringStream* skslBuffer = static_cast<StringStream*>(fOut);

    String sksl = skslBuffer->str();
    // Empty the accumulation buffer since its current contents are consumed.
    skslBuffer->reset();

    // Switch to the cpp buffer
    fOut = fCPPBuffer;

    // Iterate through the sksl, keeping track of where the last statement ended (e.g. the latest
    // encountered ';', '{', or '}'). If an extra emit code block token is encountered then the
    // code from 0 to last statement end is sent to writeCodeAppend, the extra code block is
    // appended to the cpp buffer, and then the sksl string is trimmed to start where the last
    // statement left off (minus the encountered token).
    size_t i = 0;
    int flushPoint = -1;
    int tokenStart = -1;
    while (i < sksl.size()) {
        if (tokenStart >= 0) {
            // Looking for the end of the token
            if (sksl[i] == '}') {
                // Must append the sksl from 0 to flushPoint (inclusive) then the extra code
                // accumulated in the block with index parsed from chars [tokenStart+2, i-1]
                String toFlush = String(sksl.c_str(), flushPoint + 1);
                // writeCodeAppend automatically removes the format args that it consumed, so
                // fFormatArgs will be in a valid state for any future sksl
                this->writeCodeAppend(toFlush);

                int codeBlock = stoi(String(sksl.c_str() + tokenStart + 2, i - tokenStart - 2));
                SkASSERT(codeBlock < (int) fExtraEmitCodeBlocks.size());
                if (fExtraEmitCodeBlocks[codeBlock].size() > 0) {
                    this->write(fExtraEmitCodeBlocks[codeBlock].c_str());
                }

                // Now reset the sksl buffer to start after the flush point, but remove the token.
                String compacted = String(sksl.c_str() + flushPoint + 1,
                                          tokenStart - flushPoint - 1);
                if (i < sksl.size() - 1) {
                    compacted += String(sksl.c_str() + i + 1, sksl.size() - i - 1);
                }
                sksl = compacted;

                // And reset iteration
                i = -1;
                flushPoint = -1;
                tokenStart = -1;
            }
        } else {
            // Looking for the start of extra emit block tokens, and tracking when statements end
            if (sksl[i] == ';' || sksl[i] == '{' || sksl[i] == '}') {
                flushPoint = i;
            } else if (i < sksl.size() - 1 && sksl[i] == '$' && sksl[i + 1] == '{') {
                // found an extra emit code block token
                tokenStart = i++;
            }
        }
        i++;
    }

    // Once we've gone through the sksl string to this point, there are no remaining extra emit
    // code blocks to interleave, so append the remainder as usual.
    this->writeCodeAppend(sksl);

    // After appending, switch back to the emptied sksl buffer and reset the extra code blocks
    fOut = skslBuffer;
    fExtraEmitCodeBlocks.clear();
}

void CPPCodeGenerator::writeCodeAppend(const String& code) {
    // codeAppendf can only handle appending 1024 bytes at a time, so we need to break the string
    // into chunks. Unfortunately we can't tell exactly how long the string is going to end up,
    // because printf escape sequences get replaced by strings of unknown length, but keeping the
    // format string below 512 bytes is probably safe.
    static constexpr size_t maxChunkSize = 512;
    size_t start = 0;
    size_t index = 0;
    size_t argStart = 0;
    size_t argCount;
    while (index < code.size()) {
        argCount = 0;
        this->write("        fragBuilder->codeAppendf(\"");
        while (index < code.size() && index < start + maxChunkSize) {
            if ('%' == code[index]) {
                if (index == start + maxChunkSize - 1 || index == code.size() - 1) {
                    break;
                }
                if (code[index + 1] != '%') {
                    ++argCount;
                }
            } else if ('\\' == code[index] && index == start + maxChunkSize - 1) {
                // avoid splitting an escape sequence that happens to fall across a chunk boundary
                break;
            }
            ++index;
        }
        fOut->write(code.c_str() + start, index - start);
        this->write("\"");
        for (size_t i = argStart; i < argStart + argCount; ++i) {
            this->writef(", %s", fFormatArgs[i].c_str());
        }
        this->write(");\n");
        argStart += argCount;
        start = index;
    }

    // argStart is equal to the number of fFormatArgs that were consumed
    // so they should be removed from the list
    if (argStart > 0) {
        fFormatArgs.erase(fFormatArgs.begin(), fFormatArgs.begin() + argStart);
    }
}

String CPPCodeGenerator::convertSKSLExpressionToCPP(const Expression& e,
                                                    const String& cppVar) {
    // To do this conversion, we temporarily switch the sksl output stream
    // to an empty stringstream and reset the format args to empty.
    OutputStream* oldSKSL = fOut;
    StringStream exprBuffer;
    fOut = &exprBuffer;

    std::vector<String> oldArgs(fFormatArgs);
    fFormatArgs.clear();

    // Convert the argument expression into a format string and args
    this->writeExpression(e, Precedence::kTopLevel_Precedence);
    std::vector<String> newArgs(fFormatArgs);
    String expr = exprBuffer.str();

    // After generating, restore the original output stream and format args
    fFormatArgs = oldArgs;
    fOut = oldSKSL;

    // The sksl written to exprBuffer is not processed by flushEmittedCode(), so any extra emit code
    // block tokens won't get handled. So we need to strip them from the expression and stick them
    // to the end of the original sksl stream.
    String exprFormat = "";
    int tokenStart = -1;
    for (size_t i = 0; i < expr.size(); i++) {
        if (tokenStart >= 0) {
            if (expr[i] == '}') {
                // End of the token, so append the token to fOut
                fOut->write(expr.c_str() + tokenStart, i - tokenStart + 1);
                tokenStart = -1;
            }
        } else {
            if (i < expr.size() - 1 && expr[i] == '$' && expr[i + 1] == '{') {
                tokenStart = i++;
            } else {
                exprFormat += expr[i];
            }
        }
    }

    // Now build the final C++ code snippet from the format string and args
    String cppExpr;
    if (newArgs.size() == 0) {
        // This was a static expression, so we can simplify the input
        // color declaration in the emitted code to just a static string
        cppExpr = "SkString " + cppVar + "(\"" + exprFormat + "\");";
    } else {
        // String formatting must occur dynamically, so have the C++ declaration
        // use SkStringPrintf with the format args that were accumulated
        // when the expression was written.
        cppExpr = "SkString " + cppVar + " = SkStringPrintf(\"" + exprFormat + "\"";
        for (size_t i = 0; i < newArgs.size(); i++) {
            cppExpr += ", " + newArgs[i];
        }
        cppExpr += ");";
    }
    return cppExpr;
}

bool CPPCodeGenerator::writeEmitCode(std::vector<const Variable*>& uniforms) {
    this->write("    void emitCode(EmitArgs& args) override {\n"
                "        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;\n");
    this->writef("        const %s& _outer = args.fFp.cast<%s>();\n"
                 "        (void) _outer;\n",
                 fFullName.c_str(), fFullName.c_str());
    for (const auto& p : fProgram) {
        if (ProgramElement::kVar_Kind == p.fKind) {
            const VarDeclarations& decls = (const VarDeclarations&) p;
            for (const auto& raw : decls.fVars) {
                VarDeclaration& decl = (VarDeclaration&) *raw;
                String nameString(decl.fVar->fName);
                const char* name = nameString.c_str();
                if (SectionAndParameterHelper::IsParameter(*decl.fVar) &&
                    is_accessible(*decl.fVar)) {
                    this->writef("        auto %s = _outer.%s();\n"
                                 "        (void) %s;\n",
                                 name, name, name);
                }
            }
        }
    }
    this->writePrivateVarValues();
    for (const auto u : uniforms) {
        this->addUniform(*u);
    }
    this->writeSection(EMIT_CODE_SECTION);

    // Save original buffer as the CPP buffer for flushEmittedCode()
    fCPPBuffer = fOut;
    StringStream skslBuffer;
    fOut = &skslBuffer;

    this->newExtraEmitCodeBlock();
    bool result = INHERITED::generateCode();
    this->flushEmittedCode();

    // Then restore the original CPP buffer and close the function
    fOut = fCPPBuffer;
    fCPPBuffer = nullptr;
    this->write("    }\n");
    return result;
}

void CPPCodeGenerator::writeSetData(std::vector<const Variable*>& uniforms) {
    const char* fullName = fFullName.c_str();
    const Section* section = fSectionAndParameterHelper.getSection(SET_DATA_SECTION);
    const char* pdman = section ? section->fArgument.c_str() : "pdman";
    this->writef("    void onSetData(const GrGLSLProgramDataManager& %s, "
                                    "const GrFragmentProcessor& _proc) override {\n",
                 pdman);
    bool wroteProcessor = false;
    for (const auto u : uniforms) {
        if (is_uniform_in(*u)) {
            if (!wroteProcessor) {
                this->writef("        const %s& _outer = _proc.cast<%s>();\n", fullName, fullName);
                wroteProcessor = true;
                this->writef("        {\n");
            }

            const UniformCTypeMapper* mapper = UniformCTypeMapper::Get(fContext, *u);
            SkASSERT(mapper);

            String nameString(u->fName);
            const char* name = nameString.c_str();

            // Switches for setData behavior in the generated code
            bool conditionalUniform = u->fModifiers.fLayout.fWhen != "";
            bool isTracked = u->fModifiers.fLayout.fFlags & Layout::kTracked_Flag;
            bool needsValueDeclaration = isTracked || !mapper->canInlineUniformValue();

            String uniformName = HCodeGenerator::FieldName(name) + "Var";

            String indent = "        "; // 8 by default, 12 when nested for conditional uniforms
            if (conditionalUniform) {
                // Add a pre-check to make sure the uniform was emitted
                // before trying to send any data to the GPU
                this->writef("        if (%s.isValid()) {\n", uniformName.c_str());
                indent += "    ";
            }

            String valueVar = "";
            if (needsValueDeclaration) {
                valueVar.appendf("%sValue", name);
                // Use AccessType since that will match the return type of _outer's public API.
                String valueType = HCodeGenerator::AccessType(fContext, u->fType,
                                                              u->fModifiers.fLayout);
                this->writef("%s%s %s = _outer.%s();\n",
                             indent.c_str(), valueType.c_str(), valueVar.c_str(), name);
            } else {
                // Not tracked and the mapper only needs to use the value once
                // so send it a safe expression instead of the variable name
                valueVar.appendf("(_outer.%s())", name);
            }

            if (isTracked) {
                SkASSERT(mapper->supportsTracking());

                String prevVar = HCodeGenerator::FieldName(name) + "Prev";
                this->writef("%sif (%s) {\n"
                             "%s    %s;\n"
                             "%s    %s;\n"
                             "%s}\n", indent.c_str(),
                        mapper->dirtyExpression(valueVar, prevVar).c_str(), indent.c_str(),
                        mapper->saveState(valueVar, prevVar).c_str(), indent.c_str(),
                        mapper->setUniform(pdman, uniformName, valueVar).c_str(), indent.c_str());
            } else {
                this->writef("%s%s;\n", indent.c_str(),
                        mapper->setUniform(pdman, uniformName, valueVar).c_str());
            }

            if (conditionalUniform) {
                // Close the earlier precheck block
                this->writef("        }\n");
            }
        }
    }
    if (wroteProcessor) {
        this->writef("        }\n");
    }
    if (section) {
        int samplerIndex = 0;
        for (const auto& p : fProgram) {
            if (ProgramElement::kVar_Kind == p.fKind) {
                const VarDeclarations& decls = (const VarDeclarations&) p;
                for (const auto& raw : decls.fVars) {
                    VarDeclaration& decl = (VarDeclaration&) *raw;
                    String nameString(decl.fVar->fName);
                    const char* name = nameString.c_str();
                    if (decl.fVar->fType.kind() == Type::kSampler_Kind) {
                        this->writef("        GrSurfaceProxy& %sProxy = "
                                     "*_outer.textureSampler(%d).proxy();\n",
                                     name, samplerIndex);
                        this->writef("        GrTexture& %s = *%sProxy.peekTexture();\n",
                                     name, name);
                        this->writef("        (void) %s;\n", name);
                        ++samplerIndex;
                    } else if (needs_uniform_var(*decl.fVar)) {
                        this->writef("        UniformHandle& %s = %sVar;\n"
                                     "        (void) %s;\n",
                                     name, HCodeGenerator::FieldName(name).c_str(), name);
                    } else if (SectionAndParameterHelper::IsParameter(*decl.fVar) &&
                               decl.fVar->fType != *fContext.fFragmentProcessor_Type) {
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

void CPPCodeGenerator::writeOnTextureSampler() {
    bool foundSampler = false;
    for (const auto& param : fSectionAndParameterHelper.getParameters()) {
        if (param->fType.kind() == Type::kSampler_Kind) {
            if (!foundSampler) {
                this->writef(
                        "const GrFragmentProcessor::TextureSampler& %s::onTextureSampler(int "
                        "index) const {\n",
                        fFullName.c_str());
                this->writef("    return IthTextureSampler(index, %s",
                             HCodeGenerator::FieldName(String(param->fName).c_str()).c_str());
                foundSampler = true;
            } else {
                this->writef(", %s",
                             HCodeGenerator::FieldName(String(param->fName).c_str()).c_str());
            }
        }
    }
    if (foundSampler) {
        this->write(");\n}\n");
    }
}

void CPPCodeGenerator::writeClone() {
    if (!this->writeSection(CLONE_SECTION)) {
        if (fSectionAndParameterHelper.getSection(FIELDS_SECTION)) {
            fErrors.error(0, "fragment processors with custom @fields must also have a custom"
                             "@clone");
        }
        this->writef("%s::%s(const %s& src)\n"
                     ": INHERITED(k%s_ClassID, src.optimizationFlags())", fFullName.c_str(),
                     fFullName.c_str(), fFullName.c_str(), fFullName.c_str());
        for (const auto& param : fSectionAndParameterHelper.getParameters()) {
            if (param->fType == *fContext.fFragmentProcessor_Type) {
                continue;
            }
            String fieldName = HCodeGenerator::FieldName(String(param->fName).c_str());
            this->writef("\n, %s(src.%s)",
                         fieldName.c_str(),
                         fieldName.c_str());
        }
        const auto transforms = fSectionAndParameterHelper.getSections(COORD_TRANSFORM_SECTION);
        for (size_t i = 0; i < transforms.size(); ++i) {
            const Section& s = *transforms[i];
            String fieldName = HCodeGenerator::CoordTransformName(s.fArgument, i);
            this->writef("\n, %s(src.%s)", fieldName.c_str(), fieldName.c_str());
        }
        this->writef(" {\n");
        int childCount = 0;
        int samplerCount = 0;
        for (const auto& param : fSectionAndParameterHelper.getParameters()) {
            if (param->fType.kind() == Type::kSampler_Kind) {
                ++samplerCount;
            } else if (param->fType == *fContext.fFragmentProcessor_Type) {
                this->writef("    this->registerChildProcessor(src.childProcessor(%d).clone());"
                             "\n", childCount++);
            }
        }
        if (samplerCount) {
            this->writef("     this->setTextureSamplerCnt(%d);", samplerCount);
        }
        for (size_t i = 0; i < transforms.size(); ++i) {
            const Section& s = *transforms[i];
            String fieldName = HCodeGenerator::CoordTransformName(s.fArgument, i);
            this->writef("    this->addCoordTransform(&%s);\n", fieldName.c_str());
        }
        this->write("}\n");
        this->writef("std::unique_ptr<GrFragmentProcessor> %s::clone() const {\n",
                     fFullName.c_str());
        this->writef("    return std::unique_ptr<GrFragmentProcessor>(new %s(*this));\n",
                     fFullName.c_str());
        this->write("}\n");
    }
}

void CPPCodeGenerator::writeTest() {
    const Section* test = fSectionAndParameterHelper.getSection(TEST_CODE_SECTION);
    if (test) {
        this->writef(
                "GR_DEFINE_FRAGMENT_PROCESSOR_TEST(%s);\n"
                "#if GR_TEST_UTILS\n"
                "std::unique_ptr<GrFragmentProcessor> %s::TestCreate(GrProcessorTestData* %s) {\n",
                fFullName.c_str(),
                fFullName.c_str(),
                test->fArgument.c_str());
        this->writeSection(TEST_CODE_SECTION);
        this->write("}\n"
                    "#endif\n");
    }
}

void CPPCodeGenerator::writeGetKey() {
    this->writef("void %s::onGetGLSLProcessorKey(const GrShaderCaps& caps, "
                                                "GrProcessorKeyBuilder* b) const {\n",
                 fFullName.c_str());
    for (const auto& param : fSectionAndParameterHelper.getParameters()) {
        String nameString(param->fName);
        const char* name = nameString.c_str();
        if (param->fModifiers.fLayout.fKey != Layout::kNo_Key &&
            (param->fModifiers.fFlags & Modifiers::kUniform_Flag)) {
            fErrors.error(param->fOffset,
                          "layout(key) may not be specified on uniforms");
        }
        switch (param->fModifiers.fLayout.fKey) {
            case Layout::kKey_Key:
                if (param->fType == *fContext.fFloat4x4_Type) {
                    ABORT("no automatic key handling for float4x4\n");
                } else if (param->fType == *fContext.fFloat2_Type) {
                    this->writef("    b->add32(%s.fX);\n",
                                 HCodeGenerator::FieldName(name).c_str());
                    this->writef("    b->add32(%s.fY);\n",
                                 HCodeGenerator::FieldName(name).c_str());
                } else if (param->fType == *fContext.fFloat4_Type) {
                    this->writef("    b->add32(%s.x());\n",
                                 HCodeGenerator::FieldName(name).c_str());
                    this->writef("    b->add32(%s.y());\n",
                                 HCodeGenerator::FieldName(name).c_str());
                    this->writef("    b->add32(%s.width());\n",
                                 HCodeGenerator::FieldName(name).c_str());
                    this->writef("    b->add32(%s.height());\n",
                                 HCodeGenerator::FieldName(name).c_str());
                } else {
                    this->writef("    b->add32((int32_t) %s);\n",
                                 HCodeGenerator::FieldName(name).c_str());
                }
                break;
            case Layout::kIdentity_Key:
                if (param->fType.kind() != Type::kMatrix_Kind) {
                    fErrors.error(param->fOffset,
                                  "layout(key=identity) requires matrix type");
                }
                this->writef("    b->add32(%s.isIdentity() ? 1 : 0);\n",
                             HCodeGenerator::FieldName(name).c_str());
                break;
            case Layout::kNo_Key:
                break;
        }
    }
    this->write("}\n");
}

bool CPPCodeGenerator::generateCode() {
    std::vector<const Variable*> uniforms;
    for (const auto& p : fProgram) {
        if (ProgramElement::kVar_Kind == p.fKind) {
            const VarDeclarations& decls = (const VarDeclarations&) p;
            for (const auto& raw : decls.fVars) {
                VarDeclaration& decl = (VarDeclaration&) *raw;
                if ((decl.fVar->fModifiers.fFlags & Modifiers::kUniform_Flag) &&
                           decl.fVar->fType.kind() != Type::kSampler_Kind) {
                    uniforms.push_back(decl.fVar);
                }

                if (is_uniform_in(*decl.fVar)) {
                    // Validate the "uniform in" declarations to make sure they are fully supported,
                    // instead of generating surprising C++
                    const UniformCTypeMapper* mapper =
                            UniformCTypeMapper::Get(fContext, *decl.fVar);
                    if (mapper == nullptr) {
                        fErrors.error(decl.fOffset, String(decl.fVar->fName)
                                + "'s type is not supported for use as a 'uniform in'");
                        return false;
                    }
                    if (decl.fVar->fModifiers.fLayout.fFlags & Layout::kTracked_Flag) {
                        if (!mapper->supportsTracking()) {
                            fErrors.error(decl.fOffset, String(decl.fVar->fName)
                                    + "'s type does not support state tracking");
                            return false;
                        }
                    }

                } else {
                    // If it's not a uniform_in, it's an error to be tracked
                    if (decl.fVar->fModifiers.fLayout.fFlags & Layout::kTracked_Flag) {
                        fErrors.error(decl.fOffset, "Non-'in uniforms' cannot be tracked");
                        return false;
                    }
                }
            }
        }
    }
    const char* baseName = fName.c_str();
    const char* fullName = fFullName.c_str();
    this->writef("%s\n", HCodeGenerator::GetHeader(fProgram, fErrors).c_str());
    this->writef(kFragmentProcessorHeader, fullName);
    this->writef("#include \"%s.h\"\n", fullName);
    this->writeSection(CPP_SECTION);
    this->writef("#include \"glsl/GrGLSLFragmentProcessor.h\"\n"
                 "#include \"glsl/GrGLSLFragmentShaderBuilder.h\"\n"
                 "#include \"glsl/GrGLSLProgramBuilder.h\"\n"
                 "#include \"GrTexture.h\"\n"
                 "#include \"SkSLCPP.h\"\n"
                 "#include \"SkSLUtil.h\"\n"
                 "class GrGLSL%s : public GrGLSLFragmentProcessor {\n"
                 "public:\n"
                 "    GrGLSL%s() {}\n",
                 baseName, baseName);
    bool result = this->writeEmitCode(uniforms);
    this->write("private:\n");
    this->writeSetData(uniforms);
    this->writePrivateVars();
    for (const auto& u : uniforms) {
        if (needs_uniform_var(*u) && !(u->fModifiers.fFlags & Modifiers::kIn_Flag)) {
            this->writef("    UniformHandle %sVar;\n",
                         HCodeGenerator::FieldName(String(u->fName).c_str()).c_str());
        }
    }
    for (const auto& param : fSectionAndParameterHelper.getParameters()) {
        if (needs_uniform_var(*param)) {
            this->writef("    UniformHandle %sVar;\n",
                         HCodeGenerator::FieldName(String(param->fName).c_str()).c_str());
        }
    }
    this->writef("};\n"
                 "GrGLSLFragmentProcessor* %s::onCreateGLSLInstance() const {\n"
                 "    return new GrGLSL%s();\n"
                 "}\n",
                 fullName, baseName);
    this->writeGetKey();
    this->writef("bool %s::onIsEqual(const GrFragmentProcessor& other) const {\n"
                 "    const %s& that = other.cast<%s>();\n"
                 "    (void) that;\n",
                 fullName, fullName, fullName);
    for (const auto& param : fSectionAndParameterHelper.getParameters()) {
        if (param->fType == *fContext.fFragmentProcessor_Type) {
            continue;
        }
        String nameString(param->fName);
        const char* name = nameString.c_str();
        this->writef("    if (%s != that.%s) return false;\n",
                     HCodeGenerator::FieldName(name).c_str(),
                     HCodeGenerator::FieldName(name).c_str());
    }
    this->write("    return true;\n"
                "}\n");
    this->writeClone();
    this->writeOnTextureSampler();
    this->writeTest();
    this->writeSection(CPP_END_SECTION);

    result &= 0 == fErrors.errorCount();
    return result;
}

} // namespace
