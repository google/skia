
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrClipMaskManager_DEFINED
#define GrClipMaskManager_DEFINED

#include "GrContext.h"
#include "GrNoncopyable.h"
#include "GrRect.h"
#include "GrStencil.h"
#include "GrTexture.h"

#include "SkClipStack.h"
#include "SkDeque.h"
#include "SkPath.h"
#include "SkRefCnt.h"

#include "GrClipMaskCache.h"

class GrGpu;
class GrPathRenderer;
class GrPathRendererChain;
class SkPath;
class GrTexture;
class GrDrawState;

/**
 * The clip mask creator handles the generation of the clip mask. If anti
 * aliasing is requested it will (in the future) generate a single channel
 * (8bit) mask. If no anti aliasing is requested it will generate a 1-bit
 * mask in the stencil buffer. In the non anti-aliasing case, if the clip
 * mask can be represented as a rectangle then scissoring is used. In all
 * cases scissoring is used to bound the range of the clip mask.
 */
class GrClipMaskManager : public GrNoncopyable {
public:
    GR_DECLARE_RESOURCE_CACHE_DOMAIN(GetAlphaMaskDomain)

    GrClipMaskManager()
        : fGpu(NULL)
        , fCurrClipMaskType(kNone_ClipMaskType) {
    }

    /**
     * Creates a clip mask if necessary as a stencil buffer or alpha texture
     * and sets the GrGpu's scissor and stencil state. If the return is false
     * then the draw can be skipped.
     */
    bool setupClipping(const GrClipData* clipDataIn);

    void releaseResources();

    bool isClipInStencil() const {
        return kStencil_ClipMaskType == fCurrClipMaskType;
    }
    bool isClipInAlpha() const {
        return kAlpha_ClipMaskType == fCurrClipMaskType;
    }

    void invalidateStencilMask() {
        if (kStencil_ClipMaskType == fCurrClipMaskType) {
            fCurrClipMaskType = kNone_ClipMaskType;
        }
    }

    void setContext(GrContext* context) {
        fAACache.setContext(context);
    }

    GrContext* getContext() {
        return fAACache.getContext();
    }

    void setGpu(GrGpu* gpu) {
        fGpu = gpu;
    }

private:
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

    GrGpu* fGpu;

    /**
     * We may represent the clip as a mask in the stencil buffer or as an alpha
     * texture. It may be neither because the scissor rect suffices or we
     * haven't yet examined the clip.
     */
    enum ClipMaskType {
        kNone_ClipMaskType,
        kStencil_ClipMaskType,
        kAlpha_ClipMaskType,
    } fCurrClipMaskType;

    GrClipMaskCache fAACache;       // cache for the AA path

    bool createStencilClipMask(const GrClipData& clipDataIn,
                               const GrIRect& devClipBounds);
    bool createAlphaClipMask(const GrClipData& clipDataIn,
                             GrTexture** result,
                             GrIRect *devResultBounds);
    bool createSoftwareClipMask(const GrClipData& clipDataIn,
                                GrTexture** result,
                                GrIRect *devResultBounds);
    bool clipMaskPreamble(const GrClipData& clipDataIn,
                          GrTexture** result,
                          GrIRect *devResultBounds);

    bool useSWOnlyPath(const SkClipStack& clipIn);

    bool drawClipShape(GrTexture* target,
                       const SkClipStack::Iter::Clip* clip,
                       const GrIRect& resultBounds);

    void drawTexture(GrTexture* target,
                     GrTexture* texture);

    void getTemp(const GrIRect& bounds, GrAutoScratchTexture* temp);

    void setupCache(const SkClipStack& clip,
                    const GrIRect& bounds);

    /**
     * Called prior to return control back the GrGpu in setupClipping. It
     * updates the GrGpu with stencil settings that account stencil-based
     * clipping.
     */
    void setGpuStencil();

    /**
     * Adjusts the stencil settings to account for interaction with stencil
     * clipping.
     */
    void adjustStencilParams(GrStencilSettings* settings,
                             StencilClipMode mode,
                             int stencilBitCnt);

    typedef GrNoncopyable INHERITED;
};

#endif // GrClipMaskManager_DEFINED
