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
#include "GrStencil.h"
#include "GrTexture.h"
#include "SkClipStack.h"
#include "SkDeque.h"
#include "SkPath.h"
#include "SkRefCnt.h"
#include "SkTLList.h"
#include "SkTypes.h"

class GrDrawTarget;
class GrPathRenderer;
class GrPathRendererChain;
class GrTexture;
class SkPath;

/**
 * Produced by GrClipMaskManager. It provides a set of modifications to the drawing state that
 * are used to create the final GrPipeline for a GrBatch. This is a work in progress. It will
 * eventually encapsulate all mechanisms for modifying the scissor, shaders, and stencil state
 * to implement clipping.
 */
class GrAppliedClip : public SkNoncopyable {
public:
    GrAppliedClip() {}
    const GrFragmentProcessor* clipCoverageFragmentProcessor() const { return fClipCoverageFP; }

private:
    SkAutoTUnref<const GrFragmentProcessor> fClipCoverageFP;
    friend class GrClipMaskManager;

    typedef SkNoncopyable INHERITED;
};

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
    GrClipMaskManager(GrDrawTarget* owner);

    /**
     * Creates a clip mask if necessary as a stencil buffer or alpha texture
     * and sets the GrGpu's scissor and stencil state. If the return is false
     * then the draw can be skipped. The AutoRestoreEffects is initialized by
     * the manager when it must install additional effects to implement the
     * clip. devBounds is optional but can help optimize clipping.
     */
    bool setupClipping(const GrPipelineBuilder&,
                       GrPipelineBuilder::AutoRestoreStencil*,
                       GrScissorState*,
                       const SkRect* devBounds,
                       GrAppliedClip*);

    void adjustPathStencilParams(const GrStencilAttachment*, GrStencilSettings*);

private:
    inline GrContext* getContext();

    /**
     * Informs the helper function adjustStencilParams() about how the stencil
     * buffer clip is being used.
     */
    enum StencilClipMode {
        // Draw to the clip bit of the stencil buffer
        kModifyClip_StencilClipMode,
        // Clip against the existing representation of the clip in the high bit
        // of the stencil buffer.
        kRespectClip_StencilClipMode,
        // Neither writing to nor clipping against the clip bit.
        kIgnoreClip_StencilClipMode,
    };

    // Attempts to install a series of coverage effects to implement the clip. Return indicates
    // whether the element list was successfully converted to effects.
    const GrFragmentProcessor* getAnalyticClipProcessor(const GrReducedClip::ElementList&,
                                                        const SkVector& clipOffset,
                                                        const SkRect* devBounds);

    // Draws the clip into the stencil buffer
    bool createStencilClipMask(GrRenderTarget*,
                               int32_t elementsGenID,
                               GrReducedClip::InitialState initialState,
                               const GrReducedClip::ElementList& elements,
                               const SkIRect& clipSpaceIBounds,
                               const SkIPoint& clipSpaceToStencilOffset);

    // Creates an alpha mask of the clip. The mask is a rasterization of elements through the
    // rect specified by clipSpaceIBounds.
    GrTexture* createAlphaClipMask(int32_t elementsGenID,
                                   GrReducedClip::InitialState initialState,
                                   const GrReducedClip::ElementList& elements,
                                   const SkVector& clipToMaskOffset,
                                   const SkIRect& clipSpaceIBounds);

    // Similar to createAlphaClipMask but it rasterizes in SW and uploads to the result texture.
    GrTexture* createSoftwareClipMask(int32_t elementsGenID,
                                      GrReducedClip::InitialState initialState,
                                      const GrReducedClip::ElementList& elements,
                                      const SkVector& clipToMaskOffset,
                                      const SkIRect& clipSpaceIBounds);

   bool useSWOnlyPath(const GrPipelineBuilder&,
                       const SkVector& clipToMaskOffset,
                       const GrReducedClip::ElementList& elements);

    // Draws a clip element into the target alpha mask. The caller should have already setup the
    // desired blend operation. Optionally if the caller already selected a path renderer it can
    // be passed. Otherwise the function will select one if the element is a path.
    bool drawElement(GrPipelineBuilder*,
                     const SkMatrix& viewMatrix,
                     GrTexture* target,
                     const SkClipStack::Element*,
                     GrPathRenderer* pr = nullptr);

    // Determines whether it is possible to draw the element to both the stencil buffer and the
    // alpha mask simultaneously. If so and the element is a path a compatible path renderer is
    // also returned.
    bool canStencilAndDrawElement(GrPipelineBuilder*,
                                  GrTexture* target,
                                  GrPathRenderer**,
                                  const SkClipStack::Element*);

    void mergeMask(GrPipelineBuilder*,
                   GrTexture* dstMask,
                   GrTexture* srcMask,
                   SkRegion::Op op,
                   const SkIRect& dstBound,
                   const SkIRect& srcBound);

    GrTexture* createTempMask(int width, int height);

    /**
     * Called prior to return control back the GrGpu in setupClipping. It updates the
     * GrPipelineBuilder with stencil settings that account for stencil-based clipping.
     */
    void setPipelineBuilderStencil(const GrPipelineBuilder&,
                                   GrPipelineBuilder::AutoRestoreStencil*);

    /**
     * Adjusts the stencil settings to account for interaction with stencil
     * clipping.
     */
    void adjustStencilParams(GrStencilSettings* settings,
                             StencilClipMode mode,
                             int stencilBitCnt);

    GrTexture* createCachedMask(int width, int height, const GrUniqueKey& key, bool renderTarget);

    static const int kMaxAnalyticElements = 4;

    GrDrawTarget*   fDrawTarget;    // This is our owning draw target.
    StencilClipMode fClipMode;

    typedef SkNoncopyable INHERITED;
};
#endif // GrClipMaskManager_DEFINED
