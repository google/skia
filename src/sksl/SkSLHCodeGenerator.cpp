/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLHCodeGenerator.h"

#include "SkSLUtil.h"
#include "ir/SkSLEnum.h"
#include "ir/SkSLFunctionDeclaration.h"
#include "ir/SkSLFunctionDefinition.h"
#include "ir/SkSLSection.h"
#include "ir/SkSLVarDeclarations.h"

namespace SkSL {

HCodeGenerator::HCodeGenerator(const Context* context, const Program* program,
                               ErrorReporter* errors, String name, OutputStream* out)
: INHERITED(program, errors, out)
, fContext(*context)
, fName(std::move(name))
, fFullName(String::printf("Gr%s", fName.c_str()))
, fSectionAndParameterHelper(*program, *errors) {}


String HCodeGenerator::ReturnType(const Context& context, const Type& type,
                                  const Layout& layout) {
    if (layout.fCType != "") {
        return layout.fCType;
    } else if (Type::kArray_Kind == type.kind()) {
        return "const " + ReturnType(context, type.componentType(), layout) + "*";
    } else if (type == *context.fFloat_Type || type == *context.fHalf_Type) {
        return "float";
    } else if (type == *context.fFloat2_Type || type == *context.fHalf2_Type) {
        return "SkPoint";
    } else if (type == *context.fFloat3_Type || type == *context.fHalf3_Type) {
        return "SkVector3";
    } else if (type == *context.fInt4_Type || type == *context.fShort4_Type) {
        return "SkIRect";
    } else if (type == *context.fFloat4_Type || type == *context.fHalf4_Type) {
        return "SkRect";
    } else if (type == *context.fFloat4x4_Type || type == *context.fHalf4x4_Type) {
        return "SkMatrix44";
    } else if (type.kind() == Type::kSampler_Kind) {
        return "sk_sp<GrTextureProxy>";
    } else if (type == *context.fFragmentProcessor_Type) {
        return "std::unique_ptr<GrFragmentProcessor>";
    }
    return type.name();
}

String HCodeGenerator::ParameterDeclaration(const Context& context, const Type& type,
                                            const Layout& layout, const char* name) {
    if ( Type::kSampler_Kind == type.kind()) {
        return String("sk_sp<GrTextureProxy> ") + name;
    } else if (Type::kArray_Kind == type.kind()) {
        return ReturnType(context, type.componentType(), layout) + "* " + name;
    }
    return ReturnType(context, type, layout) + " " + name;
}

String HCodeGenerator::FieldDeclaration(const Context& context, const Variable& v,
                                        const char* name) {
    if (Type::kSampler_Kind == v.fType.kind()) {
        return String("TextureSampler ") + name;
    } else if (Type::kArray_Kind == v.fType.kind()) {
        ASSERT(v.fSizes.size() == 1);
        return ReturnType(context, v.fType.componentType(), v.fModifiers.fLayout) + " " + name +
                "[" + v.fSizes[0]->description() + "]";
    } else if (v.fType == *context.fFragmentProcessor_Type) {
        // we don't store fragment processors in fields, they get registered via
        // registerChildProcessor instead
        ASSERT(false);
        return "<error>";
    }
    return ReturnType(context, v.fType, v.fModifiers.fLayout) + " " + name;
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
}

void HCodeGenerator::writef(const char* s, ...) {
    va_list va;
    va_start(va, s);
    this->writef(s, va);
    va_end(va);
}

bool HCodeGenerator::writeSection(const char* name, const char* prefix) {
    const Section* s = fSectionAndParameterHelper.getSection(name);
    if (s) {
        this->writef("%s%s", prefix, s->fText.c_str());
        return true;
    }
    return false;
}

void HCodeGenerator::writeExtraConstructorParams(const char* separator) {
    // super-simple parse, just assume the last token before a comma is the name of a parameter
    // (which is true as long as there are no multi-parameter template types involved). Will replace
    // this with something more robust if the need arises.
    const Section* section = fSectionAndParameterHelper.getSection(CONSTRUCTOR_PARAMS_SECTION);
    if (section) {
        const char* s = section->fText.c_str();
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
                ASSERT(lastIdentifierLength < BUFFER_SIZE);
                lastIdentifier[lastIdentifierLength] = c;
                ++lastIdentifierLength;
            } else {
                foundBreak = true;
                if (c == ',') {
                    ASSERT(lastIdentifierLength < BUFFER_SIZE);
                    lastIdentifier[lastIdentifierLength] = 0;
                    this->writef("%s%s", separator, lastIdentifier);
                    separator = ", ";
                } else if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
                    lastIdentifierLength = 0;
                }
            }
        }
        if (lastIdentifierLength) {
            ASSERT(lastIdentifierLength < BUFFER_SIZE);
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
        for (const auto& param : fSectionAndParameterHelper.getParameters()) {
            this->writef("%s%s", separator, ParameterDeclaration(fContext, param->fType,
                                                                 param->fModifiers.fLayout,
                                                                 String(param->fName).c_str()).c_str());
            separator = ", ";
        }
        this->writeSection(CONSTRUCTOR_PARAMS_SECTION, separator);
        this->writef(") {\n"
                     "        return std::unique_ptr<GrFragmentProcessor>(new %s(",
                     fFullName.c_str());
        separator = "";
        for (const auto& param : fSectionAndParameterHelper.getParameters()) {
            if (param->fType == *fContext.fFragmentProcessor_Type) {
                this->writef("%sstd::move(%s)", separator, String(param->fName).c_str());
            } else {
                this->writef("%s%s", separator, String(param->fName).c_str());
            }
            separator = ", ";
        }
        this->writeExtraConstructorParams(separator);
        this->writef("));\n"
                     "    }\n");
    }
}

