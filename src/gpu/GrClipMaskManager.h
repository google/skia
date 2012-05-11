
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrClipMaskManager_DEFINED
#define GrClipMaskManager_DEFINED

#include "GrRect.h"
#include "SkPath.h"
#include "GrNoncopyable.h"
#include "GrClip.h"
#include "SkRefCnt.h"
#include "GrTexture.h"
#include "SkDeque.h"

class GrGpu;
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
 * The stencil buffer stores the last clip path - providing a single entry
 * "cache". This class provides similar functionality for AA clip paths
 */
class GrClipMaskCache : public GrNoncopyable {
public:
    GrClipMaskCache() 
    : fStack(sizeof(GrClipStackFrame)) {
        // We need an initial frame to capture the clip state prior to 
        // any pushes
        new (fStack.push_back()) GrClipStackFrame();
    }

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

        if (back->fLastMask &&
            back->fLastMask->width() >= width &&
            back->fLastMask->height() >= height &&
            clip == back->fLastClip) {
            return true;
        }

        return false;
    }

    void set(const GrClip& clip, GrTexture* mask, const GrRect& bound) {

        if (fStack.empty()) {
            GrAssert(false);
            return;
        }

        GrClipStackFrame* back = (GrClipStackFrame*) fStack.back();

        back->fLastClip = clip;
        SkSafeRef(mask);
        back->fLastMask.reset(mask);
        back->fLastBound = bound;
    }

    void reset() {
        if (fStack.empty()) {
            GrAssert(false);
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
    void push() {
        new (fStack.push_back()) GrClipStackFrame();
    }

    void pop() {
        GrAssert(!fStack.empty());

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

        return back->fLastMask.get();
    }

    const GrTexture* getLastMask() const {

        if (fStack.empty()) {
            GrAssert(false);
            return NULL;
        }

        GrClipStackFrame* back = (GrClipStackFrame*) fStack.back();

        return back->fLastMask.get();
    }

    GrTexture* detachLastMask() {

        if (fStack.empty()) {
            GrAssert(false);
            return NULL;
        }

        GrClipStackFrame* back = (GrClipStackFrame*) fStack.back();

        return back->fLastMask.detach();
    }

    int getLastMaskWidth() const {

        if (fStack.empty()) {
            GrAssert(false);
            return -1;
        }

        GrClipStackFrame* back = (GrClipStackFrame*) fStack.back();

        if (NULL == back->fLastMask.get()) {
            return -1;
        }

        return back->fLastMask.get()->width();
    }

    int getLastMaskHeight() const {

        if (fStack.empty()) {
            GrAssert(false);
            return -1;
        }

        GrClipStackFrame* back = (GrClipStackFrame*) fStack.back();

        if (NULL == back->fLastMask.get()) {
            return -1;
        }

        return back->fLastMask.get()->height();
    }

    void getLastBound(GrRect* bound) const {

        if (fStack.empty()) {
            GrAssert(false);
            bound->setEmpty();
            return;
        }

        GrClipStackFrame* back = (GrClipStackFrame*) fStack.back();

        *bound = back->fLastBound;
    }

protected:
private:
    struct GrClipStackFrame {

        GrClipStackFrame() {
            reset();
        }

        void reset () {
            fLastClip.setEmpty();
            fLastMask.reset(NULL);
            fLastBound.setEmpty();
        }

        GrClip                  fLastClip;
        // The mask's width & height values are used in setupDrawStateAAClip to 
        // correctly scale the uvs for geometry drawn with this mask
        SkAutoTUnref<GrTexture> fLastMask;
        // fLastBound stores the bounding box of the clip mask in canvas 
        // space. The left and top fields are used to offset the uvs for 
        // geometry drawn with this mask (in setupDrawStateAAClip)
        GrRect                  fLastBound;
    };

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
    GrClipMaskCache fAACache;       // cache for the AA path

    // must be instantiated after GrGpu object has been given its owning
    // GrContext ptr. (GrGpu is constructed first then handed off to GrContext).
    GrPathRendererChain*        fPathRendererChain;

    bool createStencilClipMask(GrGpu* gpu, 
                               const GrClip& clip, 
                               const GrRect& bounds,
                               ScissoringSettings* scissorSettings);
    bool createAlphaClipMask(GrGpu* gpu,
                             const GrClip& clipIn,
                             GrTexture** result,
                             GrRect *resultBounds);
    bool createSoftwareClipMask(GrGpu* gpu,
                                const GrClip& clipIn,
                                GrTexture** result,
                                GrRect *resultBounds);
    bool clipMaskPreamble(GrGpu* gpu,
                          const GrClip& clipIn,
                          GrTexture** result,
                          GrRect *resultBounds,
                          GrTexture** maskStorage);

    bool drawPath(GrGpu* gpu,
                  const SkPath& path,
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

    void getAccum(GrGpu* gpu, const GrRect& bounds, GrTexture** accum);

    // determines the path renderer used to draw a clip path element.
    GrPathRenderer* getClipPathRenderer(GrGpu* gpu,
                                        const SkPath& path, 
                                        GrPathFill fill,
                                        bool antiAlias);

    typedef GrNoncopyable INHERITED;
};

#endif // GrClipMaskManager_DEFINED
