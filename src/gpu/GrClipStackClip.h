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
class GrTexture;
class GrTextureProvider;
class GrUniqueKey;

/**
 * GrClipStackClip can apply a generic SkClipStack to the draw state. It may need to generate an
 * 8-bit alpha clip mask and/or modify the stencil buffer during apply().
 */
class GrClipStackClip final : public GrClip {
public:
    GrClipStackClip(const SkClipStack* stack = nullptr, const SkIPoint* origin = nullptr) {
        this->reset(stack, origin);
    }

    void reset(const SkClipStack* stack = nullptr, const SkIPoint* origin = nullptr) {
        fOrigin = origin ? *origin : SkIPoint::Make(0, 0);
        fStack.reset(SkSafeRef(stack));
    }

    bool quickContains(const SkRect&) const final;
    bool quickContains(const SkRRect&) const final;
    void getConservativeBounds(int width, int height, SkIRect* devResult,
                               bool* isIntersectionOfRects) const final;
    bool apply(GrContext*, GrDrawContext*, bool useHWAA, bool hasUserStencilSettings,
               GrAppliedClip* out) const final;

    bool isRRect(const SkRect& rtBounds, SkRRect* rr, bool* aa) const override;

private:
    static bool PathNeedsSWRenderer(GrContext* context,
                                    bool hasUserStencilSettings,
                                    const GrDrawContext*,
                                    const SkMatrix& viewMatrix,
                                    const SkClipStack::Element* element,
                                    GrPathRenderer** prOut,
                                    bool needsStencil);

    // Creates an alpha mask of the clip. The mask is a rasterization of elements through the
    // rect specified by clipSpaceIBounds.
    static sk_sp<GrTexture> CreateAlphaClipMask(GrContext*, const GrReducedClip&);

    // Similar to createAlphaClipMask but it rasterizes in SW and uploads to the result texture.
    static sk_sp<GrTexture> CreateSoftwareClipMask(GrTextureProvider*, const GrReducedClip&);

   static bool UseSWOnlyPath(GrContext*,
                             bool hasUserStencilSettings,
                             const GrDrawContext*,
                             const GrReducedClip&);

    static GrTexture* CreateCachedMask(int width, int height, const GrUniqueKey& key,
                                       bool renderTarget);

    SkIPoint                          fOrigin;
    SkAutoTUnref<const SkClipStack>   fStack;
};

#endif // GrClipStackClip_DEFINED
