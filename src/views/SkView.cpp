/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkView.h"
#include "SkCanvas.h"

static inline uint32_t SkSetClearShift(uint32_t bits, bool cond, unsigned shift) {
    SkASSERT((int)cond == 0 || (int)cond == 1);
    return (bits & ~(1 << shift)) | ((int)cond << shift);
}

////////////////////////////////////////////////////////////////////////

SkView::SkView(uint32_t flags) : fFlags(SkToU8(flags)) {
    fWidth = fHeight = 0;
    fLoc.set(0, 0);
    fParent = fFirstChild = fNextSibling = fPrevSibling = nullptr;
    fMatrix.setIdentity();
    fContainsFocus = 0;
}

SkView::~SkView() {
    this->detachAllChildren();
}

void SkView::setFlags(uint32_t flags) {
    SkASSERT((flags & ~kAllFlagMasks) == 0);

    uint32_t diff = fFlags ^ flags;

    if (diff & kVisible_Mask)
        this->inval(nullptr);

    fFlags = SkToU8(flags);

    if (diff & kVisible_Mask) {
        this->inval(nullptr);
    }
}

void SkView::setVisibleP(bool pred) {
    this->setFlags(SkSetClearShift(fFlags, pred, kVisible_Shift));
}

void SkView::setEnabledP(bool pred) {
    this->setFlags(SkSetClearShift(fFlags, pred, kEnabled_Shift));
}

void SkView::setFocusableP(bool pred) {
    this->setFlags(SkSetClearShift(fFlags, pred, kFocusable_Shift));
}

void SkView::setClipToBounds(bool pred) {
    this->setFlags(SkSetClearShift(fFlags, !pred, kNoClip_Shift));
}

void SkView::setSize(SkScalar width, SkScalar height) {
    width = SkMaxScalar(0, width);
    height = SkMaxScalar(0, height);

    if (fWidth != width || fHeight != height)
    {
        this->inval(nullptr);
        fWidth = width;
        fHeight = height;
        this->inval(nullptr);
        this->onSizeChange();
        this->invokeLayout();
    }
}

void SkView::setLoc(SkScalar x, SkScalar y) {
    if (fLoc.fX != x || fLoc.fY != y) {
        this->inval(nullptr);
        fLoc.set(x, y);
        this->inval(nullptr);
    }
}

void SkView::offset(SkScalar dx, SkScalar dy) {
    if (dx || dy)
        this->setLoc(fLoc.fX + dx, fLoc.fY + dy);
}

void SkView::setLocalMatrix(const SkMatrix& matrix) {
    this->inval(nullptr);
    fMatrix = matrix;
    this->inval(nullptr);
}

void SkView::draw(SkCanvas* canvas) {
    if (fWidth && fHeight && this->isVisible()) {
        SkRect    r;
        r.set(fLoc.fX, fLoc.fY, fLoc.fX + fWidth, fLoc.fY + fHeight);
        if (this->isClipToBounds() && canvas->quickReject(r)) {
            return;
        }

        SkAutoCanvasRestore    as(canvas, true);

        if (this->isClipToBounds()) {
            canvas->clipRect(r);
        }

        canvas->translate(fLoc.fX, fLoc.fY);
        canvas->concat(fMatrix);

        if (fParent) {
            fParent->beforeChild(this, canvas);
        }

        int sc = canvas->save();
        this->onDraw(canvas);
        canvas->restoreToCount(sc);

        if (fParent) {
            fParent->afterChild(this, canvas);
        }

        B2FIter    iter(this);
        SkView*    child;

        SkCanvas* childCanvas = this->beforeChildren(canvas);

        while ((child = iter.next()) != nullptr)
            child->draw(childCanvas);

        this->afterChildren(canvas);
    }
}

