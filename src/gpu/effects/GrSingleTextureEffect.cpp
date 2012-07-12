/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrSingleTextureEffect.h"
#include "GrTexture.h"

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture)
    : fTexture (texture) {
    SkSafeRef(fTexture);
}

GrSingleTextureEffect::~GrSingleTextureEffect() {
    SkSafeUnref(fTexture);
}

unsigned int GrSingleTextureEffect::numTextures() const {
    return 1;
}

GrTexture* GrSingleTextureEffect::texture(unsigned int index) const {
    GrAssert(0 == index);
    return fTexture;
}


