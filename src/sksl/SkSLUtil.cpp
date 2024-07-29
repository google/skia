/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLUtil.h"

#include "src/core/SkSLTypeShared.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLOutputStream.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/ir/SkSLType.h"

#include <string>

namespace SkSL {

// TODO: Once Graphite has its own GPU-caps system, SK_GRAPHITE should get its own mode.
// At the moment, it either mimics what GrShaderCaps reports, or it uses these hard-coded values
// depending on the build.
#if defined(SKSL_STANDALONE) || !defined(SK_GANESH)
std::unique_ptr<ShaderCaps> ShaderCapsFactory::MakeShaderCaps() {
    std::unique_ptr<ShaderCaps> standalone = std::make_unique<ShaderCaps>();
    standalone->fShaderDerivativeSupport = true;
    standalone->fExplicitTextureLodSupport = true;
    standalone->fFlatInterpolationSupport = true;
    standalone->fNoPerspectiveInterpolationSupport = true;
    standalone->fSampleMaskSupport = true;
    standalone->fExternalTextureSupport = true;
    return standalone;
}
#else
std::unique_ptr<ShaderCaps> ShaderCapsFactory::MakeShaderCaps() {
    return std::make_unique<ShaderCaps>();
}
#endif  // defined(SKSL_STANDALONE) || !defined(SK_GANESH)

void write_stringstream(const StringStream& s, OutputStream& out) {
    out.write(s.str().c_str(), s.str().size());
}

bool type_to_sksltype(const Context& context, const Type& type, SkSLType* outType) {
    // If a new GrSL type is added, this function will need to be updated.
    static_assert(kSkSLTypeCount == 41);

    if (type.matches(*context.fTypes.fVoid    )) { *outType = SkSLType::kVoid;     return true; }
    if (type.matches(*context.fTypes.fBool    )) { *outType = SkSLType::kBool;     return true; }
    if (type.matches(*context.fTypes.fBool2   )) { *outType = SkSLType::kBool2;    return true; }
    if (type.matches(*context.fTypes.fBool3   )) { *outType = SkSLType::kBool3;    return true; }
    if (type.matches(*context.fTypes.fBool4   )) { *outType = SkSLType::kBool4;    return true; }
    if (type.matches(*context.fTypes.fShort   )) { *outType = SkSLType::kShort;    return true; }
    if (type.matches(*context.fTypes.fShort2  )) { *outType = SkSLType::kShort2;   return true; }
    if (type.matches(*context.fTypes.fShort3  )) { *outType = SkSLType::kShort3;   return true; }
    if (type.matches(*context.fTypes.fShort4  )) { *outType = SkSLType::kShort4;   return true; }
    if (type.matches(*context.fTypes.fUShort  )) { *outType = SkSLType::kUShort;   return true; }
    if (type.matches(*context.fTypes.fUShort2 )) { *outType = SkSLType::kUShort2;  return true; }
    if (type.matches(*context.fTypes.fUShort3 )) { *outType = SkSLType::kUShort3;  return true; }
    if (type.matches(*context.fTypes.fUShort4 )) { *outType = SkSLType::kUShort4;  return true; }
    if (type.matches(*context.fTypes.fFloat   )) { *outType = SkSLType::kFloat;    return true; }
    if (type.matches(*context.fTypes.fFloat2  )) { *outType = SkSLType::kFloat2;   return true; }
    if (type.matches(*context.fTypes.fFloat3  )) { *outType = SkSLType::kFloat3;   return true; }
    if (type.matches(*context.fTypes.fFloat4  )) { *outType = SkSLType::kFloat4;   return true; }
    if (type.matches(*context.fTypes.fFloat2x2)) { *outType = SkSLType::kFloat2x2; return true; }
    if (type.matches(*context.fTypes.fFloat3x3)) { *outType = SkSLType::kFloat3x3; return true; }
    if (type.matches(*context.fTypes.fFloat4x4)) { *outType = SkSLType::kFloat4x4; return true; }
    if (type.matches(*context.fTypes.fHalf    )) { *outType = SkSLType::kHalf;     return true; }
    if (type.matches(*context.fTypes.fHalf2   )) { *outType = SkSLType::kHalf2;    return true; }
    if (type.matches(*context.fTypes.fHalf3   )) { *outType = SkSLType::kHalf3;    return true; }
    if (type.matches(*context.fTypes.fHalf4   )) { *outType = SkSLType::kHalf4;    return true; }
    if (type.matches(*context.fTypes.fHalf2x2 )) { *outType = SkSLType::kHalf2x2;  return true; }
    if (type.matches(*context.fTypes.fHalf3x3 )) { *outType = SkSLType::kHalf3x3;  return true; }
    if (type.matches(*context.fTypes.fHalf4x4 )) { *outType = SkSLType::kHalf4x4;  return true; }
    if (type.matches(*context.fTypes.fInt     )) { *outType = SkSLType::kInt;      return true; }
    if (type.matches(*context.fTypes.fInt2    )) { *outType = SkSLType::kInt2;     return true; }
    if (type.matches(*context.fTypes.fInt3    )) { *outType = SkSLType::kInt3;     return true; }
    if (type.matches(*context.fTypes.fInt4    )) { *outType = SkSLType::kInt4;     return true; }
    if (type.matches(*context.fTypes.fUInt    )) { *outType = SkSLType::kUInt;     return true; }
    if (type.matches(*context.fTypes.fUInt2   )) { *outType = SkSLType::kUInt2;    return true; }
    if (type.matches(*context.fTypes.fUInt3   )) { *outType = SkSLType::kUInt3;    return true; }
    if (type.matches(*context.fTypes.fUInt4   )) { *outType = SkSLType::kUInt4;    return true; }
    return false;
}

}  // namespace SkSL
