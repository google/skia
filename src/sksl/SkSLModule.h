/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_MODULE
#define SKSL_MODULE

#include <memory>
#include <string>
#include <vector>

namespace SkSL {

class ProgramElement;
class SymbolTable;

enum class ModuleType : int8_t {
    unknown,
    sksl_shared,
    sksl_compute,
    sksl_frag,
    sksl_gpu,
    sksl_public,
    sksl_rt_shader,
    sksl_vert,
    sksl_graphite_frag,
    sksl_graphite_frag_es2,
    sksl_graphite_vert,
    sksl_graphite_vert_es2,
};

struct Module {
    const Module*                                fParent = nullptr;
    std::unique_ptr<SymbolTable>                 fSymbols;
    std::vector<std::unique_ptr<ProgramElement>> fElements;
    ModuleType                                   fModuleType = ModuleType::unknown;
};

std::string GetModuleData(ModuleType type, const char* filename);

}  // namespace SkSL

#endif  // SKSL_MODULE
