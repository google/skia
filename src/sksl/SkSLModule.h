/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_MODULE
#define SKSL_MODULE

#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLSymbolTable.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace SkSL {

// A list of modules used in SkSL.
// Using an X-Macro (https://en.wikipedia.org/wiki/X_Macro) to manage the list.
#define SKSL_MODULE_LIST(M)   \
    M(sksl_shared)            \
    M(sksl_compute)           \
    M(sksl_frag)              \
    M(sksl_gpu)               \
    M(sksl_public)            \
    M(sksl_rt_shader)         \
    M(sksl_vert)              \
    M(sksl_graphite_frag)     \
    M(sksl_graphite_frag_es2) \
    M(sksl_graphite_vert)     \
    M(sksl_graphite_vert_es2)

enum class ModuleType : int8_t {
    unknown = 0,
#define M(type) type,
    SKSL_MODULE_LIST(M)
#undef M
};

struct Module {
    const Module*                                fParent = nullptr;
    std::unique_ptr<SymbolTable>                 fSymbols;
    std::vector<std::unique_ptr<ProgramElement>> fElements;
    ModuleType                                   fModuleType = ModuleType::unknown;
};

// Given a ModuleType, returns its name.
const char* ModuleTypeToString(ModuleType type);

std::string GetModuleData(ModuleType type, const char* filename);

}  // namespace SkSL

#endif  // SKSL_MODULE
