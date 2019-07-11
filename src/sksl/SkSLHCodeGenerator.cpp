/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLHCodeGenerator.h"

#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLOutputStream.h"
#include "src/sksl/SkSLParser.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLEnum.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLSection.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

#include <set>

namespace SkSL {

HCodeGenerator::HCodeGenerator(IRGenerator* irGenerator, const Program* program, String name,
                               OutputStream* out)
: INHERITED(program, &irGenerator->fErrors, out)
, fIRGenerator(*irGenerator)
, fName(std::move(name))
, fFullName(String::printf("Gr%s", fName.c_str()))
, fSectionAndParameterHelper(*program, irGenerator->fErrors) {}

String HCodeGenerator::ParameterType(const Context& context, IRNode::ID type,
                                     const Layout& layout) {
    Layout::CType ctype = ParameterCType(context, type, layout);
    if (ctype != Layout::CType::kDefault) {
        return Layout::CTypeToStr(ctype);
    }
    return type.typeNode().name();
}

Layout::CType HCodeGenerator::ParameterCType(const Context& context, IRNode::ID type,
                                             const Layout& layout) {
    if (layout.fCType != Layout::CType::kDefault) {
        return layout.fCType;
    }
    const Type& typeNode = type.typeNode();
    if (typeNode.kind() == Type::kNullable_Kind) {
        return ParameterCType(context, typeNode.componentType(), layout);
    } else if (type == context.fFloat_Type || type == context.fHalf_Type) {
        return Layout::CType::kFloat;
    } else if (type == context.fInt_Type ||
               type == context.fShort_Type ||
               type == context.fByte_Type) {
        return Layout::CType::kInt32;
    } else if (type == context.fFloat2_Type || type == context.fHalf2_Type) {
        return Layout::CType::kSkPoint;
    } else if (type == context.fInt2_Type ||
               type == context.fShort2_Type ||
               type == context.fByte2_Type) {
        return Layout::CType::kSkIPoint;
    } else if (type == context.fInt4_Type ||
               type == context.fShort4_Type ||
               type == context.fByte4_Type) {
        return Layout::CType::kSkIRect;
    } else if (type == context.fFloat4_Type || type == context.fHalf4_Type) {
        return Layout::CType::kSkRect;
    } else if (type == context.fFloat3x3_Type || type == context.fHalf3x3_Type) {
        return Layout::CType::kSkMatrix;
    } else if (type == context.fFloat4x4_Type || type == context.fHalf4x4_Type) {
        return Layout::CType::kSkMatrix44;
    } else if (typeNode.kind() == Type::kSampler_Kind) {
        return Layout::CType::kGrTextureProxy;
    } else if (type == context.fFragmentProcessor_Type) {
        return Layout::CType::kGrFragmentProcessor;
    }
    return Layout::CType::kDefault;
}

String HCodeGenerator::FieldType(const Context& context, IRNode::ID type,
                                 const Layout& layout) {
    if (type.typeNode().kind() == Type::kSampler_Kind) {
        return "TextureSampler";
    } else if (type == context.fFragmentProcessor_Type) {
        // we don't store fragment processors in fields, they get registered via
        // registerChildProcessor instead
        SkASSERT(false);
        return "<error>";
    }
    return ParameterType(context, type, layout);
}

String HCodeGenerator::AccessType(const Context& context, IRNode::ID type, const Layout& layout) {
    static const std::set<String> primitiveTypes = { "int32_t", "float", "bool", "SkPMColor" };

    String fieldType = FieldType(context, type, layout);
    bool isPrimitive = primitiveTypes.find(fieldType) != primitiveTypes.end();
    if (isPrimitive) {
        return fieldType;
    } else {
        return String::printf("const %s&", fieldType.c_str());
    }
}

void HCodeGenerator::writef(const char* s, va_list va) {
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

void HCodeGenerator::writef(const char* s, ...) {
    va_list va;
    va_start(va, s);
    this->writef(s, va);
    va_end(va);
}

bool HCodeGenerator::writeSection(const char* name, const char* prefix) {
    IRNode::ID s = fSectionAndParameterHelper.getSection(name);
    if (s) {
        this->writef("%s%s", prefix, ((Section&) s.node()).fText.c_str());
        return true;
    }
    return false;
}

void HCodeGenerator::writeExtraConstructorParams(const char* separator) {
    // super-simple parse, just assume the last token before a comma is the name of a parameter
    // (which is true as long as there are no multi-parameter template types involved). Will replace
    // this with something more robust if the need arises.
    IRNode::ID section = fSectionAndParameterHelper.getSection(CONSTRUCTOR_PARAMS_SECTION);
    if (section) {
        const char* s = ((Section&) section.node()).fText.c_str();
        #define BUFFER_SIZE 64
        char lastIdentifier[BUFFER_SIZE];
        int lastIdentifierLength = 0;
        bool foundBreak = false;
        while (*s) {
            char c = *s;
            ++s;
            if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
                c == '_') {
                if (foundBreak) {
                    lastIdentifierLength = 0;
                    foundBreak = false;
                }
                SkASSERT(lastIdentifierLength < BUFFER_SIZE);
                lastIdentifier[lastIdentifierLength] = c;
                ++lastIdentifierLength;
            } else {
                foundBreak = true;
                if (c == ',') {
                    SkASSERT(lastIdentifierLength < BUFFER_SIZE);
                    lastIdentifier[lastIdentifierLength] = 0;
                    this->writef("%s%s", separator, lastIdentifier);
                    separator = ", ";
                } else if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
                    lastIdentifierLength = 0;
                }
            }
        }
        if (lastIdentifierLength) {
            SkASSERT(lastIdentifierLength < BUFFER_SIZE);
            lastIdentifier[lastIdentifierLength] = 0;
            this->writef("%s%s", separator, lastIdentifier);
        }
    }
}

