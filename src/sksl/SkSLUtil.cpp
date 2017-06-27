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

void sksl_abort() {
#ifdef SKSL_STANDALONE
    abort();
#else
    sk_abort_no_print();
    exit(1);
#endif
}

void write_stringstream(const StringStream& s, OutputStream& out) {
    out.write(s.str().c_str(), s.str().size());
}

} // namespace
