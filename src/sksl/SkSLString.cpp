/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLString.h"

#include "SkSLUtil.h"
#include <cinttypes>
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

#ifdef SKSL_STANDALONE
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
    size_t size = VSNPRINTF(buffer, BUFFER_SIZE, fmt, args);
    if (BUFFER_SIZE >= size) {
        this->append(buffer, size);
    } else {
        char* newBuffer = new char[size];
        VSNPRINTF(newBuffer, size, fmt, args);
        this->append(newBuffer, size);
        delete[] newBuffer;
    }
    va_end(args);
}


bool String::startsWith(const char* s) const {
    return strncmp(c_str(), s, strlen(s));
}

bool String::endsWith(const char* s) const {
    size_t len = strlen(s);
    if (size() < len) {
        return false;
    }
    return strncmp(c_str() + size() - len, s, len);
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
    result += s2;
    return result;
}

bool operator==(const char* s1, const String& s2) {
    return s2 == s1;
}

bool operator!=(const char* s1, const String& s2) {
    return s2 != s1;
}

String to_string(int32_t value) {
    return SkSL::String::printf("%d", value);
}

String to_string(uint32_t value) {
    return SkSL::String::printf("%u", value);
}

String to_string(int64_t value) {
    return SkSL::String::printf("%" PRId64, value);
}

String to_string(uint64_t value) {
    return SkSL::String::printf("%" PRIu64, value);
}

int stoi(String s) {
    if (s.size() > 2 && s[0] == '0' && s[1] == 'x') {
        char* p;
        int result = strtoul(s.c_str() + 2, &p, 16);
        ASSERT(*p == 0);
        return result;
    }
    return atoi(s.c_str());
}

double stod(String s) {
    double result;
    std::string str(s.c_str(), s.size());
    std::stringstream buffer(str);
    buffer.imbue(std::locale::classic());
    buffer >> result;
    return result;
}

long stol(String s) {
    if (s.size() > 2 && s[0] == '0' && s[1] == 'x') {
        char* p;
        long result = strtoul(s.c_str() + 2, &p, 16);
        ASSERT(*p == 0);
        return result;
    }
    return atol(s.c_str());
}

} // namespace
