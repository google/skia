
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrClipMaskManager_DEFINED
#define GrClipMaskManager_DEFINED

#include "GrClip.h"
#include "GrContext.h"
#include "GrNoncopyable.h"
#include "GrRect.h"
#include "GrStencil.h"
#include "GrTexture.h"

#include "SkDeque.h"
#include "SkPath.h"
#include "SkRefCnt.h"

class GrGpu;
class GrPathRenderer;
class GrPathRendererChain;
class SkPath;
class GrTexture;
class GrDrawState;

/**
 * The stencil buffer stores the last clip path - providing a single entry
 * "cache". This class provides similar functionality for AA clip paths
 */
class GrClipMaskCache : public GrNoncopyable {
public:
    GrClipMaskCache();

    ~GrClipMaskCache() {

        while (!fStack.empty()) {
            GrClipStackFrame* temp = (GrClipStackFrame*) fStack.back();
            temp->~GrClipStackFrame();
            fStack.pop_back();
        }
    }

    bool canReuse(const GrClip& clip, int width, int height) {

        if (fStack.empty()) {
            GrAssert(false);
            return false;
        }

        GrClipStackFrame* back = (GrClipStackFrame*) fStack.back();

        if (back->fLastMask.texture() &&
            back->fLastMask.texture()->width() >= width &&
            back->fLastMask.texture()->height() >= height &&
            clip == back->fLastClip) {
            return true;
        }

        return false;
    }

    void reset() {
        if (fStack.empty()) {
//            GrAssert(false);
            return;
        }

        GrClipStackFrame* back = (GrClipStackFrame*) fStack.back();

        back->reset();
    }

    /**
     * After a "push" the clip state is entirely open. Currently, the
     * entire clip stack will be re-rendered into a new clip mask.
     * TODO: can we take advantage of the nested nature of the clips to
     * reduce the mask creation cost?
     */
    void push();

    void pop() {
        //GrAssert(!fStack.empty());

        if (!fStack.empty()) {
            GrClipStackFrame* back = (GrClipStackFrame*) fStack.back();

            back->~GrClipStackFrame();
            fStack.pop_back();
        }
    }

    void getLastClip(GrClip* clip) const {

        if (fStack.empty()) {
            GrAssert(false);
            clip->setEmpty();
            return;
        }

        GrClipStackFrame* back = (GrClipStackFrame*) fStack.back();

        *clip = back->fLastClip;
    }

    GrTexture* getLastMask() {

        if (fStack.empty()) {
            GrAssert(false);
            return NULL;
        }

        GrClipStackFrame* back = (GrClipStackFrame*) fStack.back();

        return back->fLastMask.texture();
    }

    const GrTexture* getLastMask() const {

        if (fStack.empty()) {
            GrAssert(false);
            return NULL;
        }

        GrClipStackFrame* back = (GrClipStackFrame*) fStack.back();

        return back->fLastMask.texture();
    }

    void acquireMask(const GrClip& clip,
                     const GrTextureDesc& desc,
                     const GrIRect& bound) {

        if (fStack.empty()) {
            GrAssert(false);
            return;
        }

        GrClipStackFrame* back = (GrClipStackFrame*) fStack.back();

        back->acquireMask(fContext, clip, desc, bound);
    }

    int getLastMaskWidth() const {

        if (fStack.empty()) {
            GrAssert(false);
            return -1;
        }

        GrClipStackFrame* back = (GrClipStackFrame*) fStack.back();

        if (NULL == back->fLastMask.texture()) {
            return -1;
        }

        return back->fLastMask.texture()->width();
    }

    int getLastMaskHeight() const {

        if (fStack.empty()) {
            GrAssert(false);
            return -1;
        }

        GrClipStackFrame* back = (GrClipStackFrame*) fStack.back();

        if (NULL == back->fLastMask.texture()) {
            return -1;
        }

        return back->fLastMask.texture()->height();
    }

    void getLastBound(GrIRect* bound) const {

        if (fStack.empty()) {
            GrAssert(false);
            bound->setEmpty();
            return;
        }

        GrClipStackFrame* back = (GrClipStackFrame*) fStack.back();

        *bound = back->fLastBound;
    }

    void setContext(GrContext* context) {
        fContext = context;
    }

    GrContext* getContext() {
        return fContext;
    }

    void releaseResources() {

        SkDeque::F2BIter iter(fStack);
        for (GrClipStackFrame* frame = (GrClipStackFrame*) iter.next();
                frame != NULL;
                frame = (GrClipStackFrame*) iter.next()) {
            frame->reset();
        }
    }

protected:
private:
    struct GrClipStackFrame {

        GrClipStackFrame() {
            reset();
        }

        void acquireMask(GrContext* context,
                         const GrClip& clip, 
                         const GrTextureDesc& desc,
                         const GrIRect& bound) {

            fLastClip = clip;

            fLastMask.set(context, desc);

            fLastBound = bound;
        }

        void reset () {
            fLastClip.setEmpty();

            GrTextureDesc desc;

            fLastMask.set(NULL, desc);
            fLastBound.setEmpty();
        }

        GrClip                  fLastClip;
        // The mask's width & height values are used in setupDrawStateAAClip to 
        // correctly scale the uvs for geometry drawn with this mask
        GrAutoScratchTexture    fLastMask;
        // fLastBound stores the bounding box of the clip mask in canvas 
        // space. The left and top fields are used to offset the uvs for 
        // geometry drawn with this mask (in setupDrawStateAAClip)
        GrIRect                 fLastBound;
    };

    GrContext*   fContext;
    SkDeque      fStack;

    typedef GrNoncopyable INHERITED;
};

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
    GrClipMaskManager(GrGpu* gpu)
        : fGpu(gpu)
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
                               const GrIRect& bounds);
    bool createAlphaClipMask(const GrClipData& clipDataIn,
                             GrTexture** result,
                             GrIRect *resultBounds);
    bool createSoftwareClipMask(const GrClipData& clipDataIn,
                                GrTexture** result,
                                GrIRect *resultBounds);
    bool clipMaskPreamble(const GrClipData& clipDataIn,
                          GrTexture** result,
                          GrIRect *resultBounds);

    bool useSWOnlyPath(const GrClip& clipIn);

    bool drawClipShape(GrTexture* target,
                       const GrClip::Iter::Clip* clip,
                       const GrIRect& resultBounds);

    void drawTexture(GrTexture* target,
                     GrTexture* texture);

    void getTemp(const GrIRect& bounds, GrAutoScratchTexture* temp);

    void setupCache(const GrClip& clip, 
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
