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

    bool canReuse(int32_t clipGenID, const SkIRect& bounds) {

        SkASSERT(clipGenID != SkClipStack::kWideOpenGenID);
        SkASSERT(clipGenID != SkClipStack::kEmptyGenID);

        if (SkClipStack::kInvalidGenID == clipGenID) {
            return false;
        }

        GrClipStackFrame* back = (GrClipStackFrame*) fStack.back();

        // We could reuse the mask if bounds is a subset of last bounds. We'd have to communicate
        // an offset to the caller.
        if (back->fLastMask.texture() &&
            back->fLastBound == bounds &&
            back->fLastClipGenID == clipGenID) {
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

    int32_t getLastClipGenID() const {

        if (fStack.empty()) {
            return SkClipStack::kInvalidGenID;
        }

        return ((GrClipStackFrame*) fStack.back())->fLastClipGenID;
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

    void acquireMask(int32_t clipGenID,
                     const GrTextureDesc& desc,
                     const GrIRect& bound) {

        if (fStack.empty()) {
            GrAssert(false);
            return;
        }

        GrClipStackFrame* back = (GrClipStackFrame*) fStack.back();

        back->acquireMask(fContext, clipGenID, desc, bound);
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

private:
    struct GrClipStackFrame {

        GrClipStackFrame() {
            this->reset();
        }

        void acquireMask(GrContext* context,
                         int32_t clipGenID,
                         const GrTextureDesc& desc,
                         const GrIRect& bound) {

            fLastClipGenID = clipGenID;

            fLastMask.set(context, desc);

            fLastBound = bound;
        }

        void reset () {
            fLastClipGenID = SkClipStack::kInvalidGenID;

            GrTextureDesc desc;

            fLastMask.set(NULL, desc);
            fLastBound.setEmpty();
        }

        int32_t                 fLastClipGenID;
        // The mask's width & height values are used by GrClipMaskManager to correctly scale the
        // texture coords for the geometry drawn with this mask.
        GrAutoScratchTexture    fLastMask;
        // fLastBound stores the bounding box of the clip mask in clip-stack space. This rect is
        // used by GrClipMaskManager to position a rect and compute texture coords for the mask.
        GrIRect                 fLastBound;
    };

    GrContext*   fContext;
    SkDeque      fStack;

    typedef GrNoncopyable INHERITED;
};

#endif // GrClipMaskCache_DEFINED
