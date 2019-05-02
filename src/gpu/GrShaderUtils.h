/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrShaderUtils_DEFINED
#define GrShaderUtils_DEFINED

#include "include/core/SkTypes.h"
#include "include/gpu/GrContextOptions.h"
#include "src/sksl/SkSLString.h"

namespace GrShaderUtils {

SkSL::String PrettyPrint(const SkSL::String& string);
void PrintLineByLine(const char* header, const SkSL::String& text);
GrContextOptions::ShaderErrorHandler* DefaultShaderErrorHandler();

}

#endif
