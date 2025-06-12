/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShaderUtils_DEFINED
#define SkShaderUtils_DEFINED

#include "include/core/SkSpan.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

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

// Prints binary shaders one line at the time. This ensures they don't get truncated by the adb log.
std::string SpirvAsHexStream(SkSpan<const uint32_t> spirv);

// Convert compiled binary generated in a std::string to std::vector<uint32_t>. This can be removed
// if SPIR-V generator is modified to directly generate std::vector.
inline std::vector<uint32_t> StringToBinary(const std::string& binary) {
    const uint32_t* words = reinterpret_cast<const uint32_t*>(binary.data());
    SkASSERT(binary.size() % sizeof(uint32_t) == 0);
    const size_t wordCount = binary.size() / sizeof(uint32_t);
    return std::vector<uint32_t>(words, words + wordCount);
}

// Combines raw shader and error text into an easier-to-read error message with line numbers.
std::string BuildShaderErrorMessage(const char* shader, const char* errors);

void PrintShaderBanner(SkSL::ProgramKind programKind);

}  // namespace SkShaderUtils

#endif
