/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSplicer_shared_DEFINED
#define SkSplicer_shared_DEFINED

// This file contains definitions shared by SkSplicer.cpp (compiled normally as part of Skia)
// and SkSplicer_stages.cpp (compiled offline into SkSplicer_stages.h).  Keep it simple.

#include <stdint.h>

// SkSplicer stages can use constant literals only if they end up baked into the instruction,
// like bit shifts and rounding modes.  Any other constant values must be pulled from this struct.
// (Except 0 and 0.0f, which always end up as some sort of xor instruction.)

struct SkSplicer_constants {
    uint32_t _0x000000ff;  // 0x000000ff
    float    _1;           // 1.0f
    float    _255;         // 255.0f
    float    _1_255;       // 1/255.0f
};

#endif//SkSplicer_shared_DEFINED
