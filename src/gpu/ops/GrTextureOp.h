/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrTextureOp_DEFINED
#define GrTextureOp_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkRefCnt.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrSamplerState.h"

class GrColorSpaceXform;
class GrDrawOp;
class GrTextureProxy;
struct SkRect;
class SkMatrix;

namespace GrTextureOp {

/**
 * Controls whether saturate() is called after the texture is color-converted to ensure all
 * color values are in 0..1 range.
 */
enum class Saturate : bool { kNo = false, kYes = true };

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
std::unique_ptr<GrDrawOp> Make(GrRecordingContext*,
                               sk_sp<GrTextureProxy>,
                               GrColorType srcColorType,
                               sk_sp<GrColorSpaceXform>,
                               GrSamplerState::Filter,
                               const SkPMColor4f&,
                               Saturate,
                               SkBlendMode,
                               GrAAType,
                               GrQuadAAFlags,
                               const GrQuad& deviceQuad,
                               const GrQuad& localQuad,
                               const SkRect* domain = nullptr);

// Unlike the single-proxy factory, this only supports src-over blending.
std::unique_ptr<GrDrawOp> MakeSet(GrRecordingContext*,
                                  const GrRenderTargetContext::TextureSetEntry[],
                                  int cnt,
                                  GrSamplerState::Filter,
                                  Saturate,
                                  GrAAType,
                                  SkCanvas::SrcRectConstraint,
                                  const SkMatrix& viewMatrix,
                                  sk_sp<GrColorSpaceXform> textureXform);

}
#endif  // GrTextureOp_DEFINED
