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

#if !defined(SKSL_STANDALONE)
bool type_to_grsltype(const Context& context, const Type& type, GrSLType* outType) {
    if (type == *context.fTypes.fFloat)    { *outType = kFloat_GrSLType;    return true; }
    if (type == *context.fTypes.fHalf)     { *outType = kHalf_GrSLType;     return true; }
    if (type == *context.fTypes.fFloat2)   { *outType = kFloat2_GrSLType;   return true; }
    if (type == *context.fTypes.fHalf2)    { *outType = kHalf2_GrSLType;    return true; }
    if (type == *context.fTypes.fFloat3)   { *outType = kFloat3_GrSLType;   return true; }
    if (type == *context.fTypes.fHalf3)    { *outType = kHalf3_GrSLType;    return true; }
    if (type == *context.fTypes.fFloat4)   { *outType = kFloat4_GrSLType;   return true; }
    if (type == *context.fTypes.fHalf4)    { *outType = kHalf4_GrSLType;    return true; }
    if (type == *context.fTypes.fFloat2x2) { *outType = kFloat2x2_GrSLType; return true; }
    if (type == *context.fTypes.fHalf2x2)  { *outType = kHalf2x2_GrSLType;  return true; }
    if (type == *context.fTypes.fFloat3x3) { *outType = kFloat3x3_GrSLType; return true; }
    if (type == *context.fTypes.fHalf3x3)  { *outType = kHalf3x3_GrSLType;  return true; }
    if (type == *context.fTypes.fFloat4x4) { *outType = kFloat4x4_GrSLType; return true; }
    if (type == *context.fTypes.fHalf4x4)  { *outType = kHalf4x4_GrSLType;  return true; }
    if (type == *context.fTypes.fVoid)     { *outType = kVoid_GrSLType;     return true; }
    return false;
}
#endif

}  // namespace SkSL
