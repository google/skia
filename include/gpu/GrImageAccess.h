/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrImageAccess_DEFINED
#define GrImageAccess_DEFINED

#include "GrGpuResourceRef.h"
#include "GrTexture.h"
#include "GrTypesPriv.h"

/**
 * This is used by a GrProcessor to access a texture using image load/store in its shader code.
 * Image accesses currently always have "top left" semantics regardless of the texture's origin.
 */
class GrImageAccess : public SkNoncopyable {
public:
    GrImageAccess(sk_sp<GrTexture> textureImage, GrIOType ioType,
                  GrShaderFlags visibility = kFragment_GrShaderFlag) {
        fTexture.set(textureImage.release(), ioType);
        fVisibility = visibility;
    }

    bool operator==(const GrImageAccess& that) const {
        return this->texture() == that.texture() && fVisibility == that.fVisibility;
    }

    bool operator!=(const GrImageAccess& that) const { return !(*this == that); }

    GrTexture* texture() const { return fTexture.get(); }
    GrShaderFlags visibility() const { return fVisibility; }
    GrIOType ioType() const { return fTexture.ioType(); }

    /**
     * For internal use by GrProcessor.
     */
    const GrGpuResourceRef* getProgramBuffer() const { return &fTexture;}

private:
    GrTGpuResourceRef<GrTexture>  fTexture;
    GrShaderFlags                 fVisibility;

    typedef SkNoncopyable INHERITED;
};

#endif
