/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSoftwarePathRenderer_DEFINED
#define GrSoftwarePathRenderer_DEFINED

#include "src/gpu/GrPathRenderer.h"
#include "src/gpu/GrSurfaceProxyView.h"

class GrProxyProvider;

/**
 * This class uses the software side to render a path to an SkBitmap and
 * then uploads the result to the gpu
 */
class GrSoftwarePathRenderer : public GrPathRenderer {
public:
    const char* name() const final { return "SW"; }

    GrSoftwarePathRenderer(GrProxyProvider* proxyProvider, bool allowCaching)
            : fProxyProvider(proxyProvider)
            , fAllowCaching(allowCaching) {
    }

    static bool GetShapeAndClipBounds(GrSurfaceDrawContext*,
                                      const GrClip* clip,
                                      const GrStyledShape& shape,
                                      const SkMatrix& matrix,
                                      SkIRect* unclippedDevShapeBounds,
                                      SkIRect* clippedDevShapeBounds,
                                      SkIRect* devClipBounds);

private:
    static void DrawNonAARect(GrSurfaceDrawContext* surfaceDrawContext,
                              GrPaint&& paint,
                              const GrUserStencilSettings& userStencilSettings,
                              const GrClip* clip,
                              const SkMatrix& viewMatrix,
                              const SkRect& rect,
                              const SkMatrix& localMatrix);
    static void DrawAroundInvPath(GrSurfaceDrawContext* surfaceDrawContext,
                                  GrPaint&& paint,
                                  const GrUserStencilSettings& userStencilSettings,
                                  const GrClip* clip,
                                  const SkMatrix& viewMatrix,
                                  const SkIRect& devClipBounds,
                                  const SkIRect& devPathBounds);

    // This utility draws a path mask using a provided paint. The rectangle is drawn in device
    // space. The 'viewMatrix' will be used to ensure the correct local coords are provided to
    // any fragment processors in the paint.
    static void DrawToTargetWithShapeMask(GrSurfaceProxyView,
                                          GrSurfaceDrawContext* surfaceDrawContext,
                                          GrPaint&& paint,
                                          const GrUserStencilSettings& userStencilSettings,
                                          const GrClip* clip,
                                          const SkMatrix& viewMatrix,
                                          const SkIPoint& textureOriginInDeviceSpace,
                                          const SkIRect& deviceSpaceRectToDraw);

    StencilSupport onGetStencilSupport(const GrStyledShape&) const override {
        return GrPathRenderer::kNoSupport_StencilSupport;
    }

    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const override;

    bool onDrawPath(const DrawPathArgs&) override;

private:
    GrProxyProvider*       fProxyProvider;
    bool                   fAllowCaching;

    using INHERITED = GrPathRenderer;
};

#endif
