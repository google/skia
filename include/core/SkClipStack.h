
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

struct SkRect;
class SkPath;

class SK_API SkClipStack {
public:
    SkClipStack();
    SkClipStack(const SkClipStack& b);
    ~SkClipStack();

    SkClipStack& operator=(const SkClipStack& b);
    bool operator==(const SkClipStack& b) const;
    bool operator!=(const SkClipStack& b) const { return !(*this == b); }

    void reset();

    int getSaveCount() const { return fSaveCount; }
    void save();
    void restore();

    void clipDevRect(const SkIRect& ir, SkRegion::Op op) {
        SkRect r;
        r.set(ir);
        this->clipDevRect(r, op, false);
    }
    void clipDevRect(const SkRect&, SkRegion::Op, bool doAA);
    void clipDevPath(const SkPath&, SkRegion::Op, bool doAA);

private:
    struct Rec;

public:
    class Iter {
    public:
        enum IterStart {
            kFront_IterStart = SkDeque::Iter::kFront_IterStart,
            kBack_IterStart = SkDeque::Iter::kBack_IterStart
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
         * Restarts the iterator on a clip stack.
         */
        void reset(const SkClipStack& stack, IterStart startLoc);

    private:
        Clip             fClip;
        SkDeque::Iter    fIter;

        /**
         * updateClip updates fClip to the current state of fIter. It unifies
         * functionality needed by both next() and prev().
         */
        const Clip* updateClip(const SkClipStack::Rec* rec);
    };

    // Inherit privately from Iter to prevent access to reverse iteration
    class B2FIter : private Iter {
    public:
        B2FIter() {}

        /**
         * Wrap Iter's 2 parameter ctor to force initialization to the 
         * beginning of the deque
         */
        B2FIter(const SkClipStack& stack) 
        : INHERITED(stack, kFront_IterStart) {
        }

        using Iter::Clip;
        using Iter::next;

        /**
         * Wrap Iter::reset to force initialization to the 
         * beginning of the deque
         */
        void reset(const SkClipStack& stack) {
            this->INHERITED::reset(stack, kFront_IterStart);
        }

    private:

        typedef Iter INHERITED;
    };

private:
    friend class Iter;

    SkDeque fDeque;
    int     fSaveCount;
};

#endif

