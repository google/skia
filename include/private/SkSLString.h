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

#ifndef SKSL_STANDALONE
#include "include/core/SkString.h"
#endif

namespace SkSL {

class String;

// Represents a (not necessarily null-terminated) slice of a string.
struct StringFragment {
    StringFragment()
    : fChars("")
    , fLength(0) {}

    StringFragment(const char* chars)
    : fChars(chars)
    , fLength(strlen(chars)) {}

    StringFragment(const char* chars, size_t length)
    : fChars(chars)
    , fLength(length) {}

    const char* begin() const { return fChars; }
    const char* end() const { return fChars + fLength; }

    const char* data() const { return fChars; }
    size_t size() const { return fLength; }
    size_t length() const { return fLength; }
    char operator[](size_t idx) const { return fChars[idx]; }

    bool startsWith(const char prefix[]) const;
    bool endsWith(const char suffix[]) const;

    bool operator==(const char* s) const;
    bool operator!=(const char* s) const;
    bool operator==(StringFragment s) const;
    bool operator!=(StringFragment s) const;
    bool operator<(StringFragment s) const;
    String operator+(const char* s) const;
    String operator+(const StringFragment& s) const;
    String operator+(const String& s) const;

#ifndef SKSL_STANDALONE
    operator SkString() const { return SkString(fChars, fLength); }
#endif

    const char* fChars;
    size_t fLength;
};

bool operator==(const char* s1, StringFragment s2);

bool operator!=(const char* s1, StringFragment s2);

class SK_API String : public std::string {
public:
    using std::string::string;

    explicit String(std::string s) : INHERITED(std::move(s)) {}
    String(StringFragment s) : INHERITED(s.fChars, s.fLength) {}

    static String printf(const char* fmt, ...) SK_PRINTF_LIKE(1, 2);
    void appendf(const char* fmt, ...) SK_PRINTF_LIKE(2, 3);
    void vappendf(const char* fmt, va_list va);

    bool startsWith(const char prefix[]) const {
        return StringFragment(data(), size()).startsWith(prefix);
    }
    bool endsWith(const char suffix[]) const {
        return StringFragment(data(), size()).endsWith(suffix);
    }

    bool consumeSuffix(const char suffix[]);

    String operator+(const char* s) const;
    String operator+(const String& s) const;
    String operator+(StringFragment s) const;
    String& operator+=(char c);
    String& operator+=(const char* s);
    String& operator+=(const String& s);
    String& operator+=(StringFragment s);
    bool operator==(const char* s) const;
    bool operator!=(const char* s) const;
    bool operator==(const String& s) const;
    bool operator!=(const String& s) const;
    friend String operator+(const char* s1, const String& s2);
    friend bool operator==(const char* s1, const String& s2);
    friend bool operator!=(const char* s1, const String& s2);

private:
    using INHERITED = std::string;
};

String operator+(const char* s1, const String& s2);
bool operator!=(const char* s1, const String& s2);

String to_string(double value);
String to_string(int32_t value);
String to_string(uint32_t value);
String to_string(int64_t value);
String to_string(uint64_t value);

bool stod(const StringFragment& s, SKSL_FLOAT* value);
bool stoi(const StringFragment& s, SKSL_INT* value);

} // namespace SkSL

namespace std {
    template<> struct hash<SkSL::StringFragment> {
        size_t operator()(const SkSL::StringFragment& s) const {
            size_t result = 0;
            for (size_t i = 0; i < s.fLength; ++i) {
                result = result * 101 + (size_t) s.fChars[i];
            }
            return result;
        }
    };

    template<> struct hash<SkSL::String> {
        size_t operator()(const SkSL::String& s) const {
            return hash<std::string>{}(s);
        }
    };
} // namespace std

#endif
