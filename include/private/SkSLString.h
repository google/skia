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

class String;

class SK_API String : public std::string {
public:
    using std::string::string;

    explicit String(std::string s) : INHERITED(std::move(s)) {}
    explicit String(std::string_view s) : INHERITED(s.data(), s.length()) {}

    static String printf(const char* fmt, ...) SK_PRINTF_LIKE(1, 2);
    static void appendf(String* str, const char* fmt, ...) SK_PRINTF_LIKE(2, 3);
    static void vappendf(String* str, const char* fmt, va_list va);

    String operator+(const char* s) const;
    String operator+(const String& s) const;
    String operator+(std::string_view s) const;
    String& operator+=(char c);
    String& operator+=(const char* s);
    String& operator+=(const String& s);
    String& operator+=(std::string_view s);
    friend String operator+(const char* s1, const String& s2);

private:
    using INHERITED = std::string;
};

String operator+(std::string_view left, std::string_view right);

String to_string(double value);
String to_string(int32_t value);
String to_string(uint32_t value);
String to_string(int64_t value);
String to_string(uint64_t value);

bool stod(std::string_view s, SKSL_FLOAT* value);
bool stoi(std::string_view s, SKSL_INT* value);

} // namespace SkSL

namespace std {
    template<> struct hash<SkSL::String> {
        size_t operator()(const SkSL::String& s) const {
            return hash<std::string>{}(s);
        }
    };
} // namespace std

#endif
