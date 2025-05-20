/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLGraphiteModules.h"
#include "src/sksl/SkSLModule.h"

#include <string>

// We include minified SkSL module code and pass it directly to the compiler.
#if defined(SK_ENABLE_OPTIMIZE_SIZE) || !defined(SK_DEBUG)
#include "src/sksl/generated/sksl_compute.minified.sksl"
#include "src/sksl/generated/sksl_frag.minified.sksl"
#include "src/sksl/generated/sksl_gpu.minified.sksl"
#include "src/sksl/generated/sksl_public.minified.sksl"
#include "src/sksl/generated/sksl_rt_shader.minified.sksl"
#include "src/sksl/generated/sksl_shared.minified.sksl"
#include "src/sksl/generated/sksl_vert.minified.sksl"
#else
#include "src/sksl/generated/sksl_compute.unoptimized.sksl"
#include "src/sksl/generated/sksl_frag.unoptimized.sksl"
#include "src/sksl/generated/sksl_gpu.unoptimized.sksl"
#include "src/sksl/generated/sksl_public.unoptimized.sksl"
#include "src/sksl/generated/sksl_rt_shader.unoptimized.sksl"
#include "src/sksl/generated/sksl_shared.unoptimized.sksl"
#include "src/sksl/generated/sksl_vert.unoptimized.sksl"
#endif

// We don't load the graphite modules by default because we don't want to bloat the Ganesh-only
// build with Graphite's modules. These will be filled in during initialization of the
// Graphite backend.
static const char* sdata_sksl_graphite_frag = "";
static const char* sdata_sksl_graphite_vert = "";

namespace SkSL {

std::string GetModuleData(ModuleType type, const char* /*filename*/) {
#define M(name) case ModuleType::name: return std::string(SKSL_MINIFIED_##name);
// Creating a std::string with a nullptr is UB
#define G(name)                               \
    case ModuleType::name:                    \
        if (sdata_##name) {                   \
            return std::string(sdata_##name); \
        } else {                              \
            return "";                        \
        }

    switch (type) {
        M(sksl_shared)
        M(sksl_compute)
        M(sksl_frag)
        M(sksl_gpu)
        M(sksl_public)
        M(sksl_rt_shader)
        M(sksl_vert)

        G(sksl_graphite_frag)
        G(sksl_graphite_vert)
        default:
            SkUNREACHABLE;
    }
#undef M
}

namespace Loader {
void SetGraphiteModuleData(const GraphiteModules& modules) {
    SkASSERTF(sdata_sksl_graphite_frag[0] == '\0', "We should only initialize this once");
    sdata_sksl_graphite_frag = modules.fFragmentShader;
    sdata_sksl_graphite_vert = modules.fVertexShader;

    SkASSERT(sdata_sksl_graphite_frag[0] != '\0');
    SkASSERT(sdata_sksl_graphite_vert[0] != '\0');
}
}  // namespace Loader

}  // namespace SkSL
