/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrClipMaskCache_DEFINED
#define GrClipMaskCache_DEFINED

#include "GrResourceProvider.h"
#include "SkClipStack.h"
#include "SkTypes.h"

class GrTexture;

/**
 * The stencil buffer stores the last clip path - providing a single entry
 * "cache". This class provides similar functionality for AA clip paths
 */
class GrClipMaskCache : SkNoncopyable {
public:
    GrClipMaskCache(GrResourceProvider*);

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

        GrClipStackFrame* back = (GrClipStackFrame*) fStack.back();

        // We could reuse the mask if bounds is a subset of last bounds. We'd have to communicate
        // an offset to the caller.
        if (back->fLastMask &&
            !back->fLastMask->wasDestroyed() &&
            back->fLastBound == bounds &&
            back->fLastClipGenID == clipGenID) {
            return true;
        }

        return false;
    }

    void reset() {
        if (fStack.empty()) {
//            SkASSERT(false);
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
        //SkASSERT(!fStack.empty());

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
            SkASSERT(false);
            return NULL;
        }

        GrClipStackFrame* back = (GrClipStackFrame*) fStack.back();

        return back->fLastMask;
    }

    const GrTexture* getLastMask() const {

        if (fStack.empty()) {
            SkASSERT(false);
            return NULL;
        }

        GrClipStackFrame* back = (GrClipStackFrame*) fStack.back();

        return back->fLastMask;
    }

    void acquireMask(int32_t clipGenID,
                     const GrSurfaceDesc& desc,
                     const SkIRect& bound) {

        if (fStack.empty()) {
            SkASSERT(false);
            return;
        }

        GrClipStackFrame* back = (GrClipStackFrame*) fStack.back();

        back->acquireMask(fResourceProvider, clipGenID, desc, bound);
    }

    int getLastMaskWidth() const {

        if (fStack.empty()) {
            SkASSERT(false);
            return -1;
        }

        GrClipStackFrame* back = (GrClipStackFrame*) fStack.back();

        if (NULL == back->fLastMask) {
            return -1;
        }

        return back->fLastMask->width();
    }

    int getLastMaskHeight() const {

        if (fStack.empty()) {
            SkASSERT(false);
            return -1;
        }

        GrClipStackFrame* back = (GrClipStackFrame*) fStack.back();

        if (NULL == back->fLastMask) {
            return -1;
        }

        return back->fLastMask->height();
    }

    void getLastBound(SkIRect* bound) const {

        if (fStack.empty()) {
            SkASSERT(false);
            bound->setEmpty();
            return;
        }

        GrClipStackFrame* back = (GrClipStackFrame*) fStack.back();

        *bound = back->fLastBound;
    }

    //  TODO: Remove this when we hold cache keys instead of refs to textures.
    void purgeResources() {
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

        void acquireMask(GrResourceProvider* resourceProvider,
                         int32_t clipGenID,
                         const GrSurfaceDesc& desc,
                         const SkIRect& bound) {

            fLastClipGenID = clipGenID;

            // TODO: Determine if we really need the NoPendingIO flag anymore.
            // (http://skbug.com/4156)
            static const uint32_t kFlags = GrResourceProvider::kNoPendingIO_Flag;
            fLastMask.reset(resourceProvider->createApproxTexture(desc, kFlags));

            fLastBound = bound;
        }

        void reset () {
            fLastClipGenID = SkClipStack::kInvalidGenID;

            GrSurfaceDesc desc;

            fLastMask.reset(NULL);
            fLastBound.setEmpty();
        }

        int32_t                 fLastClipGenID;
        // The mask's width & height values are used by GrClipMaskManager to correctly scale the
        // texture coords for the geometry drawn with this mask. TODO: This should be a cache key
        // and not a hard ref to a texture.
        SkAutoTUnref<GrTexture> fLastMask;
        // fLastBound stores the bounding box of the clip mask in clip-stack space. This rect is
        // used by GrClipMaskManager to position a rect and compute texture coords for the mask.
        SkIRect                 fLastBound;
    };

    SkDeque             fStack;
    GrResourceProvider* fResourceProvider;

    typedef SkNoncopyable INHERITED;
};

#endif // GrClipMaskCache_DEFINED
