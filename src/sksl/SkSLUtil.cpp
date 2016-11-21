/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLUtil.h"

#include <cinttypes>

namespace SkSL {

SkString to_string(double value) {
#ifdef SK_BUILD_FOR_WIN
    #define SNPRINTF    _snprintf
#else
    #define SNPRINTF    snprintf
#endif
#define MAX_DOUBLE_CHARS 25
    char buffer[MAX_DOUBLE_CHARS];
    SkDEBUGCODE(int len = )SNPRINTF(buffer, sizeof(buffer), "%.17g", value);
    ASSERT(len < MAX_DOUBLE_CHARS);
    SkString result(buffer);
    if (!strchr(buffer, '.') && !strchr(buffer, 'e')) {
        result += ".0";
    }
    return result;
#undef SNPRINTF
#undef MAX_DOUBLE_CHARS
}

SkString to_string(int32_t value) {
    return SkStringPrintf("%d", value);
}

SkString to_string(uint32_t value) {
    return SkStringPrintf("%u", value);
}

SkString to_string(int64_t value) {
    return SkStringPrintf("%" PRId64, value);
}

SkString to_string(uint64_t value) {
    return SkStringPrintf("%" PRIu64, value);
}

int stoi(SkString s) {
    if (s.size() > 2 && s[0] == '0' && s[1] == 'x') {
        char* p;
        int result = strtoul(s.c_str() + 2, &p, 16);
        ASSERT(*p == 0);
        return result;
    }
    return atoi(s.c_str());
}

double stod(SkString s) {
    return atof(s.c_str());
}

long stol(SkString s) {
    if (s.size() > 2 && s[0] == '0' && s[1] == 'x') {
        char* p;
        long result = strtoul(s.c_str() + 2, &p, 16);
        ASSERT(*p == 0);
        return result;
    }
    return atol(s.c_str());
}

void sksl_abort() {
#ifdef SKIA
    sk_abort_no_print();
    exit(1);
#else
    abort();
#endif
}

void write_data(const SkData& data, SkWStream& out) {
    out.write(data.data(), data.size());
}

SkString operator+(const SkString& s, const char* c) {
    SkString result(s);
    result += c;
    return result;
}

SkString operator+(const char* c, const SkString& s) {
    SkString result(c);
    result += s;
    return result;
}

SkString operator+(const SkString& s1, const SkString& s2) {
    SkString result(s1);
    result += s2;
    return result;
}

bool operator==(const SkString& s1, const char* s2) {
    return !strcmp(s1.c_str(), s2);
}

bool operator!=(const SkString& s1, const char* s2) {
    return strcmp(s1.c_str(), s2);
}

bool operator!=(const char* s1, const SkString& s2) {
    return strcmp(s1, s2.c_str());
}
} // namespace
