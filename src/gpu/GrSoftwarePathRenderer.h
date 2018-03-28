/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSoftwarePathRenderer_DEFINED
#define GrSoftwarePathRenderer_DEFINED

#include "GrPathRenderer.h"

class GrProxyProvider;

/**
 * This class uses the software side to render a path to an SkBitmap and
 * then uploads the result to the gpu
 */
class GrSoftwarePathRenderer : public GrPathRenderer {
public:
    GrSoftwarePathRenderer(GrProxyProvider* proxyProvider, bool allowCaching)
            : fProxyProvider(proxyProvider), fAllowCaching(allowCaching) {
        SkASSERT(fProxyProvider);
    }

    bool willDrawCached(StencilSupport minStencilSupport, const CanDrawPathArgs& args,
                        StencilSupport* outStencilSupport) {
        if (!this->canDrawPath(minStencilSupport, args, outStencilSupport)) {
            return false;
        }
        bool inverseFilled;
        SkIRect unclippedDevShapeBounds, boundsForMask;
        return this->shouldUseCache(*args.fCaps, *args.fClipConservativeBounds, *args.fViewMatrix,
                                    *args.fShape, args.fAAType, &inverseFilled,
                                    &unclippedDevShapeBounds, &boundsForMask);
    }

private:
    static void DrawNonAARect(GrRenderTargetContext* renderTargetContext,
                              GrPaint&& paint,
                              const GrUserStencilSettings& userStencilSettings,
                              const GrClip& clip,
                              const SkMatrix& viewMatrix,
                              const SkRect& rect,
                              const SkMatrix& localMatrix);
    static void DrawAroundInvPath(GrRenderTargetContext* renderTargetContext,
                                  GrPaint&& paint,
                                  const GrUserStencilSettings& userStencilSettings,
                                  const GrClip& clip,
                                  const SkMatrix& viewMatrix,
                                  const SkIRect& devClipBounds,
                                  const SkIRect& devPathBounds);

    // This utility draws a path mask using a provided paint. The rectangle is drawn in device
    // space. The 'viewMatrix' will be used to ensure the correct local coords are provided to
    // any fragment processors in the paint.
    static void DrawToTargetWithShapeMask(sk_sp<GrTextureProxy> proxy,
                                          GrRenderTargetContext* renderTargetContext,
                                          GrPaint&& paint,
                                          const GrUserStencilSettings& userStencilSettings,
                                          const GrClip& clip,
                                          const SkMatrix& viewMatrix,
                                          const SkIPoint& textureOriginInDeviceSpace,
                                          const SkIRect& deviceSpaceRectToDraw);

    bool shouldUseCache(const GrCaps&, const SkIRect& clipConservativeBounds, const SkMatrix&,
                        const GrShape&, GrAAType, bool* inverseFilled,
                        SkIRect* unclippedDevShapeBounds, SkIRect* boundsForMask) const;

    StencilSupport onGetStencilSupport(const GrShape&) const override {
        return GrPathRenderer::kNoSupport_StencilSupport;
    }

    bool onCanDrawPath(const CanDrawPathArgs&) const override;

    bool onDrawPath(const DrawPathArgs&) override;

private:
    GrProxyProvider*       fProxyProvider;
    bool                   fAllowCaching;

    typedef GrPathRenderer INHERITED;
};

#endif
