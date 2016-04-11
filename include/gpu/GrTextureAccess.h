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

/** 
 * Used to represent a texture that is required by a GrProcessor. It holds a GrTexture along with
 * an associated GrTextureParams
 */
class GrTextureAccess : public SkNoncopyable {
public:
    /**
     * Must be initialized before adding to a GrProcessor's texture access list.
     */
    GrTextureAccess();

    GrTextureAccess(GrTexture*, const GrTextureParams&);

    explicit GrTextureAccess(GrTexture*,
                             GrTextureParams::FilterMode = GrTextureParams::kNone_FilterMode,
                             SkShader::TileMode tileXAndY = SkShader::kClamp_TileMode,
                             GrShaderFlags visibility = kFragment_GrShaderFlag);

    void reset(GrTexture*, const GrTextureParams&,
               GrShaderFlags visibility = kFragment_GrShaderFlag);
    void reset(GrTexture*,
               GrTextureParams::FilterMode = GrTextureParams::kNone_FilterMode,
               SkShader::TileMode tileXAndY = SkShader::kClamp_TileMode,
               GrShaderFlags visibility = kFragment_GrShaderFlag);

    bool operator==(const GrTextureAccess& that) const {
        return this->getTexture() == that.getTexture() &&
               fParams == that.fParams &&
               fVisibility == that.fVisibility;
    }

    bool operator!=(const GrTextureAccess& other) const { return !(*this == other); }

    GrTexture* getTexture() const { return fTexture.get(); }
    GrShaderFlags getVisibility() const { return fVisibility; }

    /**
     * For internal use by GrProcessor.
     */
    const GrGpuResourceRef* getProgramTexture() const { return &fTexture; }

    const GrTextureParams& getParams() const { return fParams; }

private:

    typedef GrTGpuResourceRef<GrTexture> ProgramTexture;

    ProgramTexture                  fTexture;
    GrTextureParams                 fParams;
    GrShaderFlags                   fVisibility;

    typedef SkNoncopyable INHERITED;
};

#endif
