
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkClipStack.h"
#include "SkPath.h"
#include <new>

struct SkClipStack::Rec {
    enum State {
        kEmpty_State,
        kRect_State,
        kPath_State
    };

    SkPath          fPath;
    SkRect          fRect;
    int             fSaveCount;
    SkRegion::Op    fOp;
    State           fState;
    bool            fDoAA;

    Rec(int saveCount, const SkRect& rect, SkRegion::Op op, bool doAA) : fRect(rect) {
        fSaveCount = saveCount;
        fOp = op;
        fState = kRect_State;
        fDoAA = doAA;
    }

    Rec(int saveCount, const SkPath& path, SkRegion::Op op, bool doAA) : fPath(path) {
        fRect.setEmpty();
        fSaveCount = saveCount;
        fOp = op;
        fState = kPath_State;
        fDoAA = doAA;
    }

    bool operator==(const Rec& b) const {
        if (fSaveCount != b.fSaveCount || fOp != b.fOp || fState != b.fState ||
                fDoAA != b.fDoAA) {
            return false;
        }
        switch (fState) {
            case kEmpty_State:
                return true;
            case kRect_State:
                return fRect == b.fRect;
            case kPath_State:
                return fPath == b.fPath;
        }
        return false;  // Silence the compiler.
    }

    bool operator!=(const Rec& b) const {
        return !(*this == b);
    }


    /**
     *  Returns true if this Rec can be intersected in place with a new clip
     */
    bool canBeIntersected(int saveCount, SkRegion::Op op) const {
        if (kEmpty_State == fState && (
                    SkRegion::kDifference_Op == op ||
                    SkRegion::kIntersect_Op == op)) {
            return true;
        }
        return  fSaveCount == saveCount &&
                SkRegion::kIntersect_Op == fOp &&
                SkRegion::kIntersect_Op == op;
    }
};

SkClipStack::SkClipStack() : fDeque(sizeof(Rec)) {
    fSaveCount = 0;
}

SkClipStack::SkClipStack(const SkClipStack& b) : fDeque(sizeof(Rec)) {
    *this = b;
}

SkClipStack::~SkClipStack() {
    reset();
}

SkClipStack& SkClipStack::operator=(const SkClipStack& b) {
    if (this == &b) {
        return *this;
    }
    reset();

    fSaveCount = b.fSaveCount;
    SkDeque::F2BIter recIter(b.fDeque);
    for (const Rec* rec = (const Rec*)recIter.next();
            rec != NULL;
            rec = (const Rec*)recIter.next()) {
        new (fDeque.push_back()) Rec(*rec);
    }

    return *this;
}

bool SkClipStack::operator==(const SkClipStack& b) const {
    if (fSaveCount != b.fSaveCount || fDeque.count() != b.fDeque.count()) {
        return false;
    }
    SkDeque::F2BIter myIter(fDeque);
    SkDeque::F2BIter bIter(b.fDeque);
    const Rec* myRec = (const Rec*)myIter.next();
    const Rec* bRec = (const Rec*)bIter.next();

    while (myRec != NULL && bRec != NULL) {
        if (*myRec != *bRec) {
            return false;
        }
        myRec = (const Rec*)myIter.next();
        bRec = (const Rec*)bIter.next();
    }
    return myRec == NULL && bRec == NULL;
}

void SkClipStack::reset() {
    // We used a placement new for each object in fDeque, so we're responsible
    // for calling the destructor on each of them as well.
    while (!fDeque.empty()) {
        Rec* rec = (Rec*)fDeque.back();
        rec->~Rec();
        fDeque.pop_back();
    }

    fSaveCount = 0;
}

void SkClipStack::save() {
    fSaveCount += 1;
}

void SkClipStack::restore() {
    fSaveCount -= 1;
    while (!fDeque.empty()) {
        Rec* rec = (Rec*)fDeque.back();
        if (rec->fSaveCount <= fSaveCount) {
            break;
        }
        rec->~Rec();
        fDeque.pop_back();
    }
}

void SkClipStack::clipDevRect(const SkRect& rect, SkRegion::Op op, bool doAA) {
    Rec* rec = (Rec*)fDeque.back();
    if (rec && rec->canBeIntersected(fSaveCount, op)) {
        switch (rec->fState) {
            case Rec::kEmpty_State:
                return;
            case Rec::kRect_State:
                if (!rec->fRect.intersect(rect)) {
                    rec->fState = Rec::kEmpty_State;
                }
                return;
            case Rec::kPath_State:
                if (!SkRect::Intersects(rec->fPath.getBounds(), rect)) {
                    rec->fState = Rec::kEmpty_State;
                    return;
                }
                break;
        }
    }
    new (fDeque.push_back()) Rec(fSaveCount, rect, op, doAA);
}

