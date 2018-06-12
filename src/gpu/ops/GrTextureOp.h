/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrColor.h"
#include "GrSamplerState.h"
#include "GrTypesPriv.h"
#include "SkCanvas.h"
#include "SkRefCnt.h"

class GrColorSpaceXform;
class GrDrawOp;
class GrTextureProxy;
struct SkRect;
class SkMatrix;

namespace GrTextureOp {

/**
 * Creates an op that draws a sub-rectangle of a texture. The passed color is modulated by the
 * texture's color. 'srcRect' specifies the rectangle of the texture to draw. 'dstRect' specifies
 * the rectangle to draw in local coords which will be transformed by 'viewMatrix' to be in device
 * space. 'viewMatrix' must be affine.
 */
std::unique_ptr<GrDrawOp> Make(GrContext*,
                               sk_sp<GrTextureProxy>,
                               GrSamplerState::Filter,
                               GrColor,
                               const SkRect& srcRect,
                               const SkRect& dstRect,
                               GrAAType,
                               SkCanvas::SrcRectConstraint,
                               const SkMatrix& viewMatrix,
                               sk_sp<GrColorSpaceXform>);
}
