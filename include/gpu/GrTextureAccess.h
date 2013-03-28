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
#include "SkShader.h"

class GrTexture;

/**
 * Represents the filtering and tile modes used to access a texture. It is mostly used with
 * GrTextureAccess (defined below). Also, some of the texture cache methods require knowledge about
 * filtering and tiling to perform a cache lookup. If it wasn't for this latter usage this would
 * be folded into GrTextureAccess.
 */
class GrTextureParams {
public:
    GrTextureParams() {
        this->reset();
    }

    GrTextureParams(SkShader::TileMode tileXAndY, bool bilerp) {
        this->reset(tileXAndY, bilerp);
    }

    GrTextureParams(SkShader::TileMode tileModes[2], bool bilerp) {
        this->reset(tileModes, bilerp);
    }

    GrTextureParams(const GrTextureParams& params) {
        *this = params;
    }

    GrTextureParams& operator= (const GrTextureParams& params) {
        fTileModes[0] = params.fTileModes[0];
        fTileModes[1] = params.fTileModes[1];
        fBilerp = params.fBilerp;
        return *this;
    }

    void reset() {
        this->reset(SkShader::kClamp_TileMode, false);
    }

    void reset(SkShader::TileMode tileXAndY, bool bilerp) {
        fTileModes[0] = fTileModes[1] = tileXAndY;
        fBilerp = bilerp;
    }

    void reset(SkShader::TileMode tileModes[2], bool bilerp) {
        fTileModes[0] = tileModes[0];
        fTileModes[1] = tileModes[1];
        fBilerp = bilerp;
    }

    void setClampNoFilter() {
        fTileModes[0] = fTileModes[1] = SkShader::kClamp_TileMode;
        fBilerp = false;
    }

    void setClamp() {
        fTileModes[0] = fTileModes[1] = SkShader::kClamp_TileMode;
    }

    void setBilerp(bool bilerp) { fBilerp = bilerp; }

    void setTileModeX(const SkShader::TileMode tm) { fTileModes[0] = tm; }
    void setTileModeY(const SkShader::TileMode tm) { fTileModes[1] = tm; }
    void setTileModeXAndY(const SkShader::TileMode tm) { fTileModes[0] = fTileModes[1] = tm; }

    SkShader::TileMode getTileModeX() const { return fTileModes[0]; }

    SkShader::TileMode getTileModeY() const { return fTileModes[1]; }

    bool isTiled() const {
        return SkShader::kClamp_TileMode != fTileModes[0] ||
               SkShader::kClamp_TileMode != fTileModes[1];
    }

    bool isBilerp() const { return fBilerp; }

    bool operator== (const GrTextureParams& other) const {
        return fTileModes[0] == other.fTileModes[0] &&
               fTileModes[1] == other.fTileModes[1] &&
               fBilerp == other.fBilerp;
    }

    bool operator!= (const GrTextureParams& other) const { return !(*this == other); }

private:

    SkShader::TileMode fTileModes[2];
    bool               fBilerp;
};

/** A class representing the swizzle access pattern for a texture. Note that if the texture is
 *  an alpha-only texture then the alpha channel is substituted for other components. Any mangling
 *  to handle the r,g,b->a conversions for alpha textures is automatically included in the stage
 *  key. However, if a GrEffect uses different swizzles based on its input then it must
 *  consider that variation in its key-generation.
 */
class GrTextureAccess : GrNoncopyable {
public:
    /**
     * A default GrTextureAccess must have reset() called on it in a GrEffect subclass's
     * constructor if it will be accessible via GrEffect::textureAccess().
     */
    GrTextureAccess();

    /**
     * Uses the default swizzle, "rgba".
     */
    GrTextureAccess(GrTexture*, const GrTextureParams&);
    explicit GrTextureAccess(GrTexture*,
                             bool bilerp = false,
                             SkShader::TileMode tileXAndY = SkShader::kClamp_TileMode);

    /**
     * swizzle must be a string between one and four (inclusive) characters containing only 'r',
     * 'g', 'b',  and/or 'a'.
     */
    GrTextureAccess(GrTexture*, const char* swizzle, const GrTextureParams&);
    GrTextureAccess(GrTexture*,
                    const char* swizzle,
                    bool bilerp = false,
                    SkShader::TileMode tileXAndY = SkShader::kClamp_TileMode);

    void reset(GrTexture*, const GrTextureParams&);
    void reset(GrTexture*,
               bool bilerp = false,
               SkShader::TileMode tileXAndY = SkShader::kClamp_TileMode);
    void reset(GrTexture*, const char* swizzle, const GrTextureParams&);
    void reset(GrTexture*,
               const char* swizzle,
               bool bilerp = false,
               SkShader::TileMode tileXAndY = SkShader::kClamp_TileMode);

    bool operator== (const GrTextureAccess& other) const {
#if GR_DEBUG
        // below assumes all chars in fSwizzle are initialized even if string is < 4 chars long.
        GrAssert(memcmp(fSwizzle, other.fSwizzle, sizeof(fSwizzle)-1) ==
                 strcmp(fSwizzle, other.fSwizzle));
#endif
        return fParams == other.fParams &&
               (fTexture.get() == other.fTexture.get()) &&
               (0 == memcmp(fSwizzle, other.fSwizzle, sizeof(fSwizzle)-1));
    }

    bool operator!= (const GrTextureAccess& other) const { return !(*this == other); }

    GrTexture* getTexture() const { return fTexture.get(); }

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

    GrTextureParams         fParams;
    SkAutoTUnref<GrTexture> fTexture;
    uint32_t                fSwizzleMask;
    char                    fSwizzle[5];

    typedef GrNoncopyable INHERITED;
};

#endif
