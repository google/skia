/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_STRING
#define SKSL_STRING

#include "include/private/SkSLDefines.h"
#include <cstring>
#include <stdarg.h>
#include <string>
#include <string_view>

#ifndef SKSL_STANDALONE
#include "include/core/SkString.h"
#endif

namespace SkSL {

bool stod(std::string_view s, SKSL_FLOAT* value);
bool stoi(std::string_view s, SKSL_INT* value);

namespace String {

std::string printf(const char* fmt, ...) SK_PRINTF_LIKE(1, 2);
void appendf(std::string* str, const char* fmt, ...) SK_PRINTF_LIKE(2, 3);
void vappendf(std::string* str, const char* fmt, va_list va);

}  // namespace String
}  // namespace SkSL

namespace skstd {

// For most types, pass-through to std::to_string as-is.
template <typename T> std::string to_string(T value) {
    return std::to_string(value);
}

// We customize the output from to_string(float|double) slightly.
template <> std::string to_string(float value);
template <> std::string to_string(double value);

}  // namespace skstd

#endif
