/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrClipMaskManager_DEFINED
#define GrClipMaskManager_DEFINED

#include "GrPipelineBuilder.h"
#include "GrReducedClip.h"
#include "GrTexture.h"
#include "SkClipStack.h"
#include "SkDeque.h"
#include "SkPath.h"
#include "SkRefCnt.h"
#include "SkTLList.h"
#include "SkTypes.h"

class GrAppliedClip;
class GrClipStackClip;
class GrDrawTarget;
class GrPathRenderer;
class GrPathRendererChain;
class GrResourceProvider;
class GrTexture;
class SkPath;

/**
 * The clip mask creator handles the generation of the clip mask. If anti
 * aliasing is requested it will (in the future) generate a single channel
 * (8bit) mask. If no anti aliasing is requested it will generate a 1-bit
 * mask in the stencil buffer. In the non anti-aliasing case, if the clip
 * mask can be represented as a rectangle then scissoring is used. In all
 * cases scissoring is used to bound the range of the clip mask.
 */
class GrClipMaskManager : SkNoncopyable {
public:
    GrClipMaskManager(GrDrawTarget* owner) : fDrawTarget(owner) {}

    /**
     * Creates a clip mask if necessary as a stencil buffer or alpha texture
     * and sets the GrGpu's scissor and stencil state. If the return is false
     * then the draw can be skipped. devBounds is optional but can help optimize
     * clipping.
     */
    bool setupClipping(const GrPipelineBuilder&, const GrClipStackClip&, const SkRect* devBounds,
                       GrAppliedClip*);

private:
    inline GrContext* getContext();
    inline const GrCaps* caps() const;
    inline GrResourceProvider* resourceProvider();

    static bool PathNeedsSWRenderer(GrContext* context,
                                    bool hasUserStencilSettings,
                                    const GrRenderTarget* rt,
                                    const SkMatrix& viewMatrix,
                                    const SkClipStack::Element* element,
                                    GrPathRenderer** prOut,
                                    bool needsStencil);
    static GrPathRenderer* GetPathRenderer(GrContext* context,
                                           GrTexture* texture,
                                           const SkMatrix& viewMatrix,
                                           const SkClipStack::Element* element);

    // Attempts to install a series of coverage effects to implement the clip. Return indicates
    // whether the element list was successfully converted to processors. *fp may be nullptr even
    // when the function succeeds because all the elements were ignored. TODO: Make clip reduction
    // bounds-aware and stop checking bounds in this function. Similarly, we shouldn't need to pass
    // abortIfAA, but we don't yet know if all the AA elements will be eliminated.
    bool getAnalyticClipProcessor(const GrReducedClip::ElementList&,
                                  bool abortIfAA,
                                  SkVector& clipOffset,
                                  const SkRect* devBounds,
                                  const GrFragmentProcessor** fp);

    // Draws the clip into the stencil buffer
    bool createStencilClipMask(GrRenderTarget*,
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
    static sk_sp<GrTexture> CreateSoftwareClipMask(GrContext*,
                                                   int32_t elementsGenID,
                                                   GrReducedClip::InitialState initialState,
                                                   const GrReducedClip::ElementList& elements,
                                                   const SkVector& clipToMaskOffset,
                                                   const SkIRect& clipSpaceIBounds);

   static bool UseSWOnlyPath(GrContext*,
                             const GrPipelineBuilder&,
                             const GrRenderTarget* rt,
                             const SkVector& clipToMaskOffset,
                             const GrReducedClip::ElementList& elements);

    GrTexture* createCachedMask(int width, int height, const GrUniqueKey& key, bool renderTarget);

    static const int kMaxAnalyticElements = 4;

    GrDrawTarget*   fDrawTarget;    // This is our owning draw target.

    typedef SkNoncopyable INHERITED;
};
#endif // GrClipMaskManager_DEFINED