void HCodeGenerator::failOnSection(const char* section, const char* msg) {
    std::vector<const Section*> s = fSectionAndParameterHelper.getSections(section);
    if (s.size()) {
        fErrors.error(s[0]->fOffset, String("@") + section + " " + msg);
    }
}

void HCodeGenerator::writeConstructor() {
    if (this->writeSection(CONSTRUCTOR_SECTION)) {
        const char* msg = "may not be present when constructor is overridden";
        this->failOnSection(CONSTRUCTOR_CODE_SECTION, msg);
        this->failOnSection(CONSTRUCTOR_PARAMS_SECTION, msg);
        this->failOnSection(COORD_TRANSFORM_SECTION, msg);
        this->failOnSection(INITIALIZERS_SECTION, msg);
        this->failOnSection(OPTIMIZATION_FLAGS_SECTION, msg);
    }
    this->writef("    %s(", fFullName.c_str());
    const char* separator = "";
    for (const auto& param : fSectionAndParameterHelper.getParameters()) {
        this->writef("%s%s", separator, ParameterDeclaration(fContext, param->fType,
                                                             param->fModifiers.fLayout,
                                                             String(param->fName).c_str()).c_str());
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
    for (const auto& param : fSectionAndParameterHelper.getParameters()) {
        String nameString(param->fName);
        const char* name = nameString.c_str();
        if (param->fType.kind() == Type::kSampler_Kind) {
            this->writef("\n    , %s(std::move(%s)", FieldName(name).c_str(), name);
            for (const Section* s : fSectionAndParameterHelper.getSections(
                                                                          SAMPLER_PARAMS_SECTION)) {
                if (s->fArgument == name) {
                    this->writef(", %s", s->fText.c_str());
                }
            }
            this->writef(")");
        } else if (param->fType == *fContext.fFragmentProcessor_Type ||
                   param->fType.kind() == Type::kArray_Kind) {
            // do nothing
        } else {
            this->writef("\n    , %s(%s)", FieldName(name).c_str(), name);
        }
    }
    for (const Section* s : fSectionAndParameterHelper.getSections(COORD_TRANSFORM_SECTION)) {
        String field = FieldName(s->fArgument.c_str());
        this->writef("\n    , %sCoordTransform(%s, %s.proxy())", field.c_str(), s->fText.c_str(),
                     field.c_str());
    }
    this->writef(" {\n");
    this->writeSection(CONSTRUCTOR_CODE_SECTION);
    for (const auto& param : fSectionAndParameterHelper.getParameters()) {
        if (param->fType.kind() == Type::kArray_Kind) {
            String nameString(param->fName);
            const char* name = nameString.c_str();
            this->writef("        memcpy(%s, %s, sizeof(%s));\n", FieldName(name).c_str(), name,
                         FieldName(name).c_str());
        }
    }

    for (const auto& param : fSectionAndParameterHelper.getParameters()) {
        if (param->fType.kind() == Type::kSampler_Kind) {
            this->writef("        this->addTextureSampler(&%s);\n",
                         FieldName(String(param->fName).c_str()).c_str());
        } else if (param->fType == *fContext.fFragmentProcessor_Type) {
            this->writef("        this->registerChildProcessor(std::move(%s));",
                         String(param->fName).c_str());
        }
    }
    for (const Section* s : fSectionAndParameterHelper.getSections(COORD_TRANSFORM_SECTION)) {
        String field = FieldName(s->fArgument.c_str());
        this->writef("        this->addCoordTransform(&%sCoordTransform);\n", field.c_str());
    }
    this->writef("    }\n");
}

void HCodeGenerator::writeFields() {
    this->writeSection(FIELDS_SECTION);
    for (const auto& param : fSectionAndParameterHelper.getParameters()) {
        if (param->fType == *fContext.fFragmentProcessor_Type) {
            continue;
        }
        String name = String(param->fName);
        this->writef("    %s;\n", FieldDeclaration(fContext, *param,
                                                   FieldName(name.c_str()).c_str()).c_str());
    }
    for (const Section* s : fSectionAndParameterHelper.getSections(COORD_TRANSFORM_SECTION)) {
        this->writef("    GrCoordTransform %sCoordTransform;\n",
                     FieldName(s->fArgument.c_str()).c_str());
    }
}

void HCodeGenerator::WriteConstants(const Program& program, OutputStream* out) {
    for (const auto& p : program.fElements) {
        if (ProgramElement::kVar_Kind == p->fKind) {
            const VarDeclarations* decls = (const VarDeclarations*) p.get();
            for (const auto& raw : decls->fVars) {
                const VarDeclaration& decl = (VarDeclaration&) *raw;
                const Variable& v = *decl.fVar;
                if (v.fModifiers.fFlags & Modifiers::kConst_Flag) {
                    ASSERT(decl.fValue);
                    out->writeString("static constexpr " + v.fType.name() + " " + v.fName +
                                 " = " + decl.fValue->description() + ";\n");
                }
            }
        }
    }
}

bool HCodeGenerator::generateCode() {
    this->writef(kFragmentProcessorHeader, fFullName.c_str());
    this->writef("#ifndef %s_DEFINED\n"
                 "#define %s_DEFINED\n",
                 fFullName.c_str(),
                 fFullName.c_str());
    this->writef("#include \"SkTypes.h\"\n"
                 "#if SK_SUPPORT_GPU\n");
    this->writeSection(HEADER_SECTION);
    this->writef("#include \"GrFragmentProcessor.h\"\n"
                 "#include \"GrCoordTransform.h\"\n"
                 "#include \"SkPoint3.h\"\n");
    this->writef("class %s : public GrFragmentProcessor {\n"
                 "public:\n",
                 fFullName.c_str());
    for (const auto& p : fProgram.fElements) {
        if (ProgramElement::kEnum_Kind == p->fKind && !((Enum&) *p).fBuiltin) {
            this->writef("%s\n", p->description().c_str());
        }
    }
    WriteConstants(fProgram, fOut);
    this->writeSection(CLASS_SECTION);
    for (const auto& param : fSectionAndParameterHelper.getParameters()) {
        if (param->fType.kind() == Type::kSampler_Kind ||
            param->fType.kind() == Type::kOther_Kind) {
            continue;
        }
        String nameString(param->fName);
        const char* name = nameString.c_str();
        this->writef("    %s %s() const { return %s; }\n",
                     ReturnType(fContext, param->fType, param->fModifiers.fLayout).c_str(), name,
                     FieldName(name).c_str());
    }
    this->writeMake();
    this->writef("    %s(const %s& src);\n"
                 "    std::unique_ptr<GrFragmentProcessor> clone() const override;\n"
                 "    const char* name() const override { return \"%s\"; }\n"
                 "private:\n",
                 fFullName.c_str(), fFullName.c_str(), fName.c_str());
    this->writeConstructor();
    this->writef("    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;\n"
                 "    void onGetGLSLProcessorKey(const GrShaderCaps&,"
                                                "GrProcessorKeyBuilder*) const override;\n"
                 "    bool onIsEqual(const GrFragmentProcessor&) const override;\n"
                 "    GR_DECLARE_FRAGMENT_PROCESSOR_TEST\n");
    this->writeFields();
    this->writef("    typedef GrFragmentProcessor INHERITED;\n"
                "};\n");
    this->writeSection(HEADER_END_SECTION);
    this->writef("#endif\n"
                 "#endif\n");
    return 0 == fErrors.errorCount();
}

} // namespace
