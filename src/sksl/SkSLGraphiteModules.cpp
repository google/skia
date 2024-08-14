/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLGraphiteModules.h"

#include "include/core/SkTypes.h"

// We include minified SkSL module code and pass it directly to the compiler.
#if defined(SK_ENABLE_OPTIMIZE_SIZE) || !defined(SK_DEBUG)
#include "src/sksl/generated/sksl_graphite_frag.minified.sksl"
#include "src/sksl/generated/sksl_graphite_vert.minified.sksl"
#include "src/sksl/generated/sksl_graphite_frag_es2.minified.sksl"
#include "src/sksl/generated/sksl_graphite_vert_es2.minified.sksl"
#else
#include "src/sksl/generated/sksl_graphite_frag.unoptimized.sksl"
#include "src/sksl/generated/sksl_graphite_frag_es2.unoptimized.sksl"
#include "src/sksl/generated/sksl_graphite_vert.unoptimized.sksl"
#include "src/sksl/generated/sksl_graphite_vert_es2.unoptimized.sksl"
#endif

namespace SkSL::Loader {

GraphiteModules GetGraphiteModules() {
#define M(name) SKSL_MINIFIED_##name
    return GraphiteModules{
            M(sksl_graphite_frag),
            M(sksl_graphite_frag_es2),
            M(sksl_graphite_vert),
            M(sksl_graphite_vert_es2),
    };
}

}  // namespace SkSL::Loader
