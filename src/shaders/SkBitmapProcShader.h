/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBitmapProcShader_DEFINED
#define SkBitmapProcShader_DEFINED

#include "src/core/SkImagePriv.h"
#include "src/shaders/SkShaderBase.h"

class SkImage_Base;

class SkBitmapProcLegacyShader : public SkShaderBase {
private:
    friend class SkImageShader;

    static Context* MakeContext(const SkShaderBase&, SkTileMode tmx, SkTileMode tmy,
                                const SkImage_Base*, const ContextRec&, SkArenaAlloc* alloc);

    typedef SkShaderBase INHERITED;
};

#endif
