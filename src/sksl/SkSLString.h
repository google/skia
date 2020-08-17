/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_STRING
#define SKSL_STRING

#include "src/sksl/SkSLDefines.h"
#include <cstring>
#include <stdarg.h>
#include <string>

#ifndef SKSL_STANDALONE
#include "include/core/SkString.h"
#endif

namespace SkSL {

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

    char operator[](size_t idx) const {
        return fChars[idx];
    }

    bool operator==(const char* s) const;
    bool operator!=(const char* s) const;
    bool operator==(StringFragment s) const;
    bool operator!=(StringFragment s) const;
    bool operator<(StringFragment s) const;

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

    String(StringFragment s) : INHERITED(s.fChars, s.fLength) {}

    static String printf(const char* fmt, ...);
    void appendf(const char* fmt, ...);
    void vappendf(const char* fmt, va_list va);

    bool startsWith(const char* prefix) const;
    bool endsWith(const char* suffix) const;

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
    typedef std::string INHERITED;
};

String operator+(const char* s1, const String& s2);
bool operator!=(const char* s1, const String& s2);

String to_string(double value);
String to_string(int32_t value);
String to_string(uint32_t value);
String to_string(int64_t value);
String to_string(uint64_t value);

SKSL_INT stoi(const String& s);
SKSL_FLOAT stod(const String& s);
long stol(const String& s);

} // namespace  SkSL

namespace std {
    template<> struct hash<SkSL::StringFragment> {
        size_t operator()(const SkSL::StringFragment& s) const {
            size_t result = 0;
            for (size_t i = 0; i < s.fLength; ++i) {
                result = result * 101 + s.fChars[i];
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
