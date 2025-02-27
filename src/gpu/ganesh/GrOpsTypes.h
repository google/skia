/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOpsTypes_DEFINED
#define GrOpsTypes_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "src/core/SkColorData.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"

enum SkAlphaType : int;
enum class GrQuadAAFlags;
struct SkPoint;

/** Used by SDC::drawQuadSet and FillRectOp */
struct GrQuadSetEntry {
    SkRect fRect;
    SkPMColor4f fColor; // Overrides any color on the GrPaint
    SkMatrix fLocalMatrix;
    GrQuadAAFlags fAAFlags;
};

/** Used by SDC::drawTextureSet and TextureOp */
struct GrTextureSetEntry {
    GrSurfaceProxyView fProxyView;
    SkAlphaType fSrcAlphaType;
    SkRect fSrcRect;
    SkRect fDstRect;
    const SkPoint* fDstClipQuad; // Must be null, or point to an array of 4 points
    const SkMatrix* fPreViewMatrix; // If not null, entry's CTM is 'viewMatrix' * fPreViewMatrix
    SkPMColor4f   fColor; // {a,a,a,a} for rgb textures, {r,g,b,a} for alpha-only textures
    GrQuadAAFlags fAAFlags;
};

#endif
