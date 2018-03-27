/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrClipStackClip_DEFINED
#define GrClipStackClip_DEFINED

#include "GrClip.h"
#include "GrReducedClip.h"
#include "SkClipStack.h"

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
    bool apply(GrContext*, GrRenderTargetContext*, bool useHWAA, bool hasUserStencilSettings,
               GrAppliedClip* out, SkRect* bounds) const final;

    bool isRRect(const SkRect& rtBounds, SkRRect* rr, GrAA* aa) const override;

    sk_sp<GrTextureProxy> testingOnly_createClipMask(GrContext*) const;
    static const char kMaskTestTag[];

private:
    bool applyClipMask(GrContext*, GrRenderTargetContext*, const GrReducedClip&,
                       bool hasUserStencilSettings, GrAppliedClip*) const;

    // Creates an alpha mask of the remaining reduced clip elements that could not be handled
    // analytically on the GPU. The mask fills the reduced clip's scissor rect.
    sk_sp<GrTextureProxy> createSoftwareClipMask(GrContext*, const GrReducedClip&,
                                                 GrRenderTargetContext*) const;

    const SkClipStack*  fStack;
};

#endif // GrClipStackClip_DEFINED
