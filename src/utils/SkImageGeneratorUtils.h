/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageGeneratorUtils_DEFINED
#define SkImageGeneratorUtils_DEFINED

#include "SkImageGenerator.h"

class SkImage;

class SkImageGeneratorUtils {
public:
    // Returns a generator of the specified dimensions, but will always fail to return anything
    static SkImageGenerator* NewEmpty(const SkImageInfo&);

    // If the bitmap is mutable, it will make a copy first
    static SkImageGenerator* NewFromBitmap(const SkBitmap&);

    // Ref's the provided texture, so it had better be const!
    static SkImageGenerator* NewFromTexture(GrContext*, GrTexture*);

    static SkImageGenerator* NewFromImage(const SkImage*);
};

#endif
