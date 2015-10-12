/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureParams_DEFINED
#define GrTextureParams_DEFINED

#include "GrTypes.h"
#include "SkShader.h"

/**
 * Represents the filtering and tile modes used to access a texture.
 */
class GrTextureParams {
public:
    static const GrTextureParams& ClampNoFilter() {
        static const GrTextureParams gParams;
        return gParams;
    }
    static const GrTextureParams& ClampBilerp() {
        static const GrTextureParams gParams(SkShader::kClamp_TileMode, kBilerp_FilterMode);
        return gParams;
    }

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
#endif