void HCodeGenerator::writeMake() {
    const char* separator;
    if (!this->writeSection(MAKE_SECTION)) {
        this->writef("    static std::unique_ptr<GrFragmentProcessor> Make(");
        separator = "";
        for (IRNode::ID paramID : fSectionAndParameterHelper.getParameters()) {
            Variable& param = (Variable&) paramID.node();
            this->writef("%s%s %s", separator, ParameterType(fIRGenerator.fContext, param.fType,
                                                             param.fModifiers.fLayout).c_str(),
                         String(param.fName).c_str());
            separator = ", ";
        }
        this->writeSection(CONSTRUCTOR_PARAMS_SECTION, separator);
        this->writef(") {\n"
                     "        return std::unique_ptr<GrFragmentProcessor>(new %s(",
                     fFullName.c_str());
        separator = "";
        for (IRNode::ID paramID : fSectionAndParameterHelper.getParameters()) {
            Variable& param = (Variable&) paramID.node();
            if (Type::nonnullable(param.fType) ==
                fIRGenerator.fContext.fFragmentProcessor_Type) {
                this->writef("%sstd::move(%s)", separator, String(param.fName).c_str());
            } else {
                this->writef("%s%s", separator, String(param.fName).c_str());
            }
            separator = ", ";
        }
        this->writeExtraConstructorParams(separator);
        this->writef("));\n"
                     "    }\n");
    }
}

void HCodeGenerator::failOnSection(const char* section, const char* msg) {
    std::vector<IRNode::ID> s = fSectionAndParameterHelper.getSections(section);
    if (s.size()) {
        fErrors.error(s[0].node().fOffset, String("@") + section + " " + msg);
    }
}

