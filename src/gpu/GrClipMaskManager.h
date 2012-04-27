
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrClipMaskManager_DEFINED
#define GrClipMaskManager_DEFINED

#include "GrRect.h"
#include "GrPath.h"

class GrGpu;
class GrClip;
class GrPathRenderer;
class GrPathRendererChain;
class SkPath;
class GrTexture;
class GrDrawState;

/**
 * Scissoring needs special handling during stencil clip mask creation
 * since the creation process re-entrantly invokes setupClipAndFlushState.
 * During this process the call stack is used to keep 
 * track of (and apply to the GPU) the current scissor settings.
 */
struct ScissoringSettings {
    bool    fEnableScissoring;
    GrIRect fScissorRect;

    void setupScissoring(GrGpu* gpu);
};


/**
 * The clip mask creator handles the generation of the clip mask. If anti 
 * aliasing is requested it will (in the future) generate a single channel 
 * (8bit) mask. If no anti aliasing is requested it will generate a 1-bit 
 * mask in the stencil buffer. In the non anti-aliasing case, if the clip
 * mask can be represented as a rectangle then scissoring is used. In all
 * cases scissoring is used to bound the range of the clip mask.
 */
class GrClipMaskManager {
public:
    GrClipMaskManager()
        : fClipMaskInStencil(false)
        , fClipMaskInAlpha(false)
        , fPathRendererChain(NULL) {
    }

    bool createClipMask(GrGpu* gpu, 
                        const GrClip& clip, 
                        ScissoringSettings* scissorSettings);

    void freeResources();

    bool isClipInStencil() const { return fClipMaskInStencil; }
    bool isClipInAlpha() const { return fClipMaskInAlpha; }

    void resetMask() {
        fClipMaskInStencil = false;
    }

protected:
private:
    bool fClipMaskInStencil;        // is the clip mask in the stencil buffer?
    bool fClipMaskInAlpha;          // is the clip mask in an alpha texture?

    // must be instantiated after GrGpu object has been given its owning
    // GrContext ptr. (GrGpu is constructed first then handed off to GrContext).
    GrPathRendererChain*        fPathRendererChain;

    bool createStencilClipMask(GrGpu* gpu, 
                               const GrClip& clip, 
                               const GrRect& bounds,
                               ScissoringSettings* scissorSettings);
    bool createAlphaClipMask(GrGpu* gpu, const GrClip& clipIn);

    bool drawPath(GrGpu* gpu,
                  const GrPath& path,
                  GrPathFill fill,
                  bool doAA);

    bool drawClipShape(GrGpu* gpu,
                       GrTexture* target,
                       const GrClip& clipIn,
                       int index);

    void drawTexture(GrGpu* gpu,
                     GrTexture* target,
                     const GrRect& rect,
                     GrTexture* texture);

    // determines the path renderer used to draw a clip path element.
    GrPathRenderer* getClipPathRenderer(GrGpu* gpu,
                                        const SkPath& path, 
                                        GrPathFill fill,
                                        bool antiAlias);

};

#endif // GrClipMaskManager_DEFINED
