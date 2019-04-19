/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBitmapProcShader_DEFINED
#define SkBitmapProcShader_DEFINED

#include "SkImagePriv.h"
#include "SkShaderBase.h"

class SkBitmapProvider;

class SkBitmapProcLegacyShader : public SkShaderBase {
private:
    friend class SkImageShader;

    static Context* MakeContext(const SkShaderBase&, SkTileMode tmx, SkTileMode tmy,
                                const SkBitmapProvider&, const ContextRec&, SkArenaAlloc* alloc);

    typedef SkShaderBase INHERITED;
};

#endif
