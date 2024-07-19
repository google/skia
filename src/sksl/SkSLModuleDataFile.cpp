/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLModule.h"

#include "include/core/SkString.h"
#include "src/utils/SkGetExecutablePath.h"
#include "src/utils/SkOSPath.h"

#include <fstream>

namespace SkSL {

std::string GetModuleData(ModuleType /*name*/, const char* filename) {
    std::string exePath = SkGetExecutablePath();
    SkString exeDir = SkOSPath::Dirname(exePath.c_str());
    SkString modulePath = SkOSPath::Join(exeDir.c_str(), filename);
    std::ifstream in(std::string{modulePath.c_str()});
    std::string moduleSource{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
    if (in.rdstate()) {
        SK_ABORT("Error reading %s\n", modulePath.c_str());
    }
    return moduleSource;
}

}  // namespace SkSL
