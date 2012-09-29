/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrClipMaskCache_DEFINED
#define GrClipMaskCache_DEFINED

#include "GrContext.h"
#include "GrNoncopyable.h"
#include "SkClipStack.h"

class GrTexture;

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

    bool canReuse(const SkClipStack& clip, int width, int height) {

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

    void getLastClip(SkClipStack* clip) const {

        if (fStack.empty()) {
            GrAssert(false);
            clip->reset();
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

    void acquireMask(const SkClipStack& clip,
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
                         const SkClipStack& clip,
                         const GrTextureDesc& desc,
                         const GrIRect& bound) {

            fLastClip = clip;

            fLastMask.set(context, desc);

            fLastBound = bound;
        }

        void reset () {
            fLastClip.reset();

            GrTextureDesc desc;

            fLastMask.set(NULL, desc);
            fLastBound.setEmpty();
        }

        SkClipStack             fLastClip;
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

#endif // GrClipMaskCache_DEFINED
