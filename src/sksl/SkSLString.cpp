/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLString.h"
#include "src/sksl/SkSLUtil.h"
#include <algorithm>
#include <cinttypes>
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
    va_end(args);
    return result;
}

void String::appendf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    this->vappendf(fmt, args);
    va_end(args);
}

void String::vappendf(const char* fmt, va_list args) {
    #define BUFFER_SIZE 256
    char buffer[BUFFER_SIZE];
    va_list reuse;
    va_copy(reuse, args);
    size_t size = vsnprintf(buffer, BUFFER_SIZE, fmt, args);
    if (BUFFER_SIZE >= size + 1) {
        this->append(buffer, size);
    } else {
        auto newBuffer = std::unique_ptr<char[]>(new char[size + 1]);
        vsnprintf(newBuffer.get(), size + 1, fmt, reuse);
        this->append(newBuffer.get(), size);
    }
    va_end(reuse);
}

bool StringFragment::startsWith(const char prefix[]) const {
    return !strncmp(fChars, prefix, strlen(prefix));
}

bool StringFragment::endsWith(const char suffix[]) const {
    size_t suffixLength = strlen(suffix);
    if (fLength < suffixLength) {
        return false;
    }
    return !strncmp(fChars + fLength - suffixLength, suffix, suffixLength);
}

bool String::consumeSuffix(const char suffix[]) {
    size_t suffixLength = strlen(suffix);
    if (this->length() < suffixLength) {
        return false;
    }
    if (0 != strncmp(this->data() + this->size() - suffixLength, suffix, suffixLength)) {
        return false;
    }
    this->resize(this->length() - suffixLength);
    return true;
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

String StringFragment::operator+(const char* other) const {
    return String(*this) + other;
}

String StringFragment::operator+(const StringFragment& other) const {
    return String(*this) + other;
}

String StringFragment::operator+(const String& other) const {
    return String(*this) + other;
}

bool operator==(const char* s1, StringFragment s2) {
    return s2 == s1;
}

bool operator!=(const char* s1, StringFragment s2) {
    return s2 != s1;
}

String to_string(int32_t value) {
    return SkSL::String(std::to_string(value));
}

String to_string(uint32_t value) {
    return SkSL::String(std::to_string(value));
}

String to_string(int64_t value) {
    return SkSL::String(std::to_string(value));
}

String to_string(uint64_t value) {
    return SkSL::String(std::to_string(value));
}

String to_string(double value) {
    std::stringstream buffer;
    buffer.imbue(std::locale::classic());
    buffer.precision(17);
    buffer << value;
    bool needsDotZero = true;
    const std::string str = buffer.str();
    for (int i = str.size() - 1; i >= 0; --i) {
        char c = str[i];
        if (c == '.' || c == 'e') {
            needsDotZero = false;
            break;
        }
    }
    if (needsDotZero) {
        buffer << ".0";
    }
    return String(buffer.str().c_str());
}

bool stod(const StringFragment& s, SKSL_FLOAT* value) {
    std::string str(s.data(), s.size());
    std::stringstream buffer(str);
    buffer.imbue(std::locale::classic());
    buffer >> *value;
    return !buffer.fail();
}

bool stoi(const StringFragment& s, SKSL_INT* value) {
    char* p;
    errno = 0;
    unsigned long long result = strtoull(s.begin(), &p, /*base=*/0);
    *value = static_cast<SKSL_INT>(result);
    return p == s.end() && errno == 0 && result <= 0xFFFFFFFF;
}

}  // namespace SkSL
