/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkVM_fwd_DEFINED
#define SkVM_fwd_DEFINED
#if defined(SK_ENABLE_SKVM)

namespace skvm {
    class Assembler;
    class Builder;
    class Program;
    struct Ptr;
    struct I32;
    struct F32;
    struct Color;
    struct Coord;
    struct Uniforms;
}  // namespace skvm

#endif  // defined(SK_ENABLE_SKVM)
#endif  // SkVM_fwd_DEFINED
