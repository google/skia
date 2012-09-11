/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureAccess_DEFINED
#define GrTextureAccess_DEFINED

#include "GrNoncopyable.h"
#include "SkRefCnt.h"

class GrTexture;

/** A class representing the swizzle access pattern for a texture. Note that if the texture is
 *  an alpha-only texture then the alpha channel is substituted for other components. Any mangling
 *  to handle the r,g,b->a conversions for alpha textures is automatically included in the stage
 *  key. However, if a GrCustomStage uses different swizzles based on its input then it must
 *  consider that variation in its key-generation.
 */
class GrTextureAccess : GrNoncopyable {
public:
    /**
     * A default GrTextureAccess must have reset() called on it in a GrCustomStage subclass's
     * constructor if it will be accessible via GrCustomStage::textureAccess().
     */
    GrTextureAccess();

    /**
     * swizzle must be a string between one and four (inclusive) characters containing only 'r',
     * 'g', 'b',  and/or 'a'.
     */
    GrTextureAccess(GrTexture*, const char* swizzle);

    /**
     * Uses the default swizzle, "rgba".
     */
    GrTextureAccess(GrTexture*);

    void reset(GrTexture*, const char* swizzle);
    void reset(GrTexture*);

    GrTexture* getTexture() const { return fTexture.get(); }

    /**
     * Returns a string representing the swizzle. The string is is null-terminated.
     */
    const char* getSwizzle() const { return fSwizzle; }

    enum {
        kR_SwizzleFlag = 0x1,
        kG_SwizzleFlag = 0x2,
        kB_SwizzleFlag = 0x4,
        kA_SwizzleFlag = 0x8,

        kRGB_SwizzleMask = (kR_SwizzleFlag |  kG_SwizzleFlag | kB_SwizzleFlag),
    };

    /** Returns a mask indicating which components are referenced in the swizzle. */
    uint32_t swizzleMask() const { return fSwizzleMask; }

private:
    SkAutoTUnref<GrTexture> fTexture;
    uint32_t                fSwizzleMask;
    char                    fSwizzle[5];
};

#endif