void HCodeGenerator::writeConstructor() {
    if (this->writeSection(CONSTRUCTOR_SECTION)) {
        const char* msg = "may not be present when constructor is overridden";
        this->failOnSection(CONSTRUCTOR_CODE_SECTION, msg);
        this->failOnSection(CONSTRUCTOR_PARAMS_SECTION, msg);
        this->failOnSection(INITIALIZERS_SECTION, msg);
        this->failOnSection(OPTIMIZATION_FLAGS_SECTION, msg);
        return;
    }
    this->writef("    %s(", fFullName.c_str());
    const char* separator = "";
    for (IRNode::ID paramID : fSectionAndParameterHelper.getParameters()) {
        Variable& param = (Variable&) paramID.node();
        this->writef("%s%s %s", separator, ParameterType(fIRGenerator.fContext, param.fType,
                                                         param.fModifiers.fLayout).c_str(),
                     String(param.fName).c_str());
        separator = ", ";
    }
    this->writeSection(CONSTRUCTOR_PARAMS_SECTION, separator);
    this->writef(")\n"
                 "    : INHERITED(k%s_ClassID", fFullName.c_str());
    if (!this->writeSection(OPTIMIZATION_FLAGS_SECTION, ", (OptimizationFlags) ")) {
        this->writef(", kNone_OptimizationFlags");
    }
    this->writef(")");
    this->writeSection(INITIALIZERS_SECTION, "\n    , ");
    const auto transforms = fSectionAndParameterHelper.getSections(COORD_TRANSFORM_SECTION);
    for (size_t i = 0; i < transforms.size(); ++i) {
        const Section& s = (Section&) transforms[i].node();
        String field = CoordTransformName(s.fArgument.c_str(), i);
        if (s.fArgument.size()) {
            this->writef("\n    , %s(%s, %s.get())", field.c_str(), s.fText.c_str(),
                         FieldName(s.fArgument.c_str()).c_str());
        }
        else {
            this->writef("\n    , %s(%s)", field.c_str(), s.fText.c_str());
        }
    }
    for (IRNode::ID paramID : fSectionAndParameterHelper.getParameters()) {
        Variable& param = (Variable&) paramID.node();
        String nameString(param.fName);
        const char* name = nameString.c_str();
        IRNode::ID type = Type::nonnullable(param.fType);
        if (type.typeNode().kind() == Type::kSampler_Kind) {
            this->writef("\n    , %s(std::move(%s)", FieldName(name).c_str(), name);
            for (IRNode::ID sID : fSectionAndParameterHelper.getSections(SAMPLER_PARAMS_SECTION)) {
                const Section& s = (Section&) sID.node();
                if (s.fArgument == name) {
                    this->writef(", %s", s.fText.c_str());
                }
            }
            this->writef(")");
        } else if (type == fIRGenerator.fContext.fFragmentProcessor_Type) {
            // do nothing
        } else {
            this->writef("\n    , %s(%s)", FieldName(name).c_str(), name);
        }
    }
    this->writef(" {\n");
    this->writeSection(CONSTRUCTOR_CODE_SECTION);
    int samplerCount = 0;
    for (IRNode::ID paramID : fSectionAndParameterHelper.getParameters()) {
        Variable& param = (Variable&) paramID.node();
        if (param.fType.typeNode().kind() == Type::kSampler_Kind) {
            ++samplerCount;
        } else if (Type::nonnullable(param.fType) ==
                   fIRGenerator.fContext.fFragmentProcessor_Type) {
            if (param.fType.typeNode().kind() == Type::kNullable_Kind) {
                this->writef("        if (%s) {\n", String(param.fName).c_str());
            } else {
                this->writef("        SkASSERT(%s);", String(param.fName).c_str());
            }
            this->writef("            %s_index = this->numChildProcessors();",
                         FieldName(String(param.fName).c_str()).c_str());
            this->writef("            this->registerChildProcessor(std::move(%s));",
                         String(param.fName).c_str());
            if (param.fType.typeNode().kind() == Type::kNullable_Kind) {
                this->writef("       }");
            }
        }
    }
    if (samplerCount) {
        this->writef("        this->setTextureSamplerCnt(%d);", samplerCount);
    }
    for (size_t i = 0; i < transforms.size(); ++i) {
        const Section& s = (Section&) transforms[i].node();
        String field = CoordTransformName(s.fArgument.c_str(), i);
        this->writef("        this->addCoordTransform(&%s);\n", field.c_str());
    }
    this->writef("    }\n");
}

