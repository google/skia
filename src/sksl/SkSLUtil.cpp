/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLUtil.h"

namespace SkSL {

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
