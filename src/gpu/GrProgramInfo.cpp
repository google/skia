/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrProgramInfo.h"

#include "src/gpu/GrStencilSettings.h"

GrStencilSettings GrProgramInfo::nonGLStencilSettings() const {
    GrStencilSettings stencil;

    if (this->isStencilEnabled()) {
        stencil.reset(*fUserStencilSettings, this->pipeline().hasStencilClip(), 8);
    }

    return stencil;
}

#ifdef SK_DEBUG
#include "src/gpu/GrTexture.h"

void GrProgramInfo::validate(bool flushTime) const {
    if (flushTime) {
        SkASSERT(fPipeline->allProxiesInstantiated());
    }
}

void GrProgramInfo::checkAllInstantiated() const {
    this->pipeline().visitProxies([](GrSurfaceProxy* proxy, GrMipmapped) {
        SkASSERT(proxy->isInstantiated());
        return true;
    });
}

void GrProgramInfo::checkMSAAAndMIPSAreResolved() const {
    this->pipeline().visitTextureEffects([](const GrTextureEffect& te) {
        GrTexture* tex = te.texture();
        SkASSERT(tex);

        // Ensure mipmaps were all resolved ahead of time by the DAG.
        if (te.samplerState().mipmapped() == GrMipmapped::kYes &&
            (tex->width() != 1 || tex->height() != 1)) {
            // There are some cases where we might be given a non-mipmapped texture with a
            // mipmap filter. See skbug.com/7094.
            SkASSERT(tex->mipmapped() != GrMipmapped::kYes || !tex->mipmapsAreDirty());
        }
    });
}

#endif
