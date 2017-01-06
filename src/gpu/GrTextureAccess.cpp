/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureAccess.h"
#include "GrColor.h"
#include "GrTexture.h"
#include "GrTexturePriv.h"

GrTextureAccess::GrTextureAccess() {}

GrTextureAccess::GrTextureAccess(GrTexture* texture, const GrTextureParams& params) {
    this->reset(texture, params);
}

GrTextureAccess::GrTextureAccess(GrTexture* texture,
                                 GrTextureParams::FilterMode filterMode,
                                 SkShader::TileMode tileXAndY,
                                 GrShaderFlags visibility) {
    this->reset(texture, filterMode, tileXAndY, visibility);
}

void GrTextureAccess::reset(GrTexture* texture,
                            const GrTextureParams& params,
                            GrShaderFlags visibility) {
    SkASSERT(texture);
    fTexture.set(SkRef(texture), kRead_GrIOType);
    fParams = params;
    fParams.setFilterMode(SkTMin(params.filterMode(), texture->texturePriv().highestFilterMode()));
    fVisibility = visibility;
}

void GrTextureAccess::reset(GrTexture* texture,
                            GrTextureParams::FilterMode filterMode,
                            SkShader::TileMode tileXAndY,
                            GrShaderFlags visibility) {
    SkASSERT(texture);
    fTexture.set(SkRef(texture), kRead_GrIOType);
    filterMode = SkTMin(filterMode, texture->texturePriv().highestFilterMode());
    fParams.reset(tileXAndY, filterMode);
    fVisibility = visibility;
}
