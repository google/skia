/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_MODULEDATA
#define SKSL_MODULEDATA

#include <string>

namespace SkSL {

enum class ModuleName {
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

std::string GetModuleData(ModuleName name, const char* filename);

}  // namespace SkSL

#endif  // SKSL_MODULEDATA
