/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJumper_DEFINED
#define SkJumper_DEFINED

// This file contains definitions shared by SkJumper.cpp (compiled normally as part of Skia)
// and SkJumper_stages.cpp (compiled into Skia _and_ offline into SkJumper_generated.h).
// Keep it simple!

#include <stdint.h>

// SkJumper_stages.cpp has some unusual constraints on what constants it can use.
//
// If the constant is baked into the instruction, that's ok.
// If the constant is synthesized through code, that's ok.
// If the constant is loaded from memory, that's no good.
//
// We offer a couple facilities to get at any other constants you need:
//   - the C() function usually constrains constants to be directly baked into an instruction; or
//   - the _i and _f user-defined literal operators call C() for you in a prettier way; or
//   - you can load values from this struct.

struct SkJumper_constants {
    float iota[8];      //  0,1,2,3,4,5,6,7
};

#endif//SkJumper_DEFINED
