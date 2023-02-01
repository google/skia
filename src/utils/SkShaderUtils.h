/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShaderUtils_DEFINED
#define SkShaderUtils_DEFINED

#include "include/private/base/SkDebug.h"

#include <cstdint>
#include <functional>
#include <string>

namespace SkSL { enum class ProgramKind : int8_t; }

namespace SkShaderUtils {

std::string PrettyPrint(const std::string& string);

void VisitLineByLine(const std::string& text,
                     const std::function<void(int lineNumber, const char* lineText)>&);

// Prints shaders one line at the time. This ensures they don't get truncated by the adb log.
inline void PrintLineByLine(const std::string& text) {
    VisitLineByLine(text, [](int lineNumber, const char* lineText) {
        SkDebugf("%4i\t%s\n", lineNumber, lineText);
    });
}

// Combines raw shader and error text into an easier-to-read error message with line numbers.
std::string BuildShaderErrorMessage(const char* shader, const char* errors);

void PrintShaderBanner(SkSL::ProgramKind programKind);

}  // namespace SkShaderUtils

#endif