void SkClipStack::clipDevPath(const SkPath& path, SkRegion::Op op, bool doAA) {
    SkRect alt;
    if (path.isRect(&alt)) {
        return this->clipDevRect(alt, op, doAA);
    }
    Rec* rec = (Rec*)fDeque.back();
    if (rec && rec->canBeIntersected(fSaveCount, op)) {
        const SkRect& pathBounds = path.getBounds();
        switch (rec->fState) {
            case Rec::kEmpty_State:
                return;
            case Rec::kRect_State:
                if (!SkRect::Intersects(rec->fRect, pathBounds)) {
                    rec->fState = Rec::kEmpty_State;
                    return;
                }
                break;
            case Rec::kPath_State:
                if (!SkRect::Intersects(rec->fPath.getBounds(), pathBounds)) {
                    rec->fState = Rec::kEmpty_State;
                    return;
                }
                break;
        }
    }
    new (fDeque.push_back()) Rec(fSaveCount, path, op, doAA);
}

///////////////////////////////////////////////////////////////////////////////

SkClipStack::Iter::Iter() : fStack(NULL) {
}

bool operator==(const SkClipStack::Iter::Clip& a,
                const SkClipStack::Iter::Clip& b) {
    return a.fOp == b.fOp && a.fDoAA == b.fDoAA &&
           ((a.fRect == NULL && b.fRect == NULL) ||
               (a.fRect != NULL && b.fRect != NULL && *a.fRect == *b.fRect)) &&
           ((a.fPath == NULL && b.fPath == NULL) ||
               (a.fPath != NULL && b.fPath != NULL && *a.fPath == *b.fPath));
}

bool operator!=(const SkClipStack::Iter::Clip& a,
                const SkClipStack::Iter::Clip& b) {
    return !(a == b);
}

SkClipStack::Iter::Iter(const SkClipStack& stack, IterStart startLoc)
    : fStack(&stack) {
    this->reset(stack, startLoc);
}

const SkClipStack::Iter::Clip* SkClipStack::Iter::updateClip(
                        const SkClipStack::Rec* rec) {
    switch (rec->fState) {
        case SkClipStack::Rec::kEmpty_State:
            fClip.fRect = NULL;
            fClip.fPath = NULL;
            break;
        case SkClipStack::Rec::kRect_State:
            fClip.fRect = &rec->fRect;
            fClip.fPath = NULL;
            break;
        case SkClipStack::Rec::kPath_State:
            fClip.fRect = NULL;
            fClip.fPath = &rec->fPath;
            break;
    }
    fClip.fOp = rec->fOp;
    fClip.fDoAA = rec->fDoAA;
    return &fClip;
}

const SkClipStack::Iter::Clip* SkClipStack::Iter::next() {
    const SkClipStack::Rec* rec = (const SkClipStack::Rec*)fIter.next();
    if (NULL == rec) {
        return NULL;
    }

    return this->updateClip(rec);
}

const SkClipStack::Iter::Clip* SkClipStack::Iter::prev() {
    const SkClipStack::Rec* rec = (const SkClipStack::Rec*)fIter.prev();
    if (NULL == rec) {
        return NULL;
    }

    return this->updateClip(rec);
}

const SkClipStack::Iter::Clip* SkClipStack::Iter::skipToLast(SkRegion::Op op) {

    if (NULL == fStack) {
        return NULL;
    }

    fIter.reset(fStack->fDeque, SkDeque::Iter::kBack_IterStart);

    const SkClipStack::Rec* rec = NULL;

    for (rec = (const SkClipStack::Rec*) fIter.prev();
         NULL != rec;
         rec = (const SkClipStack::Rec*) fIter.prev()) {

        if (op == rec->fOp) {
            // The Deque's iterator is actually one pace ahead of the
            // returned value. So while "rec" is the element we want to 
            // return, the iterator is actually pointing at (and will
            // return on the next "next" or "prev" call) the element
            // in front of it in the deque. Bump the iterator forward a 
            // step so we get the expected result.
            if (NULL == fIter.next()) {
                // The reverse iterator has run off the front of the deque
                // (i.e., the "op" clip is the first clip) and can't
                // recover. Reset the iterator to start at the front.
                fIter.reset(fStack->fDeque, SkDeque::Iter::kFront_IterStart);
            }
            break;
        }
    }

    if (NULL == rec) {
        // There were no "op" clips
        fIter.reset(fStack->fDeque, SkDeque::Iter::kFront_IterStart);
    }

    return this->next();
}

void SkClipStack::Iter::reset(const SkClipStack& stack, IterStart startLoc) {
    fIter.reset(stack.fDeque, static_cast<SkDeque::Iter::IterStart>(startLoc));
}
