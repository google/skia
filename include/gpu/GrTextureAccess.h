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
#include "SkRefCnt.h"
#include "SkShader.h"

/**
 * Represents the filtering and tile modes used to access a texture. It is mostly used with
 * GrTextureAccess (defined below). Also, some of the texture cache methods require knowledge about
 * filtering and tiling to perform a cache lookup. If it wasn't for this latter usage this would
 * be folded into GrTextureAccess. The default is clamp tile modes and no filtering.
 */
class GrTextureParams {
public:
    GrTextureParams() {
        this->reset();
    }

    enum FilterMode {
        kNone_FilterMode,
        kBilerp_FilterMode,
        kMipMap_FilterMode
    };

    GrTextureParams(SkShader::TileMode tileXAndY, FilterMode filterMode) {
        this->reset(tileXAndY, filterMode);
    }

    GrTextureParams(const SkShader::TileMode tileModes[2], FilterMode filterMode) {
        this->reset(tileModes, filterMode);
    }

    GrTextureParams(const GrTextureParams& params) {
        *this = params;
    }

    GrTextureParams& operator= (const GrTextureParams& params) {
        fTileModes[0] = params.fTileModes[0];
        fTileModes[1] = params.fTileModes[1];
        fFilterMode = params.fFilterMode;
        return *this;
    }

    void reset() {
        this->reset(SkShader::kClamp_TileMode, kNone_FilterMode);
    }

    void reset(SkShader::TileMode tileXAndY, FilterMode filterMode) {
        fTileModes[0] = fTileModes[1] = tileXAndY;
        fFilterMode = filterMode;
    }

    void reset(const SkShader::TileMode tileModes[2], FilterMode filterMode) {
        fTileModes[0] = tileModes[0];
        fTileModes[1] = tileModes[1];
        fFilterMode = filterMode;
    }

    void setClampNoFilter() {
        fTileModes[0] = fTileModes[1] = SkShader::kClamp_TileMode;
        fFilterMode = kNone_FilterMode;
    }

    void setClamp() {
        fTileModes[0] = fTileModes[1] = SkShader::kClamp_TileMode;
    }

    void setFilterMode(FilterMode filterMode) { fFilterMode = filterMode; }

    void setTileModeX(const SkShader::TileMode tm) { fTileModes[0] = tm; }
    void setTileModeY(const SkShader::TileMode tm) { fTileModes[1] = tm; }
    void setTileModeXAndY(const SkShader::TileMode tm) { fTileModes[0] = fTileModes[1] = tm; }

    SkShader::TileMode getTileModeX() const { return fTileModes[0]; }

    SkShader::TileMode getTileModeY() const { return fTileModes[1]; }

    bool isTiled() const {
        return SkShader::kClamp_TileMode != fTileModes[0] ||
               SkShader::kClamp_TileMode != fTileModes[1];
    }

    FilterMode filterMode() const { return fFilterMode; }

    bool operator== (const GrTextureParams& other) const {
        return fTileModes[0] == other.fTileModes[0] &&
               fTileModes[1] == other.fTileModes[1] &&
               fFilterMode == other.fFilterMode;
    }

    bool operator!= (const GrTextureParams& other) const { return !(*this == other); }

private:

    SkShader::TileMode fTileModes[2];
    FilterMode         fFilterMode;
};

/** A class representing the swizzle access pattern for a texture. Note that if the texture is
 *  an alpha-only texture then the alpha channel is substituted for other components. Any mangling
 *  to handle the r,g,b->a conversions for alpha textures is automatically included in the stage
 *  key. However, if a GrProcessor uses different swizzles based on its input then it must
 *  consider that variation in its key-generation.
 */
class GrTextureAccess : public SkNoncopyable {
public:
    SK_DECLARE_INST_COUNT_ROOT(GrTextureAccess);

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
