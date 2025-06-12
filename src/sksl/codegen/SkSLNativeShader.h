/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_NATIVESHADER
#define SKSL_NATIVESHADER

#include <cstdint>
#include <string>
#include <vector>

namespace SkSL {

// After compiling SkSL to native, the result is either in text or binary form. Currently, only
// SPIR-V is in binary form.
struct NativeShader {
    std::string fText;
    std::vector<uint32_t> fBinary;

    bool isBinary() const { return !fBinary.empty(); }
};

}  // namespace SkSL

#endif  // SKSL_NATIVESHADER
