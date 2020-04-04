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

class GrTextureOp {
public:

    /**
     * Controls whether saturate() is called after the texture is color-converted to ensure all
     * color values are in 0..1 range.
     */
    enum class Saturate : bool { kNo = false, kYes = true };

    /**
     * Creates an op that draws a sub-quadrilateral of a texture. The passed color is modulated by
     * the texture's color. 'deviceQuad' specifies the device-space coordinates to draw, using
     * 'localQuad' to map into the proxy's texture space. If non-null, 'domain' represents the
     * boundary for the strict src rect constraint. If GrAAType is kCoverage then AA is applied to
     * the edges indicated by GrQuadAAFlags. Otherwise, GrQuadAAFlags is ignored.
     *
     * This is functionally very similar to GrFillRectOp::Make, except that the GrPaint has been
     * deconstructed into the texture, filter, modulating color, and blend mode. When blend mode is
     * src over, this will return a GrFillRectOp with a paint that samples the proxy.
     */
    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext*,
                                          GrSurfaceProxyView,
                                          SkAlphaType srcAlphaType,
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

    // Automatically falls back to using one GrFillRectOp per entry if dynamic states are not
    // supported, or if the blend mode is not src-over. 'cnt' is the size of the entry array.
    // 'proxyCnt' <= 'cnt' and represents the number of proxy switches within the array.
    static void AddTextureSetOps(GrRenderTargetContext*,
                                 const GrClip& clip,
                                 GrRecordingContext*,
                                 GrRenderTargetContext::TextureSetEntry[],
                                 int cnt,
                                 int proxyRunCnt,
                                 GrSamplerState::Filter,
                                 Saturate,
                                 SkBlendMode,
                                 GrAAType,
                                 SkCanvas::SrcRectConstraint,
                                 const SkMatrix& viewMatrix,
                                 sk_sp<GrColorSpaceXform> textureXform);

#if GR_TEST_UTILS
    static uint32_t ClassID();
#endif

private:
    class BatchSizeLimiter;
};

#endif  // GrTextureOp_DEFINED
