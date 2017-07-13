/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SECTIONANDPARAMETERHELPER
#define SKSL_SECTIONANDPARAMETERHELPER

#include "SkSLErrorReporter.h"
#include "ir/SkSLProgram.h"
#include "ir/SkSLSection.h"
#include "ir/SkSLVarDeclarations.h"
#include <unordered_map>
#include <vector>

namespace SkSL {

#define CLASS_SECTION              "class"
#define CONSTRUCTOR_SECTION        "constructor"
#define CONSTRUCTOR_CODE_SECTION   "constructorCode"
#define CONSTRUCTOR_PARAMS_SECTION "constructorParams"
#define COORD_TRANSFORM_SECTION    "coordTransform"
#define CPP_SECTION                "cpp"
#define CPP_END_SECTION            "cppEnd"
#define HEADER_SECTION             "header"
#define HEADER_END_SECTION         "headerEnd"
#define EMIT_CODE_SECTION          "emitCode"
#define FIELDS_SECTION             "fields"
#define INITIALIZERS_SECTION       "initializers"
#define MAKE_SECTION               "make"
#define OPTIMIZATION_FLAGS_SECTION "optimizationFlags"
#define SAMPLER_PARAMS_SECTION     "samplerParams"
#define SET_DATA_SECTION           "setData"
#define TEST_CODE_SECTION          "test"

class SectionAndParameterHelper {
public:
    SectionAndParameterHelper(const Program& program, ErrorReporter& errors) {
        for (const auto& p : program.fElements) {
            switch (p->fKind) {
                case ProgramElement::kVar_Kind: {
                    const VarDeclarations* decls = (const VarDeclarations*) p.get();
                    for (const auto& raw : decls->fVars) {
                        const VarDeclaration& decl = (VarDeclaration&) *raw;
                        if (IsParameter(*decl.fVar)) {
                            fParameters.push_back(decl.fVar);
                        }
                    }
                    break;
                }
                case ProgramElement::kSection_Kind: {
                    const Section* s = (const Section*) p.get();
                    if (IsSupportedSection(s->fName.c_str())) {
                        if (SectionAcceptsArgument(s->fName.c_str())) {
                            if (!s->fArgument.size()) {
                                errors.error(s->fPosition,
                                             ("section '@" + s->fName +
                                              "' requires one parameter").c_str());
                            }
                        } else if (s->fArgument.size()) {
                            errors.error(s->fPosition,
                                         ("section '@" + s->fName + "' has no parameters").c_str());
                        }
                    } else {
                        errors.error(s->fPosition,
                                     ("unsupported section '@" + s->fName + "'").c_str());
                    }
                    if (!SectionPermitsDuplicates(s->fName.c_str()) &&
                            fSections.find(s->fName) != fSections.end()) {
                        errors.error(s->fPosition,
                                     ("duplicate section '@" + s->fName + "'").c_str());
                    }
                    fSections[s->fName].push_back(s);
                    break;
                }
                default:
                    break;
            }
        }
    }

    const Section* getSection(const char* name) {
        ASSERT(!SectionPermitsDuplicates(name));
        auto found = fSections.find(name);
        if (found == fSections.end()) {
            return nullptr;
        }
        ASSERT(found->second.size() == 1);
        return found->second[0];
    }

    std::vector<const Section*> getSections(const char* name) {
        auto found = fSections.find(name);
        if (found == fSections.end()) {
            return std::vector<const Section*>();
        }
        return found->second;
    }

    const std::vector<const Variable*>& getParameters() {
        return fParameters;
    }

    static bool IsParameter(const Variable& var) {
        return (var.fModifiers.fFlags & Modifiers::kIn_Flag) &&
               -1 == var.fModifiers.fLayout.fBuiltin;
    }

    static bool IsSupportedSection(const char* name) {
        return !strcmp(name, CLASS_SECTION) ||
               !strcmp(name, CONSTRUCTOR_SECTION) ||
               !strcmp(name, CONSTRUCTOR_CODE_SECTION) ||
               !strcmp(name, CONSTRUCTOR_PARAMS_SECTION) ||
               !strcmp(name, COORD_TRANSFORM_SECTION) ||
               !strcmp(name, CPP_SECTION) ||
               !strcmp(name, CPP_END_SECTION) ||
               !strcmp(name, EMIT_CODE_SECTION) ||
               !strcmp(name, FIELDS_SECTION) ||
               !strcmp(name, HEADER_SECTION) ||
               !strcmp(name, HEADER_END_SECTION) ||
               !strcmp(name, INITIALIZERS_SECTION) ||
               !strcmp(name, MAKE_SECTION) ||
               !strcmp(name, OPTIMIZATION_FLAGS_SECTION) ||
               !strcmp(name, SAMPLER_PARAMS_SECTION) ||
               !strcmp(name, SET_DATA_SECTION) ||
               !strcmp(name, TEST_CODE_SECTION);
    }

    static bool SectionAcceptsArgument(const char* name) {
        return !strcmp(name, COORD_TRANSFORM_SECTION) ||
               !strcmp(name, SAMPLER_PARAMS_SECTION) ||
               !strcmp(name, SET_DATA_SECTION) ||
               !strcmp(name, TEST_CODE_SECTION);
    }

    static bool SectionPermitsDuplicates(const char* name) {
        return !strcmp(name, COORD_TRANSFORM_SECTION) ||
               !strcmp(name, SAMPLER_PARAMS_SECTION);
    }

private:
    std::vector<const Variable*> fParameters;
    std::unordered_map<String, std::vector<const Section*>> fSections;
};

} // namespace SkSL

#endif
