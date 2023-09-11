/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_STRING
#define SKSL_STRING

#include "include/core/SkTypes.h"
#include "src/base/SkNoDestructor.h"
#include "src/sksl/SkSLDefines.h"

#include <stdarg.h>
#include <string>
#include <string_view>

namespace SkSL {

bool stod(std::string_view s, SKSL_FLOAT* value);
bool stoi(std::string_view s, SKSL_INT* value);

namespace String {

std::string printf(const char* fmt, ...) SK_PRINTF_LIKE(1, 2);
void appendf(std::string* str, const char* fmt, ...) SK_PRINTF_LIKE(2, 3);
void vappendf(std::string* str, const char* fmt, va_list va) SK_PRINTF_LIKE(2, 0);

inline auto Separator() {
    // This returns a lambda which emits "" the first time it is called, and ", " every subsequent
    // time it is called.
    struct Output {
        const std::string fSpace, fComma;
    };
    static const SkNoDestructor<Output> kOutput(Output{{}, {", "}});

    return [firstSeparator = true]() mutable -> const std::string& {
        if (firstSeparator) {
            firstSeparator = false;
            return kOutput->fSpace;
        } else {
            return kOutput->fComma;
        }
    };
}

}  // namespace String
}  // namespace SkSL

namespace skstd {

// We use a custom to_string(float|double) which ignores locale settings and writes `1.0` instead
// of `1.00000`.
std::string to_string(float value);
std::string to_string(double value);

}  // namespace skstd

#endif