void HCodeGenerator::writeFields() {
    this->writeSection(FIELDS_SECTION);
    const auto transforms = fSectionAndParameterHelper.getSections(COORD_TRANSFORM_SECTION);
    for (size_t i = 0; i < transforms.size(); ++i) {
        const Section& s = (Section&) transforms[i].node();
        this->writef("    GrCoordTransform %s;\n",
                     CoordTransformName(s.fArgument.c_str(), i).c_str());
    }
    for (IRNode::ID paramID : fSectionAndParameterHelper.getParameters()) {
        Variable& param = (Variable&) paramID.node();
        String name = FieldName(String(param.fName).c_str());
        if (Type::nonnullable(param.fType) == fIRGenerator.fContext.fFragmentProcessor_Type) {
            this->writef("    int %s_index = -1;\n", name.c_str());
        } else {
            this->writef("    %s %s;\n", FieldType(fIRGenerator.fContext, param.fType,
                                                   param.fModifiers.fLayout).c_str(),
                                         name.c_str());
        }
    }
}

String HCodeGenerator::GetHeader(IRGenerator& irGenerator, const Program& program) {
    SymbolTable types(&irGenerator);
    Parser parser(program.fSource->c_str(), program.fSource->length(), types, &irGenerator);
    for (;;) {
        Token header = parser.nextRawToken();
        switch (header.fKind) {
            case Token::WHITESPACE:
                break;
            case Token::BLOCK_COMMENT:
                return String(program.fSource->c_str() + header.fOffset, header.fLength);
            default:
                return "";
        }
    }
}

bool HCodeGenerator::generateCode() {
    this->writef("%s\n", GetHeader(fIRGenerator, fProgram).c_str());
    this->writef(kFragmentProcessorHeader, fFullName.c_str());
    this->writef("#ifndef %s_DEFINED\n"
                 "#define %s_DEFINED\n",
                 fFullName.c_str(),
                 fFullName.c_str());
    this->writef("#include \"include/core/SkTypes.h\"\n");
    this->writeSection(HEADER_SECTION);
    this->writef("\n"
                 "#include \"src/gpu/GrCoordTransform.h\"\n"
                 "#include \"src/gpu/GrFragmentProcessor.h\"\n");
    this->writef("class %s : public GrFragmentProcessor {\n"
                 "public:\n",
                 fFullName.c_str());
    for (IRNode::ID pID : fProgram) {
        ProgramElement& p = (ProgramElement&) pID.node();
        if (ProgramElement::kEnum_Kind == p.fKind && !((Enum&) p).fBuiltin) {
            this->writef("%s\n", p.description().c_str());
        }
    }
    this->writeSection(CLASS_SECTION);
    this->writeMake();
    this->writef("    %s(const %s& src);\n"
                 "    std::unique_ptr<GrFragmentProcessor> clone() const override;\n"
                 "    const char* name() const override { return \"%s\"; }\n",
                 fFullName.c_str(), fFullName.c_str(), fName.c_str());
    this->writeFields();
    this->writef("private:\n");
    this->writeConstructor();
    this->writef("    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;\n"
                 "    void onGetGLSLProcessorKey(const GrShaderCaps&,"
                                                "GrProcessorKeyBuilder*) const override;\n"
                 "    bool onIsEqual(const GrFragmentProcessor&) const override;\n");
    for (IRNode::ID param : fSectionAndParameterHelper.getParameters()) {
        if (((Variable&) param).fType.typeNode().kind() == Type::kSampler_Kind) {
            this->writef("    const TextureSampler& onTextureSampler(int) const override;");
            break;
        }
    }
    this->writef("    GR_DECLARE_FRAGMENT_PROCESSOR_TEST\n");
    this->writef("    typedef GrFragmentProcessor INHERITED;\n"
                "};\n");
    this->writeSection(HEADER_END_SECTION);
    this->writef("#endif\n");
    return 0 == fErrors.errorCount();
}

} // namespace
