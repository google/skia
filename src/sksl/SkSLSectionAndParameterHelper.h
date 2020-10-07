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

inline constexpr char kClassSection[] =              "class";
inline constexpr char kCloneSection[] =              "clone";
inline constexpr char kConstructorSection[] =        "constructor";
inline constexpr char kConstructorCodeSection[] =    "constructorCode";
inline constexpr char kConstructorParamsSection[] =  "constructorParams";
inline constexpr char kCppSection[] =                "cpp";
inline constexpr char kCppEndSection[] =             "cppEnd";
inline constexpr char kDumpInfoSection[] =           "dumpInfo";
inline constexpr char kEmitCodeSection[] =           "emitCode";
inline constexpr char kFieldsSection[] =             "fields";
inline constexpr char kHeaderSection[] =             "header";
inline constexpr char kHeaderEndSection[] =          "headerEnd";
inline constexpr char kInitializersSection[] =       "initializers";
inline constexpr char kMakeSection[] =               "make";
inline constexpr char kOptimizationFlagsSection[] =  "optimizationFlags";
inline constexpr char kSamplerParamsSection[] =      "samplerParams";
inline constexpr char kSetDataSection[] =            "setData";
inline constexpr char kTestCodeSection[] =           "test";

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
        return (var.modifiers().fFlags & Modifiers::kIn_Flag) &&
               -1 == var.modifiers().fLayout.fBuiltin;
    }

    static bool IsSupportedSection(const char* name) {
        return !strcmp(name, kClassSection) ||
               !strcmp(name, kCloneSection) ||
               !strcmp(name, kConstructorSection) ||
               !strcmp(name, kConstructorCodeSection) ||
               !strcmp(name, kConstructorParamsSection) ||
               !strcmp(name, kCppSection) ||
               !strcmp(name, kCppEndSection) ||
               !strcmp(name, kDumpInfoSection) ||
               !strcmp(name, kEmitCodeSection) ||
               !strcmp(name, kFieldsSection) ||
               !strcmp(name, kHeaderSection) ||
               !strcmp(name, kHeaderEndSection) ||
               !strcmp(name, kInitializersSection) ||
               !strcmp(name, kMakeSection) ||
               !strcmp(name, kOptimizationFlagsSection) ||
               !strcmp(name, kSamplerParamsSection) ||
               !strcmp(name, kSetDataSection) ||
               !strcmp(name, kTestCodeSection);
    }

    static bool SectionAcceptsArgument(const char* name) {
        return !strcmp(name, kSamplerParamsSection) ||
               !strcmp(name, kSetDataSection) ||
               !strcmp(name, kTestCodeSection);
    }

    static bool SectionRequiresArgument(const char* name) {
        return !strcmp(name, kSamplerParamsSection) ||
               !strcmp(name, kSetDataSection) ||
               !strcmp(name, kTestCodeSection);
    }

    static bool SectionPermitsDuplicates(const char* name) {
        return !strcmp(name, kSamplerParamsSection);
    }

private:
    const Program& fProgram;
    std::vector<const Variable*> fParameters;
    std::unordered_map<String, std::vector<const Section*>> fSections;
};

} // namespace SkSL

#endif
