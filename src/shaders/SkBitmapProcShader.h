/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBitmapProcShader_DEFINED
#define SkBitmapProcShader_DEFINED

#include "include/core/SkSamplingOptions.h"
#include "src/shaders/SkShaderBase.h"

class SkArenaAlloc;
class SkImage_Base;
enum class SkTileMode;

class SkBitmapProcLegacyShader : public SkShaderBase {
private:
    friend class SkImageShader;

    static Context* MakeContext(const SkShaderBase&, SkTileMode tmx, SkTileMode tmy,
                                const SkSamplingOptions&, const SkImage_Base*,
                                const ContextRec&, SkArenaAlloc* alloc);

    using INHERITED = SkShaderBase;
};

#endif
