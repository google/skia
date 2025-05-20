/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_GRAPHITE_MODULES
#define SKSL_GRAPHITE_MODULES

namespace SkSL::Loader {
struct GraphiteModules {
    const char* fFragmentShader;
    const char* fVertexShader;
};

// These need to be two different functions so we can implement them in two different files.
// In particular, the GetGraphiteModules() needs to be implemented in a graphite specific
// file, but SetGraphiteModuleLoader needs to be implemented in the common file (used by both
// Ganesh and Graphite).
GraphiteModules GetGraphiteModules();
void SetGraphiteModuleData(const GraphiteModules&);

}  // namespace SkSL::Loader

#endif  // SKSL_GRAPHITE_MODULES
