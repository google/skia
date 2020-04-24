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
    GrClipStackClip(const SkClipStack* stack = nullptr) { this->reset(stack); }

    void reset(const SkClipStack* stack) { fStack = stack; }

    bool quickContains(const SkRect&) const final;
    bool quickContains(const SkRRect&) const final;
    void getConservativeBounds(int width, int height, SkIRect* devResult,
                               bool* isIntersectionOfRects) const final;
    bool apply(GrRecordingContext*, GrRenderTargetContext*, bool useHWAA,
               bool hasUserStencilSettings, GrAppliedClip* out, SkRect* bounds) const final;

    bool isRRect(const SkRect& rtBounds, SkRRect* rr, GrAA* aa) const override;

    sk_sp<GrTextureProxy> testingOnly_createClipMask(GrContext*) const;
    static const char kMaskTestTag[];

private:
    static bool PathNeedsSWRenderer(GrRecordingContext* context,
                                    const SkIRect& scissorRect,
                                    bool hasUserStencilSettings,
                                    const GrRenderTargetContext*,
                                    const SkMatrix& viewMatrix,
                                    const SkClipStack::Element* element,
                                    GrPathRenderer** prOut,
                                    bool needsStencil);

    bool applyClipMask(GrRecordingContext*, GrRenderTargetContext*, const GrReducedClip&,
                       bool hasUserStencilSettings, GrAppliedClip*) const;

    // Creates an alpha mask of the clip. The mask is a rasterization of elements through the
    // rect specified by clipSpaceIBounds.
    sk_sp<GrTextureProxy> createAlphaClipMask(GrRecordingContext*, const GrReducedClip&) const;

    // Similar to createAlphaClipMask but it rasterizes in SW and uploads to the result texture.
    sk_sp<GrTextureProxy> createSoftwareClipMask(GrRecordingContext*, const GrReducedClip&,
                                                 GrRenderTargetContext*) const;

    static bool UseSWOnlyPath(GrRecordingContext*,
                              bool hasUserStencilSettings,
                              const GrRenderTargetContext*,
                              const GrReducedClip&);

    const SkClipStack*  fStack;
};

#endif // GrClipStackClip_DEFINED
