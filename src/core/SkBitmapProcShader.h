/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBitmapProcShader_DEFINED
#define SkBitmapProcShader_DEFINED

#include "SkImagePriv.h"
#include "SkShader.h"

class SkBitmapProvider;

class SkBitmapProcLegacyShader : public SkShader {
private:
    friend class SkImageShader;

    static size_t ContextSize(const ContextRec&, const SkImageInfo& srcInfo);
    static Context* MakeContext(const SkShader&, TileMode tmx, TileMode tmy,
                                const SkBitmapProvider&, const ContextRec&, SkArenaAlloc* alloc);

    typedef SkShader INHERITED;
};

#endif
