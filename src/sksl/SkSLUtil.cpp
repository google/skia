/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLUtil.h"

#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/ir/SkSLType.h"

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

namespace SkSL {

#if defined(SKSL_STANDALONE) || !SK_SUPPORT_GPU
StandaloneShaderCaps standaloneCaps;

ShaderCapsPointer ShaderCapsFactory::MakeShaderCaps() {
    return std::make_shared<StandaloneShaderCaps>();
}
#else
ShaderCapsPointer ShaderCapsFactory::MakeShaderCaps() {
    return sk_make_sp<GrShaderCaps>(GrContextOptions());
}
#endif  // defined(SKSL_STANDALONE) || !SK_SUPPORT_GPU

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

}  // namespace SkSL
