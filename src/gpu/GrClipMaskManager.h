/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrClipMaskManager_DEFINED
#define GrClipMaskManager_DEFINED

#include "GrReducedClip.h"
#include "SkClipStack.h"
#include "SkTypes.h"

class GrAppliedClip;
class GrClipStackClip;
class GrContext;
class GrDrawContext;
class GrFixedClip;
class GrPathRenderer;
class GrPathRendererChain;
class GrPipelineBuilder;
class GrResourceProvider;
class GrTexture;
class GrTextureProvider;
class GrUniqueKey;
struct GrUserStencilSettings;


/**
 * The clip mask creator handles the generation of the clip mask. If anti
 * aliasing is requested it will (in the future) generate a single channel
 * (8bit) mask. If no anti aliasing is requested it will generate a 1-bit
 * mask in the stencil buffer. In the non anti-aliasing case, if the clip
 * mask can be represented as a rectangle then scissoring is used. In all
 * cases scissoring is used to bound the range of the clip mask.
 */
// This has to remain a class, for now, so it can be friended (by GrDrawContext & GrContext)
class GrClipMaskManager {
public:
    /**
     * Creates a clip mask if necessary as a stencil buffer or alpha texture
     * and sets the GrGpu's scissor and stencil state. If the return is false
     * then the draw can be skipped. devBounds is optional but can help optimize
     * clipping.
     */
    static bool SetupClipping(GrContext*, const GrPipelineBuilder&, GrDrawContext*,
                              const GrClipStackClip&, const SkRect* devBounds, GrAppliedClip*);

private:
    static void DrawNonAARect(GrDrawContext* drawContext,
                              const GrFixedClip& clip,
                              const SkMatrix& viewMatrix,
                              const SkRect& rect,
                              bool isAA,
                              const GrUserStencilSettings* stencilSettings);

    static bool PathNeedsSWRenderer(GrContext* context,
                                    bool hasUserStencilSettings,
                                    const GrDrawContext*,
                                    const SkMatrix& viewMatrix,
                                    const SkClipStack::Element* element,
                                    GrPathRenderer** prOut,
                                    bool needsStencil);

    // Draws the clip into the stencil buffer
    static bool CreateStencilClipMask(GrContext*,
                                      GrDrawContext*,
                                      int32_t elementsGenID,
                                      GrReducedClip::InitialState initialState,
                                      const GrReducedClip::ElementList& elements,
                                      const SkIRect& clipSpaceIBounds,
                                      const SkIPoint& clipSpaceToStencilOffset);

    // Creates an alpha mask of the clip. The mask is a rasterization of elements through the
    // rect specified by clipSpaceIBounds.
    static sk_sp<GrTexture> CreateAlphaClipMask(GrContext*,
                                                int32_t elementsGenID,
                                                GrReducedClip::InitialState initialState,
                                                const GrReducedClip::ElementList& elements,
                                                const SkVector& clipToMaskOffset,
                                                const SkIRect& clipSpaceIBounds);

    // Similar to createAlphaClipMask but it rasterizes in SW and uploads to the result texture.
    static sk_sp<GrTexture> CreateSoftwareClipMask(GrTextureProvider*,
                                                   int32_t elementsGenID,
                                                   GrReducedClip::InitialState initialState,
                                                   const GrReducedClip::ElementList& elements,
                                                   const SkVector& clipToMaskOffset,
                                                   const SkIRect& clipSpaceIBounds);

   static bool UseSWOnlyPath(GrContext*,
                             const GrPipelineBuilder&,
                             const GrDrawContext*,
                             const SkVector& clipToMaskOffset,
                             const GrReducedClip::ElementList& elements);

    static GrTexture* CreateCachedMask(int width, int height, const GrUniqueKey& key,
                                       bool renderTarget);
};

#endif // GrClipMaskManager_DEFINED
