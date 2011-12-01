
/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkGLCanvas_DEFINED
#define SkGLCanvas_DEFINED

#include "SkCanvas.h"

// Deprecated.  You should now use SkGLDevice and SkGLDeviceFactory with
// SkCanvas.
class SkGLCanvas : public SkCanvas {
public:
    SkGLCanvas();

    static size_t GetTextureCacheMaxCount();
    static void SetTextureCacheMaxCount(size_t count);

    static size_t GetTextureCacheMaxSize();
    static void SetTextureCacheMaxSize(size_t size);

    static void DeleteAllTextures();

    static void AbandonAllTextures();
};

#endif
