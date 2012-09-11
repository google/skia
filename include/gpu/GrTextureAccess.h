/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureAccess_DEFINED
#define GrTextureAccess_DEFINED

#include "GrTypes.h"

class GrTexture;
class SkString;

/** A class representing the swizzle access pattern for a texture.
 */
class GrTextureAccess {
public:
    typedef char Swizzle[4];

    GrTextureAccess(const GrTexture* texture, const SkString& swizzle);

    const GrTexture* getTexture() const { return fTexture; }
    const Swizzle& getSwizzle() const { return fSwizzle; }

    bool referencesAlpha() const {
        return fSwizzle[0] == 'a' || fSwizzle[1] == 'a' || fSwizzle[2] == 'a' || fSwizzle[3] == 'a';
    }

private:
    const GrTexture* fTexture;
    Swizzle fSwizzle;
};

#endif
