/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkVMBuilders_DEFINED
#define SkVMBuilders_DEFINED

#include "src/core/SkVM.h"

// SkVM builders used by both SkVMBench.cpp and SkVMTest.cpp.

struct SrcoverBuilder_F32 : public skvm::Builder {
    enum class Fmt { A8, G8, RGBA_8888 };
    SrcoverBuilder_F32(Fmt srcFmt = Fmt::RGBA_8888,
                       Fmt dstFmt = Fmt::RGBA_8888);
};

struct SrcoverBuilder_I32_Naive : public skvm::Builder {
    SrcoverBuilder_I32_Naive();  // 8888 over 8888
};

struct SrcoverBuilder_I32 : public skvm::Builder {
    SrcoverBuilder_I32();  // 8888 over 8888
};

struct SrcoverBuilder_I32_SWAR : public skvm::Builder {
    SrcoverBuilder_I32_SWAR();  // 8888 over 8888
};

#endif//SkVMBuilders_DEFINED
