/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SECTIONANDPARAMETERHELPER
#define SKSL_SECTIONANDPARAMETERHELPER

#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLSection.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include <unordered_map>
#include <vector>

namespace SkSL {

inline constexpr char CLASS_SECTION[] =              "class";
inline constexpr char CLONE_SECTION[] =              "clone";
inline constexpr char CONSTRUCTOR_SECTION[] =        "constructor";
inline constexpr char CONSTRUCTOR_CODE_SECTION[] =   "constructorCode";
inline constexpr char CONSTRUCTOR_PARAMS_SECTION[] = "constructorParams";
inline constexpr char CPP_SECTION[] =                "cpp";
inline constexpr char CPP_END_SECTION[] =            "cppEnd";
inline constexpr char HEADER_SECTION[] =             "header";
inline constexpr char HEADER_END_SECTION[] =         "headerEnd";
inline constexpr char EMIT_CODE_SECTION[] =          "emitCode";
inline constexpr char FIELDS_SECTION[] =             "fields";
inline constexpr char INITIALIZERS_SECTION[] =       "initializers";
inline constexpr char MAKE_SECTION[] =               "make";
inline constexpr char OPTIMIZATION_FLAGS_SECTION[] = "optimizationFlags";
inline constexpr char SAMPLER_PARAMS_SECTION[] =     "samplerParams";
inline constexpr char SET_DATA_SECTION[] =           "setData";
inline constexpr char TEST_CODE_SECTION[] =          "test";

class SectionAndParameterHelper {
public:
    SectionAndParameterHelper(const Program* program, ErrorReporter& errors);

    const Section* getSection(const char* name) {
        SkASSERT(!SectionPermitsDuplicates(name));
        auto found = fSections.find(name);
        if (found == fSections.end()) {
            return nullptr;
        }
        SkASSERT(found->second.size() == 1);
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
               !strcmp(name, CLONE_SECTION) ||
               !strcmp(name, CONSTRUCTOR_SECTION) ||
               !strcmp(name, CONSTRUCTOR_CODE_SECTION) ||
               !strcmp(name, CONSTRUCTOR_PARAMS_SECTION) ||
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
        return !strcmp(name, SAMPLER_PARAMS_SECTION) ||
               !strcmp(name, SET_DATA_SECTION) ||
               !strcmp(name, TEST_CODE_SECTION);
    }

    static bool SectionRequiresArgument(const char* name) {
        return !strcmp(name, SAMPLER_PARAMS_SECTION) ||
               !strcmp(name, SET_DATA_SECTION) ||
               !strcmp(name, TEST_CODE_SECTION);
    }

    static bool SectionPermitsDuplicates(const char* name) {
        return !strcmp(name, SAMPLER_PARAMS_SECTION);
    }

private:
    const Program& fProgram;
    std::vector<const Variable*> fParameters;
    std::unordered_map<String, std::vector<const Section*>> fSections;
};

} // namespace SkSL

#endif
