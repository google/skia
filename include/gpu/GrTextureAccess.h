/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureAccess_DEFINED
#define GrTextureAccess_DEFINED

#include "GrGpuResourceRef.h"
#include "GrTexture.h"
#include "GrTextureParams.h"
#include "SkRefCnt.h"
#include "SkShader.h"

/** A class representing the swizzle access pattern for a texture. Note that if the texture is
 *  an alpha-only texture then the alpha channel is substituted for other components. Any mangling
 *  to handle the r,g,b->a conversions for alpha textures is automatically included in the stage
 *  key. However, if a GrProcessor uses different swizzles based on its input then it must
 *  consider that variation in its key-generation.
 */
class GrTextureAccess : public SkNoncopyable {
public:
    /**
     * A default GrTextureAccess must have reset() called on it in a GrProcessor subclass's
     * constructor if it will be accessible via GrProcessor::textureAccess().
     */
    GrTextureAccess();

    /**
     * Uses the default swizzle, "rgba".
     */
    GrTextureAccess(GrTexture*, const GrTextureParams&);
    explicit GrTextureAccess(GrTexture*,
                             GrTextureParams::FilterMode = GrTextureParams::kNone_FilterMode,
                             SkShader::TileMode tileXAndY = SkShader::kClamp_TileMode);

    /**
     * swizzle must be a string between one and four (inclusive) characters containing only 'r',
     * 'g', 'b',  and/or 'a'.
     */
    GrTextureAccess(GrTexture*, const char* swizzle, const GrTextureParams&);
    GrTextureAccess(GrTexture*,
                    const char* swizzle,
                    GrTextureParams::FilterMode = GrTextureParams::kNone_FilterMode,
                    SkShader::TileMode tileXAndY = SkShader::kClamp_TileMode);

    void reset(GrTexture*, const GrTextureParams&);
    void reset(GrTexture*,
               GrTextureParams::FilterMode = GrTextureParams::kNone_FilterMode,
               SkShader::TileMode tileXAndY = SkShader::kClamp_TileMode);
    void reset(GrTexture*, const char* swizzle, const GrTextureParams&);
    void reset(GrTexture*,
               const char* swizzle,
               GrTextureParams::FilterMode = GrTextureParams::kNone_FilterMode,
               SkShader::TileMode tileXAndY = SkShader::kClamp_TileMode);

    bool operator== (const GrTextureAccess& other) const {
#ifdef SK_DEBUG
        // below assumes all chars in fSwizzle are initialized even if string is < 4 chars long.
        SkASSERT(memcmp(fSwizzle, other.fSwizzle, sizeof(fSwizzle)-1) ==
                 strcmp(fSwizzle, other.fSwizzle));
#endif
        return fParams == other.fParams &&
               (this->getTexture() == other.getTexture()) &&
               (0 == memcmp(fSwizzle, other.fSwizzle, sizeof(fSwizzle)-1));
    }

    bool operator!= (const GrTextureAccess& other) const { return !(*this == other); }

    GrTexture* getTexture() const { return fTexture.get(); }

    /**
     * For internal use by GrProcessor.
     */
    const GrGpuResourceRef* getProgramTexture() const { return &fTexture; }

    /**
     * Returns a string representing the swizzle. The string is is null-terminated.
     */
    const char* getSwizzle() const { return fSwizzle; }

    /** Returns a mask indicating which components are referenced in the swizzle. The return
        is a bitfield of GrColorComponentFlags. */
    uint32_t swizzleMask() const { return fSwizzleMask; }

    const GrTextureParams& getParams() const { return fParams; }

private:
    void setSwizzle(const char*);

    typedef GrTGpuResourceRef<GrTexture> ProgramTexture;

    ProgramTexture                  fTexture;
    GrTextureParams                 fParams;
    uint32_t                        fSwizzleMask;
    char                            fSwizzle[5];

    typedef SkNoncopyable INHERITED;
};

#endif
