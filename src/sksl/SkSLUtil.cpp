/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLUtil.h"

namespace SkSL {

std::string to_string(double value) {
    std::stringstream buffer;
    buffer << std::setprecision(std::numeric_limits<double>::digits10) << value;
    std::string result = buffer.str();
    if (result.find_last_of(".") == std::string::npos && 
        result.find_last_of("e") == std::string::npos) {
        result += ".0";
    }
    return result;
}

std::string to_string(int32_t value) {
    std::stringstream buffer;
    buffer << value;
    return buffer.str();
}

std::string to_string(uint32_t value) {
    std::stringstream buffer;
    buffer << value;
    return buffer.str();
}

std::string to_string(int64_t value) {
    std::stringstream buffer;
    buffer << value;
    return buffer.str();
}

std::string to_string(uint64_t value) {
    std::stringstream buffer;
    buffer << value;
    return buffer.str();
}

int stoi(std::string s) {
    return atoi(s.c_str());
}

double stod(std::string s) {
    return atof(s.c_str());
}

long stol(std::string s) {
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

} // namespace
