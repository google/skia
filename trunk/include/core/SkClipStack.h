
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkClipStack_DEFINED
#define SkClipStack_DEFINED

#include "SkDeque.h"
#include "SkRegion.h"
#include "SkTDArray.h"

struct SkRect;
class SkPath;

// Because a single save/restore state can have multiple clips, this class
// stores the stack depth (fSaveCount) and clips (fDeque) separately.
// Each clip in fDeque stores the stack state to which it belongs
// (i.e., the fSaveCount in force when it was added). Restores are thus
// implemented by removing clips from fDeque that have an fSaveCount larger
// then the freshly decremented count.
class SK_API SkClipStack {
public:
    SkClipStack();
    SkClipStack(const SkClipStack& b);
    explicit SkClipStack(const SkRect& r);
    explicit SkClipStack(const SkIRect& r);
    ~SkClipStack();

    SkClipStack& operator=(const SkClipStack& b);
    bool operator==(const SkClipStack& b) const;
    bool operator!=(const SkClipStack& b) const { return !(*this == b); }

    void reset();

    int getSaveCount() const { return fSaveCount; }
    void save();
    void restore();

    enum BoundsType {
        // The bounding box contains all the pixels that can be written to
        kNormal_BoundsType,
        // The bounding box contains all the pixels that cannot be written to.
        // The real bound extends out to infinity and all the pixels outside
        // of the bound can be written to. Note that some of the pixels inside
        // the bound may also be writeable but all pixels that cannot be
        // written to are guaranteed to be inside.
        kInsideOut_BoundsType
    };

    /**
     * getBounds places the current finite bound in its first parameter. In its
     * second, it indicates which kind of bound is being returned. If
     * 'canvFiniteBound' is a normal bounding box then it encloses all writeable
     * pixels. If 'canvFiniteBound' is an inside out bounding box then it
     * encloses all the un-writeable pixels and the true/normal bound is the
     * infinite plane. isIntersectionOfRects is an optional parameter
     * that is true if 'canvFiniteBound' resulted from an intersection of rects.
     */
    void getBounds(SkRect* canvFiniteBound,
                   BoundsType* boundType,
                   bool* isIntersectionOfRects = NULL) const;

    void clipDevRect(const SkIRect& ir, SkRegion::Op op) {
        SkRect r;
        r.set(ir);
        this->clipDevRect(r, op, false);
    }
    void clipDevRect(const SkRect&, SkRegion::Op, bool doAA);
    void clipDevPath(const SkPath&, SkRegion::Op, bool doAA);
    // An optimized version of clipDevRect(emptyRect, kIntersect, ...)
    void clipEmpty();

    /**
     * isWideOpen returns true if the clip state corresponds to the infinite
     * plane (i.e., draws are not limited at all)
     */
    bool isWideOpen() const;

    /**
     * Add a callback function that will be called whenever a clip state
     * is no longer viable. This will occur whenever restore
     * is called or when a clipDevRect or clipDevPath call updates the
     * clip within an existing save/restore state. Each clip state is
     * represented by a unique generation ID.
     */
    typedef void (*PFPurgeClipCB)(int genID, void* data);
    void addPurgeClipCallback(PFPurgeClipCB callback, void* data) const;

    /**
     * Remove a callback added earlier via addPurgeClipCallback
     */
    void removePurgeClipCallback(PFPurgeClipCB callback, void* data) const;

    /**
     * The generation ID has three reserved values to indicate special
     * (potentially ignoreable) cases
     */
    static const int32_t kInvalidGenID = 0;
    static const int32_t kEmptyGenID = 1;       // no pixels writeable
    static const int32_t kWideOpenGenID = 2;    // all pixels writeable

    int32_t getTopmostGenID() const;

private:
    struct Rec;

public:
    class Iter {
    public:
        enum IterStart {
            kBottom_IterStart = SkDeque::Iter::kFront_IterStart,
            kTop_IterStart = SkDeque::Iter::kBack_IterStart
        };

        /**
         * Creates an uninitialized iterator. Must be reset()
         */
        Iter();

        Iter(const SkClipStack& stack, IterStart startLoc);

