//
// Copyright (c) 2011-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//
// length_limits.h
//

#ifndef COMPILER_TRANSLATOR_LENGTHLIMITS_H_
#define COMPILER_TRANSLATOR_LENGTHLIMITS_H_

#include "GLSLANG/ShaderLang.h"

// These constants are factored out from the rest of the headers to
// make it easier to reference them from the compiler sources.

size_t GetGlobalMaxTokenSize(ShShaderSpec spec);

#endif // COMPILER_TRANSLATOR_LENGTHLIMITS_H_
