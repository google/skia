/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrClipStackClip_DEFINED
#define GrClipStackClip_DEFINED

#include "src/core/SkClipStack.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrReducedClip.h"

class GrPathRenderer;
class GrTextureProxy;

/**
 * GrClipStackClip can apply a generic SkClipStack to the draw state. It may need to generate an
 * 8-bit alpha clip mask and/or modify the stencil buffer during apply().
 */
class GrClipStackClip final : public GrClip {
public:
    GrClipStackClip(const SkISize& dimensions,
                    const SkClipStack* stack = nullptr,
                    const SkMatrixProvider* matrixProvider = nullptr)
            : fDeviceSize(dimensions)
            , fStack(stack)
            , fMatrixProvider(matrixProvider) {}

    SkIRect getConservativeBounds() const final;
    Effect apply(GrRecordingContext*, GrSurfaceDrawContext*, GrAAType aaType,
                     bool hasUserStencilSettings, GrAppliedClip* out, SkRect* bounds) const final;
    PreClipResult preApply(const SkRect& drawBounds, GrAA aa) const final;

    sk_sp<GrTextureProxy> testingOnly_createClipMask(GrRecordingContext*) const;
    static const char kMaskTestTag[];

private:
    static bool PathNeedsSWRenderer(GrRecordingContext* context,
                                    const SkIRect& scissorRect,
                                    bool hasUserStencilSettings,
                                    const GrSurfaceDrawContext*,
                                    const SkMatrix& viewMatrix,
                                    const SkClipStack::Element* element,
                                    bool needsStencil);

    bool applyClipMask(GrRecordingContext*, GrSurfaceDrawContext*, const GrReducedClip&,
                       bool hasUserStencilSettings, GrAppliedClip*) const;

    // Creates an alpha mask of the clip. The mask is a rasterization of elements through the
    // rect specified by clipSpaceIBounds.
    GrSurfaceProxyView createAlphaClipMask(GrRecordingContext*, const GrReducedClip&) const;

    // Similar to createAlphaClipMask but it rasterizes in SW and uploads to the result texture.
    GrSurfaceProxyView createSoftwareClipMask(GrRecordingContext*, const GrReducedClip&,
                                              GrSurfaceDrawContext*) const;

    static bool UseSWOnlyPath(GrRecordingContext*,
                              bool hasUserStencilSettings,
                              const GrSurfaceDrawContext*,
                              const GrReducedClip&);

    // SkClipStack does not track device bounds explicitly, but it will refine these device bounds
    // as clip elements are added to the stack.
    SkISize                 fDeviceSize;
    const SkClipStack*      fStack;
    const SkMatrixProvider* fMatrixProvider; // for applying clip shaders
};

#endif // GrClipStackClip_DEFINED
