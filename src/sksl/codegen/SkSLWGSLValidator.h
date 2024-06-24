/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_WGSLVALIDATOR
#define SKSL_WGSLVALIDATOR

#include <string>
#include <string_view>

namespace SkSL {

class ErrorReporter;

bool ValidateWGSL(ErrorReporter& reporter, std::string_view wgsl, std::string* warnings);
bool ValidateWGSLVerbose(ErrorReporter& reporter, std::string_view wgsl, std::string* warnings);

}  // namespace SkSL

#endif