void SkView::inval(SkRect* rect) {
    SkView*    view = this;
    SkRect storage;

    for (;;) {
        if (!view->isVisible()) {
            return;
        }
        if (view->isClipToBounds()) {
            SkRect bounds;
            view->getLocalBounds(&bounds);
            if (rect && !bounds.intersect(*rect)) {
                return;
            }
            storage = bounds;
            rect = &storage;
        }
        if (view->handleInval(rect)) {
            return;
        }

        SkView* parent = view->fParent;
        if (parent == nullptr) {
            return;
        }

        if (rect) {
            rect->offset(view->fLoc.fX, view->fLoc.fY);
        }
        view = parent;
    }
}

////////////////////////////////////////////////////////////////////////////

bool SkView::setFocusView(SkView* fv) {
    SkView* view = this;

    do {
        if (view->onSetFocusView(fv)) {
            return true;
        }
    } while ((view = view->fParent) != nullptr);
    return false;
}

SkView* SkView::getFocusView() const {
    SkView*         focus = nullptr;
    const SkView*   view = this;
    do {
        if (view->onGetFocusView(&focus)) {
            break;
        }
    } while ((view = view->fParent) != nullptr);
    return focus;
}

bool SkView::hasFocus() const {
    return this == this->getFocusView();
}

bool SkView::acceptFocus() {
    return this->isFocusable() && this->setFocusView(this);
}

/*
    Try to give focus to this view, or its children
*/
SkView* SkView::acceptFocus(FocusDirection dir) {
    if (dir == kNext_FocusDirection) {
        if (this->acceptFocus()) {
            return this;
        }
        B2FIter    iter(this);
        SkView*    child, *focus;
        while ((child = iter.next()) != nullptr) {
            if ((focus = child->acceptFocus(dir)) != nullptr) {
                return focus;
            }
        }
    } else { // prev
        F2BIter    iter(this);
        SkView*    child, *focus;
        while ((child = iter.next()) != nullptr) {
            if ((focus = child->acceptFocus(dir)) != nullptr) {
                return focus;
            }
        }
        if (this->acceptFocus()) {
            return this;
        }
    }
    return nullptr;
}

SkView* SkView::moveFocus(FocusDirection dir) {
    SkView* focus = this->getFocusView();

    if (focus == nullptr) {    // start with the root
        focus = this;
        while (focus->fParent) {
            focus = focus->fParent;
        }
    }

    SkView* child, *parent;

    if (dir == kNext_FocusDirection) {
        parent = focus;
        child = focus->fFirstChild;
        if (child)
            goto FIRST_CHILD;
        else
            goto NEXT_SIB;

        do {
            while (child != parent->fFirstChild) {
    FIRST_CHILD:
                if ((focus = child->acceptFocus(dir)) != nullptr)
                    return focus;
                child = child->fNextSibling;
            }
    NEXT_SIB:
            child = parent->fNextSibling;
            parent = parent->fParent;
        } while (parent != nullptr);
    } else {    // prevfocus
        parent = focus->fParent;
        if (parent == nullptr) {    // we're the root
            return focus->acceptFocus(dir);
        } else {
            child = focus;
            while (parent) {
                while (child != parent->fFirstChild) {
                    child = child->fPrevSibling;
                    if ((focus = child->acceptFocus(dir)) != nullptr) {
                        return focus;
                    }
                }
                if (parent->acceptFocus()) {
                    return parent;
                }
                child = parent;
                parent = parent->fParent;
            }
        }
    }
    return nullptr;
}

void SkView::onFocusChange(bool gainFocusP) {
    this->inval(nullptr);
}

////////////////////////////////////////////////////////////////////////////

SkView::Click::Click(SkView* target) {
    SkASSERT(target);
    fTargetID = target->getSinkID();
    fType = nullptr;
    fWeOwnTheType = false;
    fOwner = nullptr;
}

SkView::Click::~Click() {
    this->resetType();
}

void SkView::Click::resetType() {
    if (fWeOwnTheType) {
        sk_free(fType);
        fWeOwnTheType = false;
    }
    fType = nullptr;
}

bool SkView::Click::isType(const char type[]) const {
    const char* t = fType;

    if (type == t) {
        return true;
    }
    if (type == nullptr) {
        type = "";
    }
    if (t == nullptr) {
        t = "";
    }
    return !strcmp(t, type);
}

