/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

 #include "tools/testrunners/common/compilation_mode_keys/CompilationModeKeys.h"

 std::map<std::string, std::string> GetCompilationModeGoldAndPerfKeyValuePairs() {
   return std::map<std::string, std::string>{
     {"compilation_mode", SKIA_COMPILATION_MODE_KEY}, // Defined in BUILD.bazel.
   };
 }
