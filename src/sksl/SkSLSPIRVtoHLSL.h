/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SPIRVTOHLSL
#define SKSL_SPIRVTOHLSL

#include "src/sksl/SkSLString.h"

namespace SkSL {

bool SPIRVtoHLSL(const String& spirv, String* hlsl);

}

#endif
