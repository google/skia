/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLModule.h"

namespace SkSL {

const char* ModuleTypeToString(ModuleType type) {
#define M(type) case ModuleType::type: return #type;
    switch (type) {
        SKSL_MODULE_LIST(M)
        default: return "unknown";
    }
#undef M
}

}  // namespace SkSL
