/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SoftwarePathRenderer_DEFINED
#define SoftwarePathRenderer_DEFINED

#include "src/gpu/GrSurfaceProxyView.h"
#include "src/gpu/v1/PathRenderer.h"

class GrProxyProvider;

namespace skgpu::v1 {

/**
 * This class uses the software side to render a path to an SkBitmap and
 * then uploads the result to the gpu
 */
class SoftwarePathRenderer final : public PathRenderer {
public:
    const char* name() const override { return "SW"; }

    SoftwarePathRenderer(GrProxyProvider* proxyProvider, bool allowCaching)
            : fProxyProvider(proxyProvider)
            , fAllowCaching(allowCaching) {
    }

    static bool GetShapeAndClipBounds(SurfaceDrawContext*,
                                      const GrClip*,
                                      const GrStyledShape&,
                                      const SkMatrix& viewMatrix,
                                      SkIRect* unclippedDevShapeBounds,
                                      SkIRect* clippedDevShapeBounds,
                                      SkIRect* devClipBounds);

private:
    static void DrawNonAARect(SurfaceDrawContext*,
                              GrPaint&&,
                              const GrUserStencilSettings&,
                              const GrClip*,
                              const SkMatrix& viewMatrix,
                              const SkRect& rect,
                              const SkMatrix& localMatrix);
    static void DrawAroundInvPath(SurfaceDrawContext*,
                                  GrPaint&&,
                                  const GrUserStencilSettings&,
                                  const GrClip*,
                                  const SkMatrix& viewMatrix,
                                  const SkIRect& devClipBounds,
                                  const SkIRect& devPathBounds);

    // This utility draws a path mask using a provided paint. The rectangle is drawn in device
    // space. The 'viewMatrix' will be used to ensure the correct local coords are provided to
    // any fragment processors in the paint.
    static void DrawToTargetWithShapeMask(GrSurfaceProxyView,
                                          SurfaceDrawContext*,
                                          GrPaint&&,
                                          const GrUserStencilSettings&,
                                          const GrClip*,
                                          const SkMatrix& viewMatrix,
                                          const SkIPoint& textureOriginInDeviceSpace,
                                          const SkIRect& deviceSpaceRectToDraw);

    StencilSupport onGetStencilSupport(const GrStyledShape&) const override {
        return PathRenderer::kNoSupport_StencilSupport;
    }

    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const override;

    bool onDrawPath(const DrawPathArgs&) override;

private:
    GrProxyProvider* fProxyProvider;
    bool             fAllowCaching;
};

} // namespace skgpu::v1

#endif // SoftwarePathRenderer_DEFINED
