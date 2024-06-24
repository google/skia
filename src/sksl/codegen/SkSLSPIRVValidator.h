/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SPIRVVALIDATOR
#define SKSL_SPIRVVALIDATOR

#include <string_view>

namespace SkSL {

class ErrorReporter;

// SPIRV issues will cause an SkDEBUGFAILF to be triggered with the error message
// and false will be returned (i.e. invalid SPIRV is a fatal issue in debug builds).
bool ValidateSPIRV(ErrorReporter&, std::string_view);

// SPIRV issues will be sent to the provided ErrorReporter along with a disassembly
// of the code. This will also return false, but not be fatal in debug builds.
bool ValidateSPIRVAndDissassemble(ErrorReporter&, std::string_view);

}  // namespace SkSL

#endif
