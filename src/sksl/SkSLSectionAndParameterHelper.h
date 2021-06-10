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
inline constexpr char kSetDataSection[] =            "setData";
inline constexpr char kTestCodeSection[] =           "test";

class SectionAndParameterHelper {
public:
    SectionAndParameterHelper(const Program* program, ErrorReporter& errors);

    const Section* getSection(skstd::string_view name) {
        auto found = fSections.find(name);
        if (found == fSections.end()) {
            return nullptr;
        }
        SkASSERT(found->second.size() == 1);
        return found->second[0];
    }

    std::vector<const Section*> getSections(skstd::string_view name) {
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

    static bool IsSupportedSection(skstd::string_view name) {
        return name == kClassSection ||
               name == kCloneSection ||
               name == kConstructorSection ||
               name == kConstructorCodeSection ||
               name == kConstructorParamsSection ||
               name == kCppSection ||
               name == kCppEndSection ||
               name == kDumpInfoSection ||
               name == kEmitCodeSection ||
               name == kFieldsSection ||
               name == kHeaderSection ||
               name == kHeaderEndSection ||
               name == kInitializersSection ||
               name == kMakeSection ||
               name == kOptimizationFlagsSection ||
               name == kSetDataSection ||
               name == kTestCodeSection;
    }

    static bool SectionAcceptsArgument(skstd::string_view name) {
        return name == kSetDataSection || name == kTestCodeSection;
    }

    static bool SectionRequiresArgument(skstd::string_view name) {
        return name == kSetDataSection || name == kTestCodeSection;
    }

private:
    const Program& fProgram;
    std::vector<const Variable*> fParameters;
    std::unordered_map<skstd::string_view, std::vector<const Section*>> fSections;
};

} // namespace SkSL

#endif
