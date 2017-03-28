/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLUtil.h"

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

namespace SkSL {

#ifdef SKSL_STANDALONE
StandaloneShaderCaps standaloneCaps;
#endif

String to_string(double value) {
#ifdef SK_BUILD_FOR_WIN
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

void sksl_abort() {
#ifdef SKIA
    sk_abort_no_print();
    exit(1);
#else
    abort();
#endif
}

void write_stringstream(const StringStream& s, OutputStream& out) {
    out.write(s.data(), s.size());
}

} // namespace
