/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLString.h"

#include "SkSLUtil.h"
#include <errno.h>
#include <limits.h>
#include <locale>
#include <sstream>
#include <string>

namespace SkSL {

String String::printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    String result;
    result.vappendf(fmt, args);
    return result;
}

#ifdef SKSL_USE_STD_STRING
void String::appendf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    this->vappendf(fmt, args);
}
#endif

void String::vappendf(const char* fmt, va_list args) {
#ifdef SKSL_BUILD_FOR_WIN
    #define VSNPRINTF    _vsnprintf
#else
    #define VSNPRINTF    vsnprintf
#endif
    #define BUFFER_SIZE 256
    char buffer[BUFFER_SIZE];
    va_list reuse;
    va_copy(reuse, args);
    size_t size = VSNPRINTF(buffer, BUFFER_SIZE, fmt, args);
    if (BUFFER_SIZE >= size) {
        this->append(buffer, size);
    } else {
        auto newBuffer = std::unique_ptr<char[]>(new char[size + 1]);
        VSNPRINTF(newBuffer.get(), size + 1, fmt, reuse);
        this->append(newBuffer.get(), size);
    }
}


bool String::startsWith(const char* s) const {
    return !strncmp(c_str(), s, strlen(s));
}

bool String::endsWith(const char* s) const {
    size_t len = strlen(s);
    if (size() < len) {
        return false;
    }
    return !strncmp(c_str() + size() - len, s, len);
}

String String::operator+(const char* s) const {
    String result(*this);
    result.append(s);
    return result;
}

String String::operator+(const String& s) const {
    String result(*this);
    result.append(s);
    return result;
}

String String::operator+(StringFragment s) const {
    String result(*this);
    result.append(s.fChars, s.fLength);
    return result;
}

String& String::operator+=(char c) {
    INHERITED::operator+=(c);
    return *this;
}

String& String::operator+=(const char* s) {
    INHERITED::operator+=(s);
    return *this;
}

String& String::operator+=(const String& s) {
    INHERITED::operator+=(s);
    return *this;
}

String& String::operator+=(StringFragment s) {
    this->append(s.fChars, s.fLength);
    return *this;
}

bool String::operator==(const String& s) const {
    return this->size() == s.size() && !memcmp(c_str(), s.c_str(), this->size());
}

bool String::operator!=(const String& s) const {
    return !(*this == s);
}

bool String::operator==(const char* s) const {
    return this->size() == strlen(s) && !memcmp(c_str(), s, this->size());
}

bool String::operator!=(const char* s) const {
    return !(*this == s);
}

String operator+(const char* s1, const String& s2) {
    String result(s1);
    result.append(s2);
    return result;
}

bool operator==(const char* s1, const String& s2) {
    return s2 == s1;
}

bool operator!=(const char* s1, const String& s2) {
    return s2 != s1;
}

bool StringFragment::operator==(StringFragment s) const {
    if (fLength != s.fLength) {
        return false;
    }
    return !memcmp(fChars, s.fChars, fLength);
}

bool StringFragment::operator!=(StringFragment s) const {
    if (fLength != s.fLength) {
        return true;
    }
    return memcmp(fChars, s.fChars, fLength);
}

bool StringFragment::operator==(const char* s) const {
    for (size_t i = 0; i < fLength; ++i) {
        if (fChars[i] != s[i]) {
            return false;
        }
    }
    return 0 == s[fLength];
}

bool StringFragment::operator!=(const char* s) const {
    for (size_t i = 0; i < fLength; ++i) {
        if (fChars[i] != s[i]) {
            return true;
        }
    }
    return 0 != s[fLength];
}

bool StringFragment::operator<(StringFragment other) const {
    int comparison = strncmp(fChars, other.fChars, std::min(fLength, other.fLength));
    if (comparison) {
        return comparison < 0;
    }
    return fLength < other.fLength;
}

bool operator==(const char* s1, StringFragment s2) {
    return s2 == s1;
}

bool operator!=(const char* s1, StringFragment s2) {
    return s2 != s1;
}

String to_string(int32_t value) {
    return SkSL::String::printf("%d", value);
}

String to_string(uint32_t value) {
    return SkSL::String::printf("%u", value);
}

String to_string(int64_t value) {
    std::stringstream buffer;
    buffer << value;
    return String(buffer.str().c_str());
}

String to_string(uint64_t value) {
    std::stringstream buffer;
    buffer << value;
    return String(buffer.str().c_str());
}

String to_string(double value) {
#ifdef SKSL_BUILD_FOR_WIN
    #define SNPRINTF    _snprintf
#else
    #define SNPRINTF    snprintf
#endif
#define MAX_DOUBLE_CHARS 25
    char buffer[MAX_DOUBLE_CHARS];
    SKSL_DEBUGCODE(int len = )SNPRINTF(buffer, sizeof(buffer), "%.17g", value);
    ASSERT(len < MAX_DOUBLE_CHARS);
    String result(buffer);
    if (!strchr(buffer, '.') && !strchr(buffer, 'e')) {
        result += ".0";
    }
    return result;
#undef SNPRINTF
#undef MAX_DOUBLE_CHARS
}

int stoi(const String& s) {
    char* p;
    SKSL_DEBUGCODE(errno = 0;)
    long result = strtoul(s.c_str(), &p, 0);
    ASSERT(*p == 0);
    ASSERT(!errno);
    return (int) result;
}

double stod(const String& s) {
    double result;
    std::string str(s.c_str(), s.size());
    std::stringstream buffer(str);
    buffer.imbue(std::locale::classic());
    buffer >> result;
    ASSERT(!buffer.fail());
    return result;
}

long stol(const String& s) {
    char* p;
    SKSL_DEBUGCODE(errno = 0;)
    long result = strtoul(s.c_str(), &p, 0);
    ASSERT(*p == 0);
    ASSERT(!errno);
    return result;
}

} // namespace