void SkView::Click::setType(const char type[]) {
    this->resetType();
    fType = (char*)type;
}

void SkView::Click::copyType(const char type[]) {
    if (fType != type) {
        this->resetType();
        if (type) {
            size_t len = strlen(type) + 1;
            fType = (char*)sk_malloc_throw(len);
            memcpy(fType, type, len);
            fWeOwnTheType = true;
        }
    }
}

SkView::Click* SkView::findClickHandler(SkScalar x, SkScalar y, unsigned modi) {
    if (x < 0 || y < 0 || x >= fWidth || y >= fHeight) {
        return nullptr;
    }

    if (this->onSendClickToChildren(x, y, modi)) {
        F2BIter    iter(this);
        SkView*    child;

        while ((child = iter.next()) != nullptr) {
            SkPoint p;
#if 0
            if (!child->globalToLocal(x, y, &p)) {
                continue;
            }
#else
            // the above seems broken, so just respecting fLoc for now <reed>
            p.set(x - child->fLoc.x(), y - child->fLoc.y());
#endif

            Click* click = child->findClickHandler(p.fX, p.fY, modi);

            if (click) {
                return click;
            }
        }
    }

    return this->onFindClickHandler(x, y, modi);
}

void SkView::DoClickDown(Click* click, int x, int y, unsigned modi) {
    SkASSERT(click);

    SkView* target = (SkView*)SkEventSink::FindSink(click->fTargetID);
    if (nullptr == target) {
        return;
    }

    click->fIOrig.set(x, y);
    click->fICurr = click->fIPrev = click->fIOrig;

    click->fOrig.iset(x, y);
    if (!target->globalToLocal(&click->fOrig)) {
        // no history to let us recover from this failure
        return;
    }
    click->fPrev = click->fCurr = click->fOrig;

    click->fState = Click::kDown_State;
    click->fModifierKeys = modi;
    target->onClick(click);
}

void SkView::DoClickMoved(Click* click, int x, int y, unsigned modi) {
    SkASSERT(click);

    SkView* target = (SkView*)SkEventSink::FindSink(click->fTargetID);
    if (nullptr == target) {
        return;
    }

    click->fIPrev = click->fICurr;
    click->fICurr.set(x, y);

    click->fPrev = click->fCurr;
    click->fCurr.iset(x, y);
    if (!target->globalToLocal(&click->fCurr)) {
        // on failure pretend the mouse didn't move
        click->fCurr = click->fPrev;
    }

    click->fState = Click::kMoved_State;
    click->fModifierKeys = modi;
    target->onClick(click);
}

void SkView::DoClickUp(Click* click, int x, int y, unsigned modi) {
    SkASSERT(click);

    SkView* target = (SkView*)SkEventSink::FindSink(click->fTargetID);
    if (nullptr == target) {
        return;
    }

    click->fIPrev = click->fICurr;
    click->fICurr.set(x, y);

    click->fPrev = click->fCurr;
    click->fCurr.iset(x, y);
    if (!target->globalToLocal(&click->fCurr)) {
        // on failure pretend the mouse didn't move
        click->fCurr = click->fPrev;
    }

    click->fState = Click::kUp_State;
    click->fModifierKeys = modi;
    target->onClick(click);
}

//////////////////////////////////////////////////////////////////////

void SkView::invokeLayout() {
    SkView::Layout* layout = this->getLayout();

    if (layout) {
        layout->layoutChildren(this);
    }
}

void SkView::onDraw(SkCanvas* canvas) {
    Artist* artist = this->getArtist();

    if (artist) {
        artist->draw(this, canvas);
    }
}

void SkView::onSizeChange() {}

bool SkView::onSendClickToChildren(SkScalar x, SkScalar y, unsigned modi) {
    return true;
}

SkView::Click* SkView::onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) {
    return nullptr;
}

bool SkView::onClick(Click*) {
    return false;
}

bool SkView::handleInval(const SkRect*) {
    return false;
}

//////////////////////////////////////////////////////////////////////

