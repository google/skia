/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef CompilationModeKeys_DEFINED
#define CompilationModeKeys_DEFINED

#include <map>
#include <string>

// Returns any key/value pairs pertaining compilation mode that should be included in Gold or Perf
// traces.
std::map<std::string, std::string> GetCompilationModeGoldAndPerfKeyValuePairs();

#endif  // CompilationModeKeys_DEFINED
