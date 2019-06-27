/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/GrSamplerState.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrRenderTargetContext.h"

class GrColorSpaceXform;
class GrDrawOp;
class GrTextureProxy;
struct SkRect;
class SkMatrix;

namespace GrTextureOp {

/**
 * Creates an op that draws a sub-quadrilateral of a texture. The passed color is modulated by the
 * texture's color. 'deviceQuad' specifies the device-space coordinates to draw, using 'localQuad'
 * to map into the proxy's texture space. If non-null, 'domain' represents the boundary for the
 * strict src rect constraint. If GrAAType is kCoverage then AA is applied to the edges
 * indicated by GrQuadAAFlags. Otherwise, GrQuadAAFlags is ignored.
 *
 * This is functionally very similar to GrFillRectOp::Make, except that the GrPaint has been
 * deconstructed into the texture, filter, modulating color, and blend mode. When blend mode is
 * src over, this will return a GrFillRectOp with a paint that samples the proxy.
 */
std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                               sk_sp<GrTextureProxy> proxy,
                               sk_sp<GrColorSpaceXform> textureXform,
                               GrSamplerState::Filter filter,
                               const SkPMColor4f& color,
                               SkBlendMode blendMode,
                               GrAAType aaType,
                               GrQuadAAFlags aaFlags,
                               const GrQuad& deviceQuad,
                               const GrQuad& localQuad,
                               const SkRect* domain = nullptr);

// Unlike the single-proxy factory, this only supports src-over blending.
std::unique_ptr<GrDrawOp> MakeSet(GrRecordingContext*,
                                  const GrRenderTargetContext::TextureSetEntry[],
                                  int cnt,
                                  GrSamplerState::Filter,
                                  GrAAType,
                                  SkCanvas::SrcRectConstraint,
                                  const SkMatrix& viewMatrix,
                                  sk_sp<GrColorSpaceXform> textureXform);

}