void SkView::getLocalBounds(SkRect* bounds) const {
    if (bounds) {
        bounds->set(0, 0, fWidth, fHeight);
    }
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void SkView::detachFromParent_NoLayout() {
    this->validate();
    if (fParent == nullptr) {
        return;
    }

    if (fContainsFocus) {
        (void)this->setFocusView(nullptr);
    }

    this->inval(nullptr);

    SkView* next = nullptr;

    if (fNextSibling != this) {   // do we have any siblings
        fNextSibling->fPrevSibling = fPrevSibling;
        fPrevSibling->fNextSibling = fNextSibling;
        next = fNextSibling;
    }

    if (fParent->fFirstChild == this) {
        fParent->fFirstChild = next;
    }

    fParent = fNextSibling = fPrevSibling = nullptr;

    this->validate();
    this->unref();
}

void SkView::detachFromParent() {
    this->validate();
    SkView* parent = fParent;

    if (parent) {
        this->detachFromParent_NoLayout();
        parent->invokeLayout();
    }
}

SkView* SkView::attachChildToBack(SkView* child) {
    this->validate();
    SkASSERT(child != this);

    if (child == nullptr || fFirstChild == child)
        goto DONE;

    child->ref();
    child->detachFromParent_NoLayout();

    if (fFirstChild == nullptr) {
        child->fNextSibling = child;
        child->fPrevSibling = child;
    } else {
        child->fNextSibling = fFirstChild;
        child->fPrevSibling = fFirstChild->fPrevSibling;
        fFirstChild->fPrevSibling->fNextSibling = child;
        fFirstChild->fPrevSibling = child;
    }

    fFirstChild = child;
    child->fParent = this;
    child->inval(nullptr);

    this->validate();
    this->invokeLayout();
DONE:
    return child;
}

SkView* SkView::attachChildToFront(SkView* child) {
    this->validate();
    SkASSERT(child != this);

    if (child == nullptr || (fFirstChild && fFirstChild->fPrevSibling == child))
        goto DONE;

    child->ref();
    child->detachFromParent_NoLayout();

    if (fFirstChild == nullptr) {
        fFirstChild = child;
        child->fNextSibling = child;
        child->fPrevSibling = child;
    } else {
        child->fNextSibling = fFirstChild;
        child->fPrevSibling = fFirstChild->fPrevSibling;
        fFirstChild->fPrevSibling->fNextSibling = child;
        fFirstChild->fPrevSibling = child;
    }

    child->fParent = this;
    child->inval(nullptr);

    this->validate();
    this->invokeLayout();
DONE:
    return child;
}

void SkView::detachAllChildren() {
    this->validate();
    while (fFirstChild)
        fFirstChild->detachFromParent_NoLayout();
}

void SkView::localToGlobal(SkMatrix* matrix) const {
    if (matrix) {
        matrix->reset();
        const SkView* view = this;
        while (view)
        {
            matrix->preConcat(view->getLocalMatrix());
            matrix->preTranslate(-view->fLoc.fX, -view->fLoc.fY);
            view = view->fParent;
        }
    }
}

bool SkView::globalToLocal(SkScalar x, SkScalar y, SkPoint* local) const {
    SkASSERT(this);

    if (local) {
        SkMatrix m;
        this->localToGlobal(&m);
        if (!m.invert(&m)) {
            return false;
        }
        SkPoint p;
        m.mapXY(x, y, &p);
        local->set(p.fX, p.fY);
    }

    return true;
}

//////////////////////////////////////////////////////////////////

/*    Even if the subclass overrides onInflate, they should always be
    sure to call the inherited method, so that we get called.
*/
void SkView::onInflate(const SkDOM& dom, const SkDOM::Node* node) {
    SkScalar x, y;

    x = this->locX();
    y = this->locY();
    (void)dom.findScalar(node, "x", &x);
    (void)dom.findScalar(node, "y", &y);
    this->setLoc(x, y);

    x = this->width();
    y = this->height();
    (void)dom.findScalar(node, "width", &x);
    (void)dom.findScalar(node, "height", &y);
    this->setSize(x, y);

    // inflate the flags

    static const char* gFlagNames[] = {
        "visible", "enabled", "focusable", "flexH", "flexV"
    };
    SkASSERT(SK_ARRAY_COUNT(gFlagNames) == kFlagShiftCount);

    bool     b;
    uint32_t flags = this->getFlags();
    for (unsigned i = 0; i < SK_ARRAY_COUNT(gFlagNames); i++) {
        if (dom.findBool(node, gFlagNames[i], &b)) {
            flags = SkSetClearShift(flags, b, i);
        }
    }
    this->setFlags(flags);
}

void SkView::inflate(const SkDOM& dom, const SkDOM::Node* node) {
    this->onInflate(dom, node);
}

void SkView::onPostInflate(const SkTDict<SkView*>&) {
    // override in subclass as needed
}

void SkView::postInflate(const SkTDict<SkView*>& dict) {
    this->onPostInflate(dict);

    B2FIter    iter(this);
    SkView*    child;
    while ((child = iter.next()) != nullptr)
        child->postInflate(dict);
}

//////////////////////////////////////////////////////////////////

SkView* SkView::sendEventToParents(const SkEvent& evt) {
    SkView* parent = fParent;

    while (parent) {
        if (parent->doEvent(evt)) {
            return parent;
        }
        parent = parent->fParent;
    }
    return nullptr;
}

SkView* SkView::sendQueryToParents(SkEvent* evt) {
    SkView* parent = fParent;

    while (parent) {
        if (parent->doQuery(evt)) {
            return parent;
        }
        parent = parent->fParent;
    }
    return nullptr;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

SkView::F2BIter::F2BIter(const SkView* parent) {
    fFirstChild = parent ? parent->fFirstChild : nullptr;
    fChild = fFirstChild ? fFirstChild->fPrevSibling : nullptr;
}

SkView* SkView::F2BIter::next() {
    SkView* curr = fChild;

    if (fChild) {
        if (fChild == fFirstChild) {
            fChild = nullptr;
        } else {
            fChild = fChild->fPrevSibling;
        }
    }
    return curr;
}

SkView::B2FIter::B2FIter(const SkView* parent) {
    fFirstChild = parent ? parent->fFirstChild : nullptr;
    fChild = fFirstChild;
}

SkView* SkView::B2FIter::next() {
    SkView* curr = fChild;

    if (fChild) {
        SkView* next = fChild->fNextSibling;
        if (next == fFirstChild)
            next = nullptr;
        fChild = next;
    }
    return curr;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

void SkView::validate() const {
//    SkASSERT(this->getRefCnt() > 0 && this->getRefCnt() < 100);
    if (fParent) {
        SkASSERT(fNextSibling);
        SkASSERT(fPrevSibling);
    } else {
        bool nextNull = nullptr == fNextSibling;
        bool prevNull = nullptr == fNextSibling;
        SkASSERT(nextNull == prevNull);
    }
}

static inline void show_if_nonzero(const char name[], SkScalar value) {
    if (value) {
        SkDebugf("%s=\"%g\"", name, value/65536.);
    }
}

static void tab(int level) {
    for (int i = 0; i < level; i++) {
        SkDebugf("    ");
    }
}

static void dumpview(const SkView* view, int level, bool recurse) {
    tab(level);

    SkDebugf("<view");
    show_if_nonzero(" x", view->locX());
    show_if_nonzero(" y", view->locY());
    show_if_nonzero(" width", view->width());
    show_if_nonzero(" height", view->height());

    if (recurse) {
        SkView::B2FIter    iter(view);
        SkView*            child;
        bool            noChildren = true;

        while ((child = iter.next()) != nullptr) {
            if (noChildren) {
                SkDebugf(">\n");
            }
            noChildren = false;
            dumpview(child, level + 1, true);
        }

        if (!noChildren) {
            tab(level);
            SkDebugf("</view>\n");
        } else {
            goto ONELINER;
        }
    } else {
    ONELINER:
        SkDebugf(" />\n");
    }
}

void SkView::dump(bool recurse) const {
    dumpview(this, 0, recurse);
}

#endif
