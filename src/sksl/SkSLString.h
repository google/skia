/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_STRING
#define SKSL_STRING


#ifdef SKSL_STANDALONE
    #define SKSL_STRING_BASE std::string
    #include <string>
#else
    #define SKSL_STRING_BASE SkString
    #include "SkString.h"
#endif

namespace SkSL {

class String : public SKSL_STRING_BASE {
public:
    String() = default;
    String(const String&) = default;
    String(String&&) = default;
    String& operator=(const String&) = default;
    String& operator=(String&&) = default;

#ifndef SKSL_STANDALONE
    String(const SkString& s)
    : INHERITED(s) {}
#endif

    String(const char* s)
    : INHERITED(s) {}

    String(const char* s, size_t size)
    : INHERITED(s, size) {}

    static String printf(const char* fmt, ...);

#ifdef SKSL_STANDALONE
    void appendf(const char* fmt, ...);
#endif
    void vappendf(const char* fmt, va_list va);

    bool startsWith(const char* s) const;
    bool endsWith(const char* s) const;

    String operator+(const char* s) const;
    String operator+(const String& s) const;
    bool operator==(const char* s) const;
    bool operator!=(const char* s) const;
    bool operator==(const String& s) const;
    bool operator!=(const String& s) const;
    friend String operator+(const char* s1, const String& s2);
    friend bool operator==(const char* s1, const String& s2);
    friend bool operator!=(const char* s1, const String& s2);

private:
    typedef SKSL_STRING_BASE INHERITED;
};

String operator+(const char* s1, const String& s2);
bool operator!=(const char* s1, const String& s2);

String to_string(double value);

String to_string(int32_t value);

String to_string(uint32_t value);

String to_string(int64_t value);

String to_string(uint64_t value);

int stoi(String s);

double stod(String s);

long stol(String s);

} // namespace

#ifdef SKSL_STANDALONE
namespace std {
    template<> struct hash<SkSL::String> {
        size_t operator()(const SkSL::String& s) const {
            return hash<std::string>{}(s);
        }
    };
} // namespace
#else
#include "SkOpts.h"
namespace std {
    template<> struct hash<SkSL::String> {
        size_t operator()(const SkSL::String& s) const {
            return SkOpts::hash_fn(s.c_str(), s.size(), 0);
        }
    };
} // namespace
#endif // SKIA_STANDALONE

#endif
