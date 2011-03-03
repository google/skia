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

    Rec(int saveCount, const SkRect& rect, SkRegion::Op op) : fRect(rect) {
        fSaveCount = saveCount;
        fOp = op;
        fState = kRect_State;
    }

    Rec(int saveCount, const SkPath& path, SkRegion::Op op) : fPath(path) {
        fSaveCount = saveCount;
        fOp = op;
        fState = kPath_State;
    }

    /**
     *  Returns true if this Rec can be intersected in place with a new clip
     */
    bool canBeIntersected(int saveCount, SkRegion::Op op) const {
        if (kEmpty_State == fState) {
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

void SkClipStack::reset() {
    // don't have a reset() on SkDeque, so fake it here
    fDeque.~SkDeque();
    new (&fDeque) SkDeque(sizeof(Rec));

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

void SkClipStack::clipDevRect(const SkRect& rect, SkRegion::Op op) {
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
    new (fDeque.push_back()) Rec(fSaveCount, rect, op);
}

void SkClipStack::clipDevPath(const SkPath& path, SkRegion::Op op) {
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
    new (fDeque.push_back()) Rec(fSaveCount, path, op);
}

///////////////////////////////////////////////////////////////////////////////

SkClipStack::B2FIter::B2FIter() {
}

SkClipStack::B2FIter::B2FIter(const SkClipStack& stack) {
    this->reset(stack);
}

const SkClipStack::B2FIter::Clip* SkClipStack::B2FIter::next() {
    const SkClipStack::Rec* rec = (const SkClipStack::Rec*)fIter.next();
    if (NULL == rec) {
        return NULL;
    }

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
    return &fClip;
}

void SkClipStack::B2FIter::reset(const SkClipStack& stack) {
    fIter.reset(stack.fDeque);
}
