/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSplicer_shared_DEFINED
#define SkSplicer_shared_DEFINED

// This file contains definitions shared by SkSplicer.cpp (compiled normally as part of Skia)
// and SkSplicer_stages.cpp (compiled offline into SkSplicer_generated.h).  Keep it simple!

#include <stdint.h>

// SkSplicer Stages can use constant literals only if they end up baked into the instruction,
// like bit shifts and rounding modes.  Any other constant values must be pulled from this struct
// (except 0 and 0.0f, which always end up as some sort of xor instruction).
//
// This constraint makes it much easier to move and reorder the code for each Stage.

struct SkSplicer_constants {
    float    _1;           //  1.0f
    float    _255;         //  255.0f
    float    _1_255;       //  1/255.0f
    uint32_t _0x000000ff;  //  0x000000ff

    // from_srgb
    float    _00025;       //  0.0025f
    float    _06975;       //  0.6975f
    float    _03000;       //  0.3000f
    float    _1_1292;      //  1/12.92f
    float    _0055;        //  0.055f

    // to_srgb
    float    _1246;        //  12.46f
    float    _0411192;     //  0.411192f
    float    _0689206;     //  0.689206f
    float   n_00988;       // -0.0988f
    float    _00043;       //  0.0043f
};

struct SkSplicer_constants_lowp {
    uint16_t _1;           // 0x8000 == 32768 == 1.0
    uint16_t _0x8081;      // 0x8081 == 32897, closest value to 32768 * (256/255).
};

#endif//SkSplicer_shared_DEFINED
