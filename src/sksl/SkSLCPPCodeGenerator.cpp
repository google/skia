/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLCPPCodeGenerator.h"

#include "include/private/SkSLSampleUsage.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLCPPUniformCTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLHCodeGenerator.h"
#include "src/sksl/ir/SkSLEnum.h"

#include <algorithm>

#if defined(SKSL_STANDALONE) || GR_TEST_UTILS

namespace SkSL {

static bool needs_uniform_var(const Variable& var) {
    return (var.modifiers().fFlags & Modifiers::kUniform_Flag) &&
            var.type().typeKind() != Type::TypeKind::kSampler;
}

CPPCodeGenerator::CPPCodeGenerator(const Context* context, const Program* program,
                                   ErrorReporter* errors, String name, OutputStream* out)
    : INHERITED(context, program, errors, out)
    , fName(std::move(name))
    , fFullName(String::printf("Gr%s", fName.c_str()))
    , fSectionAndParameterHelper(program, *errors) {
    fLineEnding = "\n";
    fTextureFunctionOverride = "sample";
}

void CPPCodeGenerator::writef(const char* s, va_list va) {
    static constexpr int BUFFER_SIZE = 1024;
    va_list copy;
    va_copy(copy, va);
    char buffer[BUFFER_SIZE];
    int length = std::vsnprintf(buffer, BUFFER_SIZE, s, va);
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
    const Expression& left = *b.left();
    const Expression& right = *b.right();
    Operator op = b.getOperator();
    if (op.kind() == Token::Kind::TK_PERCENT) {
        // need to use "%%" instead of "%" b/c the code will be inside of a printf
        Precedence precedence = op.getBinaryPrecedence();
        if (precedence >= parentPrecedence) {
            this->write("(");
        }
        this->writeExpression(left, precedence);
        this->write(" %% ");
        this->writeExpression(right, precedence);
        if (precedence >= parentPrecedence) {
            this->write(")");
        }
    } else {
        INHERITED::writeBinaryExpression(b, parentPrecedence);
    }
}

static String default_value(const Type& type) {
    if (type.isBoolean()) {
        return "false";
    }
    switch (type.typeKind()) {
        case Type::TypeKind::kScalar: return "0";
        case Type::TypeKind::kVector: return type.name() + "(0)";
        case Type::TypeKind::kMatrix: return type.name() + "(1)";
        default: SK_ABORT("unsupported default_value type");
    }
}

static String default_value(const Variable& var) {
    if (var.modifiers().fLayout.fCType == SkSL::Layout::CType::kSkPMColor4f) {
        return "{SK_FloatNaN, SK_FloatNaN, SK_FloatNaN, SK_FloatNaN}";
    }
    return default_value(var.type());
}

static bool is_private(const Variable& var) {
    const Modifiers& modifiers = var.modifiers();
    return !(modifiers.fFlags & Modifiers::kUniform_Flag) &&
           !(modifiers.fFlags & Modifiers::kIn_Flag) &&
           var.storage() == Variable::Storage::kGlobal &&
           modifiers.fLayout.fBuiltin == -1;
}

static bool is_uniform_in(const Variable& var) {
    const Modifiers& modifiers = var.modifiers();
    return (modifiers.fFlags & Modifiers::kUniform_Flag) &&
           (modifiers.fFlags & Modifiers::kIn_Flag) &&
           var.type().typeKind() != Type::TypeKind::kSampler;
}

String CPPCodeGenerator::formatRuntimeValue(const Type& type,
                                            const Layout& layout,
                                            const String& cppCode,
                                            std::vector<String>* formatArgs) {
    if (type.isArray()) {
        String result("[");
        const char* separator = "";
        for (int i = 0; i < type.columns(); i++) {
            result += separator + this->formatRuntimeValue(type.componentType(), layout,
                                                           "(" + cppCode + ")[" + to_string(i) +
                                                           "]", formatArgs);
            separator = ",";
        }
        result += "]";
        return result;
    }
    if (type.isFloat()) {
        formatArgs->push_back(cppCode);
        return "%f";
    }
    if (type == *fContext.fTypes.fInt) {
        formatArgs->push_back(cppCode);
        return "%d";
    }
    if (type == *fContext.fTypes.fBool) {
        formatArgs->push_back("(" + cppCode + " ? \"true\" : \"false\")");
        return "%s";
    }
    if (type == *fContext.fTypes.fFloat2 || type == *fContext.fTypes.fHalf2) {
        formatArgs->push_back(cppCode + ".fX");
        formatArgs->push_back(cppCode + ".fY");
        return type.name() + "(%f, %f)";
    }
    if (type == *fContext.fTypes.fFloat3 || type == *fContext.fTypes.fHalf3) {
        formatArgs->push_back(cppCode + ".fX");
        formatArgs->push_back(cppCode + ".fY");
        formatArgs->push_back(cppCode + ".fZ");
        return type.name() + "(%f, %f, %f)";
    }
    if (type == *fContext.fTypes.fFloat4 || type == *fContext.fTypes.fHalf4) {
        switch (layout.fCType) {
            case Layout::CType::kSkPMColor:
                formatArgs->push_back("SkGetPackedR32(" + cppCode + ") / 255.0");
                formatArgs->push_back("SkGetPackedG32(" + cppCode + ") / 255.0");
                formatArgs->push_back("SkGetPackedB32(" + cppCode + ") / 255.0");
                formatArgs->push_back("SkGetPackedA32(" + cppCode + ") / 255.0");
                break;
            case Layout::CType::kSkPMColor4f:
                formatArgs->push_back(cppCode + ".fR");
                formatArgs->push_back(cppCode + ".fG");
                formatArgs->push_back(cppCode + ".fB");
                formatArgs->push_back(cppCode + ".fA");
                break;
            case Layout::CType::kSkV4:
                formatArgs->push_back(cppCode + ".x");
                formatArgs->push_back(cppCode + ".y");
                formatArgs->push_back(cppCode + ".z");
                formatArgs->push_back(cppCode + ".w");
                break;
            case Layout::CType::kSkRect:
            case Layout::CType::kDefault:
                formatArgs->push_back(cppCode + ".left()");
                formatArgs->push_back(cppCode + ".top()");
                formatArgs->push_back(cppCode + ".right()");
                formatArgs->push_back(cppCode + ".bottom()");
                break;
            default:
                SkASSERT(false);
        }
        return type.name() + "(%f, %f, %f, %f)";
    }
    if (type.isMatrix()) {
        SkASSERT(type.componentType() == *fContext.fTypes.fFloat ||
                 type.componentType() == *fContext.fTypes.fHalf);

        String format = type.name() + "(";
        for (int c = 0; c < type.columns(); ++c) {
            for (int r = 0; r < type.rows(); ++r) {
                formatArgs->push_back(String::printf("%s.rc(%d, %d)", cppCode.c_str(), r, c));
                format += "%f, ";
            }
        }

        // Replace trailing ", " with ")".
        format.pop_back();
        format.back() = ')';
        return format;
    }
    if (type.isEnum()) {
        formatArgs->push_back("(int) " + cppCode);
        return "%d";
    }
    if (type == *fContext.fTypes.fInt4 ||
        type == *fContext.fTypes.fShort4 ||
        type == *fContext.fTypes.fByte4) {
        formatArgs->push_back(cppCode + ".left()");
        formatArgs->push_back(cppCode + ".top()");
        formatArgs->push_back(cppCode + ".right()");
        formatArgs->push_back(cppCode + ".bottom()");
        return type.name() + "(%d, %d, %d, %d)";
    }

    SkDEBUGFAILF("unsupported runtime value type '%s'\n", String(type.name()).c_str());
    return "";
}

void CPPCodeGenerator::writeRuntimeValue(const Type& type, const Layout& layout,
                                         const String& cppCode) {
    this->write(this->formatRuntimeValue(type, layout, cppCode, &fFormatArgs));
}

void CPPCodeGenerator::writeVarInitializer(const Variable& var, const Expression& value) {
    if (is_private(var)) {
        this->writeRuntimeValue(var.type(), var.modifiers().fLayout, var.name());
    } else {
        this->writeExpression(value, Precedence::kTopLevel);
    }
}

String CPPCodeGenerator::getSamplerHandle(const Variable& var) {
    int samplerCount = 0;
    for (const auto param : fSectionAndParameterHelper.getParameters()) {
        if (&var == param) {
            return "args.fTexSamplers[" + to_string(samplerCount) + "]";
        }
        if (param->type().typeKind() == Type::TypeKind::kSampler) {
            ++samplerCount;
        }
    }
    SK_ABORT("should have found sampler in parameters\n");
}

void CPPCodeGenerator::writeIntLiteral(const IntLiteral& i) {
    this->write(to_string(i.value()));
}

void CPPCodeGenerator::writeSwizzle(const Swizzle& swizzle) {
    if (fCPPMode) {
        // no support for multiple swizzle components yet
        SkASSERT(swizzle.components().size() == 1);
        this->writeExpression(*swizzle.base(), Precedence::kPostfix);
        switch (swizzle.components()[0]) {
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
        this->write(ref.variable()->name());
        return;
    }
    switch (ref.variable()->modifiers().fLayout.fBuiltin) {
        case SK_MAIN_COORDS_BUILTIN:
            this->write("%s");
            fFormatArgs.push_back(String("args.fSampleCoord"));
            fAccessSampleCoordsDirectly = true;
            break;
        case SK_WIDTH_BUILTIN:
            this->write("sk_Width");
            break;
        case SK_HEIGHT_BUILTIN:
            this->write("sk_Height");
            break;
        default:
            const Variable& var = *ref.variable();
            if (var.type().typeKind() == Type::TypeKind::kSampler) {
                this->write("%s");
                fFormatArgs.push_back("fragBuilder->getProgramBuilder()->samplerVariable(" +
                                      this->getSamplerHandle(*ref.variable()) + ")");
                return;
            }
            if (var.modifiers().fFlags & Modifiers::kUniform_Flag) {
                this->write("%s");
                String name = var.name();
                String varCode = String::printf("args.fUniformHandler->getUniformCStr(%sVar)",
                                                HCodeGenerator::FieldName(name.c_str()).c_str());
                String code;
                if (var.modifiers().fLayout.fWhen.fLength) {
                    code = String::printf("%sVar.isValid() ? %s : \"%s\"",
                                          HCodeGenerator::FieldName(name.c_str()).c_str(),
                                          varCode.c_str(),
                                          default_value(var.type()).c_str());
                } else {
                    code = varCode;
                }
                fFormatArgs.push_back(code);
            } else if (SectionAndParameterHelper::IsParameter(var)) {
                String name(var.name());
                this->writeRuntimeValue(var.type(), var.modifiers().fLayout,
                                        String::printf("_outer.%s", name.c_str()).c_str());
            } else {
                this->write(var.name());
            }
    }
}

void CPPCodeGenerator::writeIfStatement(const IfStatement& s) {
    if (s.isStatic()) {
        this->write("@");
    }
    INHERITED::writeIfStatement(s);
}

void CPPCodeGenerator::writeReturnStatement(const ReturnStatement& s) {
    INHERITED::writeReturnStatement(s);
}

void CPPCodeGenerator::writeSwitchStatement(const SwitchStatement& s) {
    if (s.isStatic()) {
        this->write("@");
    }
    INHERITED::writeSwitchStatement(s);
}

int CPPCodeGenerator::getChildFPIndex(const Variable& var) const {
    int index = 0;
    for (const ProgramElement* p : fProgram.elements()) {
        if (p->is<GlobalVarDeclaration>()) {
            const VarDeclaration& decl =
                                  p->as<GlobalVarDeclaration>().declaration()->as<VarDeclaration>();
            if (&decl.var() == &var) {
                return index;
            } else if (decl.var().type() == *fContext.fTypes.fFragmentProcessor) {
                ++index;
            }
        }
    }
    SkDEBUGFAIL("child fragment processor not found");
    return 0;
}

String CPPCodeGenerator::getSampleVarName(const char* prefix, int sampleCounter) {
    return String::printf("%s%d", prefix, sampleCounter);
}

void CPPCodeGenerator::writeFunctionCall(const FunctionCall& c) {
    const FunctionDeclaration& function = c.function();
    const ExpressionArray& arguments = c.arguments();
    if (function.isBuiltin() && function.name() == "sample" &&
        arguments[0]->type().typeKind() != Type::TypeKind::kSampler) {
        int sampleCounter = fSampleCounter++;

        // Validity checks that are detected by function definition in sksl_fp.inc
        SkASSERT(arguments.size() >= 1 && arguments.size() <= 3);
        SkASSERT("fragmentProcessor"  == arguments[0]->type().name());

        // Actually fail during compilation if arguments with valid types are
        // provided that are not variable references, since sample() is a
        // special function that impacts code emission.
        if (!arguments[0]->is<VariableReference>()) {
            fErrors.error(arguments[0]->fOffset,
                    "sample()'s fragmentProcessor argument must be a variable reference\n");
            return;
        }
        const Variable& child = *arguments[0]->as<VariableReference>().variable();

        // Start a new extra emit code section so that the emitted child processor can depend on
        // sksl variables defined in earlier sksl code.
        this->newExtraEmitCodeBlock();

        String inputColor;
        if (arguments.size() > 1 && arguments[1]->type().name() == "half4") {
            // Use the invokeChild() variant that accepts an input color, so convert the 2nd
            // argument's expression into C++ code that produces sksl stored in an SkString.
            String inputColorName = this->getSampleVarName("_input", sampleCounter);
            addExtraEmitCodeLine(convertSKSLExpressionToCPP(*arguments[1], inputColorName));

            // invokeChild() needs a char* and a pre-pended comma
            inputColor = ", " + inputColorName + ".c_str()";
        }

        String inputCoord;
        String invokeFunction = "invokeChild";
        if (arguments.back()->type().name() == "float2") {
            // Invoking child with explicit coordinates at this call site
            inputCoord = this->getSampleVarName("_coords", sampleCounter);
            addExtraEmitCodeLine(convertSKSLExpressionToCPP(*arguments.back(), inputCoord));
            inputCoord.append(".c_str()");
        } else if (arguments.back()->type().name() == "float3x3") {
            // Invoking child with a matrix, sampling relative to the input coords.
            invokeFunction = "invokeChildWithMatrix";
            SampleUsage usage = Analysis::GetSampleUsage(fProgram, child);

            if (!usage.hasUniformMatrix()) {
                inputCoord = this->getSampleVarName("_matrix", sampleCounter);
                addExtraEmitCodeLine(convertSKSLExpressionToCPP(*arguments.back(), inputCoord));
                inputCoord.append(".c_str()");
            }
            // else pass in the empty string to rely on invokeChildWithMatrix's automatic uniform
            // resolution
        }
        if (!inputCoord.empty()) {
            inputCoord = ", " + inputCoord;
        }

        // Write the output handling after the possible input handling
        String childName = this->getSampleVarName("_sample", sampleCounter);
        String childIndexStr = to_string(this->getChildFPIndex(child));
        addExtraEmitCodeLine("SkString " + childName + " = this->" + invokeFunction + "(" +
                             childIndexStr + inputColor + ", args" + inputCoord + ");");

        this->write("%s");
        fFormatArgs.push_back(childName + ".c_str()");
        return;
    }
    if (function.isBuiltin()) {
        INHERITED::writeFunctionCall(c);
    } else {
        this->write("%s");
        fFormatArgs.push_back((String(function.name()) + "_name.c_str()").c_str());
        this->write("(");
        const char* separator = "";
        for (const auto& arg : arguments) {
            this->write(separator);
            separator = ", ";
            this->writeExpression(*arg, Precedence::kSequence);
        }
        this->write(")");
    }
    if (function.isBuiltin() && function.name() == "sample") {
        this->write(".%s");
        SkASSERT(arguments.size() >= 1);
        SkASSERT(arguments[0]->is<VariableReference>());
        String sampler =
                this->getSamplerHandle(*arguments[0]->as<VariableReference>().variable());
        fFormatArgs.push_back("fragBuilder->getProgramBuilder()->samplerSwizzle(" + sampler +
                              ").asString().c_str()");
    }
}

static const char* glsltype_string(const Context& context, const Type& type) {
    // If a new GrSL type is added, this function will need to be updated.
    static_assert(kGrSLTypeCount == 49);

    if (type == *context.fTypes.fVoid    ) { return "kVoid_GrSLType";     }
    if (type == *context.fTypes.fBool    ) { return "kBool_GrSLType";     }
    if (type == *context.fTypes.fBool2   ) { return "kBool2_GrSLType";    }
    if (type == *context.fTypes.fBool3   ) { return "kBool3_GrSLType";    }
    if (type == *context.fTypes.fBool4   ) { return "kBool4_GrSLType";    }
    if (type == *context.fTypes.fByte    ) { return "kByte_GrSLType";     }
    if (type == *context.fTypes.fByte2   ) { return "kByte2_GrSLType";    }
    if (type == *context.fTypes.fByte3   ) { return "kByte3_GrSLType";    }
    if (type == *context.fTypes.fByte4   ) { return "kByte4_GrSLType";    }
    if (type == *context.fTypes.fUByte   ) { return "kUByte_GrSLType";    }
    if (type == *context.fTypes.fUByte2  ) { return "kUByte2_GrSLType";   }
    if (type == *context.fTypes.fUByte3  ) { return "kUByte3_GrSLType";   }
    if (type == *context.fTypes.fUByte4  ) { return "kUByte4_GrSLType";   }
    if (type == *context.fTypes.fShort   ) { return "kShort_GrSLType";    }
    if (type == *context.fTypes.fShort2  ) { return "kShort2_GrSLType";   }
    if (type == *context.fTypes.fShort3  ) { return "kShort3_GrSLType";   }
    if (type == *context.fTypes.fShort4  ) { return "kShort4_GrSLType";   }
    if (type == *context.fTypes.fUShort  ) { return "kUShort_GrSLType";   }
    if (type == *context.fTypes.fUShort2 ) { return "kUShort2_GrSLType";  }
    if (type == *context.fTypes.fUShort3 ) { return "kUShort3_GrSLType";  }
    if (type == *context.fTypes.fUShort4 ) { return "kUShort4_GrSLType";  }
    if (type == *context.fTypes.fFloat   ) { return "kFloat_GrSLType";    }
    if (type == *context.fTypes.fFloat2  ) { return "kFloat2_GrSLType";   }
    if (type == *context.fTypes.fFloat3  ) { return "kFloat3_GrSLType";   }
    if (type == *context.fTypes.fFloat4  ) { return "kFloat4_GrSLType";   }
    if (type == *context.fTypes.fFloat2x2) { return "kFloat2x2_GrSLType"; }
    if (type == *context.fTypes.fFloat3x3) { return "kFloat3x3_GrSLType"; }
    if (type == *context.fTypes.fFloat4x4) { return "kFloat4x4_GrSLType"; }
    if (type == *context.fTypes.fHalf    ) { return "kHalf_GrSLType";     }
    if (type == *context.fTypes.fHalf2   ) { return "kHalf2_GrSLType";    }
    if (type == *context.fTypes.fHalf3   ) { return "kHalf3_GrSLType";    }
    if (type == *context.fTypes.fHalf4   ) { return "kHalf4_GrSLType";    }
    if (type == *context.fTypes.fHalf2x2 ) { return "kHalf2x2_GrSLType";  }
    if (type == *context.fTypes.fHalf3x3 ) { return "kHalf3x3_GrSLType";  }
    if (type == *context.fTypes.fHalf4x4 ) { return "kHalf4x4_GrSLType";  }
    if (type == *context.fTypes.fInt     ) { return "kInt_GrSLType";      }
    if (type == *context.fTypes.fInt2    ) { return "kInt2_GrSLType";     }
    if (type == *context.fTypes.fInt3    ) { return "kInt3_GrSLType";     }
    if (type == *context.fTypes.fInt4    ) { return "kInt4_GrSLType";     }
    if (type == *context.fTypes.fUInt    ) { return "kUint_GrSLType";     }
    if (type == *context.fTypes.fUInt2   ) { return "kUint2_GrSLType";    }
    if (type == *context.fTypes.fUInt3   ) { return "kUint3_GrSLType";    }
    if (type == *context.fTypes.fUInt4   ) { return "kUint4_GrSLType";    }
    if (type.isEnum())                     { return "kInt_GrSLType";      }

    SkDEBUGFAILF("unsupported type: %s", type.description().c_str());
    return nullptr;
}

void CPPCodeGenerator::prepareHelperFunction(const FunctionDeclaration& decl) {
    if (decl.isBuiltin() || decl.name() == "main") {
        return;
    }

    String funcName = decl.name();
    this->addExtraEmitCodeLine(
            String::printf("SkString %s_name = fragBuilder->getMangledFunctionName(\"%s\");",
                           funcName.c_str(),
                           funcName.c_str()));

    String args = String::printf("const GrShaderVar %s_args[] = { ", funcName.c_str());
    const char* separator = "";
    for (const Variable* param : decl.parameters()) {
        String paramName = param->name();
        args.appendf("%sGrShaderVar(\"%s\", %s)", separator, paramName.c_str(),
                                                  glsltype_string(fContext, param->type()));
        separator = ", ";
    }
    args += " };";

    this->addExtraEmitCodeLine(args.c_str());
}

void CPPCodeGenerator::prototypeHelperFunction(const FunctionDeclaration& decl) {
    String funcName = decl.name();
    this->addExtraEmitCodeLine(String::printf(
            "fragBuilder->emitFunctionPrototype(%s, %s_name.c_str(), {%s_args, %zu});",
            glsltype_string(fContext, decl.returnType()),
            funcName.c_str(),
            funcName.c_str(),
            decl.parameters().size()));
}

void CPPCodeGenerator::writeFunction(const FunctionDefinition& f) {
    const FunctionDeclaration& decl = f.declaration();
    if (decl.isBuiltin()) {
        return;
    }
    fFunctionHeader = "";
    OutputStream* oldOut = fOut;
    StringStream buffer;
    fOut = &buffer;
    if (decl.name() == "main") {
        fInMain = true;
        for (const std::unique_ptr<Statement>& s : f.body()->as<Block>().children()) {
            this->writeStatement(*s);
            this->writeLine();
        }
        fInMain = false;

        fOut = oldOut;
        this->write(fFunctionHeader);
        this->write(buffer.str());
    } else {
        for (const std::unique_ptr<Statement>& s : f.body()->as<Block>().children()) {
            this->writeStatement(*s);
            this->writeLine();
        }

        fOut = oldOut;
        String funcName = decl.name();

        String funcImpl;
        if (!fFormatArgs.empty()) {
            this->addExtraEmitCodeLine("const String " + funcName + "_impl = String::printf(" +
                                       assembleCodeAndFormatArgPrintf(buffer.str()).c_str() + ");");
            funcImpl = String::printf(" %s_impl.c_str()", funcName.c_str());
        } else {
            funcImpl = "\nR\"SkSL(" + buffer.str() + ")SkSL\"";
        }

        this->addExtraEmitCodeLine(String::printf(
                "fragBuilder->emitFunction(%s, %s_name.c_str(), {%s_args, %zu},%s);",
                glsltype_string(fContext, decl.returnType()),
                funcName.c_str(),
                funcName.c_str(),
                decl.parameters().size(),
                funcImpl.c_str()));
    }
}

void CPPCodeGenerator::writeSetting(const Setting& s) {
    this->writef("sk_Caps.%s", s.name().c_str());
}

bool CPPCodeGenerator::writeSection(const char* name, const char* prefix) {
    const Section* s = fSectionAndParameterHelper.getSection(name);
    if (s) {
        this->writef("%s%s", prefix, s->text().c_str());
        return true;
    }
    return false;
}

void CPPCodeGenerator::writeProgramElement(const ProgramElement& p) {
    switch (p.kind()) {
        case ProgramElement::Kind::kSection:
            return;
        case ProgramElement::Kind::kGlobalVar: {
            const GlobalVarDeclaration& decl = p.as<GlobalVarDeclaration>();
            const Variable& var = decl.declaration()->as<VarDeclaration>().var();
            if (var.modifiers().fFlags & (Modifiers::kIn_Flag | Modifiers::kUniform_Flag) ||
                -1 != var.modifiers().fLayout.fBuiltin) {
                return;
            }
            break;
        }
        case ProgramElement::Kind::kFunctionPrototype: {
            // Function prototypes are handled at the C++ level (in writeEmitCode).
            // We don't want prototypes to be emitted inside the FP's main() function.
            return;
        }
        default:
            break;
    }
    INHERITED::writeProgramElement(p);
}

void CPPCodeGenerator::addUniform(const Variable& var) {
    if (!needs_uniform_var(var)) {
        return;
    }
    if (var.modifiers().fLayout.fWhen.fLength) {
        this->writef("        if (%s) {\n    ", String(var.modifiers().fLayout.fWhen).c_str());
    }
    String name(var.name());
    if (!var.type().isArray()) {
        this->writef("        %sVar = args.fUniformHandler->addUniform(&_outer, "
                     "kFragment_GrShaderFlag, %s, \"%s\");\n",
                     HCodeGenerator::FieldName(name.c_str()).c_str(),
                     glsltype_string(fContext, var.type()),
                     name.c_str());
    } else {
        this->writef("        %sVar = args.fUniformHandler->addUniformArray(&_outer, "
                     "kFragment_GrShaderFlag, %s, \"%s\", %d);\n",
                     HCodeGenerator::FieldName(name.c_str()).c_str(),
                     glsltype_string(fContext, var.type().componentType()),
                     name.c_str(),
                     var.type().columns());
    }
    if (var.modifiers().fLayout.fWhen.fLength) {
        this->write("        }\n");
    }
}

void CPPCodeGenerator::writeInputVars() {
}

void CPPCodeGenerator::writePrivateVars() {
    for (const ProgramElement* p : fProgram.elements()) {
        if (p->is<GlobalVarDeclaration>()) {
            const GlobalVarDeclaration& global = p->as<GlobalVarDeclaration>();
            const Variable& var = global.declaration()->as<VarDeclaration>().var();
            if (is_private(var)) {
                if (var.type() == *fContext.fTypes.fFragmentProcessor) {
                    fErrors.error(global.fOffset,
                                  "fragmentProcessor variables must be declared 'in'");
                    return;
                }
                this->writef("%s %s = %s;\n",
                             HCodeGenerator::FieldType(fContext, var.type(),
                                                       var.modifiers().fLayout).c_str(),
                             String(var.name()).c_str(),
                             default_value(var).c_str());
            } else if (var.modifiers().fLayout.fFlags & Layout::kTracked_Flag) {
                // An auto-tracked uniform in variable, so add a field to hold onto the prior
                // state. Note that tracked variables must be uniform in's and that is validated
                // before writePrivateVars() is called.
                const UniformCTypeMapper* mapper = UniformCTypeMapper::Get(fContext, var);
                SkASSERT(mapper && mapper->supportsTracking());

                String name = HCodeGenerator::FieldName(String(var.name()).c_str());
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

void CPPCodeGenerator::writePrivateVarValues() {
    for (const ProgramElement* p : fProgram.elements()) {
        if (p->is<GlobalVarDeclaration>()) {
            const GlobalVarDeclaration& global = p->as<GlobalVarDeclaration>();
            const VarDeclaration& decl = global.declaration()->as<VarDeclaration>();
            if (is_private(decl.var()) && decl.value()) {
                this->writef("%s = ", String(decl.var().name()).c_str());
                fCPPMode = true;
                this->writeExpression(*decl.value(), Precedence::kAssignment);
                fCPPMode = false;
                this->write(";\n");
            }
        }
    }
}

static bool is_accessible(const Variable& var) {
    const Type& type = var.type();
    return Type::TypeKind::kSampler != type.typeKind() &&
           Type::TypeKind::kOther != type.typeKind();
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

                SKSL_INT codeBlock;
                SkAssertResult(
                        stoi(StringFragment(sksl.c_str() + tokenStart + 2, i - tokenStart - 2),
                             &codeBlock));
                SkASSERT((size_t)codeBlock < fExtraEmitCodeBlocks.size());
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

String CPPCodeGenerator::assembleCodeAndFormatArgPrintf(const String& code) {
    // Count % format specifiers.
    size_t argCount = 0;
    for (size_t index = 0; index < code.size(); ++index) {
        if ('%' == code[index]) {
            if (index == code.size() - 1) {
                SkDEBUGFAIL("found a dangling format specifier at the end of a string");
                break;
            }
            if (code[index + 1] == '%') {
                // %% indicates a literal % sign, not a format argument. Skip over the next
                // character to avoid mistakenly counting that one as an argument.
                ++index;
            } else {
                // Count the format argument that we found.
                ++argCount;
            }
        }
    }

    // Assemble the printf arguments.
    String result = String::printf("R\"SkSL(%s)SkSL\"\n", code.c_str());
    for (size_t i = 0; i < argCount; ++i) {
        result += ", ";
        result += fFormatArgs[i].c_str();
    }

    // argCount is equal to the number of fFormatArgs that were consumed, so they should be
    // removed from the list.
    if (argCount > 0) {
        fFormatArgs.erase(fFormatArgs.begin(), fFormatArgs.begin() + argCount);
    }

    return result;
}

void CPPCodeGenerator::writeCodeAppend(const String& code) {
    if (!code.empty()) {
        this->write("        fragBuilder->codeAppendf(\n");
        this->write(assembleCodeAndFormatArgPrintf(code));
        this->write(");\n");
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
    this->writeExpression(e, Precedence::kTopLevel);
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
    if (newArgs.empty()) {
        // This was a static expression, so we can simplify the input
        // color declaration in the emitted code to just a static string
        cppExpr = "SkString " + cppVar + "(\"" + exprFormat + "\");";
    } else if (newArgs.size() == 1 && exprFormat == "%s") {
        // If the format expression is simply "%s", we can avoid an expensive call to printf.
        // This happens fairly often in codegen so it is worth simplifying.
        cppExpr = "SkString " + cppVar + "(" + newArgs[0] + ");";
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
    for (const ProgramElement* p : fProgram.elements()) {
        if (p->is<GlobalVarDeclaration>()) {
            const GlobalVarDeclaration& global = p->as<GlobalVarDeclaration>();
            const VarDeclaration& decl = global.declaration()->as<VarDeclaration>();
            String nameString(decl.var().name());
            const char* name = nameString.c_str();
            if (SectionAndParameterHelper::IsParameter(decl.var()) &&
                is_accessible(decl.var())) {
                this->writef("        auto %s = _outer.%s;\n"
                             "        (void) %s;\n",
                             name, name, name);
            }
        }
    }
    this->writePrivateVarValues();
    for (const auto u : uniforms) {
        this->addUniform(*u);
    }
    this->writeSection(kEmitCodeSection);

    // Save original buffer as the CPP buffer for flushEmittedCode()
    fCPPBuffer = fOut;
    StringStream skslBuffer;
    fOut = &skslBuffer;

    this->newExtraEmitCodeBlock();

    // Generate mangled names and argument lists for helper functions.
    std::unordered_set<const FunctionDeclaration*> definedHelpers;
    for (const ProgramElement* p : fProgram.elements()) {
        if (p->is<FunctionDefinition>()) {
            const FunctionDeclaration* decl = &p->as<FunctionDefinition>().declaration();
            definedHelpers.insert(decl);
            this->prepareHelperFunction(*decl);
        }
    }

    // Emit prototypes for defined helper functions that originally had prototypes in the FP file.
    // (If a function was prototyped but never defined, we skip it, since it wasn't prepared above.)
    for (const ProgramElement* p : fProgram.elements()) {
        if (p->is<FunctionPrototype>()) {
            const FunctionDeclaration* decl = &p->as<FunctionPrototype>().declaration();
            if (definedHelpers.find(decl) != definedHelpers.end()) {
                this->prototypeHelperFunction(*decl);
            }
        }
    }

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
    const Section* section = fSectionAndParameterHelper.getSection(kSetDataSection);
    const char* pdman = section ? section->argument().c_str() : "pdman";
    this->writef("    void onSetData(const GrGLSLProgramDataManager& %s, "
                                    "const GrFragmentProcessor& _proc) override {\n",
                 pdman);
    bool wroteProcessor = false;
    for (const Variable* u : uniforms) {
        if (is_uniform_in(*u)) {
            if (!wroteProcessor) {
                this->writef("        const %s& _outer = _proc.cast<%s>();\n", fullName, fullName);
                wroteProcessor = true;
                this->writef("        {\n");
            }

            const UniformCTypeMapper* mapper = UniformCTypeMapper::Get(fContext, *u);
            SkASSERT(mapper);

            String nameString(u->name());
            const char* name = nameString.c_str();

            // Switches for setData behavior in the generated code
            bool conditionalUniform = u->modifiers().fLayout.fWhen != "";
            bool isTracked = u->modifiers().fLayout.fFlags & Layout::kTracked_Flag;
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
                String valueType = HCodeGenerator::AccessType(fContext, u->type(),
                                                              u->modifiers().fLayout);
                this->writef("%s%s %s = _outer.%s;\n",
                             indent.c_str(), valueType.c_str(), valueVar.c_str(), name);
            } else {
                // Not tracked and the mapper only needs to use the value once
                // so send it a safe expression instead of the variable name
                valueVar.appendf("(_outer.%s)", name);
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
        for (const ProgramElement* p : fProgram.elements()) {
            if (p->is<GlobalVarDeclaration>()) {
                const GlobalVarDeclaration& global = p->as<GlobalVarDeclaration>();
                const VarDeclaration& decl = global.declaration()->as<VarDeclaration>();
                const Variable& variable = decl.var();
                String nameString(variable.name());
                const char* name = nameString.c_str();
                if (variable.type().typeKind() == Type::TypeKind::kSampler) {
                    this->writef("        const GrSurfaceProxyView& %sView = "
                                 "_outer.textureSampler(%d).view();\n",
                                 name, samplerIndex);
                    this->writef("        GrTexture& %s = *%sView.proxy()->peekTexture();\n",
                                 name, name);
                    this->writef("        (void) %s;\n", name);
                    ++samplerIndex;
                } else if (needs_uniform_var(variable)) {
                    this->writef("        UniformHandle& %s = %sVar;\n"
                                    "        (void) %s;\n",
                                    name, HCodeGenerator::FieldName(name).c_str(), name);
                } else if (SectionAndParameterHelper::IsParameter(variable) &&
                            variable.type() != *fContext.fTypes.fFragmentProcessor) {
                    if (!wroteProcessor) {
                        this->writef("        const %s& _outer = _proc.cast<%s>();\n", fullName,
                                     fullName);
                        wroteProcessor = true;
                    }

                    if (variable.type() != *fContext.fTypes.fFragmentProcessor) {
                        this->writef("        auto %s = _outer.%s;\n"
                                        "        (void) %s;\n",
                                        name, name, name);
                    }
                }
            }
        }
        this->writeSection(kSetDataSection);
    }
    this->write("    }\n");
}

void CPPCodeGenerator::writeOnTextureSampler() {
    bool foundSampler = false;
    for (const auto& param : fSectionAndParameterHelper.getParameters()) {
        if (param->type().typeKind() == Type::TypeKind::kSampler) {
            if (!foundSampler) {
                this->writef(
                        "const GrFragmentProcessor::TextureSampler& %s::onTextureSampler(int "
                        "index) const {\n",
                        fFullName.c_str());
                this->writef("    return IthTextureSampler(index, %s",
                             HCodeGenerator::FieldName(String(param->name()).c_str()).c_str());
                foundSampler = true;
            } else {
                this->writef(", %s",
                             HCodeGenerator::FieldName(String(param->name()).c_str()).c_str());
            }
        }
    }
    if (foundSampler) {
        this->write(");\n}\n");
    }
}

void CPPCodeGenerator::writeClone() {
    if (!this->writeSection(kCloneSection)) {
        if (fSectionAndParameterHelper.getSection(kFieldsSection)) {
            fErrors.error(/*offset=*/0, "fragment processors with custom @fields must also have a "
                                        "custom @clone");
        }
        this->writef("%s::%s(const %s& src)\n"
                     ": INHERITED(k%s_ClassID, src.optimizationFlags())", fFullName.c_str(),
                     fFullName.c_str(), fFullName.c_str(), fFullName.c_str());
        for (const Variable* param : fSectionAndParameterHelper.getParameters()) {
            String fieldName = HCodeGenerator::FieldName(String(param->name()).c_str());
            if (param->type() != *fContext.fTypes.fFragmentProcessor) {
                this->writef("\n, %s(src.%s)",
                             fieldName.c_str(),
                             fieldName.c_str());
            }
        }
        this->writef(" {\n");
        this->writef("        this->cloneAndRegisterAllChildProcessors(src);\n");
        int samplerCount = 0;
        for (const auto& param : fSectionAndParameterHelper.getParameters()) {
            if (param->type().typeKind() == Type::TypeKind::kSampler) {
                ++samplerCount;
            }
        }
        if (samplerCount) {
            this->writef("     this->setTextureSamplerCnt(%d);", samplerCount);
        }
        if (fAccessSampleCoordsDirectly) {
            this->writef("    this->setUsesSampleCoordsDirectly();\n");
        }
        this->write("}\n");
        this->writef("std::unique_ptr<GrFragmentProcessor> %s::clone() const {\n",
                     fFullName.c_str());
        this->writef("    return std::make_unique<%s>(*this);\n",
                     fFullName.c_str());
        this->write("}\n");
    }
}

void CPPCodeGenerator::writeDumpInfo() {
    this->writef("#if GR_TEST_UTILS\n"
                 "SkString %s::onDumpInfo() const {\n", fFullName.c_str());

    if (!this->writeSection(kDumpInfoSection)) {
        if (fSectionAndParameterHelper.getSection(kFieldsSection)) {
            fErrors.error(/*offset=*/0, "fragment processors with custom @fields must also have a "
                                        "custom @dumpInfo");
        }

        String formatString;
        std::vector<String> argumentList;

        for (const Variable* param : fSectionAndParameterHelper.getParameters()) {
            // dumpInfo() doesn't need to log child FPs.
            if (param->type() == *fContext.fTypes.fFragmentProcessor) {
                continue;
            }

            // Add this field onto the format string and argument list.
            String fieldName = HCodeGenerator::FieldName(String(param->name()).c_str());
            String runtimeValue = this->formatRuntimeValue(param->type(),
                                                           param->modifiers().fLayout,
                                                           param->name(),
                                                           &argumentList);
            formatString.appendf("%s%s=%s",
                                 formatString.empty() ? "" : ", ",
                                 fieldName.c_str(),
                                 runtimeValue.c_str());
        }

        if (!formatString.empty()) {
            // Emit the finished format string and associated arguments.
            this->writef("    return SkStringPrintf(\"(%s)\"", formatString.c_str());

            for (const String& argument : argumentList) {
                this->writef(", %s", argument.c_str());
            }

            this->write(");");
        } else {
            // No fields to dump at all; just return an empty string.
            this->write("    return SkString();");
        }
    }

    this->write("\n"
                "}\n"
                "#endif\n");
}

void CPPCodeGenerator::writeTest() {
    const Section* test = fSectionAndParameterHelper.getSection(kTestCodeSection);
    if (test) {
        this->writef(
                "GR_DEFINE_FRAGMENT_PROCESSOR_TEST(%s);\n"
                "#if GR_TEST_UTILS\n"
                "std::unique_ptr<GrFragmentProcessor> %s::TestCreate(GrProcessorTestData* %s) {\n",
                fFullName.c_str(),
                fFullName.c_str(),
                test->argument().c_str());
        this->writeSection(kTestCodeSection);
        this->write("}\n"
                    "#endif\n");
    }
}

static int bits_needed(uint32_t v) {
    int bits = 1;
    while (v >= (1u << bits)) {
        bits++;
    }
    return bits;
}

void CPPCodeGenerator::writeGetKey() {
    auto bitsForEnum = [&](const Type& type) {
        for (const ProgramElement* e : fProgram.elements()) {
            if (!e->is<Enum>() || type.name() != e->as<Enum>().typeName()) {
                continue;
            }
            SKSL_INT minVal = 0, maxVal = 0;
            auto gatherEnumRange = [&](StringFragment, SKSL_INT value) {
                minVal = std::min(minVal, value);
                maxVal = std::max(maxVal, value);
            };
            e->as<Enum>().foreach(gatherEnumRange);
            if (minVal < 0) {
                // Found a negative value in the enum, just use 32 bits
                return 32;
            }
            SkASSERT(SkTFitsIn<uint32_t>(maxVal));
            return bits_needed(maxVal);
        }
        SK_ABORT("Didn't find declaring element for enum type!");
        return 32;
    };

    this->writef("void %s::onGetGLSLProcessorKey(const GrShaderCaps& caps, "
                                                "GrProcessorKeyBuilder* b) const {\n",
                 fFullName.c_str());
    for (const ProgramElement* p : fProgram.elements()) {
        if (p->is<GlobalVarDeclaration>()) {
            const GlobalVarDeclaration& global = p->as<GlobalVarDeclaration>();
            const VarDeclaration& decl = global.declaration()->as<VarDeclaration>();
            const Variable& var = decl.var();
            const Type& varType = var.type();
            String nameString(var.name());
            const char* name = nameString.c_str();
            if (var.modifiers().fLayout.fFlags & Layout::kKey_Flag) {
                if (var.modifiers().fFlags & Modifiers::kUniform_Flag) {
                    fErrors.error(var.fOffset, "layout(key) may not be specified on uniforms");
                }
                if (is_private(var)) {
                    this->writef(
                            "%s %s =",
                            HCodeGenerator::FieldType(fContext, varType, var.modifiers().fLayout)
                                    .c_str(),
                            String(var.name()).c_str());
                    if (decl.value()) {
                        fCPPMode = true;
                        this->writeExpression(*decl.value(), Precedence::kAssignment);
                        fCPPMode = false;
                    } else {
                        this->writef("%s", default_value(var).c_str());
                    }
                    this->write(";\n");
                }
                if (var.modifiers().fLayout.fWhen.fLength) {
                    this->writef("if (%s) {", String(var.modifiers().fLayout.fWhen).c_str());
                }
                if (varType == *fContext.fTypes.fHalf4) {
                    this->writef("    uint16_t red = SkFloatToHalf(%s.fR);\n",
                                 HCodeGenerator::FieldName(name).c_str());
                    this->writef("    uint16_t green = SkFloatToHalf(%s.fG);\n",
                                 HCodeGenerator::FieldName(name).c_str());
                    this->writef("    uint16_t blue = SkFloatToHalf(%s.fB);\n",
                                 HCodeGenerator::FieldName(name).c_str());
                    this->writef("    uint16_t alpha = SkFloatToHalf(%s.fA);\n",
                                 HCodeGenerator::FieldName(name).c_str());
                    this->writef("    b->add32(((uint32_t)red << 16) | green, \"%s.rg\");\n", name);
                    this->writef("    b->add32(((uint32_t)blue << 16) | alpha, \"%s.ba\");\n",
                                 name);
                } else if (varType == *fContext.fTypes.fHalf ||
                           varType == *fContext.fTypes.fFloat) {
                    this->writef("    b->add32(sk_bit_cast<uint32_t>(%s), \"%s\");\n",
                                 HCodeGenerator::FieldName(name).c_str(), name);
                } else if (varType.isBoolean()) {
                    this->writef("    b->addBool(%s, \"%s\");\n",
                                 HCodeGenerator::FieldName(name).c_str(), name);
                } else if (varType.isEnum()) {
                    this->writef("    b->addBits(%d, (uint32_t) %s, \"%s\");\n",
                                 bitsForEnum(varType), HCodeGenerator::FieldName(name).c_str(),
                                 name);
                } else if (varType.isInteger()) {
                    this->writef("    b->add32((uint32_t) %s, \"%s\");\n",
                                 HCodeGenerator::FieldName(name).c_str(), name);
                } else {
                    SK_ABORT("NOT YET IMPLEMENTED: automatic key handling for %s\n",
                             varType.displayName().c_str());
                }
                if (var.modifiers().fLayout.fWhen.fLength) {
                    this->write("}");
                }
            }
        }
    }
    this->write("}\n");
}

bool CPPCodeGenerator::generateCode() {
    std::vector<const Variable*> uniforms;
    for (const ProgramElement* p : fProgram.elements()) {
        if (p->is<GlobalVarDeclaration>()) {
            const GlobalVarDeclaration& global = p->as<GlobalVarDeclaration>();
            const VarDeclaration& decl = global.declaration()->as<VarDeclaration>();
            if ((decl.var().modifiers().fFlags & Modifiers::kUniform_Flag) &&
                        decl.var().type().typeKind() != Type::TypeKind::kSampler) {
                uniforms.push_back(&decl.var());
            }

            if (is_uniform_in(decl.var())) {
                // Validate the "uniform in" declarations to make sure they are fully supported,
                // instead of generating surprising C++
                const UniformCTypeMapper* mapper =
                        UniformCTypeMapper::Get(fContext, decl.var());
                if (mapper == nullptr) {
                    fErrors.error(decl.fOffset, String(decl.var().name())
                            + "'s type is not supported for use as a 'uniform in'");
                    return false;
                }
                if (decl.var().modifiers().fLayout.fFlags & Layout::kTracked_Flag) {
                    if (!mapper->supportsTracking()) {
                        fErrors.error(decl.fOffset, String(decl.var().name())
                                + "'s type does not support state tracking");
                        return false;
                    }
                }

            } else {
                // If it's not a uniform_in, it's an error to be tracked
                if (decl.var().modifiers().fLayout.fFlags & Layout::kTracked_Flag) {
                    fErrors.error(decl.fOffset, "Non-'in uniforms' cannot be tracked");
                    return false;
                }
            }
        }
    }
    const char* baseName = fName.c_str();
    const char* fullName = fFullName.c_str();
    this->writef("%s\n", HCodeGenerator::GetHeader(fProgram, fErrors).c_str());
    this->writef(kFragmentProcessorHeader, fullName);
    this->writef("#include \"%s.h\"\n\n", fullName);
    this->writeSection(kCppSection);
    this->writef("#include \"src/core/SkUtils.h\"\n"
                 "#include \"src/gpu/GrTexture.h\"\n"
                 "#include \"src/gpu/glsl/GrGLSLFragmentProcessor.h\"\n"
                 "#include \"src/gpu/glsl/GrGLSLFragmentShaderBuilder.h\"\n"
                 "#include \"src/gpu/glsl/GrGLSLProgramBuilder.h\"\n"
                 "#include \"src/sksl/SkSLCPP.h\"\n"
                 "#include \"src/sksl/SkSLUtil.h\"\n"
                 "class GrGLSL%s : public GrGLSLFragmentProcessor {\n"
                 "public:\n"
                 "    GrGLSL%s() {}\n",
                 baseName, baseName);
    bool result = this->writeEmitCode(uniforms);
    this->write("private:\n");
    this->writeSetData(uniforms);
    this->writePrivateVars();
    for (const auto& u : uniforms) {
        if (needs_uniform_var(*u) && !(u->modifiers().fFlags & Modifiers::kIn_Flag)) {
            this->writef("    UniformHandle %sVar;\n",
                         HCodeGenerator::FieldName(String(u->name()).c_str()).c_str());
        }
    }
    for (const auto& param : fSectionAndParameterHelper.getParameters()) {
        if (needs_uniform_var(*param)) {
            this->writef("    UniformHandle %sVar;\n",
                         HCodeGenerator::FieldName(String(param->name()).c_str()).c_str());
        }
    }
    this->writef("};\n"
                 "std::unique_ptr<GrGLSLFragmentProcessor> %s::onMakeProgramImpl() const {\n"
                 "    return std::make_unique<GrGLSL%s>();\n"
                 "}\n",
                 fullName, baseName);
    this->writeGetKey();
    this->writef("bool %s::onIsEqual(const GrFragmentProcessor& other) const {\n"
                 "    const %s& that = other.cast<%s>();\n"
                 "    (void) that;\n",
                 fullName, fullName, fullName);
    for (const auto& param : fSectionAndParameterHelper.getParameters()) {
        if (param->type() == *fContext.fTypes.fFragmentProcessor) {
            continue;
        }
        String nameString(param->name());
        const char* name = nameString.c_str();
        this->writef("    if (%s != that.%s) return false;\n",
                     HCodeGenerator::FieldName(name).c_str(),
                     HCodeGenerator::FieldName(name).c_str());
    }
    this->write("    return true;\n"
                "}\n");
    this->writeClone();
    this->writeDumpInfo();
    this->writeOnTextureSampler();
    this->writeTest();
    this->writeSection(kCppEndSection);

    result &= 0 == fErrors.errorCount();
    return result;
}

}  // namespace SkSL

#endif // defined(SKSL_STANDALONE) || GR_TEST_UTILS