        struct Clip {
            Clip() : fRect(NULL), fPath(NULL), fOp(SkRegion::kIntersect_Op),
                     fDoAA(false) {}
            friend bool operator==(const Clip& a, const Clip& b);
            friend bool operator!=(const Clip& a, const Clip& b);
            const SkRect*   fRect;  // if non-null, this is a rect clip
            const SkPath*   fPath;  // if non-null, this is a path clip
            SkRegion::Op    fOp;
            bool            fDoAA;
            int32_t         fGenID;
        };

        /**
         *  Return the clip for this element in the iterator. If next() returns
         *  NULL, then the iterator is done. The type of clip is determined by
         *  the pointers fRect and fPath:
         *
         *  fRect==NULL  fPath!=NULL    path clip
         *  fRect!=NULL  fPath==NULL    rect clip
         *  fRect==NULL  fPath==NULL    empty clip
         */
        const Clip* next();
        const Clip* prev();

        /**
         * Moves the iterator to the topmost clip with the specified RegionOp
         * and returns that clip. If no clip with that op is found,
         * returns NULL.
         */
        const Clip* skipToTopmost(SkRegion::Op op);

        /**
         * Restarts the iterator on a clip stack.
         */
        void reset(const SkClipStack& stack, IterStart startLoc);

    private:
        const SkClipStack* fStack;
        Clip               fClip;
        SkDeque::Iter      fIter;

        /**
         * updateClip updates fClip to the current state of fIter. It unifies
         * functionality needed by both next() and prev().
         */
        const Clip* updateClip(const SkClipStack::Rec* rec);
    };

    /**
     * The B2TIter iterates from the bottom of the stack to the top.
     * It inherits privately from Iter to prevent access to reverse iteration.
     */
    class B2TIter : private Iter {
    public:
        B2TIter() {}

        /**
         * Wrap Iter's 2 parameter ctor to force initialization to the
         * beginning of the deque/bottom of the stack
         */
        B2TIter(const SkClipStack& stack)
        : INHERITED(stack, kBottom_IterStart) {
        }

        using Iter::Clip;
        using Iter::next;

        /**
         * Wrap Iter::reset to force initialization to the
         * beginning of the deque/bottom of the stack
         */
        void reset(const SkClipStack& stack) {
            this->INHERITED::reset(stack, kBottom_IterStart);
        }

    private:

        typedef Iter INHERITED;
    };

    /**
     * GetConservativeBounds returns a conservative bound of the current clip.
     * Since this could be the infinite plane (if inverse fills were involved) the
     * maxWidth and maxHeight parameters can be used to limit the returned bound
     * to the expected drawing area. Similarly, the offsetX and offsetY parameters
     * allow the caller to offset the returned bound to account for translated
     * drawing areas (i.e., those resulting from a saveLayer). For finite bounds,
     * the translation (+offsetX, +offsetY) is applied before the clamp to the
     * maximum rectangle: [0,maxWidth) x [0,maxHeight).
     * isIntersectionOfRects is an optional parameter that is true when
     * 'devBounds' is the result of an intersection of rects. In this case
     * 'devBounds' is the exact answer/clip.
     */
    void getConservativeBounds(int offsetX,
                               int offsetY,
                               int maxWidth,
                               int maxHeight,
                               SkRect* devBounds,
                               bool* isIntersectionOfRects = NULL) const;

private:
    friend class Iter;

    SkDeque fDeque;
    int     fSaveCount;

    // Generation ID for the clip stack. This is incremented for each
    // clipDevRect and clipDevPath call. 0 is reserved to indicate an
    // invalid ID.
    static int32_t     gGenID;

    struct ClipCallbackData {
        PFPurgeClipCB   fCallback;
        void*           fData;

        friend bool operator==(const ClipCallbackData& a,
                               const ClipCallbackData& b) {
            return a.fCallback == b.fCallback && a.fData == b.fData;
        }
    };

    mutable SkTDArray<ClipCallbackData> fCallbackData;

    /**
     * Invoke all the purge callbacks passing in rec's generation ID.
     */
    void purgeClip(Rec* rec);

    /**
     * Return the next unique generation ID.
     */
    static int32_t GetNextGenID();
};

#endif

