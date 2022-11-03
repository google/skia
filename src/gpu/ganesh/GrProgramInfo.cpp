/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrProgramInfo.h"

#include "src/gpu/ganesh/GrStencilSettings.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"

GrStencilSettings GrProgramInfo::nonGLStencilSettings() const {
    GrStencilSettings stencil;

    if (this->isStencilEnabled()) {
        stencil.reset(*fUserStencilSettings, this->pipeline().hasStencilClip(), 8);
    }

    return stencil;
}

#ifdef SK_DEBUG
#include "src/gpu/ganesh/GrTexture.h"

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
        if (te.samplerState().mipmapped() == GrMipmapped::kYes) {
            // Ensure mipmaps were all resolved ahead of time by the DAG.
            tex->assertMipmapsNotDirty(te);
        }
    });
}

#endif
